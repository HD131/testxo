#pragma once

#include "CameraDevice.h"
#include "D3D.h"
#include <vector>
#include <map>

#pragma warning( disable : 4996 )

#define ID_AM 0   //!< Ambient 
#define ID_DI 1   //!< Diffuse
#define ID_SP 2   //!< Specular
#define ID_SH 3   //!< Glossiness (Shininess in 3ds Max release 2.0 and earlier)
#define ID_SS 4   //!< Specular Level (Shininess strength in 3ds Max release 2.0 and earlier)
#define ID_SI 5   //!< Self-illumination
#define ID_OP 6   //!< Opacity
#define ID_FI 7   //!< Filter color
#define ID_BU 8   //!< Bump 
#define ID_RL 9   //!< Reflection
#define ID_RR 10  //!< Refraction 
#define ID_DP 11  //!< Displacement 

struct MeshHeader
{
	DWORD flags[16];
};

// структура для хранения вертекса
struct MyVertex
{
	float x, y, z;				// pos
	float tx, ty, tz;			// tangent
	float bx, by, bz;			// binormal
	float nx, ny, nz;			// normal
	float tu1, tv1, tu2, tv2;	// texcoords
	DWORD color;				// vertex color
};

struct CMesh
{
	DWORD							MatID;
	WORD							VertexCount;
	DWORD							IndexCount;
	std::vector< MyVertex >			Vertex;
	std::vector< WORD >				Indices;
	std::map< int, std::string >	Textures;
};

class C3DModel
{
public:
	C3DModel();
	~C3DModel();

	HRESULT	Parse( const char * szFile );
	void	RenderMesh( CameraDevice * pCamera, const D3DXMATRIX & MatrixWorld, const CShader * pShader );
	void	Release();	

private:
	IDirect3DVertexBuffer9*			m_pVertexBuffer;
	IDirect3DIndexBuffer9*			m_pIndexBuffer;
	DWORD							m_dwFVF;
	DWORD							m_dwNumVertices;
	DWORD							m_dwNumIndices;
	DWORD							m_dwNumFaces;

	DWORD							m_MatID;
	std::vector< MyVertex >			m_Vertices;
	std::vector< WORD >				m_Indices;
	std::map< int, std::string >	m_Textures;
};