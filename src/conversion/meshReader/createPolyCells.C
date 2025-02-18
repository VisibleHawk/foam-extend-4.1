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

Description
    create cellPolys
    - use pointCells when searching for connectivity
    - initialize the cell connectivity with '-1'
    - find both cell faces corresponding to the baffles and mark them
      to prevent a connection
    - standard connectivity checks

    - added baffle support

\*---------------------------------------------------------------------------*/

#include "meshReader.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::meshReader::createPolyCells()
{
    // loop through all cell faces and create connectivity. This will produce
    // a global face list and will describe all cells as lists of face labels

    const faceListList& cFaces = cellFaces();

    // count the maximum number of faces and set the size of the cellPolys_
    cellPolys_.setSize(cFaces.size());

    label maxFaces = 0;

    forAll(cellPolys_, cellI)
    {
        cellPolys_[cellI].setSize(cFaces[cellI].size(), -1);

        maxFaces += cFaces[cellI].size();
    }

    Info<< "Maximum possible number of faces in mesh: " << maxFaces << endl;

    meshFaces_.setSize(maxFaces);

    // set reference to point-cell addressing
    const labelListList& ptCells = pointCells();

    // size the baffle lists and initialize to -1
    baffleIds_.setSize(baffleFaces_.size());
    forAll(baffleIds_, baffleI)
    {
        baffleIds_[baffleI].setSize(2);
    }

    // block off baffles first
    //
    // To prevent internal faces, we'll mark the cell faces
    // with negative cell ids (offset by nCells).
    // eg,
    //    cellI = -(nCells + baffleI)
    //
    // To distinguish these from the normal '-1' marker, we require
    //    cellI = -(nCells + baffleI) < -1
    //
    // This condition is met provided that nCells > 1.
    // ie., baffles require at least 2 volume cells

    label baffleOffset = cFaces.size();
    forAll(baffleFaces_, baffleI)
    {
        label cellI = -(baffleOffset + baffleI);
        const face& curFace = baffleFaces_[baffleI];

        // get the list of labels
        const labelList& curPoints = curFace;

        // a baffle is a single face - only need to match one face
        // get the list of cells sharing this point
        const labelList& curNeighbours = ptCells[curPoints[0]];

        label nNeighbours = 0;

        // For all neighbours
        forAll(curNeighbours, neiI)
        {
            label curNei = curNeighbours[neiI];

            // get the list of search faces
            const faceList& searchFaces = cFaces[curNei];

            forAll(searchFaces, neiFaceI)
            {
                int cmp = face::compare(curFace, searchFaces[neiFaceI]);

                if (cmp)
                {
                    // maintain baffle orientation
                    // side0: baffle normal same as attached face
                    // side1: baffle normal opposite from attached face
                    //
                    label side = 0;
                    if (cmp < 0)
                    {
                        side = 1;
                    }

#ifdef DEBUG_FACE_ORDERING
                    Info<< "cmp " << cmp << " matched " << curFace
                        << " with " << searchFaces[neiFaceI]
                        << endl;


                    Info<< "match " << baffleI
                        << " (" << origCellId_[baffleOffset+baffleI] << ")"
                        << " side " << side
                        << " against cell " << curNei
                        << " face " << neiFaceI
                        << " curFace " << curFace[1]
                        << " neiFace " << searchFaces[neiFaceI][1]
                        << endl;
#endif

                    if (baffleIds_[baffleI][side].unused())
                    {
                        baffleIds_[baffleI][side] = cellFaceIdentifier
                        (
                            curNei,
                            neiFaceI
                        );

                        nNeighbours++;
                    }
                    else
                    {
                        Info<< "multiple matches for side " << side
                            << " of baffle " << baffleI
                            << " (original cell "
                            << origCellId_[baffleOffset+baffleI] << ")"
                            << endl;
                    }
                    break;
                }
            }
            if (nNeighbours >= 2) break;
        }

        if (nNeighbours == 2)
        {
            for (label side = 0; side < nNeighbours; ++side)
            {
                label neiCell = baffleIds_[baffleI][side].cell;
                label neiFace = baffleIds_[baffleI][side].face;

                if (baffleIds_[baffleI][side].used())
                {
                    cellPolys_[neiCell][neiFace] = cellI;
                }
            }
        }
        else
        {
            Info<< "drop baffle " << baffleI
                << " (original cell "
                << origCellId_[baffleOffset+baffleI] << ")"
                << " with " << nNeighbours << " neighbours" << endl;

            baffleFaces_[baffleI].clear();
            baffleIds_[baffleI].clear();
        }
    }

#ifdef DEBUG_CELLPOLY
    Info<< "cellPolys_" << cellPolys_ << endl;
    Info<< "baffleFaces_" << baffleFaces_ << endl;
    Info<< "baffleIds_"   << baffleIds_ << endl;
#endif

    bool found = false;

    nInternalFaces_ = 0;

    forAll(cFaces, cellI)
    {
        // Note:
        // Insertion cannot be done in one go as the faces need to be
        // added into the list in the increasing order of neighbour
        // cells.  Therefore, all neighbours will be detected first
        // and then added in the correct order.

        const faceList& curFaces = cFaces[cellI];

        // Record the neighbour cell
        labelList neiCells(curFaces.size(), -1);

        // Record the face of neighbour cell
        labelList faceOfNeiCell(curFaces.size(), -1);

        label nNeighbours = 0;

        // For all faces ...
        forAll(curFaces, faceI)
        {
            // Skip already matched faces or those tagged by baffles
            if (cellPolys_[cellI][faceI] != -1) continue;

            found = false;

            const face& curFace = curFaces[faceI];

            // get the list of labels
            const labelList& curPoints = curFace;

            // For all points
            forAll(curPoints, pointI)
            {
                // get the list of cells sharing this point
                const labelList& curNeighbours = ptCells[curPoints[pointI]];

                // For all neighbours
                forAll(curNeighbours, neiI)
                {
                    label curNei = curNeighbours[neiI];

                    // reject neighbours with the lower label. This should
                    // also reject current cell.
                    if (curNei > cellI)
                    {
                        // get the list of search faces
                        const faceList& searchFaces = cFaces[curNei];

                        forAll(searchFaces, neiFaceI)
                        {
                            if (searchFaces[neiFaceI] == curFace)
                            {
                                // Record the neighbour cell and face
                                neiCells[faceI] = curNei;
                                faceOfNeiCell[faceI] = neiFaceI;
                                nNeighbours++;
#ifdef DEBUG_FACE_ORDERING
                                Info<< " cell " << cellI
                                    << " face " << faceI
                                    << " point " << pointI
                                    << " nei " << curNei
                                    << " neiFace " << neiFaceI
                                    << endl;
#endif
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (found) break;
                }
                if (found) break;
            } // End of current points
        } // End of current faces

        // Add the faces in the increasing order of neighbours
        for (label neiSearch = 0; neiSearch < nNeighbours; neiSearch++)
        {
            // Find the lowest neighbour which is still valid
            label nextNei = -1;
            label minNei = cellPolys_.size();

            forAll(neiCells, ncI)
            {
                if (neiCells[ncI] > -1 && neiCells[ncI] < minNei)
                {
                    nextNei = ncI;
                    minNei = neiCells[ncI];
                }
            }

            if (nextNei > -1)
            {
                // Add the face to the list of faces
                meshFaces_[nInternalFaces_] = curFaces[nextNei];

                // Mark for owner
                cellPolys_[cellI][nextNei] = nInternalFaces_;

                // Mark for neighbour
                cellPolys_[neiCells[nextNei]][faceOfNeiCell[nextNei]] =
                    nInternalFaces_;

                // Stop the neighbour from being used again
                neiCells[nextNei] = -1;

                // Increment number of faces counter
                nInternalFaces_++;
            }
            else
            {
              FatalErrorIn("meshReader::createPolyCells()")
                  << "Error in internal face insertion"
                  << abort(FatalError);
            }
        }
    }

#ifdef DEBUG_CELLPOLY
    Info<< "cellPolys = " << cellPolys_ << endl;
#endif

    // don't reset the size of internal faces, because more faces will be
    // added in createPolyBoundary()
}


// ************************************************************************* //
