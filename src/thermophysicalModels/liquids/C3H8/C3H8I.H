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

inline Foam::scalar Foam::C3H8::rho(scalar p, scalar T) const
{
    return rho_.f(p, T);
}


inline Foam::scalar Foam::C3H8::pv(scalar p, scalar T) const
{
    return pv_.f(p, T);
}


inline Foam::scalar Foam::C3H8::hl(scalar p, scalar T) const
{
    return hl_.f(p, T);
}


inline Foam::scalar Foam::C3H8::cp(scalar p, scalar T) const
{
    return cp_.f(p, T);
}


inline Foam::scalar Foam::C3H8::h(scalar p, scalar T) const
{
    return h_.f(p, T);
}


inline Foam::scalar Foam::C3H8::cpg(scalar p, scalar T) const
{
    return cpg_.f(p, T);
}


inline Foam::scalar Foam::C3H8::B(scalar p, scalar T) const
{
    return B_.f(p, T);
}


inline Foam::scalar Foam::C3H8::mu(scalar p, scalar T) const
{
    return mu_.f(p, T);
}


inline Foam::scalar Foam::C3H8::mug(scalar p, scalar T) const
{
    return mug_.f(p, T);
}


inline Foam::scalar Foam::C3H8::K(scalar p, scalar T) const
{
    return K_.f(p, T);
}


inline Foam::scalar Foam::C3H8::Kg(scalar p, scalar T) const
{
    return Kg_.f(p, T);
}


inline Foam::scalar Foam::C3H8::sigma(scalar p, scalar T) const
{
    return sigma_.f(p, T);
}


inline Foam::scalar Foam::C3H8::D(scalar p, scalar T) const
{
    return D_.f(p, T);
}


inline Foam::scalar Foam::C3H8::D(scalar p, scalar T, scalar Wb) const
{
    return D_.f(p, T, Wb);
}


// ************************************************************************* //
