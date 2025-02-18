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
    Misc helper methods and utilities

\*---------------------------------------------------------------------------*/

#include "vtkPVFoam.H"
#include "vtkPVFoamReader.h"

// Foam includes
#include "fvMesh.H"
#include "foamTime.H"
#include "IFstream.H"

// VTK includes
#include "vtkDataArraySelection.h"
#include "vtkDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkInformation.h"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::vtkPVFoam::AddToBlock
(
    vtkMultiBlockDataSet* output,
    vtkDataSet* dataset,
    const partInfo& selector,
    const label datasetNo,
    const string& datasetName
)
{
    const int blockNo = selector.block();

    vtkDataObject* blockDO = output->GetBlock(blockNo);
    vtkMultiBlockDataSet* block = vtkMultiBlockDataSet::SafeDownCast(blockDO);

    if (!block)
    {
        if (blockDO)
        {
            FatalErrorIn("Foam::vtkPVFoam::AddToBlock")
                << "Block already has a vtkDataSet assigned to it"
                << endl;
            return;
        }

        block = vtkMultiBlockDataSet::New();
        output->SetBlock(blockNo, block);
        block->Delete();
    }

    if (debug)
    {
        Info<< "block[" << blockNo << "] has "
            << block->GetNumberOfBlocks()
            <<  " datasets prior to adding set " << datasetNo
            <<  " with name: " << datasetName << endl;
    }

    block->SetBlock(datasetNo, dataset);

    // name the block when assigning dataset 0
    if (datasetNo == 0)
    {
        output->GetMetaData(blockNo)->Set
        (
            vtkCompositeDataSet::NAME(),
            selector.name()
        );
    }

    if (datasetName.size())
    {
        block->GetMetaData(datasetNo)->Set
        (
            vtkCompositeDataSet::NAME(),
            datasetName.c_str()
        );
    }
}


vtkDataSet* Foam::vtkPVFoam::GetDataSetFromBlock
(
    vtkMultiBlockDataSet* output,
    const partInfo& selector,
    const label datasetNo
)
{
    const int blockNo = selector.block();

    vtkDataObject* blockDO = output->GetBlock(blockNo);
    vtkMultiBlockDataSet* block = vtkMultiBlockDataSet::SafeDownCast(blockDO);

    if (block)
    {
        return vtkDataSet::SafeDownCast(block->GetBlock(datasetNo));
    }

    return 0;
}


// ununsed at the moment
Foam::label Foam::vtkPVFoam::GetNumberOfDataSets
(
    vtkMultiBlockDataSet* output,
    const partInfo& selector
)
{
    const int blockNo = selector.block();

    vtkDataObject* blockDO = output->GetBlock(blockNo);
    vtkMultiBlockDataSet* block = vtkMultiBlockDataSet::SafeDownCast(blockDO);
    if (block)
    {
        return block->GetNumberOfBlocks();
    }

    return 0;
}


Foam::word Foam::vtkPVFoam::getPartName(int partId)
{
    return getFirstWord(reader_->GetPartArrayName(partId));
}


Foam::wordHashSet Foam::vtkPVFoam::getSelected
(
    vtkDataArraySelection* select
)
{
    int nElem = select->GetNumberOfArrays();
    wordHashSet selections(2*nElem);

    for (int elemI=0; elemI < nElem; ++elemI)
    {
        if (select->GetArraySetting(elemI))
        {
            selections.insert(getFirstWord(select->GetArrayName(elemI)));
        }
    }

    return selections;
}


Foam::wordHashSet Foam::vtkPVFoam::getSelected
(
    vtkDataArraySelection* select,
    const partInfo& selector
)
{
    int nElem = select->GetNumberOfArrays();
    wordHashSet selections(2*nElem);

    for (int elemI = selector.start(); elemI < selector.end(); ++elemI)
    {
        if (select->GetArraySetting(elemI))
        {
            selections.insert(getFirstWord(select->GetArrayName(elemI)));
        }
    }

    return selections;
}


Foam::stringList Foam::vtkPVFoam::getSelectedArrayEntries
(
    vtkDataArraySelection* select
)
{
    stringList selections(select->GetNumberOfArrays());
    label nElem = 0;

    forAll(selections, elemI)
    {
        if (select->GetArraySetting(elemI))
        {
            selections[nElem++] = select->GetArrayName(elemI);
        }
    }
    selections.setSize(nElem);


    if (debug)
    {
        label nElem = select->GetNumberOfArrays();
        Info<< "available(";
        for (int elemI = 0; elemI < nElem; ++elemI)
        {
            Info<< " \"" << select->GetArrayName(elemI) << "\"";
        }
        Info<< " )\nselected(";

        forAll(selections, elemI)
        {
            Info<< " " << selections[elemI];
        }
        Info<< " )\n";
    }

    return selections;
}


Foam::stringList Foam::vtkPVFoam::getSelectedArrayEntries
(
    vtkDataArraySelection* select,
    const partInfo& selector
)
{
    stringList selections(selector.size());
    label nElem = 0;

    for (int elemI = selector.start(); elemI < selector.end(); ++elemI)
    {
        if (select->GetArraySetting(elemI))
        {
            selections[nElem++] = select->GetArrayName(elemI);
        }
    }
    selections.setSize(nElem);


    if (debug)
    {
        Info<< "available(";
        for (int elemI = selector.start(); elemI < selector.end(); ++elemI)
        {
            Info<< " \"" << select->GetArrayName(elemI) << "\"";
        }
        Info<< " )\nselected(";

        forAll(selections, elemI)
        {
            Info<< " " << selections[elemI];
        }
        Info<< " )\n";
    }

    return selections;
}


void Foam::vtkPVFoam::setSelectedArrayEntries
(
    vtkDataArraySelection* select,
    const stringList& selections
)
{
    const int nElem = select->GetNumberOfArrays();
    select->DisableAllArrays();

    // Loop through entries, setting values from selectedEntries
    for (int elemI=0; elemI < nElem; ++elemI)
    {
        string arrayName(select->GetArrayName(elemI));

        forAll(selections, elemI)
        {
            if (selections[elemI] == arrayName)
            {
                select->EnableArray(arrayName.c_str());
                break;
            }
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// parse these bits of info from /proc/meminfo (Linux)
//
// MemTotal:      2062660 kB
// MemFree:       1124400 kB
//
// used = MemTotal - MemFree is what the free(1) uses.
//
void Foam::vtkPVFoam::printMemory()
{
    const char* meminfo = "/proc/meminfo";

    if (exists(meminfo))
    {
        IFstream is(meminfo);
        label memTotal = 0;
        label memFree = 0;

        string line;

        while (is.getLine(line).good())
        {
            char tag[32];
            int value;

            if (sscanf(line.c_str(), "%30s %d", tag, &value) == 2)
            {
                if (!strcmp(tag, "MemTotal:"))
                {
                    memTotal = value;
                }
                else if (!strcmp(tag, "MemFree:"))
                {
                    memFree = value;
                }
            }
        }

        Info << "memUsed: " << (memTotal - memFree) << " kB\n";
    }
}


// ************************************************************************* //
