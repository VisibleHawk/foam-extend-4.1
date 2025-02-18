/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "STARCDMeshReader.H"
#include "cyclicPolyPatch.H"
#include "emptyPolyPatch.H"
#include "wallPolyPatch.H"
#include "symmetryPolyPatch.H"
#include "cellModeller.H"
#include "ListOps.H"
#include "IFstream.H"
#include "IOMap.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const char* Foam::meshReaders::STARCD::defaultBoundaryName =
    "Default_Boundary_Region";

const char* Foam::meshReaders::STARCD::defaultSolidBoundaryName =
    "Default_Boundary_Solid";

bool Foam::meshReaders::STARCD::keepSolids = false;

const int Foam::meshReaders::STARCD::starToFoamFaceAddr[4][6] =
{
    { 4, 5, 2, 3, 0, 1 },     // 11 = pro-STAR hex
    { 0, 1, 4, -1, 2, 3 },    // 12 = pro-STAR prism
    { 3, -1, 2, -1, 1, 0 },   // 13 = pro-STAR tetra
    { 0, -1, 4, 2, 1, 3 }     // 14 = pro-STAR pyramid
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::meshReaders::STARCD::readToNewline(IFstream& is)
{
    char ch = '\n';
    do
    {
        (is).get(ch);
    }
    while ((is) && ch != '\n');
}


bool Foam::meshReaders::STARCD::readHeader(IFstream& is, word fileSignature)
{
    if (!is.good())
    {
        FatalErrorIn("meshReaders::STARCD::readHeader()")
            << "cannot read " << fileSignature  << "  " << is.name()
            << abort(FatalError);
    }

    word header;
    label majorVersion;

    is >> header;
    is >> majorVersion;

    // skip the rest of the line
    readToNewline(is);

    // add other checks ...
    if (header != fileSignature)
    {
        Info<< "header mismatch " << fileSignature << "  " << is.name()
            << endl;
    }

    return true;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::meshReaders::STARCD::readAux(const objectRegistry& registry)
{
    boundaryRegion_.readDict(registry);
    cellTable_.readDict(registry);
}


// read in the points from the .vrt file
//
/*---------------------------------------------------------------------------*\
Line 1:
  PROSTAR_VERTEX [newline]

Line 2:
  <version> 0 0 0 0 0 0 0 [newline]

Body:
  <vertexId>  <x>  <y>  <z> [newline]

\*---------------------------------------------------------------------------*/
void Foam::meshReaders::STARCD::readPoints
(
    const fileName& inputName,
    const scalar scaleFactor
)
{
    const word fileSignature = "PROSTAR_VERTEX";
    label nPoints = 0, maxId = 0;

    // Pass 1:
    // get # points and maximum vertex label
    {
        IFstream is(inputName);
        readHeader(is, fileSignature);

        label lineLabel;
        scalar x, y, z;

        while ((is >> lineLabel).good())
        {
            nPoints++;
            maxId = max(maxId, lineLabel);
            is >> x >> y >> z;
        }
    }

    Info<< "Number of points  = " << nPoints << endl;

    // set sizes and reset to invalid values

    points_.setSize(nPoints);
    mapToFoamPointId_.setSize(maxId+1);

    //- original Point number for a given vertex
    // might need again in the future
    ////     labelList origPointId(nPoints);
    ////     origPointId = -1;

    mapToFoamPointId_ = -1;

    // Pass 2:
    // construct pointList and conversion table
    // from Star vertex numbers to Foam point labels
    if (nPoints > 0)
    {
        IFstream is(inputName);
        readHeader(is, fileSignature);

        label lineLabel;

        label pointI = 0;
        while ((is >> lineLabel).good())
        {
            is  >> points_[pointI].x()
                >> points_[pointI].y()
                >> points_[pointI].z();

            // might need again in the future
            ////  origPointId[pointI] = lineLabel;
            mapToFoamPointId_[lineLabel] = pointI;
            pointI++;
        }

        if (nPoints > pointI)
        {
            nPoints = pointI;
            points_.setSize(nPoints);
            // might need again in the future
            //// origPointId.setSize(nPoints);
        }

        if (scaleFactor > 1.0 + SMALL || scaleFactor < 1.0 - SMALL)
        {
            points_ *= scaleFactor;
        }
    }
    else
    {
        FatalErrorIn("meshReaders::STARCD::readPoints()")
            << "no points in file " << inputName
            << abort(FatalError);
    }

}


// read in the cells from the .cel file
//
/*---------------------------------------------------------------------------*\
Line 1:
  PROSTAR_CELL [newline]

Line 2:
  <version> 0 0 0 0 0 0 0 [newline]

Body:
  <cellId>  <shapeId>  <nLabels>  <cellTableId>  <typeId> [newline]
  <cellId>  <int1> .. <int8>
  <cellId>  <int9> .. <int16>

 with shapeId:
 *   1 = point
 *   2 = line
 *   3 = shell
 *  11 = hexa
 *  12 = prism
 *  13 = tetra
 *  14 = pyramid
 * 255 = polyhedron

 with typeId
 *   1 = fluid
 *   2 = solid
 *   3 = baffle
 *   4 = shell
 *   5 = line
 *   6 = point

For primitive cell shapes, the number of vertices will never exceed 8 (hexa)
and corresponds to <nLabels>.
For polyhedral, <nLabels> includess an index table comprising beg/end pairs
for each cell face.

Strictly speaking, we only need the cellModeller for adding boundaries.
\*---------------------------------------------------------------------------*/

void Foam::meshReaders::STARCD::readCells(const fileName& inputName)
{
    const word fileSignature = "PROSTAR_CELL";
    label nFluids = 0, nSolids = 0, nBaffles = 0, nShells = 0;
    label maxId = 0;

    bool unknownVertices = false;


    // Pass 1:
    // count nFluids, nSolids, nBaffle, nShell and maxId
    // also see if polyhedral cells were used
    {
        IFstream is(inputName);
        readHeader(is, fileSignature);

        label lineLabel, shapeId, nLabels, cellTableId, typeId;

        while ((is >> lineLabel).good())
        {
            label starCellId = lineLabel;
            is  >> shapeId
                >> nLabels
                >> cellTableId
                >> typeId;

            // skip the rest of the line
            readToNewline(is);

            // max 8 indices per line
            while (nLabels > 0)
            {
                readToNewline(is);
                nLabels -= 8;
            }

            if (typeId == starcdFluidType)
            {
                nFluids++;
                maxId = max(maxId, starCellId);

                if (!cellTable_.found(cellTableId))
                {
                    cellTable_.setName(cellTableId);
                    cellTable_.setMaterial(cellTableId, "fluid");
                }
            }
            else if (typeId == starcdSolidType)
            {
                nSolids++;
                if (keepSolids)
                {
                    maxId = max(maxId, starCellId);
                }

                if (!cellTable_.found(cellTableId))
                {
                    cellTable_.setName(cellTableId);
                    cellTable_.setMaterial(cellTableId, "solid");
                }

            }
            else if (typeId == starcdBaffleType)
            {
                // baffles have no cellTable entry
                nBaffles++;
                maxId = max(maxId, starCellId);
            }
            else if (typeId == starcdShellType)
            {
                nShells++;
                if (!cellTable_.found(cellTableId))
                {
                    cellTable_.setName(cellTableId);
                    cellTable_.setMaterial(cellTableId, "shell");
                }
            }

        }
    }

    Info<< "Number of fluids  = " << nFluids << nl
        << "Number of baffles = " << nBaffles << nl;
    if (keepSolids)
    {
        Info<< "Number of solids  = " << nSolids << nl;
    }
    else
    {
        Info<< "Ignored   solids  = " << nSolids << nl;
    }
    Info<< "Ignored   shells  = " << nShells << endl;


    label nCells;
    if (keepSolids)
    {
        nCells = nFluids + nSolids;
    }
    else
    {
        nCells = nFluids;
    }

    cellFaces_.setSize(nCells);
    cellShapes_.setSize(nCells);
    cellTableId_.setSize(nCells);

    // information for the interfaces
    baffleFaces_.setSize(nBaffles);

    // extra space for baffles
    origCellId_.setSize(nCells + nBaffles);
    mapToFoamCellId_.setSize(maxId+1);
    mapToFoamCellId_ = -1;


    // avoid undefined shapes for polyhedra
    cellShape genericShape(*unknownModel, labelList(0));

    // Pass 2:
    // construct cellFaces_ and possibly cellShapes_
    if (nCells <= 0)
    {
        FatalErrorIn("meshReaders::STARCD::readCells()")
            << "no cells in file " << inputName
            << abort(FatalError);
    }
    else
    {
        IFstream is(inputName);
        readHeader(is, fileSignature);

        labelList starLabels(64);
        label lineLabel, shapeId, nLabels, cellTableId, typeId;

        label cellI = 0;
        label baffleI = 0;

        while ((is >> lineLabel).good())
        {
            label starCellId = lineLabel;
            is  >> shapeId
                >> nLabels
                >> cellTableId
                >> typeId;

            if (nLabels > starLabels.size())
            {
                starLabels.setSize(nLabels);
            }
            starLabels = -1;

            // read indices - max 8 per line
            for (label i = 0; i < nLabels; ++i)
            {
                if ((i % 8) == 0)
                {
                    is >> lineLabel;
                }
                is >> starLabels[i];
            }

            // skip solid cells
            if (typeId == starcdSolidType && !keepSolids)
            {
                continue;
            }

            // determine the foam cell shape
            const cellModel* curModelPtr = nullptr;

            // fluid/solid cells
            switch (shapeId)
            {
                case starcdHex:
                    curModelPtr = hexModel;
                    break;
                case starcdPrism:
                    curModelPtr = prismModel;
                    break;
                case starcdTet:
                    curModelPtr = tetModel;
                    break;
                case starcdPyr:
                    curModelPtr = pyrModel;
                    break;
            }

            if (curModelPtr)
            {
                // primitive cell - use shapes

                // convert orig vertex Id to point label
                bool isBad = false;
                for (label i=0; i < nLabels; ++i)
                {
                    label pointId = mapToFoamPointId_[starLabels[i]];
                    if (pointId < 0)
                    {
                        Info<< "Cells inconsistent with vertex file. "
                            << "Star vertex " << starLabels[i]
                            << " does not exist" << endl;
                        isBad = true;
                        unknownVertices = true;
                    }
                    starLabels[i] = pointId;
                }

                if (isBad)
                {
                    continue;
                }

                // record original cell number and lookup
                origCellId_[cellI] = starCellId;
                mapToFoamCellId_[starCellId] = cellI;

                cellTableId_[cellI] = cellTableId;
                cellShapes_[cellI] = cellShape
                (
                    *curModelPtr,
                    SubList<label>(starLabels, nLabels)
                );

                cellFaces_[cellI] = cellShapes_[cellI].faces();
                cellI++;
            }
            else if (shapeId == starcdPoly)
            {
                // polyhedral cell
                label nFaces = starLabels[0] - 1;

                // convert orig vertex id to point label
                // start with offset (skip the index table)
                bool isBad = false;
                for (label i=starLabels[0]; i < nLabels; ++i)
                {
                    label pointId = mapToFoamPointId_[starLabels[i]];
                    if (pointId < 0)
                    {
                        Info<< "Cells inconsistent with vertex file. "
                            << "Star vertex " << starLabels[i]
                            << " does not exist" << endl;
                        isBad = true;
                        unknownVertices = true;
                    }
                    starLabels[i] = pointId;
                }

                if (isBad)
                {
                    continue;
                }

                // traverse beg/end indices
                faceList faces(nFaces);
                label faceI = 0;
                for (label i=0; i < nFaces; ++i)
                {
                    label beg = starLabels[i];
                    label n   = starLabels[i+1] - beg;

                    face f
                    (
                        SubList<label>(starLabels, n, beg)
                    );

                    f.collapse();

                    // valid faces only
                    if (f.size() >= 3)
                    {
                        faces[faceI++] = f;
                    }
                }

                if (nFaces > faceI)
                {
                    Info<< "star cell " << starCellId << " has "
                        << (nFaces - faceI)
                        << " empty faces - could cause boundary "
                        << "addressing problems"
                        << endl;

                    nFaces = faceI;
                    faces.setSize(nFaces);
                }

                if (nFaces < 4)
                {
                    FatalErrorIn("meshReaders::STARCD::readCells()")
                        << "star cell " << starCellId << " has " << nFaces
                        << abort(FatalError);
                }

                // record original cell number and lookup
                origCellId_[cellI] = starCellId;
                mapToFoamCellId_[starCellId] = cellI;

                cellTableId_[cellI] = cellTableId;
                cellShapes_[cellI]  = genericShape;
                cellFaces_[cellI]   = faces;
                cellI++;
            }
            else if (typeId == starcdBaffleType)
            {
                // baffles

                // convert orig vertex id to point label
                bool isBad = false;
                for (label i=0; i < nLabels; ++i)
                {
                    label pointId = mapToFoamPointId_[starLabels[i]];
                    if (pointId < 0)
                    {
                        Info<< "Baffles inconsistent with vertex file. "
                            << "Star vertex " << starLabels[i]
                            << " does not exist" << endl;
                        isBad = true;
                        unknownVertices = true;
                    }
                    starLabels[i] = pointId;
                }

                if (isBad)
                {
                    continue;
                }


                face f
                (
                    SubList<label>(starLabels, nLabels)
                );

                f.collapse();

                // valid faces only
                if (f.size() >= 3)
                {
                    baffleFaces_[baffleI] = f;
                    // insert lookup addressing in normal list
                    mapToFoamCellId_[starCellId]  = nCells + baffleI;
                    origCellId_[nCells + baffleI] = starCellId;
                    baffleI++;
                }
            }
        }

        baffleFaces_.setSize(baffleI);
    }

    if (unknownVertices)
    {
        FatalErrorIn("meshReaders::STARCD::readCells()")
            << "cells with unknown vertices"
            << abort(FatalError);
    }

    // truncate lists

#ifdef DEBUG_READING
    Info<< "CELLS READ" << endl;
#endif

    // cleanup
    mapToFoamPointId_.clear();
}


// read in the boundaries from the .bnd file
//
/*---------------------------------------------------------------------------*\
Line 1:
  PROSTAR_BOUNDARY [newline]

Line 2:
  <version> 0 0 0 0 0 0 0 [newline]

Body:
  <boundId>  <cellId>  <cellFace>  <regionId>  0  <boundaryType> [newline]

where boundaryType is truncated to 4 characters from one of the following:
INLET
PRESSSURE
OUTLET
BAFFLE
etc,
\*---------------------------------------------------------------------------*/

void Foam::meshReaders::STARCD::readBoundary(const fileName& inputName)
{
    const word fileSignature = "PROSTAR_BOUNDARY";
    label nPatches = 0, nFaces = 0, nBafflePatches = 0, maxId = 0;
    label lineLabel, starCellId, cellFaceId, starRegion, configNumber;
    word patchType;

    labelList mapToFoamPatchId(1000, label(-1));
    labelList nPatchFaces(1000, label(0));
    labelList origRegion(1000, label(0));
    patchTypes_.setSize(1000);

    // this is what we seem to need
    // these MUST correspond to starToFoamFaceAddr
    //
    Map<label> faceLookupIndex;

    faceLookupIndex.insert(hexModel->index(), 0);
    faceLookupIndex.insert(prismModel->index(), 1);
    faceLookupIndex.insert(tetModel->index(), 2);
    faceLookupIndex.insert(pyrModel->index(), 3);

    // Pass 1:
    // collect
    // no. of faces (nFaces), no. of patches (nPatches)
    // and for each of these patches the number of faces
    // (nPatchFaces[patchLabel])
    //
    // and a conversion table from Star regions to (Foam) patchLabels
    //
    // additionally note the no. of baffle patches (nBafflePatches)
    // so that we sort these to the end of the patch list
    // - this makes it easier to transfer them to an adjacent patch if reqd
    {
        IFstream is(inputName);

        if (is.good())
        {
            readHeader(is, fileSignature);

            while ((is >> lineLabel).good())
            {
                nFaces++;
                is  >> starCellId
                    >> cellFaceId
                    >> starRegion
                    >> configNumber
                    >> patchType;

                // Build translation table to convert star patch to foam patch
                label patchLabel = mapToFoamPatchId[starRegion];
                if (patchLabel == -1)
                {
                    patchLabel = nPatches;
                    mapToFoamPatchId[starRegion] = patchLabel;
                    origRegion[patchLabel] = starRegion;
                    patchTypes_[patchLabel] = patchType;

                    maxId = max(maxId, starRegion);

                    if (patchType == "BAFF")    // should actually be case-insensitive
                    {
                        nBafflePatches++;
                    }
                    nPatches++;
                }

                nPatchFaces[patchLabel]++;
            }

            if (nPatches == 0)
            {
                Info<< "No boundary faces in file " << inputName << endl;
            }
        }
        else
        {
            Info<< "Could not read boundary file " << inputName << endl;
        }
    }

    // keep empty patch region in reserve
    nPatches++;
    Info<< "Number of patches = " << nPatches
        << " (including extra for missing)" << endl;

    // resize
    origRegion.setSize(nPatches);
    patchTypes_.setSize(nPatches);
    patchNames_.setSize(nPatches);
    nPatchFaces.setSize(nPatches);

    // add our empty patch
    origRegion[nPatches-1] = 0;
    nPatchFaces[nPatches-1] = 0;
    patchTypes_[nPatches-1] = "none";

    // create names
    // - use 'Label' entry from "constant/boundaryRegion" dictionary
    forAll(patchTypes_, patchI)
    {
        bool foundName = false, foundType = false;

        Map<dictionary>::const_iterator
            iter = boundaryRegion_.find(origRegion[patchI]);

        if
        (
            iter != boundaryRegion_.end()
        )
        {
            foundType = iter().readIfPresent
            (
                "BoundaryType",
                patchTypes_[patchI]
            );

            foundName = iter().readIfPresent
            (
                "Label",
                patchNames_[patchI]
            );
        }

        // consistent names, in long form and in lowercase
        if (!foundType)
        {
            // transform
            forAllIter(string, patchTypes_[patchI], i)
            {
                *i = tolower(*i);
            }

            if (patchTypes_[patchI] == "symp")
            {
                patchTypes_[patchI] = "symplane";
            }
            else if (patchTypes_[patchI] == "cycl")
            {
                patchTypes_[patchI] = "cyclic";
            }
            else if (patchTypes_[patchI] == "baff")
            {
                patchTypes_[patchI] = "baffle";
            }
            else if (patchTypes_[patchI] == "moni")
            {
                patchTypes_[patchI] = "monitoring";
            }
        }

        // create a name if needed
        if (!foundName)
        {
            patchNames_[patchI] =
                patchTypes_[patchI] + "_" + name(origRegion[patchI]);
        }
    }

    // enforce name "Default_Boundary_Region"
    patchNames_[nPatches-1] = defaultBoundaryName;

    // sort according to ascending region numbers, but leave
    // Default_Boundary_Region as the final patch
    {
        labelList sortedIndices;
        sortedOrder(SubList<label>(origRegion, nPatches-1), sortedIndices);

        labelList oldToNew = identity(nPatches);
        forAll(sortedIndices, i)
        {
            oldToNew[sortedIndices[i]] = i;
        }

        inplaceReorder(oldToNew, origRegion);
        inplaceReorder(oldToNew, patchTypes_);
        inplaceReorder(oldToNew, patchNames_);
        inplaceReorder(oldToNew, nPatchFaces);
    }

    // re-sort to have baffles near the end
    nBafflePatches = 1;
    if (nBafflePatches)
    {
        labelList oldToNew = identity(nPatches);
        label newIndex = 0;
        label baffleIndex = (nPatches-1 - nBafflePatches);

        for (label i=0; i < oldToNew.size()-1; ++i)
        {
            if (patchTypes_[i] == "baffle")
            {
                oldToNew[i] = baffleIndex++;
            }
            else
            {
                oldToNew[i] = newIndex++;
            }
        }

        inplaceReorder(oldToNew, origRegion);
        inplaceReorder(oldToNew, patchTypes_);
        inplaceReorder(oldToNew, patchNames_);
        inplaceReorder(oldToNew, nPatchFaces);
    }

    mapToFoamPatchId.setSize(maxId+1, -1);
    forAll(origRegion, patchI)
    {
        mapToFoamPatchId[origRegion[patchI]] = patchI;
    }

    boundaryIds_.setSize(nPatches);
    forAll(boundaryIds_, patchI)
    {
        boundaryIds_[patchI].setSize(nPatchFaces[patchI]);
        nPatchFaces[patchI] = 0;
    }


    // Pass 2:
    //
    if (nPatches > 1 && mapToFoamCellId_.size() > 1)
    {
        IFstream is(inputName);
        readHeader(is, fileSignature);

        while ((is >> lineLabel).good())
        {
            is
                >> starCellId
                >> cellFaceId
                >> starRegion
                >> configNumber
                >> patchType;

            label patchI = mapToFoamPatchId[starRegion];

            // zero-based indexing
            cellFaceId--;

            label cellId = -1;

            // convert to FOAM cell number
            if (starCellId < mapToFoamCellId_.size())
            {
                cellId = mapToFoamCellId_[starCellId];
            }

            if (cellId < 0)
            {
                Info
                    << "Boundaries inconsistent with cell file. "
                    << "Star cell " << starCellId << " does not exist"
                    << endl;
            }
            else
            {
                // restrict lookup to volume cells (no baffles)
                if (cellId < cellShapes_.size())
                {
                    label index = cellShapes_[cellId].model().index();
                    if (faceLookupIndex.found(index))
                    {
                        index = faceLookupIndex[index];
                        cellFaceId = starToFoamFaceAddr[index][cellFaceId];
                    }
                }
                else
                {
                    // we currently use cellId >= nCells to tag baffles,
                    // we can also use a negative face number
                    cellFaceId = -1;
                }

                boundaryIds_[patchI][nPatchFaces[patchI]] =
                    cellFaceIdentifier(cellId, cellFaceId);

#ifdef DEBUG_BOUNDARY
                Info<< "bnd " << cellId << " " << cellFaceId << endl;
#endif
                // increment counter of faces in current patch
                nPatchFaces[patchI]++;
            }
        }
    }

    // retain original information in patchPhysicalTypes_ - overwrite latter
    patchPhysicalTypes_.setSize(patchTypes_.size());


    forAll(boundaryIds_, patchI)
    {
        // resize - avoid invalid boundaries
        if (nPatchFaces[patchI] < boundaryIds_[patchI].size())
        {
            boundaryIds_[patchI].setSize(nPatchFaces[patchI]);
        }

        word origType = patchTypes_[patchI];
        patchPhysicalTypes_[patchI] = origType;

        if (origType == "symplane")
        {
            patchTypes_[patchI] = symmetryPolyPatch::typeName;
            patchPhysicalTypes_[patchI] = patchTypes_[patchI];
        }
        else if (origType == "wall")
        {
            patchTypes_[patchI] = wallPolyPatch::typeName;
            patchPhysicalTypes_[patchI] = patchTypes_[patchI];
        }
        else if (origType == "cyclic")
        {
            // incorrect. should be cyclicPatch but this
            // requires info on connected faces.
            patchTypes_[patchI] = cyclicPolyPatch::typeName;
            patchPhysicalTypes_[patchI] = patchTypes_[patchI];
        }
        else if (origType == "baffle")
        {
            // incorrect. tag the patch until we get proper support.
            // set physical type to a canonical "baffle"
            patchTypes_[patchI] = emptyPolyPatch::typeName;
            patchPhysicalTypes_[patchI] = "baffle";
        }
        else
        {
            patchTypes_[patchI] = polyPatch::typeName;
        }

        Info<< "patch " << patchI
            << " (region " << origRegion[patchI]
            << ": " << origType << ") type: '" << patchTypes_[patchI]
            << "' name: " << patchNames_[patchI] << endl;
    }

    // cleanup
    mapToFoamCellId_.clear();
    cellShapes_.clear();
}


//
// remove unused points
//
void Foam::meshReaders::STARCD::cullPoints()
{
    label nPoints = points_.size();
    labelList oldToNew(nPoints, -1);

    // loop through cell faces and note which points are being used
    forAll(cellFaces_, cellI)
    {
        const faceList& faces = cellFaces_[cellI];
        forAll(faces, i)
        {
            const labelList& labels = faces[i];
            forAll(labels, j)
            {
                oldToNew[labels[j]]++;
            }
        }
    }

    // the new ordering and the count of unused points
    label pointI = 0;
    forAll(oldToNew, i)
    {
        if (oldToNew[i] >= 0)
        {
            oldToNew[i] = pointI++;
        }
    }

    // report unused points
    if (nPoints > pointI)
    {
        Info<< "Unused    points  = " << (nPoints - pointI) << endl;
        nPoints = pointI;

        // adjust points and truncate
        inplaceReorder(oldToNew, points_);
        points_.setSize(nPoints);

        // adjust cellFaces - with mesh shapes this might be faster
        forAll(cellFaces_, cellI)
        {
            faceList& faces = cellFaces_[cellI];
            forAll(faces, i)
            {
                inplaceRenumber(oldToNew, faces[i]);
            }
        }

        // adjust baffles
        forAll(baffleFaces_, faceI)
        {
            inplaceRenumber(oldToNew, baffleFaces_[faceI]);
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool Foam::meshReaders::STARCD::readGeometry(const scalar scaleFactor)
{
    // Info<< "called meshReaders::STARCD::readGeometry" << endl;

    readPoints(geometryFile_ + ".vrt", scaleFactor);
    readCells(geometryFile_ + ".cel");
    cullPoints();
    readBoundary(geometryFile_ + ".bnd");

    return true;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::meshReaders::STARCD::STARCD
(
    const fileName& prefix,
    const objectRegistry& registry,
    const scalar scaleFactor
)
:
    meshReader(prefix, scaleFactor),
    cellShapes_(0),
    mapToFoamPointId_(0),
    mapToFoamCellId_(0)
{
    readAux(registry);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::meshReaders::STARCD::~STARCD()
{}


// ************************************************************************* //
