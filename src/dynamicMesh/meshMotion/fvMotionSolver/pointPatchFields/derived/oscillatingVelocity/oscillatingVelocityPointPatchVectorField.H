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
    Foam::oscillatingVelocityPointPatchVectorField

Description
    Foam::oscillatingVelocityPointPatchVectorField

SourceFiles
    oscillatingVelocityPointPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef oscillatingVelocityPointPatchVectorField_H
#define oscillatingVelocityPointPatchVectorField_H

#include "fixedValuePointPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
        Class oscillatingVelocityPointPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class oscillatingVelocityPointPatchVectorField
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

        vector amplitude_;
        scalar omega_;

        pointField p0_;


public:

    //- Runtime type information
    TypeName("oscillatingVelocity");


    // Constructors

        //- Construct from patch and internal field
        oscillatingVelocityPointPatchVectorField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&
        );

        //- Construct from patch, internal field and dictionary
        oscillatingVelocityPointPatchVectorField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const dictionary&
        );

        //- Construct by mapping given patchField<vector> onto a new patch
        oscillatingVelocityPointPatchVectorField
        (
            const oscillatingVelocityPointPatchVectorField&,
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const PointPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual autoPtr<pointPatchField<vector> > clone() const
        {
            return autoPtr<pointPatchField<vector> >
            (
                new oscillatingVelocityPointPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        oscillatingVelocityPointPatchVectorField
        (
            const oscillatingVelocityPointPatchVectorField&,
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
                new oscillatingVelocityPointPatchVectorField(*this, iF)
            );
        }


    // Member functions

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            void autoMap
            (
                const PointPatchFieldMapper&
            );

            //- Reverse map the given pointPatchField onto this pointPatchField
            void rmap
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
