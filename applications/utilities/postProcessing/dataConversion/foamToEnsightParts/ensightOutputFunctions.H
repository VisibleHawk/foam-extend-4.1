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
    Miscellaneous collection of functions and template related to Ensight data

SourceFiles
    ensightOutputFunctions.C

\*---------------------------------------------------------------------------*/

#ifndef ensightOutputFunctions_H
#define ensightOutputFunctions_H

#include "ensightFile.H"
#include "CloudTemplate.H"
#include "polyMesh.H"
#include "IOobject.H"

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void ensightCaseEntry
(
    OFstream& caseFile,
    const string& ensightType,
    const word& fieldName,
    const fileName& dataMask,
    const fileName& local=fileName::null,
    const label cloudNo=-1,
    const label timeSet=1
);


void ensightParticlePositions
(
    const polyMesh& mesh,
    const fileName& dataDir,
    const fileName& subDir,
    const word& cloudName,
    IOstream::streamFormat format
);


//- write lagrangian parcels
template<class Type>
void ensightLagrangianField
(
    const IOobject& fieldObject,
    const fileName& dataDir,
    const fileName& subDir,
    const word& cloudName,
    IOstream::streamFormat format
);

//- write generalized field components
template<class Type>
void ensightVolField
(
    const ensightParts& partsList,
    const IOobject& fieldObject,
    const fvMesh& mesh,
    const fileName& dataDir,
    const fileName& subDir,
    IOstream::streamFormat format
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // namespace Foam

#ifdef NoRepository
#   include "ensightOutputFunctions.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
