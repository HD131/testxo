#include "3DModel.h"
#include "Help.h"
#include <iostream>
#include <fstream>
#include "ShaderManager.h"
#include "Class.h"

C3DModel::C3DModel():
	m_pVertexBuffer( 0 ),
	m_pIndexBuffer( 0 ),
	m_dwNumVertices( 0 ),
	m_dwNumIndices( 0 ),
	m_dwNumFaces( 0 ),
	m_MatID( 0 ),
	m_pVertexDeclaration( 0 )
{
	m_Light = D3DXVECTOR4( 100.f, 100.f, 0.f, 1.f );
	memset( m_TexturesMesh, 0, sizeof( m_TexturesMesh ) );
}

HRESULT C3DModel::Parse( const char * szFile )
{
	HRESULT hResult = E_FAIL;
	IDirect3DDevice9 * pD3DDevice = CD3DGraphic::GetDevice();
	FILE * file = 0;
	errno_t err = fopen_s( &file, szFile, "rb" );

	if( !err )
	{
#define READ( data )	fread( &data, sizeof( data ), 1, file );

		unsigned int subMeshesCount = 0;
		READ( subMeshesCount );
		READ( m_MatID );
		READ( m_dwNumVertices );
		READ( m_dwNumIndices );
		m_Indices.resize( m_dwNumIndices );
		m_Vertices.resize( m_dwNumVertices );

		fread( (char*)&m_Vertices[ 0 ], sizeof( MyVertex ) * m_dwNumVertices, 1, file );
		fread( (char*)&m_Indices[ 0 ], sizeof( WORD ) * m_dwNumIndices, 1, file );

		std::string FilePath( szFile );

		size_t f = FilePath.find_last_of( "\\" );
		if( f != -1 )
			FilePath.erase( f + 1, FilePath.size() );

		while( !feof( file ) )
		{
			int ID;
			unsigned int nLen;
			READ( ID );
			READ( nLen );
			char * s = new char[ nLen + 1 ];
			ZeroMemory( s, nLen + 1 );

			if( !feof( file ) )
				fread( s, sizeof( char ) * nLen, 1, file );
			else
				break;

			std::string szTexture( s );
			szTexture = FilePath + szTexture;
			TEXTURE pTex = LoadTexture( szTexture );
			m_TexturesMesh[ ID ] = pTex;
			delete[] s;
		}		

		fclose( file );
	}

		if( pD3DDevice )
		{
			void * pBV   = 0;
			void * pBI   = 0;
			DWORD  nSize = sizeof( MyVertex ) * m_dwNumVertices;			

			// создаём буфер вершин
			if( SUCCEEDED( pD3DDevice->CreateVertexBuffer( nSize, 0, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, 0 ) ) )		
			{
				if( SUCCEEDED( m_pVertexBuffer->Lock( 0, nSize, &pBV, 0 ) ) ) // Блокирование
				{
					// копирование данных о вершинах в буфер вершин
					memcpy( pBV, &m_Vertices[ 0 ], nSize ); 
					m_pVertexBuffer->Unlock(); // разблокирование
				}

				nSize = sizeof( WORD ) * m_dwNumIndices;

				// создаём буфер индексов
				if( SUCCEEDED( pD3DDevice->CreateIndexBuffer( nSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIndexBuffer, 0 ) ) )
				{
					if( SUCCEEDED( m_pIndexBuffer->Lock( 0, nSize, &pBI, 0 ) ) ) // Блокирование
					{
						// копирование данных о индексах в буфер индексов
						memcpy( pBI, &m_Indices[ 0 ], nSize ); 
						m_pIndexBuffer->Unlock(); // разблокирование
					}
					
					hResult = S_OK;
					std::string Error = "Create model " + std::string( szFile );
					Log( Error.c_str() );
					m_dwNumFaces = m_Indices.size() / 3;
				}
				else
				{
					std::string Error = "Error create index buffer for model " + std::string( szFile );
					Log( Error.c_str() );
				}
			}
			else
			{
				std::string Error = "Error create vertex buffer for model " + std::string( szFile );
				Log( Error.c_str() );
			}			
		}
// 		
// #undef READ
//	}
		
	return hResult;
}

void C3DModel::RenderMesh( CameraDevice * pCamera, const D3DXMATRIX & MatrixWorld, const CShader * pShader )
{
	m_Light.z -= 0.03f;

	if( m_Light.z < -100.f )
		m_Light.z = 100.f;
	
	IDirect3DDevice9 * pD3DDevice = CD3DGraphic::GetDevice();

	static const D3DVERTEXELEMENT9 pVertexElemMesh[] =
	{
		//  Stream  Offset         Type                   Method                 Usage          Usage Index       
		{     0,      0,     D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0      },
		{     0,     12,     D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,      0      },
		{     0,     24,     D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,     0      },
		{     0,     36,     D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,       0      },
		{     0,     48,     D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     0      },
		{     0,     56,     D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     1      },
		{     0,     64,     D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,        0      },
		D3DDECL_END()
	};

	if( pCamera && pShader )
	{
		const D3DXMATRIX & vp = pCamera->GetMatrixViewProject();
		D3DXMATRIX        wvp = MatrixWorld * vp;

		if( pShader->m_pConstTableVS )
		{
			pShader->m_pConstTableVS->SetMatrix( pD3DDevice, "mat_mvp",				&wvp );
			pShader->m_pConstTableVS->SetMatrix( pD3DDevice, "mat_world",			&MatrixWorld );
			pShader->m_pConstTableVS->SetVector( pD3DDevice, "vLightPos",			&m_Light );
			D3DXVECTOR3 v = pCamera->GetPosition();
			D3DXVECTOR4 vPos( v.x, v.y, v.z, 1.f );
			pShader->m_pConstTableVS->SetVector( pD3DDevice, "vViewPos",			&vPos );
		}

		if( pShader->m_pConstTablePS )
		{
			D3DXVECTOR4 m_vOffetUV( 0.f, 0.f, 0.f, 0.f );
			pShader->m_pConstTablePS->SetFloat(  pD3DDevice, "diffuse_intensity",	g_Diffuse_intensity );	
			//pShader->m_pConstTablePS->SetFloat(  m_pD3DDevice, "Alpha",				m_Alpha );
			pShader->m_pConstTablePS->SetVector( pD3DDevice, "OffsetUV",			&m_vOffetUV );
		}

		// устанавливаем шейдеры
		pD3DDevice->SetVertexShader( pShader->m_pVertexShader );
		pD3DDevice->SetPixelShader( pShader->m_pPixelShader );

		// вершины и индексы
		pD3DDevice->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof( MyVertex ) );
		pD3DDevice->SetIndices( m_pIndexBuffer );

		if( !m_pVertexDeclaration )
			pD3DDevice->CreateVertexDeclaration( pVertexElemMesh, &m_pVertexDeclaration );

		if( m_pVertexDeclaration )
			pD3DDevice->SetVertexDeclaration( m_pVertexDeclaration );

		for( unsigned int i = 0; i < MAX_ID; ++i )
		{
 			SetTexture( i, m_TexturesMesh[ i ] );				
 		}

		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_dwNumVertices, 0, m_dwNumFaces );		
	}
}

void C3DModel::Release()
{
	RELEASE_ONE( m_pVertexBuffer );
	RELEASE_ONE( m_pIndexBuffer );
	RELEASE_ONE( m_pVertexDeclaration );

	for( unsigned int i = 0; i < MAX_ID; ++i )
	{
		if( TEXTURE pTex = m_TexturesMesh[ i ] )
		{			
			FreeTexture( pTex );
		}
	}
	
	memset( m_TexturesMesh, 0, MAX_ID );
}

C3DModel::~C3DModel()
{
	Release();
}