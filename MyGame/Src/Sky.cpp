#include "Sky.h"

HRESULT CSky::InitialSky( IDirect3DDevice9* D3DDevice )
{
	void *pBV;
	void *pBI;
	if ( !D3DDevice )
	{
		Log( "error init sky " );
		return E_FAIL;
	}
	m_pD3DDevice = D3DDevice;
	m_pVerBufSky   = 0; // указатель на буфер вершин
	m_pBufIndexSky = 0; // указатель на буфер вершин

	CVertexFVF SkyVershin[4];

	SkyVershin[0] = CVertexFVF(  1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f ); // 0
	SkyVershin[1] = CVertexFVF( -1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f ); // 1	
	SkyVershin[2] = CVertexFVF( -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f ); // 2		
	SkyVershin[3] = CVertexFVF(  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f ); // 3
	// X        Y     Z    nx    ny    nz     tu    tv

	const unsigned short SkyIndex[] =
	{
		0,1,2,    2,3,0,		
	};
	if ( FAILED( m_pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBufSky, 0 ) ) ) // создаём буфер вершин		
		return E_FAIL;
	if ( FAILED( m_pVerBufSky -> Lock( 0, sizeof( SkyVershin ), ( void** )&pBV, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBV, SkyVershin, sizeof( SkyVershin ) ); // копирование данных о вершинах в буфер вершин
	m_pVerBufSky -> Unlock(); // разблокирование

	if ( FAILED( m_pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,         // создаём буфер вершин
		D3DPOOL_DEFAULT, &m_pBufIndexSky, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pBufIndexSky -> Lock( 0, sizeof( SkyIndex ), ( void** )&pBI, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBI, SkyIndex, sizeof( SkyIndex ) ); // копирование данных о вершинах в буфер вершин
	m_pBufIndexSky -> Unlock(); // разблокирование	

	m_CubeTexture = 0;
	if ( FAILED( D3DXCreateCubeTextureFromFileEx( m_pD3DDevice, "model\\sky_cube_mipmap.dds", D3DX_DEFAULT, D3DX_FROM_FILE, 0, 
		D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, 0, 0, &m_CubeTexture )))
		Log( "error load sky texture" );
	Log( "Init Sky " );
	return S_OK;
}

void CSky::RenderSky( CameraDevice const& Camera, CShader const& Shader )
{
	const D3DXVECTOR4 Scale( tan( D3DX_PI / 8 * (FLOAT)Height / Width), tan( D3DX_PI / 8 * (FLOAT)Height / Width  ), 1.0f, 1.0f );

	m_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, false );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ); 
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ); 
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP ); 
	D3DXMATRIX MatrixWorld;
	D3DXMatrixTranslation( &MatrixWorld, 1.0f, 1.0f, 1.0f );
	D3DXMATRIX tmp = MatrixWorld * Camera.m_View * Camera.m_Proj;
	if ( Shader.m_pConstTableVS )
	{
		Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_mvp",   &tmp );
		Shader.m_pConstTableVS->SetVector( m_pD3DDevice, "vec_light", &g_Light );
		Shader.m_pConstTableVS->SetVector( m_pD3DDevice, "scale",     &Scale );
		Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_view",  &Camera.m_View );
	}
	// здесь перерисовка сцены	
	m_pD3DDevice -> SetStreamSource(0, m_pVerBufSky, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
	m_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
	m_pD3DDevice -> SetIndices( m_pBufIndexSky );
	m_pD3DDevice -> SetTexture( 0, m_CubeTexture );
	// устанавливаем шейдеры
	m_pD3DDevice -> SetVertexShader( Shader.m_pVertexShader );
	m_pD3DDevice -> SetPixelShader(  Shader.m_pPixelShader  );
	// вывод примитивов
	m_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2 );

	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, true );
}

void CSky::Release()
{
	if ( m_pBufIndexSky )
		m_pBufIndexSky -> Release();
	if ( m_pVerBufSky )
		m_pVerBufSky -> Release();
	if ( m_CubeTexture )
		m_CubeTexture -> Release();
}
