/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "OSstream.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

inline Foam::OSstream::OSstream
(
    ostream& os,
    const string& name,
    streamFormat format,
    versionNumber version,
    compressionType compression
)
:
    Ostream(format, version, compression),
    name_(name),
    os_(os)
{
    if (os_.good())
    {
        setOpened();
        setGood();
        os_.precision(precision_);
	header_ = name.find("_h") != string::npos;
	values_ = name.find("_i") != string::npos;
	boundary_ = name.find("_b") != string::npos;
	usestr_ = header_ | values_ | boundary_;
	bvalues_ = false;
	zcounting_ = 0;
	zwrite_ = true;
    }
    else
    {
        setState(os_.rdstate());
    }
}
 
    // Destructor


inline Foam::OSstream::~OSstream ()
{}

// ************************************************************************* //

