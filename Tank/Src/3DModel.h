#pragma once

#include "CameraDevice.h"
#include "D3D.h"
#include "Precompile.h"

enum ETypeTexture
{
	ID_AM  = 0,   // Ambient 
	ID_DI  = 1,   // Diffuse
	ID_SP  = 2,   // Specular
	ID_SH  = 3,   // Glossiness (Shininess in 3ds Max release 2.0 and earlier)
	ID_SS  = 4,   // Specular Level (Shininess strength in 3ds Max release 2.0 and earlier)
	ID_SI  = 5,   // Self-illumination
	ID_OP  = 6,   // Opacity
	ID_FI  = 7,   // Filter color
	ID_BU  = 8,   // Bump 
	ID_RL  = 9,   // Reflection
	ID_RR  = 10,  // Refraction 
	ID_DP  = 11,  // Displacement
	MAX_ID = 12,
};

struct MeshHeader
{
	DWORD flags[16];
};

// структура для хранения вертекса
// struct MyVertex
// {
// 	D3DXVECTOR3 Position;		// pos
// 	D3DXVECTOR3 Tangent;		// tangent
// 	D3DXVECTOR3 Binormal;		// binormal
// 	D3DXVECTOR3 Normal;			// normal
// 	D3DXVECTOR2 UV1;			// texcoords1
// 	D3DXVECTOR2 UV2;			// texcoords2
// 	DWORD color;				// vertex color
// };

struct MyVertex
{
	float x, y, z; // pos
	float tx, ty, tz; // tangent
	float bx, by, bz; // binormal
	float nx, ny, nz; // normal
	float tu1, tv1, tu2, tv2; // texcoords
	DWORD color; // vertex color
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
	DWORD							m_dwNumVertices;
	DWORD							m_dwNumIndices;
	DWORD							m_dwNumFaces;

	DWORD							m_MatID;
	std::vector< MyVertex >			m_Vertices;
	std::vector< WORD >				m_Indices;
	TEXTURE  						m_TexturesMesh[ MAX_ID ];
	IDirect3DVertexDeclaration9*    m_pVertexDeclaration;
	D3DXVECTOR4						m_Light;
};