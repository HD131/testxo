#include "Init.h"
#include "Mesh.h"
#include "CameraDevice.h"


extern CD3DDevice        g_Direct3D;


HRESULT CMesh3D::InitialMesh( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{
	if ( !pD3DDevice )
	{
		Log( "Error pointer x-file" );
		return E_FAIL;
	}
	m_pD3DDevice = pD3DDevice;
	m_pMesh         = 0;
	m_pMeshMaterial = 0;
	m_pMeshTextura.resize(0);
	m_SizeFVF       = 0;
	m_Alpha         = 1.0f;	
	ID3DXBuffer *pMaterialBuffer  = 0;
	if ( FAILED( D3DXLoadMeshFromX( Name, D3DXMESH_SYSTEMMEM, m_pD3DDevice, 0, &pMaterialBuffer, 0, &m_TexturCount, &m_pMesh ) ) )
	{
		if ( m_pMesh == 0 )
		{		
			Log( "error load x-file" );
			return E_FAIL;
		}
	}
	Log( "Load x-file" );
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
			std::string FileName = std::string( "model\\" ) + std::string( MaterialMesh[i].pTextureFilename );
			if ( FAILED( D3DXCreateTextureFromFile( m_pD3DDevice, FileName.c_str(), &Tex )))
			{
				Log( "error load texture Mesh" );
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

void CMesh3D::RenderMesh( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader )
{
	D3DXMATRIX wvp;	
	
	if ( m_pMesh )
	{
		wvp = MatrixWorld * Camera.m_View * Camera.m_Proj;
		if ( Shader.m_pConstTableVS )
		{
			Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_mvp",   &wvp );
			Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_world", &MatrixWorld );
			Shader.m_pConstTableVS->SetVector( m_pD3DDevice, "vec_light", &g_Light );
			Shader.m_pConstTablePS->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );	
			Shader.m_pConstTablePS->SetFloat(  m_pD3DDevice, "Alpha", m_Alpha );	
		}
		// устанавливаем шейдеры
		m_pD3DDevice->SetVertexShader( Shader.m_pVertexShader );
		m_pD3DDevice->SetPixelShader(  Shader.m_pPixelShader );

		m_pD3DDevice->SetStreamSource( 0, m_VertexBuffer, 0, m_SizeFVF );
		m_pD3DDevice->SetIndices( m_IndexBuffer );
		for ( int i = 0; i < m_TexturCount; ++i )
		{
			m_pD3DDevice -> SetMaterial( &m_pMeshMaterial[i] );
			m_pD3DDevice -> SetTexture( 0, m_pMeshTextura[i] );
			//m_pMesh -> DrawSubset(i);
		}
		m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_pMesh->GetNumVertices(), 0, m_pMesh->GetNumFaces() ); 
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
	
	if ( m_pMesh )
	{
		m_pMesh -> Release();
		m_pMesh = 0;
	}
}