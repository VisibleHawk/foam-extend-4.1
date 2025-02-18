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

Class
    Foam::SurfaceReactionModel

Description
    Templated surface reaction model class

SourceFiles
    SurfaceReactionModel.C
    NewSurfaceReactionModel.C

\*---------------------------------------------------------------------------*/

#ifndef SurfaceReactionModel_H
#define SurfaceReactionModel_H

#include "IOdictionary.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"

#include "scalarField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class SurfaceReactionModel Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class SurfaceReactionModel
{
    // Private data

        //- The cloud dictionary
        const dictionary& dict_;

        //- Reference to the owner cloud class
        CloudType& owner_;

        //- The coefficients dictionary
        const dictionary coeffDict_;


public:

    //-Runtime type information
    TypeName("SurfaceReactionModel");


    //- Declare runtime constructor selection table
    declareRunTimeSelectionTable
    (
        autoPtr,
        SurfaceReactionModel,
        dictionary,
        (
            const dictionary& dict,
            CloudType& cloud
        ),
        (dict, cloud)
    );


    // Constructors

        //- Construct null from owner
        SurfaceReactionModel(CloudType& owner);

        //- Construct from dictionary
        SurfaceReactionModel
        (
            const dictionary& dict,
            CloudType& cloud,
            const word& type
        );


    //- Destructor
    virtual ~SurfaceReactionModel();


    //- Selector
    static autoPtr<SurfaceReactionModel<CloudType> > New
    (
        const dictionary& dict,
        CloudType& cloud
    );


    // Access

        //- Return the owner cloud object
        const CloudType& owner() const;

        //- Return the cloud dictionary
        const dictionary& dict() const;

        //- Return the coefficients dictionary
        const dictionary& coeffDict() const;


    // Member Functions

        //- Flag to indicate whether model activates surface reaction model
        virtual bool active() const = 0;

        //- Update surface reactions
        //  Returns the heat of reaction
        virtual scalar calculate
        (
            const scalar dt,
            const label cellI,
            const scalar d,
            const scalar T,
            const scalar Tc,
            const scalar pc,
            const scalar rhoc,
            const scalar mass,
            const scalarField& YGas,
            const scalarField& YLiquid,
            const scalarField& YSolid,
            const scalarField& YMixture,
            const scalar N,
            scalarField& dMassGas,
            scalarField& dMassLiquid,
            scalarField& dMassSolid,
            scalarField& dMassSRCarrier
        ) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define makeSurfaceReactionModel(CloudType)                                   \
                                                                              \
    defineNamedTemplateTypeNameAndDebug(SurfaceReactionModel<CloudType>, 0);  \
                                                                              \
    defineTemplateRunTimeSelectionTable                                       \
    (                                                                         \
        SurfaceReactionModel<CloudType>,                                      \
        dictionary                                                            \
    );


#define makeSurfaceReactionModelThermoType(SS, CloudType, ParcelType, ThermoType)\
                                                                              \
    defineNamedTemplateTypeNameAndDebug                                       \
    (                                                                         \
        SS<CloudType<ParcelType<ThermoType> > >,                              \
        0                                                                     \
    );                                                                        \
                                                                              \
    SurfaceReactionModel<CloudType<ParcelType<ThermoType> > >::               \
        adddictionaryConstructorToTable                                       \
            <SS<CloudType<ParcelType<ThermoType> > > >                        \
            add##SS##CloudType##ParcelType##ThermoType##ConstructorToTable_;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "SurfaceReactionModel.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
