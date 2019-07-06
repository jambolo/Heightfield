/** @file *//********************************************************************************************************

                                                   HeightField.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/HeightField/HeightField.cpp#15 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "HeightField.h"


using namespace std;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! This constructor creates a heightfield given an array size, scale, and an array of heights.
//!
//! @param	sizeI	Size of the heightfield along the I axis.
//! @param	sizeJ	Size of the heightfield along the J axis.
//! @param	pData	Height data to be copied. If @a pData is 0 or omitted, no data is copied. The data should be in
//!					this order: <tt>pData[i][j]</tt>.

HeightField::HeightField( int sizeI /*= 0*/, int sizeJ /*= 0*/, float const * pData /*= 0*/ )
	: m_sizeI( sizeI ), m_sizeJ( sizeJ )
{
	if ( pData )
	{
		assert( sizeI > 0 && sizeJ > 0 );

		int const	n	= sizeI * sizeJ;	// Number of elements

		m_data.resize( n );

		// Load the height for each vertex

		for ( int k = 0; k < n; k++ )
		{
			m_data[k].m_Z = *pData++;
		}
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! This constructor creates a heightfield given an array size, scale, and an array of heights.
//!
//! @param	sizeI	Size of the heightfield along the I axis.
//! @param	sizeJ	Size of the heightfield along the J axis.
//! @param	data	Height data. The data should be in this order: <tt>qData[i][j]</tt>. The HeightField takes
//!					ownership of the data.

HeightField::HeightField( int sizeI, int sizeJ, std::vector<Vertex> & data )
	: m_sizeI( sizeI ), m_sizeJ( sizeJ )

{
	assert( sizeI > 0 && sizeJ > 0 );
	assert( size_t(sizeI * sizeJ) == data.size() );

	m_data.swap( data );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! This constructor creates a heightfield given an array size, scale, and an array of heights. The values in the
//! height data are multiplied by @a zScale / 255 to get the actual heights.
//!
//! @param	sizeI	Size of the heightfield along the I axis.
//! @param	sizeJ	Size of the heightfield along the J axis.
//! @param	zScale	Scale factor for height data
//! @param	pData	Height data to be copied. The data should be in this order: <tt>pData[i][j]</tt>.
//!
//! @exception	bad_alloc	Unable to allocate an array of vertexes.
//!
//! @note	As a result of scaling, the stored heights will be in the range of 0 - @a zScale, inclusive.

HeightField::HeightField( int sizeI, int sizeJ, float zScale, unsigned __int8 const * pData )
	: m_sizeI( sizeI ), m_sizeJ( sizeJ )
{
	assert( sizeI > 0 && sizeJ > 0 );
	assert( pData != 0 );

	int const	n	= sizeI * sizeJ;	// Number of elements

	m_data.resize( n );

	// Compute the height for each vertex

	float const	heightFactor	= zScale / 255.f;

	for ( int k = 0; k < n; k++ )
	{
		m_data[k].m_Z = float( *pData++ ) * heightFactor;
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

HeightField::~HeightField()
{
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j	J index
//! @param	i	I index
//! @param	sj	width of the area along the J axis
//! @param	si	width of the area along the I axis
//!
//! @return		Lowest Z value

float HeightField::GetMinZ( int j, int i, int sj, int si ) const
{
	float minZ	=	numeric_limits< float >::max();

	for ( int y = i; y < i + si; y++ )
	{
		for ( int x = j; x < j + sj; x++ )
		{
			float const		z	= GetZ( x, y );
			if ( z < minZ )
			{
				minZ = z;
			}
		}
	}

	return minZ;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j	J index
//! @param	i	I index
//! @param	sj	width of the area along the J axis
//! @param	si	width of the area along the I axis
//!
//! @return		Highest Z value

float HeightField::GetMaxZ( int j, int i, int sj, int si ) const
{
	float maxZ	=	-numeric_limits< float >::max();

	for ( int y = i; y < i + si; y++ )
	{
		for ( int x = j; x < j + sj; x++ )
		{
			float const		z	= GetZ( x, y );
			if ( z > maxZ )
			{
				maxZ = z;
			}
		}
	}

	return maxZ;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j		j index (can be a non-integer, but must be less than the width of the heightmap)
//! @param	i		i index (can be a non-integer, but must be less than the height of the heightmap)
//! @param	step	width and height of the quad to interpolate
//!
//! The function assumes that every quad in the grid is triangulated like this:
//!
//! @begincode
//! (a,b+step)   (a+step,b+step)
//! +-------------+
//! |           / |
//! |   *<----------- (j,i)
//! |       /     |
//! |     /       |
//! |   /     *<----- or (j,i)
//! | /           |
//! +-------------+
//! (a,b)        (a+step,b)
//! @endcode
//!
//! Where a and b are integers and multiples of step

float HeightField::GetInterpolatedZ( float j, float i, int step/* = 1*/ ) const
{

	assert( i >= 0.0f && i <= m_sizeI-1 );
	assert( j >= 0.0f && j <= m_sizeJ-1 );

	float		fj0;
	float		fi0;
	float const	dj0	= modff( j / step, &fj0 );
	float const	di0	= modff( i / step, &fi0 );

	int const 	j0	= (int)fj0 * step;
	int const 	i0	= (int)fi0 * step;

	float	z	= GetZ( j0, i0 );

	if ( dj0 > di0 )
	{
		if ( j0+step < m_sizeJ )
		{
			z += ( GetZ( j0+step, i0 ) - GetZ( j0, i0 ) ) * dj0;
			if ( i0+step < m_sizeI )
			{
				z += ( GetZ( j0+step, i0+step ) - GetZ( j0+step, i0 ) ) * di0;
			}
		}
	}
	else
	{
		if ( i0+step < m_sizeI )
		{
			z += ( GetZ( j0, i0+step ) - GetZ( j0, i0 ) ) * di0;
			if ( j0+step < m_sizeJ )
			{
				z += ( GetZ( j0+step, i0+step ) - GetZ( j0, i0+step ) ) * dj0;
			}
		}
	}

	return z;
}
