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

 \*---------------------------------------------------------------------------*/

#include "error.H"
#include "OSstream.H"
#include "token.H"
#include <iostream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


Foam::Ostream& Foam::OSstream::write(const token& t)
{
  if (t.type() == token::VERBATIMSTRING)
    {
      write(char(token::HASH));
      write(char(token::BEGIN_BLOCK));
      writeQuoted(t.stringToken(), false);
      write(char(token::HASH));
      write(char(token::END_BLOCK));
    }
  else if (t.type() == token::VARIABLE)
    {
      writeQuoted( t.stringToken(), false);
    }
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const char c)
{
  os_ << c;
  setState(os_.rdstate());
  if (c == token::NL)
    {
      lineNumber_++;
    }
  if (produceFile_)
    {
      stef_ << c;
      if (c == token::NL)
	{
	  string stefst = stef_.str();
	  hsize_t lstefst = stefst.length();
	  memcpy (h5Buffer_+zcounting_,stefst.c_str(),lstefst*sizeof(char));
	  zcounting_ = zcounting_+lstefst;
	  stef_.str ("");
	}
    }

  //setState(os_.rdstate());
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const char* str)
{
  lineNumber_ += string(str).count(token::NL);
  os_ << str;
  setState(os_.rdstate());
  if (produceFile_)
    {
      stef_ << str;
      string stefst = stef_.str();
      hsize_t lstefst = stefst.length();
      if (stefst[lstefst-1] == '\n')
	{
	  memcpy (h5Buffer_+zcounting_,stefst.c_str(),lstefst*sizeof(char));
	  zcounting_ = zcounting_+lstefst;
	  stef_.str ("");
	}
    }
  //setState(os_.rdstate());
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const word& str)
{
  os_ << str;
  setState(os_.rdstate());
  if (produceFile_)
    {
      stef_ << str;
      const string stefx = stef_.str();
      if (stefx.find("boundaryField") != string::npos)
	{
	  bzcounting_ = zcounting_;
	  boundary_ = true;
	}
    }
  //setState(os_.rdstate());
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const string& str)
{
  os_ << token::BEGIN_STRING;
  if (produceFile_)
    {
      stef_ << token::BEGIN_STRING;
    }
  int backslash = 0;
  for (string::const_iterator iter = str.begin(); iter != str.end(); ++iter)
    {
      char c = *iter;

      if (c == '\\')
        {
	  backslash++;
	  // suppress output until we know if other characters follow
	  continue;
        }
      else if (c == token::NL)
        {
	  lineNumber_++;
	  backslash++;    // backslash escape for newline
        }
      else if (c == token::END_STRING)
        {
	  backslash++;    // backslash escape for quote
        }

      // output pending backslashes
      
      while (backslash)
        {
	  os_ << '\\';
	  if (produceFile_)
	    {
	      stef_ <<  '\\';
	    }
	  backslash--;
        }
      os_ << c;
      if (produceFile_)
	{
      stef_ << c;
	}
    }

  // silently drop any trailing backslashes
  // they would otherwise appear like an escaped end-quote

  os_ << token::END_STRING;
      if (produceFile_)
	{
  stef_ << token::END_STRING;
	}

  setState(os_.rdstate());
  return *this;
}


Foam::Ostream& Foam::OSstream::writeQuoted
(
 const std::string& str,
 const bool quoted
 )
{
  if (quoted)
    {
      os_ << token::BEGIN_STRING;
      if (produceFile_)
	{
	  stef_ << token::BEGIN_STRING;
	}
	
      int backslash = 0;
      
      for
        (
	 string::const_iterator iter = str.begin();
	 iter != str.end();
	 ++iter
	 )
        {
	  char c = *iter;

	  if (c == '\\')
            {
	      backslash++;
	      // suppress output until we know if other characters follow
	      continue;
            }
	  else if (c == token::NL)
            {
	      lineNumber_++;
	      backslash++;    // backslash escape for newline
            }
	  else if (c == token::END_STRING)
            {
	      backslash++;    // backslash escape for quote
            }

	  // output pending backslashes
	  while (backslash)
	    {
	      os_ << '\\';
	      if (produceFile_)
		{
		  stef_ << '\\';
		}
	      backslash--;
	    }

	  os_ << c;
	  if (produceFile_)
	    {
	      stef_ << c;
	    }
        }

      // silently drop any trailing backslashes
      // they would otherwise appear like an escaped end-quote
      os_ << token::END_STRING;
      if (produceFile_)
	{
	  stef_ << token::END_STRING;
	}
    }
  else
    {
      // output unquoted string, only advance line number on newline
      lineNumber_ += string(str).count(token::NL);
        os_ << str;
      if (produceFile_)
	{
	  stef_ << str;
	}
    }

  setState(os_.rdstate());
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const int32_t val)
{
  if (produceFile_)
    {
      if (useOFstream_)
	{
	  os_ << val;
	  setState(os_.rdstate());	  
	}
      stef_ << val;
    }
  else
    {
      os_ << val;
      setState(os_.rdstate());
    }
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const int64_t val)
{
  if (produceFile_)
    {
      if (useOFstream_)
	{
	  os_ << val;
	  setState(os_.rdstate());	  
	}
      stef_ << val;
    }
  else
    {
      os_ << val;
      setState(os_.rdstate());
    }
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const floatScalar val)
{
  if (produceFile_)
    {
      if (useOFstream_)
	{
	  os_ << val;
	  setState(os_.rdstate());	  
	}
      stef_ << val;
    }
  else
    {
      os_ << val;
      setState(os_.rdstate());
    }
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const doubleScalar val)
{
  if (produceFile_)
    {
      if (useOFstream_)
	{
	  os_ << val;
	  setState(os_.rdstate());	  
	}
      stef_ << val;
    }
  else
    {
      os_ << val;
      setState(os_.rdstate());
    }
  return *this;
}


Foam::Ostream& Foam::OSstream::write(const char* buf, std::streamsize count)
{
  if (format() != BINARY)
    {
      FatalIOErrorInFunction(*this)
	<< "stream format not binary"
	<< abort(FatalIOError);
    }
  os_ << token::BEGIN_LIST;
  os_.write(buf, count);
  os_ << token::END_LIST;
  if (produceFile_)
    {
      stef_ << token::BEGIN_LIST;
      stef_.write(buf, count);
      stef_ << token::END_LIST;
    }

  setState(os_.rdstate());
  return *this;

}


void Foam::OSstream::indent()
{
  for (unsigned short i = 0; i < indentLevel_*indentSize_; i++)
    {
      os_ << ' ';
      if (produceFile_)
	{
	  stef_ << ' ';
	}
    }
}


void Foam::OSstream::flush()
{
  os_.flush();
}


void Foam::OSstream::endl()
{
  write('\n');
  os_.flush();
}


std::ios_base::fmtflags Foam::OSstream::flags() const
{
  return os_.flags();
}


std::ios_base::fmtflags Foam::OSstream::flags(const ios_base::fmtflags f)
{
  return os_.flags(f);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int Foam::OSstream::width() const
{
  return os_.width();
}


int Foam::OSstream::width(const int w)
{
  return os_.width(w);
}


int Foam::OSstream::precision() const
{
  return os_.precision();
}


int Foam::OSstream::precision(const int p)
{
  return os_.precision(p);
}


// ************************************************************************* //
