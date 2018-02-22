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

Class
    Foam::OFstream

Description
    Output to file stream.

SourceFiles
    OFstream.C

\*---------------------------------------------------------------------------*/

#include "OFstream.H"
#include "gzstream.h"
#include "regIOobject.H"
#include "Time.H"
#include "H5Cpp.h"
#include "unistd.h"
#include <mpi.h>


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
  defineTypeNameAndDebug(OFstream, 0);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


Foam::OFstreamAllocator::OFstreamAllocator
(
 const fileName& pathname,
 IOstream::compressionType compression
 )
  :
  ofPtr_(NULL)
{
  if (pathname.empty())
    {
      if (OFstream::debug)
        {
	  InfoInFunction << "Cannot open null file " << endl;
        }
    }
  
  else if (pathname == "zippo")
    {
      ofPtr_ = new ofstream("/dev/null");
    }

  else
    {
      if (compression == IOstream::COMPRESSED)
	{
	  // get identically named uncompressed version out of the way
	  if (isFile(pathname, false))
	    {
	      rm(pathname);
	    }

	  ofPtr_ = new ogzstream((pathname + ".gz").c_str());
	}
      else
	{
	  // get identically named compressed version out of the way
	  if (isFile(pathname + ".gz", false))
	    {
	      rm(pathname + ".gz");
	    }

	  ofPtr_ = new ofstream(pathname.c_str());
	}
    }
}


Foam::OFstreamAllocator::~OFstreamAllocator()
{
  delete ofPtr_;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


//-----------------------------------------------
// 1. OpeFOAM - standard allocation
//-----------------------------------------------

Foam::OFstream::OFstream
(
 const fileName& pathname,
 streamFormat format,
 versionNumber version,
 compressionType compression
 )
  :
  OFstreamAllocator(pathname, compression),
  OSstream(*ofPtr_, pathname, format, version, compression),
  pathname_(pathname)
{

  setClosed();
  setState(ofPtr_->rdstate());

  if (!good())
    {
      if (debug)
        {
	  InfoInFunction
	    << "Could not open file " << pathname
	    << "for input\n"
	    "in stream " << info() << Foam::endl;
        }

      setBad();
    }
  else
    {
      setOpened();
      useOFstream_ = true;
    }

  lineNumber_ = 1;
}

//-----------------------------------------------
// 2. Stef - parallel code
//-----------------------------------------------

Foam::OFstream::OFstream
(
 int h5Request,
 const fileName& pathname,
 streamFormat format,
 versionNumber version,
 compressionType compression
 )
  :
  OFstreamAllocator("zippo", compression),
  OSstream(*ofPtr_, "/dev/null", format, version, compression),
  pathname_(pathname)
{

  h5Pathname_ = pathname;

  setClosed();
  setState(ofPtr_->rdstate());

  if (!good())
    {
      if (debug)
        {
	  InfoInFunction
	    << "Could not open file " << pathname
	    << "for input\n"
	    "in stream " << info() << Foam::endl;
        }

      setBad();
    }
  else
    {
      setOpened();
      useOFstream_ = false;
      if (pathname == "/dev/null")
	{
	}
      else
	{
	  produceFile_ = true;
	  nchar_= 300000000;
	  h5Buffer_ = new char[nchar_];
	}
    }
  lineNumber_ = 1;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::OFstream::~OFstream()
{

  //-------------------------------------------------------------
  // Standard OF files
  //-------------------------------------------------------------
  
  if (!produceFile_)
    {
    }
  
  //-------------------------------------------------------------
  // HDF5 Parallel files
  //-------------------------------------------------------------
  
  else
    {

      // Set MPI stuff
        
      int mpiRank, mpiSize, mpiReturn;     
      MPI_Status MPIst;
      mpiReturn = MPI_Comm_rank (MPI_COMM_WORLD, &mpiRank);
      mpiReturn = MPI_Comm_size (MPI_COMM_WORLD, &mpiSize);
      
      // Check if bunches are used and sets up synchronization

      int bunchSize = mpiSize;
      char * bunchSizeTemp = std::getenv ("OF_OUTPUT_H5_BUNCHSIZE");
      if (bunchSizeTemp !=NULL )
	{	  
	  bunchSize = atoi(bunchSizeTemp);
	  if (bunchSize <= 0 || bunchSize>=mpiSize)
	    { bunchSize = mpiSize; }
	}
            
      int firstBunch = bunchSize * (mpiRank/bunchSize);
      int lastBunch = ((firstBunch+bunchSize < mpiSize)?
		       firstBunch+bunchSize-1 : mpiSize-1);
      int bunchSizeLocal = lastBunch-firstBunch+1;
      int source = firstBunch + ((bunchSizeLocal+mpiRank-firstBunch-1) % bunchSizeLocal); 
      int dest = firstBunch + ((mpiRank-firstBunch+1) % bunchSizeLocal);
      
      // HDF5 Groups & Dataset Names

      H5std_string datasetName_;
      int pCSize = h5Pathname_.components().size();
      std::vector <string> groups_(10);
      int groupsSize_;
      //groups_.resize(10);
      int kp = -1, kk = 0;
      string ds;
      for (int jp = 0; jp<pCSize ; jp++)
	{
	  if (h5Pathname_.component(jp).find("processor") != string::npos)
	    {
	      kp = jp;
	      groups_[0] = "/"+h5Pathname_.component(jp);
	      kk++;
	    }
	  else if (kp>=0 && jp!=kp+1)
	    {
	      if (jp < pCSize - 1)
		{
		  groups_[kk] = groups_[kk-1] + "/" + h5Pathname_.component(jp);
		  kk++;
		}
	      else { ds = groups_[kk-1]+"/"+h5Pathname_.component(jp); }
	    }
	}
      groupsSize_ = kk;
      groups_.resize(groupsSize_);
      datasetName_ = ds;
  
      // HDF5 File Name (using bunches if required)
  
      H5File * ofHdf5File_;
      H5std_string ofHdf5FileName_;
      fileName fnlocal;
      if (kp >= 0)
	{
	  fileName *fnlocalt;
	  fnlocalt = new fileName(h5Pathname_.component(kp+1));
	  fnlocalt->toAbsolute();
	  fnlocal = *fnlocalt;
	  delete fnlocalt;
	}
      else
	fnlocal = h5Pathname_;
  
      if (bunchSize >= mpiSize)
	{
	  ofHdf5FileName_ = fnlocal + ".h5";
	}
      else
	{
	  char dd1[7], dd2[7];
	  sprintf (dd1,"%d",firstBunch);
	  sprintf (dd2,"%d",lastBunch);
	  ofHdf5FileName_ = fnlocal + "_pp" + dd1 + "-" + dd2 + ".h5";
	}
      
      //Synchronization part 1 (see below)
 

      int stok[1], rtok[1];
      int stag = mpiRank, rtag = source;
      stok[0] = 117;;
      
      if (mpiRank >= lastBunch)
	{
	  mpiReturn = MPI_Send (stok, 1, MPI_INT, dest, stag, MPI_COMM_WORLD);
	  mpiReturn = MPI_Recv (rtok, 1, MPI_INT, source, rtag, MPI_COMM_WORLD, &MPIst);
	}  
      else
	{ mpiReturn = MPI_Recv (rtok, 1, MPI_INT, source, rtag, MPI_COMM_WORLD, &MPIst); }
      
      Exception::dontPrint();
      
      try
	{ ofHdf5File_ = new H5File(ofHdf5FileName_, H5F_ACC_RDWR); }
      catch (...)
	{ ofHdf5File_ = new H5File(ofHdf5FileName_, H5F_ACC_EXCL); }
	  
      // Create the group if required

      std::vector <Group *> ofGroup;
      ofGroup.resize(groupsSize_);
      for (int jg=0; jg<groupsSize_; jg++)
	{
	  try
	    { ofGroup[jg] = new Group (ofHdf5File_->createGroup(groups_[jg])); }
	  catch (...)
	    { ofGroup[jg] = new Group (ofHdf5File_->openGroup(groups_[jg])); }
	}

      hsize_t dims[1];
      int h5Buffer_Size, h5Buffer_BoundarySize;
      if (boundary_)
	{
	   h5Buffer_Size = bzcounting_;
	   h5Buffer_BoundarySize = strlen(h5Buffer_)-bzcounting_;
	}
      else
	{
	   h5Buffer_Size = strlen(h5Buffer_);
	   h5Buffer_BoundarySize = 0;
	}
      dims[0] = h5Buffer_Size+1;
      char cht = h5Buffer_[h5Buffer_Size];
      h5Buffer_[h5Buffer_Size] = '\0';
      
      DataSpace * dataspace ;
      DataSet * dataset_=NULL;
      dataspace = new DataSpace(1, dims);
      try
	{
	  dataset_ = new DataSet (ofHdf5File_->createDataSet(datasetName_, PredType::C_S1, *dataspace));
	}
      catch (...)
	{
	  dataset_ = new DataSet (ofHdf5File_->openDataSet(datasetName_));
	}
      
      dataset_->write (h5Buffer_, PredType::C_S1);


      
      {
	/*
	 * Get dataspace of the dataset.
	 */
	DataSpace dataspace1 = dataset_->getSpace();
	/*
	 * Get the number of dimensions in the dataspace.
	 */
	int rank1 = dataspace1.getSimpleExtentNdims();
	/*
	 * Get the dimension size of each dimension in the dataspace and
	 * display them.
	 */
	hsize_t dims1[1];
	int ndims = dataspace1.getSimpleExtentDims( dims1, NULL);
	/*
	 * Define hyperslab in the dataset; implicitly giving strike and
	 * block NULL.
	 */
	hsize_t      offset[1];   // hyperslab offset in the file
	hsize_t      count[1];    // size of the hyperslab in the file
	offset[0] = 0;
	count[0]  = 20;
	dataspace1.selectHyperslab( H5S_SELECT_SET, count, offset );
	/*
	 * Define the memory dataspace.
	 */
	hsize_t     dimsm[1];              /* memory space dimensions */
	dimsm[0] = 20;
	DataSpace memspace( rank1, dimsm );
	/*
	 * Define memory hyperslab.
	 */
	hsize_t      offset_out[1];   // hyperslab offset in memory
	hsize_t      count_out[1];    // size of the hyperslab in memory
	offset_out[0] = 0;
	count_out[0]  = count[0];
	memspace.selectHyperslab( H5S_SELECT_SET, count_out, offset_out );
	/*
	 * Read data from hyperslab in the file into the hyperslab in
	 * memory and display the data.
	 */
	  int ncharm = dimsm[0];
	  char * data_out;
	  data_out = new char[nchar_];
	dataset_->read( data_out, PredType::C_S1, memspace, dataspace1 );
	//	offset_out[0] = offset[0];
	//count_out[0]  = count[0];
	
      }

      
      
      // write to boundary dataset
      
	  DataSet * datasetb_=NULL;

      if (boundary_)
	{
	  H5std_string datasetNameb_ = datasetName_ + "_b";
	  dims[0] = h5Buffer_BoundarySize+1;
	  h5Buffer_[h5Buffer_Size] = '\0';
          h5Buffer_[h5Buffer_Size] = cht;
	  
	  dataspace = new DataSpace(1, dims);
	  try
	    { datasetb_ = new DataSet (ofHdf5File_->createDataSet(datasetNameb_,
								  PredType::C_S1, *dataspace)); }
	  catch (...)
	    { datasetb_ = new DataSet (ofHdf5File_->openDataSet(datasetNameb_)); }
	  datasetb_->write (&h5Buffer_[h5Buffer_Size], PredType::C_S1);

	  delete datasetb_;
	}
      delete dataset_;
      for (int jg=groupsSize_-1; jg>=0; jg--)
	{ delete ofGroup[jg]; }
      delete [] h5Buffer_;
      
      ofHdf5File_->close();

      //Synchronization part 2 (see above)
      
      if (mpiRank < lastBunch)
	{ mpiReturn = MPI_Send (stok, 1, MPI_INT, dest, stag, MPI_COMM_WORLD); }
     
    }
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

std::ostream& Foam::OFstream::stdStream()
{
  if (!ofPtr_)
    {
      FatalErrorInFunction
	<< "No stream allocated." << abort(FatalError);
    }
  return *ofPtr_;
}


const std::ostream& Foam::OFstream::stdStream() const
{
  if (!ofPtr_)
    {
      FatalErrorInFunction
	<< "No stream allocated." << abort(FatalError);
    }
  return *ofPtr_;
}


void Foam::OFstream::print(Ostream& os) const
{
  os  << "    OFstream: ";
  OSstream::print(os);
}


// ************************************************************************* //
