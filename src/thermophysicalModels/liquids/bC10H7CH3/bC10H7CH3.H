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
    Foam::bC10H7CH3

Description
    betaMethylNaphthalene

SourceFiles
    bC10H7CH3.C

\*---------------------------------------------------------------------------*/

#ifndef bC10H7CH3_H
#define bC10H7CH3_H

#include "liquid.H"
#include "NSRDSfunc0.H"
#include "NSRDSfunc1.H"
#include "NSRDSfunc2.H"
#include "NSRDSfunc3.H"
#include "NSRDSfunc4.H"
#include "NSRDSfunc5.H"
#include "NSRDSfunc6.H"
#include "NSRDSfunc7.H"
#include "NSRDSfunc14.H"
#include "APIdiffCoefFunc.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class bC10H7CH3 Declaration
\*---------------------------------------------------------------------------*/

class bC10H7CH3
:
    public liquid
{
    // Private data

        NSRDSfunc5 rho_;
        NSRDSfunc1 pv_;
        NSRDSfunc6 hl_;
        NSRDSfunc0 cp_;
        NSRDSfunc0 h_;
        NSRDSfunc7 cpg_;
        NSRDSfunc4 B_;
        NSRDSfunc1 mu_;
        NSRDSfunc2 mug_;
        NSRDSfunc0 K_;
        NSRDSfunc2 Kg_;
        NSRDSfunc6 sigma_;
        APIdiffCoefFunc D_;


public:

    //- Runtime type information
    TypeName("bC11H10");


    // Constructors

        //- Construct null
        bC10H7CH3();

        //- Construct from components
        bC10H7CH3
        (
            const liquid& l,
            const NSRDSfunc5& density,
            const NSRDSfunc1& vapourPressure,
            const NSRDSfunc6& heatOfVapourisation,
            const NSRDSfunc0& heatCapacity,
            const NSRDSfunc0& enthalpy,
            const NSRDSfunc7& idealGasHeatCapacity,
            const NSRDSfunc4& secondVirialCoeff,
            const NSRDSfunc1& dynamicViscosity,
            const NSRDSfunc2& vapourDynamicViscosity,
            const NSRDSfunc0& thermalConductivity,
            const NSRDSfunc2& vapourThermalConductivity,
            const NSRDSfunc6& surfaceTension,
            const APIdiffCoefFunc& vapourDiffussivity
        );

        //- Construct from Istream
        bC10H7CH3(Istream& is);


    // Member Functions

        //- Liquid density [kg/m^3]
        inline scalar rho(scalar p, scalar T) const;

        //- Vapour pressure [Pa]
        inline scalar pv(scalar p, scalar T) const;

        //- Heat of vapourisation [J/kg]
        inline scalar hl(scalar p, scalar T) const;

        //- Liquid heat capacity [J/(kg K)]
        inline scalar cp(scalar p, scalar T) const;

        //- Liquid Enthalpy [J/(kg)]
        inline scalar h(scalar p, scalar T) const;

        //- Ideal gas heat capacity [J/(kg K)]
        inline scalar cpg(scalar p, scalar T) const;

        //- Second Virial Coefficient [m^3/kg]
        inline scalar B(scalar p, scalar T) const;

        //- Liquid viscosity [Pa s]
        inline scalar mu(scalar p, scalar T) const;

        //- Vapour viscosity [Pa s]
        inline scalar mug(scalar p, scalar T) const;

        //- Liquid thermal conductivity [W/(m K)]
        inline scalar K(scalar p, scalar T) const;

        //- Vapour thermal conductivity [W/(m K)]
        inline scalar Kg(scalar p, scalar T) const;

        //- Surface tension [N/m]
        inline scalar sigma(scalar p, scalar T) const;

        //- Vapour diffussivity [m2/s]
        inline scalar D(scalar p, scalar T) const;

        //- Vapour diffussivity [m2/s] with specified binary pair
        inline scalar D(scalar p, scalar T, scalar Wb) const;


    // I-O

        //- Write the function coefficients
        void writeData(Ostream& os) const
        {
            liquid::writeData(os); os << nl;
            rho_.writeData(os); os << nl;
            pv_.writeData(os); os << nl;
            hl_.writeData(os); os << nl;
            cp_.writeData(os); os << nl;
            cpg_.writeData(os); os << nl;
            B_.writeData(os); os << nl;
            mu_.writeData(os); os << nl;
            mug_.writeData(os); os << nl;
            K_.writeData(os); os << nl;
            Kg_.writeData(os); os << nl;
            sigma_.writeData(os); os << nl;
            D_.writeData(os); os << endl;
        }

        //- Ostream Operator
        friend Ostream& operator<<(Ostream& os, const bC10H7CH3& l)
        {
            l.writeData(os);
            return os;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "bC10H7CH3I.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
