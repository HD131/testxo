#include "D3DDevice.h"
#include "Mesh.h"
#include "CameraDevice.h"


extern CShader   g_Shader;

HRESULT CMesh3D::InitialMesh( IDirect3DDevice9* D3DDevice, LPCSTR Name, FILE *FileLog )
{
	m_pMesh         = 0;
	m_pMeshMaterial = 0;
	m_pMeshTextura.resize(0);
	m_SizeFVF       = 0;
	m_Alpha         = 1.0f;	
	m_pD3DDevice    = D3DDevice;
	ID3DXBuffer *pMaterialBuffer  = 0;
	if ( FAILED( D3DXLoadMeshFromX( Name, D3DXMESH_SYSTEMMEM, m_pD3DDevice, 0, &pMaterialBuffer, 0, &m_TexturCount, &m_pMesh ) ) )
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
			if ( FAILED( D3DXCreateTextureFromFile( m_pD3DDevice, FileName.c_str(), &Tex )))
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
void CMesh3D::DrawMyMesh( ID3DXConstantTable**     pConstTableVS, ID3DXConstantTable**       pConstTablePS,
						 IDirect3DVertexShader9** VertexShader, IDirect3DPixelShader9**     PixelShader )
{
	D3DXMATRIX  wvp;
	if ( m_pMesh )
	{
		wvp = m_MatrixWorld * m_MatrixView * m_MatrixProjection;
		if ( pConstTableVS[Diffuse] )
		{
			pConstTableVS[Diffuse]->SetMatrix( m_pD3DDevice, "mat_mvp",   &wvp );
			pConstTableVS[Diffuse]->SetMatrix( m_pD3DDevice, "mat_world", &m_MatrixWorld );
			pConstTableVS[Diffuse]->SetVector( m_pD3DDevice, "vec_light", &g_Light );
			pConstTablePS[Diffuse]->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );	
			pConstTablePS[Diffuse]->SetFloat(  m_pD3DDevice, "Alpha", m_Alpha );	
		}
		// устанавливаем шейдеры
		m_pD3DDevice->SetVertexShader( VertexShader[Diffuse] );
		m_pD3DDevice->SetPixelShader(  PixelShader [Diffuse] );

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


void CMesh3D::RenderMesh( CameraDevice const& Camera, float x, float y, float Ang )
{
	D3DXMATRIX MatrixWorld, MatrixWorldY, MatrixWorldX;

	int t = ( MaxField - 1) / 2;
	D3DXMatrixRotationY(   &MatrixWorldY, Ang );
	D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
	D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);		
	SetMatrixWorld( MatrixWorld );
	SetMatrixView( Camera.m_View );
	SetMatrixProjection( Camera.m_Proj );
	DrawMyMesh(g_Shader.m_pConstTableVS, g_Shader.m_pConstTablePS, g_Shader.m_pVertexShader, g_Shader.m_pPixelShader);
}
