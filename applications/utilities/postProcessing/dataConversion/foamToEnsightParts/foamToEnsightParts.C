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

Application
    foamToEnsightParts

Description
    Translates foam data to Ensight format.
    An Ensight part is created for each cellZone and patch.

Usage
    - foamToEnsightParts [OPTION] \n
    Translates foam data to Ensight format

    @param -ascii \n
    Write Ensight data in ASCII format instead of "C Binary"

    @param -noZero \n
    Exclude the often incomplete initial conditions.

    @param -index \<start\>\n
    Ignore the time index contained in the time file and use a
    simple indexing when creating the @c Ensight/data/######## files.

    @param -noMesh \n
    Suppress writing the geometry. Can be useful for converting partial
    results for a static geometry.

Note
    - no parallel data.
    - writes to @a Ensight directory to avoid collisions with foamToEnsight.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "timeSelector.H"

#include "volFields.H"
#include "OFstream.H"
#include "IOmanip.H"
#include "IOobjectList.H"
#include "scalarIOField.H"
#include "tensorIOField.H"

#include "ensightParts.H"
#include "ensightOutputFunctions.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Main program:

int main(int argc, char *argv[])
{
    // enable -constant
    // probably don't need -noZero though, since the fields are vetted
    // afterwards anyhow
    timeSelector::addOptions(true, false);
    argList::noParallel();
    argList::validOptions.insert("ascii", "");
    argList::validOptions.insert("index",  "start");
    argList::validOptions.insert("noMesh", "");

    // the volume field types that we handle
    wordHashSet volFieldTypes;
    volFieldTypes.insert(volScalarField::typeName);
    volFieldTypes.insert(volVectorField::typeName);
    volFieldTypes.insert(volSphericalTensorField::typeName);
    volFieldTypes.insert(volSymmTensorField::typeName);
    volFieldTypes.insert(volTensorField::typeName);

    // the lagrangian field types that we handle
    wordHashSet cloudFieldTypes;
    cloudFieldTypes.insert(scalarIOField::typeName);
    cloudFieldTypes.insert(vectorIOField::typeName);
    cloudFieldTypes.insert(tensorIOField::typeName);

    const char* geometryName = "geometry";

#   include "setRootCase.H"
#   include "createTime.H"

    // get times list
    instantList timeDirs = timeSelector::select0(runTime, args);

    // default to binary output, unless otherwise specified
    IOstream::streamFormat format = IOstream::BINARY;
    if (args.optionFound("ascii"))
    {
        format = IOstream::ASCII;
    }

    // control for renumbering iterations
    bool optIndex = false;
    label indexingNumber = 0;
    if (args.optionFound("index"))
    {
        optIndex = true;
        indexingNumber = args.optionRead<label>("index");
    }

    // always write the geometry, unless the -noMesh option is specified
    bool optNoMesh = args.optionFound("noMesh");

    fileName ensightDir = args.rootPath()/args.globalCaseName()/"Ensight";
    fileName dataDir = ensightDir/"data";
    fileName caseFileName = "Ensight.case";
    fileName dataMask = fileName("data")/ensightFile::mask();

    // Ensight and Ensight/data directories must exist
    // do not remove old data - we might wish to convert new results
    // or a particular time interval
    if (isDir(ensightDir))
    {
        Info<<"Warning: reusing existing directory" << nl
            << "    " << ensightDir << endl;
    }
    mkDir(ensightDir);
    mkDir(dataDir);

#   include "createNamedMesh.H"

    // Mesh instance (region0 gets filtered out)
    fileName regionPrefix;

    if (regionName != polyMesh::defaultRegion)
    {
        regionPrefix = regionName;
    }

    // Construct the list of ensight parts for the entire mesh
    ensightParts partsList(mesh);

    // write summary information
    {
        OFstream partsInfoFile(ensightDir/"partsInfo");

        partsInfoFile
            << "// summary of ensight parts" << nl << nl;
        partsList.writeSummary(partsInfoFile);
    }

#   include "checkHasMovingMesh.H"
#   include "findFields.H"

    if (hasMovingMesh && optNoMesh)
    {
        Info<< "mesh is moving: ignoring '-noMesh' option" << endl;
        optNoMesh = false;
    }


    // map times used
    Map<scalar>  timeIndices;

    // Track the time indices used by the volume fields
    dynamicLabelList fieldTimesUsed;

    // Track the time indices used by each cloud
    HashTable<dynamicLabelList > cloudTimesUsed;

    // Create a new DynamicList for each cloud
    forAllConstIter(HashTable<HashTable<word> >, cloudFields, cloudIter)
    {
        cloudTimesUsed.insert(cloudIter.key(), dynamicLabelList());
    }


    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

#       include "getTimeIndex.H"

        // remember the time index
        fieldTimesUsed.append(timeIndex);

        // the data/ITER subdirectory must exist
        fileName subDir = ensightFile::subDir(timeIndex);
        mkDir(dataDir/subDir);

        // place a timestamp in the directory for future reference
        {
            OFstream timeStamp(dataDir/subDir/"time");
            timeStamp
                << "#   timestep time" << nl
                << subDir.c_str() << " " << runTime.timeName() << nl;
        }

#       include "moveMesh.H"

        if (timeI == 0 || mesh.moving())
        {
            if (mesh.moving())
            {
                partsList.recalculate(mesh);
            }

            if (!optNoMesh)
            {
                fileName geomDir;
                if (hasMovingMesh)
                {
                    geomDir = dataDir/subDir;
                }

                ensightGeoFile geoFile(ensightDir/geomDir/geometryName, format);
                partsList.writeGeometry(geoFile);
                Info<< nl;
            }
        }

        Info<< "write volume field (" << flush;

        forAllConstIter(HashTable<word>, volumeFields, fieldIter)
        {
            const word& fieldName = fieldIter.key();
            const word& fieldType = fieldIter();

            IOobject fieldObject
            (
                fieldName,
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            );

            if (fieldType == volScalarField::typeName)
            {
                ensightVolField<scalar>
                (
                    partsList,
                    fieldObject,
                    mesh,
                    dataDir,
                    subDir,
                    format
                );

            }
            else if (fieldType == volVectorField::typeName)
            {
                ensightVolField<vector>
                (
                    partsList,
                    fieldObject,
                    mesh,
                    dataDir,
                    subDir,
                    format
                );

            }
            else if (fieldType == volSphericalTensorField::typeName)
            {
                ensightVolField<sphericalTensor>
                (
                    partsList,
                    fieldObject,
                    mesh,
                    dataDir,
                    subDir,
                    format
                );

            }
            else if (fieldType == volSymmTensorField::typeName)
            {
                ensightVolField<symmTensor>
                (
                    partsList,
                    fieldObject,
                    mesh,
                    dataDir,
                    subDir,
                    format
                );
            }
            else if (fieldType == volTensorField::typeName)
            {
                ensightVolField<tensor>
                (
                    partsList,
                    fieldObject,
                    mesh,
                    dataDir,
                    subDir,
                    format
                );
            }
        }
        Info<< " )" << endl;

        // check for clouds
        forAllConstIter(HashTable<HashTable<word> >, cloudFields, cloudIter)
        {
            const word& cloudName = cloudIter.key();

            if
            (
                !isDir
                (
                    runTime.timePath()/regionPrefix/
                    cloud::prefix/cloudName
                )
            )
            {
                continue;
            }

            IOobjectList cloudObjs
            (
                mesh,
                runTime.timeName(),
                cloud::prefix/cloudName
            );

            // check that the positions field is present for this time
            if (cloudObjs.lookup("positions"))
            {
                ensightParticlePositions
                (
                    mesh,
                    dataDir,
                    subDir,
                    cloudName,
                    format
                );
            }
            else
            {
                continue;
            }

            Info<< "write " << cloudName << " (" << flush;

            forAllConstIter(HashTable<word>, cloudIter(), fieldIter)
            {
                const word& fieldName = fieldIter.key();
                const word& fieldType = fieldIter();

                IOobject *fieldObject = cloudObjs.lookup(fieldName);

                if (!fieldObject)
                {
                    Info<< "missing "
                        << runTime.timeName()/cloud::prefix/cloudName
                        / fieldName
                        << endl;
                    continue;
                }

                if (fieldType == scalarIOField::typeName)
                {
                    ensightLagrangianField<scalar>
                    (
                        *fieldObject,
                        dataDir,
                        subDir,
                        cloudName,
                        format
                    );

                }
                else if (fieldType == vectorIOField::typeName)
                {
                    ensightLagrangianField<vector>
                    (
                        *fieldObject,
                        dataDir,
                        subDir,
                        cloudName,
                        format
                    );

                }
                else if (fieldType == tensorIOField::typeName)
                {
                    ensightLagrangianField<tensor>
                    (
                        *fieldObject,
                        dataDir,
                        subDir,
                        cloudName,
                        format
                    );

                }
            }

            Info<< " )" << endl;

            // remember the time index
            cloudTimesUsed[cloudName].append(timeIndex);
        }
    }

#   include "ensightOutputCase.H"

    Info<< "\nEnd\n"<< endl;

    return 0;
}


// ************************************************************************* //
