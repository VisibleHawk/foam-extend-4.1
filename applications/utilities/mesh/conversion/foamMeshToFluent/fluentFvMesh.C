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

#include <fstream>
#include <iostream>

using std::ofstream;
using std::ios;

#include "objectRegistry.H"
#include "foamTime.H"
#include "fluentFvMesh.H"
#include "primitiveMesh.H"
#include "wallFvPatch.H"
#include "symmetryFvPatch.H"
#include "cellModeller.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fluentFvMesh::fluentFvMesh(const IOobject& io)
:
    fvMesh(io)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fluentFvMesh::writeFluentMesh() const
{
    // make a directory called proInterface in the case
    mkDir(time().rootPath()/time().caseName()/"fluentInterface");

    // open a file for the mesh
    ofstream fluentMeshFile
    (
        (
            time().rootPath()/
            time().caseName()/
            "fluentInterface"/
            time().caseName() + ".msh"
        ).c_str()
    );

    Info << "Writing Header" << endl;

    fluentMeshFile
        << "(0 \"FOAM to Fluent Mesh File\")" << std::endl << std::endl
        << "(0 \"Dimension:\")" << std::endl
        << "(2 3)" << std::endl << std::endl
        << "(0 \"Grid dimensions:\")" << std::endl;

    // Writing number of points
    fluentMeshFile
            << "(10 (0 1 ";

    // Writing hex
    fluentMeshFile.setf(ios::hex, ios::basefield);

    fluentMeshFile
        << nPoints() << " 0 3))" << std::endl;

    // Writing number of cells
    fluentMeshFile
        << "(12 (0 1 "
        << nCells() << " 0 0))" << std::endl;

    // Writing number of faces
    label nFcs = nFaces();

    fluentMeshFile
            << "(13 (0 1 ";

    // Still writing hex
    fluentMeshFile
        << nFcs << " 0 0))" << std::endl << std::endl;

    // Return to dec
    fluentMeshFile.setf(ios::dec, ios::basefield);

    // Writing points
    fluentMeshFile
            << "(10 (1 1 ";

    fluentMeshFile.setf(ios::hex, ios::basefield);
    fluentMeshFile
        << nPoints() << " 1 3)"
        << std::endl << "(" << std::endl;

    fluentMeshFile.precision(10);
    fluentMeshFile.setf(ios::scientific);

    const pointField& p = points();

    forAll (p, pointI)
    {
        fluentMeshFile
            << "    "
            << p[pointI].x() << " "
            << p[pointI].y()
            << " " << p[pointI].z() << std::endl;
    }

    fluentMeshFile
        << "))" << std::endl << std::endl;

    const unallocLabelList& own = owner();
    const unallocLabelList& nei = neighbour();

    const faceList& fcs = faces();

    // Writing (mixed) internal faces
    fluentMeshFile
        << "(13 (2 1 "
        << own.size() << " 2 0)" << std::endl << "(" << std::endl;

    forAll (own, faceI)
    {
        const labelList& l = fcs[faceI];

        fluentMeshFile << "    ";

        fluentMeshFile << l.size() << " ";

        forAll (l, lI)
        {
            fluentMeshFile << l[lI] + 1 << " ";
        }

        fluentMeshFile << nei[faceI] + 1 << " ";
        fluentMeshFile << own[faceI] + 1 << std::endl;
    }

    fluentMeshFile << "))" << std::endl;

    label nWrittenFaces = own.size();

    // Writing boundary faces
    forAll (boundary(), patchI)
    {
        const unallocFaceList& patchFaces = boundaryMesh()[patchI];

        const labelList& patchFaceCells =
            boundaryMesh()[patchI].faceCells();

        // The face group will be offset by 10 from the patch label

        // Write header
        fluentMeshFile
            << "(13 (" << patchI + 10 << " " << nWrittenFaces + 1
            << " " << nWrittenFaces + patchFaces.size() << " ";

        nWrittenFaces += patchFaces.size();

        // Write patch type
        if (isA<wallFvPatch>(boundary()[patchI]))
        {
            fluentMeshFile << 3;
        }
        else if (isA<symmetryFvPatch>(boundary()[patchI]))
        {
            fluentMeshFile << 7;
        }
        else
        {
            fluentMeshFile << 4;
        }

        fluentMeshFile
            <<" 0)" << std::endl << "(" << std::endl;

        forAll (patchFaces, faceI)
        {
            const labelList& l = patchFaces[faceI];

            fluentMeshFile << "    ";

            fluentMeshFile << l.size() << " ";

            // Note: In Fluent, all boundary faces point inwards, which is
            // opposite from the FOAM convention. Turn them round on printout
            forAllReverse (l, lI)
            {
                fluentMeshFile << l[lI] + 1 << " ";
            }

            fluentMeshFile << patchFaceCells[faceI] + 1 << " 0" << std::endl;
        }

        fluentMeshFile << "))" << std::endl;
    }

    // Writing cells
    fluentMeshFile
        << "(12 (1 1 "
        << nCells() << " 1 0)(" << std::endl;

    const cellModel& hex = *(cellModeller::lookup("hex"));
    const cellModel& prism = *(cellModeller::lookup("prism"));
    const cellModel& pyr = *(cellModeller::lookup("pyr"));
    const cellModel& tet = *(cellModeller::lookup("tet"));

    const cellShapeList& cells = cellShapes();

    bool hasWarned = false;

    forAll (cells, cellI)
    {
        if (cells[cellI].model() == tet)
        {
            fluentMeshFile << " " << 2;
        }
        else if (cells[cellI].model() == hex)
        {
            fluentMeshFile << " " << 4;
        }
        else if (cells[cellI].model() == pyr)
        {
            fluentMeshFile << " " << 5;
        }
        else if (cells[cellI].model() == prism)
        {
            fluentMeshFile << " " << 6;
        }
        else
        {
            if (!hasWarned)
            {
                hasWarned = true;

                WarningIn("void fluentFvMesh::writeFluentMesh() const")
                    << "foamMeshToFluent: cell shape for cell "
                    << cellI << " only supported by Fluent polyhedral meshes."
                    << nl
                    << "    Suppressing any further messages for polyhedral"
                    << " cells." << endl;
            }
            fluentMeshFile << " " << 7;
        }
    }

    fluentMeshFile << ")())" << std::endl;

    // Return to dec
    fluentMeshFile.setf(ios::dec, ios::basefield);

    // Writing patch types
    fluentMeshFile << "(39 (1 fluid fluid-1)())" << std::endl;
    fluentMeshFile << "(39 (2 interior interior-1)())" << std::endl;

    // Writing boundary patch types
    forAll (boundary(), patchI)
    {
        fluentMeshFile
            << "(39 (" << patchI + 10 << " ";

        // Write patch type
        if (isA<wallFvPatch>(boundary()[patchI]))
        {
            fluentMeshFile << "wall ";
        }
        else if (isA<symmetryFvPatch>(boundary()[patchI]))
        {
            fluentMeshFile << "symmetry ";
        }
        else
        {
            fluentMeshFile << "pressure-outlet ";
        }

        fluentMeshFile
            << boundary()[patchI].name() << ")())" << std::endl;
    }
}


// ************************************************************************* //
