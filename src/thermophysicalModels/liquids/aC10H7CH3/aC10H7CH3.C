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

#include "aC10H7CH3.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(aC10H7CH3, 0);
    addToRunTimeSelectionTable(liquid, aC10H7CH3,);
    addToRunTimeSelectionTable(liquid, aC10H7CH3, Istream);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::aC10H7CH3::aC10H7CH3()
:
    liquid
    (
        142.2,
        772.04,
        3.66e+6,
        0.523,
        0.298,
        242.67,
        3.4929e-2,
        517.83,
        1.7012e-30,
        0.3478,
        2.0176e+4
    ),
    rho_(60.92559, 0.22408, 772.04, 0.25709),
    pv_(73.716, -9103.2, -7.2253, 2.062e-06, 2),
    hl_(772.04, 511744.022503516, 0.4164, 0, 0, 0),
    cp_(965.893108298172, 1.16216596343179, 0.00298523206751055, 0, 0, 0),
    h_
    (
        38161.6838138517,
        965.893108298172,
        0.581082981715893,
        0.00099507735583685,
        0,
        0
    ),
    cpg_(743.389592123769, 2703.5864978903, 1548.5, 2031.64556962025, 722.06),
    B_
    (
        0.00205555555555556,
       -3.34423347398031,
       -931153.305203938,
        1.87601969057665e+18,
       -2.06448663853727e+21
    ),
    mu_(-93.6, 5784, 12, 0, 0),
    mug_(2.5672e-06, 0.3566, 825.54, 0),
    K_(0.19758, -0.0001796, 0, 0, 0, 0),
    Kg_(0.3911, -0.1051, -213.52, 2318300),
    sigma_(772.04, 0.076, 1.33, 0, 0, 0),
    D_(147.18, 20.1, 142.2, 28) // note: Same as nHeptane
{}


Foam::aC10H7CH3::aC10H7CH3
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
)
:
    liquid(l),
    rho_(density),
    pv_(vapourPressure),
    hl_(heatOfVapourisation),
    cp_(heatCapacity),
    h_(enthalpy),
    cpg_(idealGasHeatCapacity),
    B_(secondVirialCoeff),
    mu_(dynamicViscosity),
    mug_(vapourDynamicViscosity),
    K_(thermalConductivity),
    Kg_(vapourThermalConductivity),
    sigma_(surfaceTension),
    D_(vapourDiffussivity)
{}


Foam::aC10H7CH3::aC10H7CH3(Istream& is)
:
    liquid(is),
    rho_(is),
    pv_(is),
    hl_(is),
    cp_(is),
    h_(is),
    cpg_(is),
    B_(is),
    mu_(is),
    mug_(is),
    K_(is),
    Kg_(is),
    sigma_(is),
    D_(is)
{}


// ************************************************************************* //
