#include "Particle.h"
#include "D3DDevice.h"

void CParticle::Initial( IDirect3DDevice9* D3DDevice, FILE* FileLog )
{
	void *pBV;
	void *pBI;

	m_VertexBuffer = 0; // указатель на буфер вершин
	m_IndexBuffer  = 0; // указатель на буфер индексов	
	m_TextureExp   = 0; // указатель на текстуру
	m_D3DDevice    = D3DDevice;	
	m_Speed		   = 0.1f;
	m_LifeTime     = 1000.0f;
	m_Kill         = false;
	m_StartTime    = 0;
	srand( time(0) );
	m_Direct.x = rand() % 200 - 100;
	m_Direct.z = rand() % 200 - 100;
	m_Direct.y = 0;
	D3DXVec3Normalize( &m_Direct, &m_Direct );
	fprintf( FileLog, "x=%f y=%f z=%f\n", m_Direct.x, m_Direct.y, m_Direct.z );
	m_Position = D3DXVECTOR3(0,5,0);
	if ( m_D3DDevice )
	{
		CVertexFVF VershinExp[] =
		{			
			{ 1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // 0
			{-1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f}, // 1	
			{-1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // 2		
			{ 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f}, // 3
			// X        Y     Z    nx    ny    nz     tu    tv	
		};
		const unsigned short IndexExp[] =
		{
			0,1,2,    2,3,0,		
		};
		if ( FAILED( m_D3DDevice->CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_VertexBuffer, 0 ) ) )// создаём буфер вершин		
			fprintf( FileLog, "error create vertex buffer Explosion\n" );
		if ( FAILED( m_VertexBuffer->Lock( 0, sizeof( VershinExp ), ( void** )&pBV, 0 ) ) ) // Блокирование
			fprintf( FileLog, "error lock vertex buffer Explosion\n" );
		memcpy( pBV, VershinExp, sizeof( VershinExp ) ); // копирование данных о вершинах в буфер вершин
		m_VertexBuffer->Unlock(); // разблокирование

		if ( FAILED( m_D3DDevice->CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_IndexBuffer, 0 ) ) )  // создаём буфер вершин		
			fprintf( FileLog, "error create index buffer Explosion\n" );
		if ( FAILED( m_IndexBuffer -> Lock( 0, sizeof( IndexExp ), ( void** )&pBI, 0 ) ) ) // Блокирование
			fprintf( FileLog, "error lock index buffer Explosion\n" );
		memcpy( pBI, IndexExp, sizeof( IndexExp ) ); // копирование данных о вершинах в буфер вершин
		m_IndexBuffer->Unlock(); // разблокирование	

		std::string FileName = "model//Explosion.jpg";
		if ( FAILED( D3DXCreateTextureFromFile( m_D3DDevice, FileName.c_str(), &m_TextureExp )))	
			fprintf( FileLog, "error load texture '%s'\n",  FileName.c_str() );
	}	
	else
		fprintf( FileLog, "error: No init Explosion\n" );
	fprintf( FileLog, "Init Explosion\n" );
}

bool CParticle::Kill()
{
	if ( !m_StartTime )
		return false;
	if ( timeGetTime() > m_StartTime + m_LifeTime ) 
		return true;
	else false;
}

bool CParticle::Render(  CameraDevice const& Camera, CShader const& Shader )
{ 
	char        str[50];
	sprintf(str, "Start=%d   End=%d   Time=%d", m_StartTime, m_StartTime + m_LifeTime, timeGetTime());		
	//DrawMyText( m_D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	
	
	if ( m_StartTime )
	{		
		D3DXMATRIX MatrixWorld,MatZ;	
		float      m_Alpha = 1.0f;
		m_Position += m_Direct * m_Speed;
		D3DXMatrixRotationX( &MatZ, 1.57f );
		D3DXMatrixTranslation( &MatrixWorld, m_Position.x, m_Position.y, m_Position.z );
		MatrixWorld = MatZ * MatrixWorld;
		D3DXMATRIX wvp = MatrixWorld * Camera.m_View * Camera.m_Proj;
		if ( Shader.m_pConstTableVS )
		{		
			Shader.m_pConstTableVS->SetMatrix( m_D3DDevice, "mat_mvp",   &wvp );
			Shader.m_pConstTableVS->SetMatrix( m_D3DDevice, "mat_world", &MatrixWorld );
			Shader.m_pConstTableVS->SetVector( m_D3DDevice, "vec_light", &g_Light );
			Shader.m_pConstTablePS->SetFloat(  m_D3DDevice, "diffuse_intensity", g_Diffuse_intensity );	
			Shader.m_pConstTablePS->SetFloat(  m_D3DDevice, "Alpha", m_Alpha );	
		}
		// здесь перерисовка сцены	
		m_D3DDevice -> SetStreamSource(0, m_VertexBuffer, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
		m_D3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
		m_D3DDevice -> SetIndices( m_IndexBuffer );
		m_D3DDevice -> SetTexture( 0, m_TextureExp );
		// устанавливаем шейдеры
		m_D3DDevice -> SetVertexShader( Shader.m_pVertexShader );
		m_D3DDevice -> SetPixelShader(  Shader.m_pPixelShader  );
		// вывод примитивов
		m_D3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2 );

		if ( timeGetTime() > m_StartTime + m_LifeTime ) 
			return true;
	}
	
return false;
}

void CParticle::Release()
{
	if ( m_IndexBuffer )
		m_IndexBuffer->Release();
	if ( m_VertexBuffer )
		m_VertexBuffer->Release();
	if ( m_TextureExp )
		m_TextureExp->Release();
}
//----------------------------------------------

void CException::Init( IDirect3DDevice9* pD3DDevice, FILE* FileLog )
{
	m_Size = 15;
	m_Time = 0;
	D3DDevice = pD3DDevice;
	m_Part.resize( m_Size );	

	for( int i = 0; i < m_Part.size(); ++i )	
		m_Part[i].Initial( D3DDevice, FileLog );
	
}

void CException::RenderParticle( CameraDevice const& Camera, CShader const& Shader )
{
  	char        str[50];
  	sprintf(str, "%d", m_Part.size() );		
 	DrawMyText( D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	
	if ( m_Part.empty() )
		return;
	for( int i = 0; i < m_Part.size(); ++i )
	{
		
			m_Part[i].m_StartTime = m_Time;
			if ( m_Part[i].Render( Camera, Shader ) )
			{
				m_Part[i].Release();
				m_Part.erase( m_Part.begin() + i );
		    }		
	}
};