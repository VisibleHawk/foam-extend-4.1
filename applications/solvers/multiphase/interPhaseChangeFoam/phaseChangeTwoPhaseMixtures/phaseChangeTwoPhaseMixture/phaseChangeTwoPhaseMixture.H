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
    Foam::phaseChangeTwoPhaseMixture

Description

SourceFiles
    phaseChangeTwoPhaseMixture.C
    newPhaseChangeModel.C

\*---------------------------------------------------------------------------*/

#ifndef phaseChangeTwoPhaseMixture_H
#define phaseChangeTwoPhaseMixture_H

#include "twoPhaseMixture.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"
#include "volFields.H"
#include "dimensionedScalar.H"
#include "autoPtr.H"
#include "Pair.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                Class phaseChangeTwoPhaseMixture Declaration
\*---------------------------------------------------------------------------*/

class phaseChangeTwoPhaseMixture
:
    public twoPhaseMixture
{

protected:

    // Protected data

        dictionary phaseChangeTwoPhaseMixtureCoeffs_;

        //- Saturation vapour pressure
        dimensionedScalar pSat_;


    // Private Member Functions

        //- Disallow copy construct
        phaseChangeTwoPhaseMixture(const phaseChangeTwoPhaseMixture&);

        //- Disallow default bitwise assignment
        void operator=(const phaseChangeTwoPhaseMixture&);


public:

    //- Runtime type information
    TypeName("phaseChangeTwoPhaseMixture");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            phaseChangeTwoPhaseMixture,
            components,
            (
                const volVectorField& U,
                const surfaceScalarField& phi,
                const word& alpha1Name
            ),
            (U, phi, alpha1Name)
        );


    // Selectors

        //- Return a reference to the selected phaseChange model
        static autoPtr<phaseChangeTwoPhaseMixture> New
        (
            const volVectorField& U,
            const surfaceScalarField& phi,
            const word& alpha1Name
        );


    // Constructors

        //- Construct from components
        phaseChangeTwoPhaseMixture
        (
            const word& type,
            const volVectorField& U,
            const surfaceScalarField& phi,
            const word& alpha1Name = "alpha1"
        );


    // Destructor

        virtual ~phaseChangeTwoPhaseMixture()
        {}


    // Member Functions

        //- Return const-access to the saturation vapour pressure
        const dimensionedScalar& pSat() const
        {
            return pSat_;
        }

        //- Return the mass condensation and vaporisation rates as a
        //  coefficient to multiply (1 - alphal) for the condensation rate
        //  and a coefficient to multiply  alphal for the vaporisation rate
        virtual Pair<tmp<volScalarField> > mDotAlphal() const = 0;

        //- Return the mass condensation and vaporisation rates as an
        //  explicit term for the condensation rate and a coefficient to
        //  multiply (p - pSat) for the vaporisation rate
        virtual Pair<tmp<volScalarField> > mDotP() const = 0;

        //- Return the volumetric condensation and vaporisation rates as a
        //  coefficient to multiply (1 - alphal) for the condensation rate
        //  and a coefficient to multiply  alphal for the vaporisation rate
        Pair<tmp<volScalarField> > vDotAlphal() const;

        //- Return the volumetric condensation and vaporisation rates as an
        //  explicit term for the condensation rate and a coefficient to
        //  multiply (p - pSat) for the vaporisation rate
        Pair<tmp<volScalarField> > vDotP() const;

        //- Correct the phaseChange model
        virtual void correct() = 0;

        //- Read the transportProperties dictionary and update
        virtual bool read() = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
