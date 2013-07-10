#include "3DModel.h"
#include "Help.h"

C3DModel::C3DModel():
	m_pVertexBuffer( 0 ),
	m_pIndexBuffer( 0 ),
	m_dwFVF( 0 ),
	m_dwNumVertices( 0 ),
	m_dwNumIndices( 0 ),
	m_dwNumFaces( 0 ),
	m_MatID( 0 )
{
}

HRESULT C3DModel::Parse( const char * szFile )
{
	HRESULT hResult = E_FAIL;
	
	if( FILE* file = fopen( szFile, "r" ) )
	{
		DWORD size;
		fseek( file, 0, SEEK_END);
		size = ftell( file );
		fseek( file, 0, SEEK_SET);
		std::vector< char > vec( size + 1 );		
		fread( &vec[0], size, sizeof( char ), file );
		DWORD nCounter = 0;

#define READ( data ) \
	if( nCounter < size - 1 ) \
	{ \
		memcpy( &data, &vec[ nCounter ], sizeof( data ) ); \
		nCounter += sizeof( data ); \
	} \

		MeshHeader meshHeader;
		ZeroMemory( &meshHeader, sizeof(MeshHeader) );
		READ( meshHeader );

		unsigned int subMeshesCount = 0;
		READ( subMeshesCount );		

		for( unsigned int i = 0; i < subMeshesCount; ++i )
		{			
			READ( m_MatID );
			READ( m_dwNumVertices );
			READ( m_dwNumIndices );
			m_Indices.resize( m_dwNumIndices );

			for( unsigned int v = 0; v < m_dwNumVertices; ++v )
			{
				MyVertex Vertex;
				READ( Vertex );
				m_Vertices.push_back( Vertex );
			}
			
			memcpy( &m_Indices[ 0 ], &vec[ nCounter ], sizeof(WORD) * m_dwNumIndices ); nCounter += sizeof(WORD) * m_dwNumIndices;

			while( nCounter < size - 1 )
			{
				int ID;
				unsigned int nLen;
				READ( ID );
				READ( nLen );
				char * s = new char[ nLen + 1 ];
				ZeroMemory( s, nLen + 1 );
				memcpy( s, &vec[ nCounter ], nLen );
				nCounter += nLen;
				m_Textures[ ID ] = s;
				delete[] s;
			}			
		}

		fclose( file );

		if( IDirect3DDevice9* pD3DDevice = CD3DGraphic::GetDevice() )
		{
			void * pBV   = 0;
			void * pBI   = 0;
			DWORD  nSize = sizeof( MyVertex ) * m_dwNumVertices;

			// создаём буфер вершин
			if( FAILED( pD3DDevice->CreateVertexBuffer( nSize, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, 0 ) ) )		
				return E_FAIL;			

			if( SUCCEEDED( m_pVertexBuffer->Lock( 0, nSize, &pBV, 0 ) ) ) // Блокирование
			{
				// копирование данных о вершинах в буфер вершин
				memcpy( pBV, &m_Vertices[ 0 ], nSize ); 
				m_pVertexBuffer->Unlock(); // разблокирование
			}

			nSize = sizeof( WORD ) * m_dwNumIndices;

			// создаём буфер индексов
			if ( FAILED( pD3DDevice->CreateIndexBuffer( nSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIndexBuffer, 0 ) ) )
				return E_FAIL;

			if( SUCCEEDED( m_pIndexBuffer->Lock( 0, nSize, &pBI, 0 ) ) ) // Блокирование
			{
				// копирование данных о индексах в буфер индексов
				memcpy( pBI, &m_Indices[ 0 ], nSize ); 
				m_pIndexBuffer->Unlock(); // разблокирование
			}
		}

		hResult = S_OK;
#undef READ
	}
	
	return hResult;
}

void C3DModel::RenderMesh( CameraDevice * pCamera, const D3DXMATRIX & MatrixWorld, const CShader * pShader )
{
	D3DXMATRIX wvp;
	IDirect3DDevice9* pD3DDevice = CD3DGraphic::GetDevice();

	if( pCamera && pShader )
	{
		D3DXMATRIX vp = pCamera->GetMatrixViewProject();
		wvp = MatrixWorld * vp;
		if ( pShader->m_pConstTableVS )
		{
			pShader->m_pConstTableVS->SetMatrix( pD3DDevice, "mat_mvp",				&wvp );
			pShader->m_pConstTableVS->SetMatrix( pD3DDevice, "mat_world",			&MatrixWorld );
			pShader->m_pConstTableVS->SetVector( pD3DDevice, "vec_light",			&g_Light );
			pShader->m_pConstTablePS->SetFloat(  pD3DDevice, "diffuse_intensity",	g_Diffuse_intensity );	
			//pShader->m_pConstTablePS->SetFloat(  m_pD3DDevice, "Alpha",				m_Alpha );
			//pShader->m_pConstTablePS->SetVector( m_pD3DDevice, "OffsetUV",			&m_vOffetUV );

			// устанавливаем шейдеры
			pD3DDevice->SetVertexShader( pShader->m_pVertexShader );
			pD3DDevice->SetPixelShader( pShader->m_pPixelShader );
			pD3DDevice->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof( MyVertex ) );
			pD3DDevice->SetIndices( m_pIndexBuffer );
			pD3DDevice->SetFVF( m_dwFVF );

// 			for( DWORD i = 0; i < m_TexturCount; ++i )
// 			{
// 				m_pD3DDevice->SetMaterial( &m_pMeshMaterial[i] );
// 				m_pD3DDevice->SetTexture( 0, m_pMeshTextura[i] );				
// 			}

			pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_dwNumVertices, 0, m_dwNumFaces );
		}
	}
}

void C3DModel::Release()
{
	RELEASE_ONE( m_pVertexBuffer );
	RELEASE_ONE( m_pIndexBuffer );
}

C3DModel::~C3DModel()
{
}