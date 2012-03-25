#include "D3DDevice.h"
#include "Mesh.h"
#include "CameraDevice.h"

extern IDirect3DDevice9* g_pD3DDevice;
extern CD3DDevice        g_Direct3D;
extern CShader           g_Diffuse;

HRESULT CMesh3D::InitialMesh(LPCSTR Name, FILE *FileLog )
{
	m_pMesh         = 0;
	m_pMeshMaterial = 0;
	m_pMeshTextura.resize(0);
	m_SizeFVF       = 0;
	m_Alpha         = 1.0f;	
	ID3DXBuffer *pMaterialBuffer  = 0;
	if ( FAILED( D3DXLoadMeshFromX( Name, D3DXMESH_SYSTEMMEM, g_pD3DDevice, 0, &pMaterialBuffer, 0, &m_TexturCount, &m_pMesh ) ) )
	{
		if ( m_pMesh == 0 )
		{		
			if ( FileLog ) 
				fprintf( FileLog, "error load x-file '%s'\n", Name );
			return E_FAIL;
		}
	}

	if ( m_pMesh->GetFVF() & D3DFVF_XYZ ) 
		m_SizeFVF += sizeof(float)*3;
	if ( m_pMesh->GetFVF() & D3DFVF_NORMAL ) 
		m_SizeFVF += sizeof(float)*3;
	if ( m_pMesh->GetFVF() & D3DFVF_TEX1 )
		m_SizeFVF += sizeof(float)*2;

	m_pMesh->GetVertexBuffer( &m_VertexBuffer );
	m_pMesh->GetIndexBuffer(  &m_IndexBuffer  );
	// Извлекаем свойства материала и названия{имена} структуры
	D3DXMATERIAL *MaterialMesh = (D3DXMATERIAL *)pMaterialBuffer->GetBufferPointer();
	m_pMeshMaterial  = new D3DMATERIAL9[m_TexturCount];

	for ( DWORD i = 0; i < m_TexturCount; i++ )
	{
		// Копируем материал
		m_pMeshMaterial[i] = MaterialMesh[i].MatD3D;
		// Установить окружающего свет
		m_pMeshMaterial[i].Ambient = m_pMeshMaterial[i].Diffuse;
		// Загружаем текстуру
		if( MaterialMesh[i].pTextureFilename != 0 )
		{		
			IDirect3DTexture9* Tex = 0;
			std::string FileName = std::string( "model//" ) + std::string( MaterialMesh[i].pTextureFilename );
			if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, FileName.c_str(), &Tex )))
			{
				fprintf( FileLog, "error load texture '%s'\n", MaterialMesh[i].pTextureFilename );
				m_pMeshTextura.push_back(0);
			}
			// Сохраняем загруженную текстуру
			m_pMeshTextura.push_back(Tex);
		}
		else 
		{
			// Нет текстуры для i-ой подгруппы
			m_pMeshTextura.push_back(0);
		}
	}
	// Уничтожаем буфер материала
	pMaterialBuffer->Release();

	return S_OK;
}

void CMesh3D::SetMatrixWorld( const D3DXMATRIX& Matrix )
{
	m_MatrixWorld = Matrix;
}

void CMesh3D::SetMatrixView( const D3DXMATRIX& Matrix )
{
	m_MatrixView = Matrix;
}

void CMesh3D::SetMatrixProjection( const D3DXMATRIX& Matrix )
{
	m_MatrixProjection = Matrix;
}

void CMesh3D::RenderMesh( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader )
{
	D3DXMATRIX wvp;

	
	
	if ( m_pMesh )
	{
		wvp = MatrixWorld * Camera.m_View * Camera.m_Proj;
		if ( Shader.m_pConstTableVS )
		{
			Shader.m_pConstTableVS->SetMatrix( g_pD3DDevice, "mat_mvp",   &wvp );
			Shader.m_pConstTableVS->SetMatrix( g_pD3DDevice, "mat_world", &MatrixWorld );
			Shader.m_pConstTableVS->SetVector( g_pD3DDevice, "vec_light", &g_Light );
			Shader.m_pConstTablePS->SetFloat(  g_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );	
			Shader.m_pConstTablePS->SetFloat(  g_pD3DDevice, "Alpha", m_Alpha );	
		}
		// устанавливаем шейдеры
		g_pD3DDevice->SetVertexShader( Shader.m_pVertexShader );
		g_pD3DDevice->SetPixelShader(  Shader.m_pPixelShader );

		g_pD3DDevice->SetStreamSource( 0, m_VertexBuffer, 0, m_SizeFVF );
		g_pD3DDevice->SetIndices( m_IndexBuffer );
		for ( int i = 0; i < m_TexturCount; ++i )
		{
			g_pD3DDevice -> SetMaterial( &m_pMeshMaterial[i] );
			g_pD3DDevice -> SetTexture( 0, m_pMeshTextura[i] );
			//m_pMesh -> DrawSubset(i);
		}
		g_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_pMesh->GetNumVertices(), 0, m_pMesh->GetNumFaces() ); 
	}
}

void CMesh3D::Release()
{
	if ( m_IndexBuffer )
		m_IndexBuffer->Release();
	if ( m_VertexBuffer )
		m_VertexBuffer->Release();

	if ( m_pMeshMaterial )
		delete[] m_pMeshMaterial;
	/*
	if ( m_pMeshTextura )
	{
	for ( int i = 1; i < m_TexturCount; ++i )
	{
	if ( m_pMeshTextura[i] )
	m_pMeshTextura[i] -> Release();
	}
	delete []m_pMeshTextura;
	}
	*/
	if ( m_pMesh )
		m_pMesh -> Release();
}