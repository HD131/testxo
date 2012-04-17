#include "Text.h"

HRESULT CText::Init( IDirect3DDevice9* D3DDevice )
{

	void *pp;
	if ( !D3DDevice )
	{
		Log( "error init Text " );
		return E_FAIL;
	}
	m_pD3DDevice = D3DDevice;
	m_pVerBuf   = 0; // указатель на буфер вершин
	m_pIndexBuf = 0; // указатель на буфер вершин

	CVertexFVF Vershin[4];

	Vershin[0] = CVertexFVF(  1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f ); // 0
	Vershin[1] = CVertexFVF( -1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f ); // 1	
	Vershin[2] = CVertexFVF( -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f ); // 2		
	Vershin[3] = CVertexFVF(  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f ); // 3
	//						  X      Y     Z     tu    tv

	const unsigned short Index[] =
	{
		0,1,2,    2,3,0,		
	};

	if ( FAILED( m_pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBuf, 0 ) ) ) // создаём буфер вершин		
		return E_FAIL;
	if ( FAILED( m_pVerBuf->Lock( 0, 0, ( void** )&pp, 0 ) ) ) 
		Log( "error lock vertex buffer Text" );
	memcpy( pp, Vershin, sizeof( Vershin )  ); // копирование данных о вершинах в буфер вершин
	m_pVerBuf->Unlock(); // разблокирование	

	if ( FAILED( m_pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,	D3DPOOL_DEFAULT, &m_pIndexBuf, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pIndexBuf -> Lock( 0, sizeof( Index ), ( void** )&pp, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pp, Index, sizeof( Index ) ); // копирование данных о вершинах в буфер вершин
	m_pIndexBuf -> Unlock(); // разблокирование

	if ( FAILED( D3DXCreateTextureFromFile( m_pD3DDevice, "model\\Number.png", &m_Texture ) ) )
		Log( "error load number texture" );
	if ( FAILED( D3DXCreateTextureFromFile( m_pD3DDevice, "model\\Target.png", &m_TextureTarget ) ) )
		Log( "error load target texture" );

	Log( "Init Text " );
return S_OK;
}

void CText::RenderInt( float x, float y, float dist, int Number, int Value, CShader const& Shader )
{
	D3DXMATRIX   MatrixWorldTrans, MatrixWorldScal; 
	D3DVIEWPORT9 ViewPort;
	char str[20];
	// перевод числа в строку и добавление нулей перед числом
	itoa( Number, str, 10);
	int d = strlen(str);
	if ( ( Value != 0 ) && ( Value > d ) )
	{
		int t = Value;
		for ( int i = d; i > -1; --i )
			str[t--] = str[i];
		for ( int i = 0; i < Value - d; ++i )
			str[i] = '0';
	}

	m_pD3DDevice->GetViewport( &ViewPort );
	float Scale = 0.05f;
	D3DXMatrixScaling( &MatrixWorldScal, Scale, Scale * float(ViewPort.Width) / float(ViewPort.Height), Scale );	
	for ( int i = 0; i < strlen(str); ++i )
	{
		D3DXMatrixTranslation( &MatrixWorldTrans, x + i * dist, y, 0 );	
		Render( Shader, m_Texture, MatrixWorldScal * MatrixWorldTrans, int(str[i]) - 48 );
	}
}

void CText::RenderImage( CShader const& Shader, float Scale, const D3DXMATRIX&  MatrixWorldTrans )
{
	D3DXMATRIX  MatrixWorldScal; 	
	D3DVIEWPORT9 ViewPort;

	m_pD3DDevice->GetViewport( &ViewPort );
	D3DXMatrixScaling( &MatrixWorldScal, Scale, Scale *float(ViewPort.Width) / float(ViewPort.Height), Scale );	
	Render( Shader, m_TextureTarget, MatrixWorldScal * MatrixWorldTrans, 0 );

}

void CText::Render( CShader const& Shader, IDirect3DTexture9* Texture, const D3DXMATRIX&  MatrixWorld, int Num )
{
	if ( Shader.m_pConstTableVS )
	{		
		Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_world", &MatrixWorld );		
		Shader.m_pConstTablePS->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );
		Shader.m_pConstTableVS->SetInt(    m_pD3DDevice, "number", Num );
	}
	// устанавливаем шейдеры
	m_pD3DDevice->SetVertexShader( Shader.m_pVertexShader );
	m_pD3DDevice->SetPixelShader(  Shader.m_pPixelShader );
	// здесь перерисовка сцены	
	m_pD3DDevice -> SetStreamSource( 0, m_pVerBuf, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
	m_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
	m_pD3DDevice -> SetIndices( m_pIndexBuf );	
	m_pD3DDevice -> SetTexture( 0, Texture );	
	m_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2 );
}

void CText::Release()
{
	if ( m_Texture )
		m_Texture->Release();
	if ( m_TextureTarget )
		m_TextureTarget->Release();
	if ( m_pIndexBuf )
		m_pIndexBuf->Release();
	if ( m_pVerBuf )
		m_pVerBuf->Release();	
}

CText::~CText()
{
	Release();
}
