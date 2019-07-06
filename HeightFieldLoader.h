/** @file *//********************************************************************************************************

                                                 HeightFieldLoader.h

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/HeightField/HeightFieldLoader.h#6 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <memory>
#include <iosfwd>

class HeightField;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A class that creates a HeightField

class HeightFieldLoader
{
public:

	//! Creates a HeightField from a TGA file
	static std::auto_ptr< HeightField > LoadTga( char const * sFileName, float zScale );

	//! Writes a HeightField to a TGA file
	static bool WriteTga( char const * sFileName, HeightField const & hf, float zScale );
};


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! Inserts a HeightField into a stream
std::ostream & operator <<( std::ostream & stream, HeightField const & hf );

//! Extracts a HeightField from a stream
std::istream & operator >>( std::istream & stream, HeightField & hf );
