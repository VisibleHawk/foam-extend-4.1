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
    Foam::uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField

Description
    Foam::uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField

SourceFiles
    uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField_H
#define uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField_H

#include "fixedValuePointPatchFields.H"
#include "sixDoFRigidBodyMotion.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
  Class uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
:
    public FixedValuePointPatchField
    <
        pointPatchField,
        pointMesh,
        pointPatch,
        DummyMatrix,
        vector
    >
{
    // Private data

        //- Six dof motion object
        sixDoFRigidBodyMotion motion_;

        //- Initial positions of points on the patch
        pointField initialPoints_;

        //- Reference density required by the forces object for
        //  incompressible calculations.  Retained here to give
        //  dictionary compatibility with other sixDoF patches.
        scalar rhoInf_;


public:

    //- Runtime type information
    TypeName("uncoupledSixDoFRigidBodyDisplacement");


    // Constructors

        //- Construct from patch and internal field
        uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&
        );

        //- Construct from patch, internal field and dictionary
        uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const dictionary&
        );

        //- Construct by mapping given patchField<vector> onto a new patch
        uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
        (
            const uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField&,
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const PointPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual autoPtr<pointPatchField<vector> > clone() const
        {
            return autoPtr<pointPatchField<vector> >
            (
                new uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
                (
                    *this
                )
            );
        }

        //- Construct as copy setting internal field reference
        uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
        (
            const uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField&,
            const DimensionedField<vector, pointMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual autoPtr<pointPatchField<vector> > clone
        (
            const DimensionedField<vector, pointMesh>& iF
        ) const
        {
            return autoPtr<pointPatchField<vector> >
            (
                new uncoupledSixDoFRigidBodyDisplacementPointPatchVectorField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const PointPatchFieldMapper&
            );

            //- Reverse map the given pointPatchField onto this pointPatchField
            virtual void rmap
            (
                const PointPatchField
                    <pointPatchField, pointMesh, pointPatch, DummyMatrix, vector>&,
                const labelList&
            );


        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
