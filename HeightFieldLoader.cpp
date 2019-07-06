/** @file *//********************************************************************************************************

                                                HeightFieldLoader.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/HeightField/HeightFieldLoader.cpp#8 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "HeightFieldLoader.h"

#include "HeightField.h"

#include "Misc/Types.h"
#include "TgaFile/TgaFile.h"

using namespace std;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! This function creates a HeightField using height data loaded from a TGA file. The size of the height field is
//! determined by the size of the image. The format of the image must be one byte per pixel (@c IMAGE_COLORMAPPED 
//! or @c IMAGE_GREYSCALE). The origin of the heightfield is the origin of the image (as specified by the pixel
//! order). The values in the image are multiplied by @a zScale / 255 to get the actual heights.
//!
//! @param	sFileName	The name of the TGA file containing the height data.
//! @param	zScale		Scale factor for height data
//!
//! @return		The address of the heightfield or 0 if the file could not be loaded
//!
//! @note	As a result of scaling, the heights will be in the range of 0 - @a zScale, inclusive.

auto_ptr<HeightField> HeightFieldLoader::LoadTga( char const * sFileName, float zScale )
{
	HeightField *	pHF	= 0;

	try
	{
		TgaFile		file( sFileName );

		// Make sure the image is 1 byte per pixel

		if ( file.m_ImageType == TgaFile::IMAGE_COLORMAPPED || file.m_ImageType == TgaFile::IMAGE_GREYSCALE )
		{
			int const	imageSize	= file.m_Width * file.m_Height;	// Number of elements

			// Load the image data.

			vector<uint8>	image( imageSize );

			if ( file.Read( &image[0], TgaFile::ORDER_BOTTOMLEFT ) )
			{
				// Create the height field

				pHF = new HeightField( file.m_Width, file.m_Height, zScale, &image[0] );
			}
		}
	}
	catch( ... )
	{
		// nothing to do. pHF is already 0.
	}

	return auto_ptr<HeightField>( pHF );
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! This function writes a HeightField to a TGA file. Only the size of the height field and the Z values are
//! saved. The value of each pixel is limited to the range [0, 255].
//!
//! @param	sFileName	Name of the file to write
//! @param	hf			HeightField to write
//! @param	zScale		Z scale factor. The value of each pixel is computed by z / zScale * 255.

bool HeightFieldLoader::WriteTga( char const * sFileName, HeightField const & hf, float zScale )
{
	(void)sFileName;
	(void)hf;
	(void)zScale;

	return false;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

ostream & operator <<( ostream & stream, HeightField const & hf )
{
	stream << hf.GetSizeI() << " " << hf.GetSizeJ() << endl;

	for ( int i = 0; i < hf.GetSizeI(); i++ )
	{
		for ( int j = 0; j < hf.GetSizeJ(); j++ )
		{
			stream << hf.GetData( j, i )->m_Z << " ";
		}
		stream << endl;
	}

	return stream;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @exception	bad_alloc	Unable to allocate a HeightField to contain the value

istream & operator >>( istream & stream, HeightField & hf )
{
	stream >> hf.m_sizeI >> hf.m_sizeJ;

	int const	n	= hf.m_sizeI * hf.m_sizeJ;

	hf.m_data.resize( n );

	for ( int k = 0; k < n; k++ )
	{
		stream >> hf.m_data[k].m_Z;
	}

	return stream;
}
