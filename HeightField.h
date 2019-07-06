/** @file *//********************************************************************************************************

                                                    HeightField.h

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/HeightField/HeightField.h#14 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <Misc/Assert.h>
#include <vector>
#include <iosfwd>


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A height field stored as an array of height values.

class HeightField
{
	friend std::ostream & operator <<( std::ostream & stream, HeightField const & hf );
	friend std::istream & operator >>( std::istream & stream, HeightField & hf );
public:

	class Vertex;

	//! Constructor
	explicit HeightField( int SizeI = 0, int SizeJ = 0, float const * pData = 0 );

	//! Constructor
	HeightField( int SizeI, int SizeJ, std::vector<Vertex> & data );

	//! Constructor
	HeightField( int SizeI, int SizeJ, float zScale, unsigned __int8 const * pData );

	// Destructor
	virtual ~HeightField();

	//! Returns the size of the heightfield along the I axis.
	int GetSizeI() const;

	//! Returns the size of the heightfield along the J axis.
	int GetSizeJ() const;

	//! Returns a pointer to a particular element
	Vertex const * GetData( int j = 0, int i = 0 ) const;

	//! Returns a pointer to an element
	Vertex * GetData( int j = 0, int i = 0 );

	//! Returns an element 
	float GetZ( int j, int i ) const;

	//! Returns the lowest Z in the specified range
	float GetMinZ( int j, int i, int sj, int si ) const;

	//! Returns the lowest Z in the heightfield
	float GetMinZ() const;

	//! Returns the highest Z in the specified range
	float GetMaxZ( int j, int i, int sj, int si ) const;

	//! Returns the highest Z in the heightfield
	float GetMaxZ() const;

	//! Returns the interpolated Z at [ @a j, @a i ]
	float GetInterpolatedZ( float j, float i, int step = 1 ) const;

private:

	int					m_sizeI;	//!< Size of the vertex array in the I direction
	int					m_sizeJ;	//!< Size of the vertex array in the J direction
	std::vector<Vertex>	m_data;		//!< Vertex array
};


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! An element of a HeightField

class HeightField::Vertex
{
public:
	float		m_Z;	//!< The height of the vertex.
};

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @return		Number of elements along the I axis

inline int HeightField::GetSizeI() const
{
	return m_sizeI;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @return		Number of elements along the J axis

inline int HeightField::GetSizeJ() const
{
	return m_sizeJ;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j	J index
//! @param	i	I index
//!
//! @return		Pointer to const element at ( @a j, @a i )

inline HeightField::Vertex const * HeightField::GetData( int j/*= 0*/, int i/*= 0*/ ) const
{
	assert_limits( 0, j, m_sizeJ-1 );
	assert_limits( 0, i, m_sizeI-1 );
	return &m_data[ i * m_sizeJ + j ];
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j	J index
//! @param	i	I index
//!
//! @return		Pointer to element at ( @a j, @a i )

inline HeightField::Vertex * HeightField::GetData( int j/*= 0*/, int i/*= 0*/ )
{
	assert_limits( 0, j, m_sizeJ-1 );
	assert_limits( 0, i, m_sizeI-1 );
	return &m_data[ i * m_sizeJ + j ];
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	j	J index
//! @param	i	I index
//!
//! @return		Z value at ( @a j, @a i )

inline float HeightField::GetZ( int j, int i ) const
{
	return GetData( j, i )->m_Z;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

inline float HeightField::GetMinZ() const
{
	return GetMinZ( 0, 0, m_sizeJ, m_sizeI );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

inline float HeightField::GetMaxZ() const
{
	return GetMaxZ( 0, 0, m_sizeJ, m_sizeI );
}
