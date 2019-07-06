/********************************************************************************************************************

                                                       main.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/HeightField/Test/main.cpp#6 $

	$NoKeywords: $

 ********************************************************************************************************************/


#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>

#include "../HeightField.h"
#include "../HeightFieldLoader.h"

#include "GlObjects/TerrainCamera/TerrainCamera.h"
#include "GlObjects/TextureLoader/TextureLoader.h"
#include "Glx/Glx.h"
#include "Misc/Etc.h"
#include "Math/Vector3.h"
#include "TgaFile/TgaFile.h"
#include "Wglx/Wglx.h"
#include "Wx/Wx.h"


#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cmath>


//#define CUSTOM_TERRAIN_MIPMAPS
//#define VIEWING_NORMALS

static float const	XY_SCALE			= 1.0f;
static float const	Z_SCALE				= 32.0f;

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static void InitializeRendering( HWND hWnd );
static void CleanUpRendering();
static void InitializeApp( HWND hWnd );
static void ShutDownApp();
static void Display();
static void Reshape( int w, int h );
static bool Update( HWND hWnd );
static void DrawTerrain();

#if defined( VIEWING_NORMALS )
static void DrawVertexNormals();
static void DrawFaceNormals();
#endif // defined( VIEWING_NORMALS )

static void DrawHud();

static Vector3 ComputeNormal( HeightField const & hf, int x, int y );
static void ComputeNormals( HeightField const & hf, Vector3 * pNormals );

static char						s_sAppName[]	 = "HeightField";
static char						s_sTitleBar[]	 = "HeightField";

static Glx::Lighting *			s_pLighting;
static Glx::DirectionalLight *	s_pDirectionalLight;

static HeightField *			s_pTerrain;
static Vector3 *				s_paTerrainNormals;
static Glx::Mesh *				s_pTerrainMesh;
static Glx::MipMappedTexture *	s_pTerrainTexture;
static Glx::Material *			s_pTerrainMaterial;

static WGlx::RenderingContext *	s_pRenderingContext;

#if defined( VIEWING_NORMALS )

static Glx::Mesh *				s_pVertexNormals;
static Glx::Mesh *				s_pFaceNormals;
static Glx::Material *			s_pVertexNormalsMaterial;
static Glx::Material *			s_pFaceNormalsMaterial;

#endif // defined( VIEWING_NORMALS )

static GlObjects::TerrainCamera *			s_pCamera;

static float					s_CameraSpeed	=	2.0f;

static WGlx::Font *				s_pFont;

/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow )
{
	if ( Wx::RegisterWindowClass( CS_OWNDC, ( WNDPROC )WindowProc, hInstance, s_sAppName ) == NULL )
	{
		MessageBox( NULL, "Wx::RegisterWindowClass() failed.", "Error", MB_OK );
		exit( 1 );
	}

	HWND hWnd = CreateWindowEx( 0,
								s_sAppName,
								s_sTitleBar,
								WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								0, 0, 640, 480,
								NULL,
								NULL,
								hInstance,
								NULL );

//	HWND hWnd = WGlx::CreateFullScreenWindow( 640, 480, 32, s_sAppName, s_sTitleBar, hInstance );

	if ( hWnd == NULL )
	{
		MessageBox( NULL, "CreateWindowEx() failed.", "Error", MB_OK );
		exit( 1 );
	}

	ShowWindow( hWnd, nCmdShow );

	return Wx::MessageLoop( hWnd, Update );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{ 
	switch( uMsg )
	{
	case WM_CREATE:
		OutputDebugString( "***WM_CREATE\n" );
		InitializeRendering( hWnd );
		InitializeApp( hWnd );
		return 0;

	case WM_SIZE:
		OutputDebugString( "***WM_SIZE\n" );
		Reshape( LOWORD( lParam ), HIWORD( lParam ) );
		return 0;


	case WM_CHAR:
	{
		switch ( wParam )
		{
		case VK_ESCAPE:	// Quit
			PostMessage( hWnd, WM_CLOSE, 0, 0 );
			break;

		case ' ':	// Forward
			s_pCamera->Move( -Vector3::ZAxis() * s_CameraSpeed );
			break;

		case 's':	// Backwards
			s_pCamera->Move( -Vector3::ZAxis() * -s_CameraSpeed );
			break;

		case 'd':	// Strafe right
			s_pCamera->Move( Vector3::XAxis() * s_CameraSpeed );
			break;

		case 'a':	// Strafe left
			s_pCamera->Move( Vector3::XAxis() * -s_CameraSpeed );
			break;

		case 'w':	// Strafe up
			s_pCamera->Move( Vector3( Vector3::ZAxis() ).Rotate( -s_pCamera->GetOrientation() ) * s_CameraSpeed );
			break;

		case 'x':	// Strafe down
			s_pCamera->Move( Vector3( Vector3::ZAxis() ).Rotate( -s_pCamera->GetOrientation() ) * -s_CameraSpeed );
			break;
		}
		return 0;
	}

	case WM_KEYDOWN:
	{
		switch ( wParam )
		{
		case VK_UP:
			s_pCamera->Pitch( 5.0f );
			break;

		case VK_DOWN:
			s_pCamera->Pitch( -5.0f );
			break;

		case VK_LEFT:
			s_pCamera->Yaw( 5.0f );
			break;

		case VK_RIGHT:
			s_pCamera->Yaw( -5.0f );
			break;

		case VK_PRIOR:
			s_pCamera->Roll( 5.0f );
			break;

		case VK_INSERT:
			s_pCamera->Roll( -5.0f );
			break;
		}
		return 0;
	}

	case WM_CLOSE:
		OutputDebugString( "***WM_CLOSE\n" );
		ShutDownApp();
		CleanUpRendering();
		DestroyWindow( hWnd );
		return 0;

	case WM_DESTROY:
		OutputDebugString( "***WM_DESTROY\n" );
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam ); 
} 


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void InitializeRendering( HWND hWnd )
{
	HDC const	hDC	= GetDC( hWnd );

	WGlx::SetPixelFormat( hDC, 0, true );

	s_pRenderingContext = new WGlx::RenderingContext( hDC );	// Current rendering context

	glClearColor( 0.65f, 0.80f, 0.90f, 1.0f );
	glEnable( GL_DEPTH_TEST );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	ReleaseDC( hWnd, hDC );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void CleanUpRendering()
{
	delete s_pRenderingContext;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void InitializeApp( HWND hWnd )
{
	Wx::DeviceContext const	dc( hWnd );

	// Create the height field

	s_pTerrain = HeightFieldLoader::Load( "hf.tga", XY_SCALE, Z_SCALE ).release();
	if ( !s_pTerrain )
	{
		MessageBox( NULL, "Unable to load terrain height field.", "HeightField", MB_OK );
		throw std::runtime_error( "Unable to load terrain height field." );
	}

	std::ostringstream	buffer;

	buffer << "Heightfield size - x: " << s_pTerrain->GetSizeI() << ", y: " << s_pTerrain->GetSizeJ() << std::endl << std::ends;
	OutputDebugString( buffer.str().c_str() );

	// Compute the normals

	s_paTerrainNormals = new Vector3[ s_pTerrain->GetSizeI() * s_pTerrain->GetSizeJ() ];
	if ( !s_pTerrain ) throw std::bad_alloc();

	ComputeNormals( *s_pTerrain, s_paTerrainNormals);

	s_pCamera	= new GlObjects::TerrainCamera( 60.0f, 1.0f, 1000.0f,
										Vector3( 0.0f, -s_pTerrain->GetSizeJ() * XY_SCALE * 0.5f, Z_SCALE ),
										0.0f, 90.0f, 0.0f );
	if ( !s_pCamera ) throw std::bad_alloc();

	s_pLighting			= new Glx::Lighting( Glx::Rgba( 0.6f, 0.6f, 0.7f ) );
	if ( !s_pLighting ) throw std::bad_alloc();
	s_pDirectionalLight	= new Glx::DirectionalLight( GL_LIGHT0, Vector3( 1.0f, -1.0f, 1.0f ).Normalize(), Glx::Rgba( 0.7f, 0.7f, 0.6f ) );
	if ( !s_pDirectionalLight ) throw std::bad_alloc();

#if defined( CUSTOM_TERRAIN_MIPMAPS )

	static char const * const files[] =
	{
		"64.tga",
		"32.tga",
		"16.tga",
		"8.tga",
		"4.tga",
		"2.tga",
//			"1.tga"
	};

	s_pTerrainTexture	= GlObjects::TextureLoader::LoadMipMapped( files, elementsof( files ) ).release();
	if ( s_pTerrainTexture == 0 ) throw std::runtime_error( "Unable to load terrain texture." );

	// Unable to generate 1x1 tga files, so just fake it here

	{
		unsigned char s_pTextureData[ 3 ] = { 103, 144, 152 };
		s_pTerrainTexture->AddMipMap( elementsof( files ), s_pTextureData );
	}

#else // defined( CUSTOM_TERRAIN_MIPMAPS )

	s_pTerrainTexture = GlObjects::TextureLoader::LoadMipMapped( "64.tga" ).release();
	if ( s_pTerrainTexture == 0 ) throw std::runtime_error( "Unable to load terrain texture." );

#endif // defined( CUSTOM_TERRAIN_MIPMAPS )

	s_pTerrainMaterial = new Glx::Material( s_pTerrainTexture/*,
												GL_MODULATE,
												Glx::Rgba::WHITE,
												Glx::Rgba::WHITE, 0.0f,
												Glx::Rgba::BLACK,
												GL_SMOOTH,
												GL_FRONT_AND_BACK*/ );
	if ( !s_pTerrainMaterial ) throw std::bad_alloc();

	s_pTerrainMesh = new Glx::Mesh( s_pTerrainMaterial );
	if ( !s_pTerrainMesh ) throw std::bad_alloc();

	s_pTerrainMesh->Begin();
	DrawTerrain();
	s_pTerrainMesh->End();

#if defined( VIEWING_NORMALS )

	s_pVertexNormalsMaterial = new Glx::Material( Glx::Rgba::BLACK,
												Glx::Rgba::BLACK, 0.0f,
												Glx::Rgba::RED/*,
												GL_FLAT,
												GL_FRONT_AND_BACK*/ );
	if ( !s_pVertexNormalsMaterial ) throw std::bad_alloc();

	s_pVertexNormals = new Glx::Mesh( s_pVertexNormalsMaterial );
	if ( !s_pVertexNormals ) throw std::bad_alloc();

	s_pVertexNormals->Begin();
	DrawVertexNormals();
	s_pVertexNormals->End();

	s_pFaceNormalsMaterial = new Glx::Material( Glx::Rgba::BLACK,
												Glx::Rgba::BLACK, 0.0f,
												Glx::Rgba::GREEN/*,
												GL_FLAT,
												GL_FRONT_AND_BACK*/ );
	if ( !s_pFaceNormalsMaterial ) throw std::bad_alloc();

	s_pFaceNormals = new Glx::Mesh( s_pFaceNormalsMaterial );
	if ( !s_pFaceNormals ) throw std::bad_alloc();

	s_pFaceNormals->Begin();
	DrawFaceNormals();
	s_pFaceNormals->End();

#endif // defined( VIEWING_NORMALS )

	s_pFont = new WGlx::Font( dc.GetDC() );
	if ( !s_pFont ) throw std::bad_alloc();
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void ShutDownApp()
{
	delete s_pFont;

#if defined( VIEWING_NORMALS )

	delete s_pFaceNormals;
	delete s_pFaceNormalsMaterial;
	delete s_pVertexNormals;
	delete s_pVertexNormalsMaterial;

#endif // defined( VIEWING_NORMALS )

	delete s_pTerrainMesh;
	delete s_pTerrainMaterial;
	delete s_pTerrainTexture;
	delete s_pDirectionalLight;
	delete s_pLighting;
	delete s_pCamera;
	delete[] s_paTerrainNormals;
	delete s_pTerrain;

}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static bool Update( HWND hWnd )
{
	Display();

	return true;	// Call me as often as possible
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void Display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();

	// Place the camera

	s_pCamera->Look();

	// Lights

	s_pDirectionalLight->Apply();

	// Draw the terrain

	s_pTerrainMesh->Apply();

#if defined( VIEWING_NORMALS )

	// Draw the vertex normals

	s_pVertexNormals->Apply();

	// Draw the face normals

	s_pFaceNormals->Apply();

#endif // defined( VIEWING_NORMALS )

	DrawHud();
	
	// Display the scene

	glFlush();
	SwapBuffers( wglGetCurrentDC() );

	// Report any errors

	Glx::ReportAnyErrors();
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void Reshape( int w, int h )
{
	glViewport( 0, 0, GLsizei( w ), GLsizei( h ) );
	s_pCamera->Reshape( w, h );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static int const			SKIP	= 1;

static void DrawTerrain()
{
	int const					sx		= s_pTerrain->GetSizeI();
	int const					sy		= s_pTerrain->GetSizeJ();
	HeightField::Vertex const *	pData	= s_pTerrain->GetData();
	float const					d		= s_pTerrain->GetScale();
	float const					x0		= -( sx - 1 ) * 0.5f * d;
	float const					y0		= -( sy - 1 ) * 0.5f * d;

	s_pTerrainMaterial->Apply();

	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer( GL_FLOAT, sizeof( *s_paTerrainNormals ), s_paTerrainNormals );

	for ( int i = 0; i < sy-SKIP; i += SKIP )
	{
		glBegin( GL_TRIANGLE_STRIP );

		for ( int j = 0; j < sx; j += SKIP )
		{
			float const	x	= x0 + j * d;
			float const	tx	= float( j );//float( j ) / float( sx - 1 );

			HeightField::Vertex const * const	pa	= &pData[ i * sx + j ];
			HeightField::Vertex const * const	pb	= pa + SKIP * sx;

			float const	yb	= y0 + ( i + SKIP ) * d;
			float const	tyb	= float( i + SKIP );//float( i + SKIP ) / float( sy - 1 );
			float const	zb	= pb->m_Z;

			glTexCoord2f( tx, tyb );
			glArrayElement( ( i + SKIP ) * sx + j );
//			glNormal3fv( pb->m_Normal.m_V );
			glVertex3f( x, yb, zb );

			float const	ya	= y0 + i * d;
			float const	tya	= float( i );//float( i ) / float( sy - 1 );
			float const	za	= pa->m_Z;

			glTexCoord2f( tx, tya );
			glArrayElement( i * sx + j );
//			glNormal3fv( pa->m_Normal.m_V );
			glVertex3f( x, ya, za );
		}

		glEnd();
	}

	glDisableClientState( GL_NORMAL_ARRAY );

}


#if defined( VIEWING_NORMALS )

/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void DrawVertexNormals()
{
	int const					sx		= s_pTerrain->GetSizeI();
	int const					sy		= s_pTerrain->GetSizeJ();
	HeightField::Vertex const *	pData	= s_pTerrain->GetData();
	float const					x0		= -( sx - 1 ) * 0.5f * XY_SCALE;
	float const					y0		= -( sy - 1 ) * 0.5f * XY_SCALE;

	glBegin( GL_LINES );

	for ( int i = 0; i < sy-SKIP; i += SKIP )
	{
		for ( int j = 0; j < sx; j += SKIP )
		{
			HeightField::Vertex const &	v	= pData[ i * sx + j ];
			float const					x	= x0 + j * XY_SCALE;
			float const					y	= y0 + i * XY_SCALE;
			float const					z	= v.m_Z;
			Vector3 const &				n	= s_paTerrainNormals[ i * sx + j ];

			glVertex3f( x, y, z );
			glVertex3f( x + n.m_X, y + n.m_Y, z + n.m_Z );
		}
	}

	glEnd();
}




/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void DrawFaceNormals()
{
	int const					sx		= s_pTerrain->GetSizeI();
	int const					sy		= s_pTerrain->GetSizeJ();
	float const					sd		= XY_SCALE;
	HeightField::Vertex const *	pData	= s_pTerrain->GetData();
	float const					x0		= -( sx - 1 ) * 0.5f * XY_SCALE;
	float const					y0		= -( sy - 1 ) * 0.5f * XY_SCALE;

	glBegin( GL_LINES );

	for ( int i = 0; i < sy-SKIP; i += SKIP )
	{
		for ( int j = 0; j < sx; j += SKIP )
		{
			HeightField::Vertex const * const	p	= &pData[ i * sx + j ];
			float const							x	= x0 + j * XY_SCALE;
			float const							y	= y0 + i * XY_SCALE;
			float const							z	= p->m_Z;

			Vector3 const		v( x, y, z );


			if ( j + 1 < sx && i + 1 < sy )
			{
				Vector3 const	fn	= Glx::ComputeFaceNormal( v,
															 Vector3( x + sd, y, p[ +1 ].m_Z ),
															 Vector3( x, y + sd, p[ +sx ].m_Z ) );
				glVertex3fv( v.m_V );
				glVertex3fv( ( v + fn ).m_V );
			}

			if ( i + 1 < sy && j - 1 >= 0 )
			{
				Vector3 const	fn	= Glx::ComputeFaceNormal( v,
															 Vector3( x, y + sd, p[ +sx ].m_Z ),
															 Vector3( x - sd, y, p[ -1 ].m_Z ) );
				glVertex3fv( v.m_V );
				glVertex3fv( ( v + fn ).m_V );
			}

			if ( j - 1 >= 0 && i - 1 >= 0 )
			{
				Vector3 const	fn	= Glx::ComputeFaceNormal( v,
															 Vector3( x - sd, y, p[ -1 ].m_Z ),
															 Vector3( x, y - sd, p[ -sx ].m_Z ) );
				glVertex3fv( v.m_V );
				glVertex3fv( ( v + fn ).m_V );
			}

			if ( i - 1 >= 0 && j + 1 < sx )
			{
				Vector3 const	fn	= Glx::ComputeFaceNormal( v,
															 Vector3( x, y - sd, p[ -sx ].m_Z ),
															 Vector3( x + sd, y, p[ +1 ].m_Z ) );
				glVertex3fv( v.m_V );
				glVertex3fv( ( v + fn ).m_V );
			}
		}
	}

	glEnd();
}


#endif // defined( VIEWING_NORMALS )



/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void ComputeNormals( HeightField const & hf, Vector3 * pNormals )
{
	for ( int y = 0; y < hf.GetSizeJ(); y++ )
	{
		for ( int x = 0; x < hf.GetSizeI(); x++ )
		{
			*pNormals = ComputeNormal( hf, x, y );
			++pNormals;
		}
	}
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static Vector3 ComputeNormal( HeightField const & hf, int x, int y )
{
	HeightField::Vertex const * const	pV	= hf.GetData( x, y );
	float const							s	= hf.GetScale();
	int const							sx	= hf.GetSizeI();
	int const							sy	= hf.GetSizeJ();

	float const							x0	= x * s;
	float const							y0	= y * s;
	Vector3 const						v0	( x0, y0, pV[ 0 ].m_Z );

	Vector3	normal	= Vector3::Origin();

	if ( x + 1 < sx && y + 1 < sy )
	{
		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0 + s, y0,     pV[  1      ].m_Z ),
										  Vector3( x0 + s, y0 + s, pV[  sx + 1 ].m_Z ) );

		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0 + s, y0 + s, pV[  sx + 1 ].m_Z ),
										  Vector3( x0,     y0 + s, pV[  sx     ].m_Z ) );
	}

	if ( x - 1 >= 0 && y + 1 < sy )
	{
		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0,     y0 + s, pV[  sx     ].m_Z ),
										  Vector3( x0 - s, y0,     pV[ -1      ].m_Z ) );

//		normal += Glx::ComputeFaceNormal( v0,
//										  Vector3( x0,     y0 + s, pV[  sx     ].m_Z ),
//										  Vector3( x0 - s, y0 + s, pV[  sx - 1 ].m_Z ) );
//
//		normal += Glx::ComputeFaceNormal( v0,
//										  Vector3( x0 - s, y0 + s, pV[  sx - 1 ].m_Z ),
//										  Vector3( x0 - s, y0,     pV[ -1      ].m_Z ) );
	}

	if ( x - 1 >= 0 && y - 1 >= 0 )
	{
		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0 - s, y0,     pV[ -1      ].m_Z ),
										  Vector3( x0 - s, y0 - s, pV[ -sx - 1 ].m_Z ) );

		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0 - s, y0 - s, pV[ -sx - 1 ].m_Z ),
										  Vector3( x0,     y0 - s, pV[ -sx     ].m_Z ) );
	}

	if ( x + 1 < sx && y - 1 >= 0 )
	{
		normal += Glx::ComputeFaceNormal( v0,
										  Vector3( x0,     y0 - s, pV[ -sx     ].m_Z ),
										  Vector3( x0 + s, y0,     pV[  1      ].m_Z ) );

//		normal += Glx::ComputeFaceNormal( v0,
//										  Vector3( x0,     y0 - s, pV[ -sx     ].m_Z ),
//										  Vector3( x0 + s, y0 - s, pV[ -sx + 1 ].m_Z ) );
//
//		normal += Glx::ComputeFaceNormal( v0,
//										  Vector3( x0 + s, y0 - s, pV[ -sx + 1 ].m_Z ),
//										  Vector3( x0 + s, y0,     pV[  1      ].m_Z ) );
	}

	return normal.Normalize();
}



/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

static void DrawHud()
{
	// Switch to ortho for 2d

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0., 1., 0., 1., 0., 1. );

	// Reset view

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glPushMatrix();

	// 2D stuff

	glColor3f( 1.0f, 1.0f, 1.0f );
	glRasterPos2f( 0.02f, 0.02f );
	s_pFont->DrawString( "Testing 1, 2, 3..." );
	
	// Restore view

	glPopMatrix();

	// Switch back to perspective

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );

}