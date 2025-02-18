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
    Foam::correlationFunction

Description

SourceFiles
    correlationFunctionI.H
    correlationFunction.C
    correlationFunctionIO.C

\*---------------------------------------------------------------------------*/

#ifndef correlationFunction_H
#define correlationFunction_H

#include "bufferedAccumulator.H"
#include "dictionary.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
class correlationFunction;

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const correlationFunction<Type>&
);

/*---------------------------------------------------------------------------*\
                    Class correlationFunction Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class correlationFunction
:
    public bufferedAccumulator<scalar>
{
    // Private data

        const polyMesh& mesh_;

        Field< Field<Type> > tZeroBuffers_;

        scalar duration_;
        scalar sampleInterval_;
        scalar averagingInterval_;

        label sampleSteps_;


    // Private Member Functions

        void setTimesAndSizes(const label);

        //- Disallow default bitwise copy construct
        correlationFunction(const correlationFunction<Type>&);

        //- Disallow default bitwise assignment
        void operator=(const correlationFunction<Type>&);


public:

    //- Component type
    typedef typename pTraits<Type>::cmptType cmptType;


    // Static data members

        static const char* const typeName;


    // Constructors

        //- Construct from dictionary
        correlationFunction
        (
            const polyMesh&,
            const dictionary&,
            const label tZeroBufferSize
        );

        //- Construct from components
        correlationFunction
        (
            const polyMesh&,
            const label tZeroBufferSize,
            const scalar duration,
            const scalar sampleInterval,
            const scalar averagingInterval

        );


    // Destructor

        ~correlationFunction();


    // Member Functions

        void calculateCorrelationFunction(const Field<Type>&);

        void calculateCorrelationFunction(const Type&);

        scalar integral() const;

        bool writeAveraged(Ostream&) const;


    // Access

        inline const Field< Field<Type> >& tZeroBuffers() const;

        inline scalar duration() const;

        inline scalar sampleInterval() const;

        inline scalar averagingInterval() const;

        inline label sampleSteps() const;

        inline label measurandFieldSize() const;


    // IOstream Operators

        friend Ostream& operator<< <Type>
        (
            Ostream&,
            const correlationFunction<Type>&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "correlationFunctionI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "correlationFunction.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
