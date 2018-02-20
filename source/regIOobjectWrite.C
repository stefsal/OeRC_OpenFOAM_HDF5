/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2016 OpenFOAM Foundation
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
     or more details.
 
     You should have received a copy of the GNU General Public License
     along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.
 
 Version Information
     This version allows the use of HDF5.
     Written by:
     Stefano Salvini,
     Oxford e-Research Centre, Dept of Engineering Science,
     University of Oxford
     Date: 28 September 2017

  Description
      write function for regIOobjects

  \*---------------------------------------------------------------------------*/

#include "regIOobject.H"
//#include "UPstream.H"
#include "OSspecific.H"
#include "Time.H"
#include "OFstream.H"
#include "fileName.H"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool Foam::regIOobject::writeObject
(
 IOstream::streamFormat fmt,
 IOstream::versionNumber ver,
 IOstream::compressionType cmp
 ) const
{
  if (!good())
    {
      SeriousErrorInFunction
	<< "bad object " << name()
	<< endl;

      return false;
    }

  if (instance().empty())
    {
      SeriousErrorInFunction
	<< "instance undefined for object " << name()
	<< endl;

      return false;
    }
  if
    (
     instance() != time().timeName()
     && instance() != time().system()
     && instance() != time().caseSystem()
     && instance() != time().constant()
     && instance() != time().caseConstant()
     )
    {
      const_cast<regIOobject&>(*this).instance() = time().timeName();
    }

  //mkDir(path());
  
  time().controlDict();

    
  if (OFstream::debug)
    {
      InfoInFunction << "Writing file " << objectPath();
    }


  bool osGood = false;

  // Check whether HDF5 is requested.
  // It needs:
  // 1. MPI in use
  // 2. Appropriate choice of environment variable OF_OUTPUT_H5 >= 2

  int useMPI;
  MPI_Initialized (&useMPI);
  
  int h5Request;
  int h5Temp = 0;
  char * stefev = std::getenv ("OF_OUTPUT_H5");
  if (stefev !=NULL)
    { h5Temp = atoi( std::getenv ("OF_OUTPUT_H5")); }
  if (!useMPI || h5Temp <= 1)
    { h5Request = 0; }
  else
    { h5Request = 2; }
      
  //================================================================
  //================================================================

  OFstream * os;
      
  // Serial or not HDF5 here

  // First create & open the output stream
      
  switch (h5Request)
    {
    case 0:
      {
	mkDir(path());
	os = new OFstream (objectPath(), fmt, ver, cmp);
	break;
      }
    
    case 2:
      {
	os = new OFstream (h5Request,objectPath(), fmt, ver, cmp);
	break;
      }
    default:
      {
	os = new OFstream(objectPath(), fmt, ver, cmp);
	break;
      }
    }
      
  // Then write to  the output stream
      
  if (!os->good())
    {
      return false;
    }
	
  if (!writeHeader(*os))
    {
      return false;
    }

  if (!writeData(*os))
    {
      return false;
    }
	
  writeEndDivider(*os);

  osGood = os->good();

  if (OFstream::debug)
    {
      Info<< " .... written" << endl;
    }

  // Only update the lastModified_ time if this object is re-readable,
  // i.e. lastModified_ is already set
  if (watchIndex_ != -1)
    {
      time().setUnmodified(watchIndex_);
    }
  delete os;
}
//================================================================
 
bool Foam::regIOobject::write() const
{
  return writeObject
    (
     time().writeFormat(),
     IOstream::currentVersion,
     time().writeCompression()
     );
}

// ************************************************************************* //
