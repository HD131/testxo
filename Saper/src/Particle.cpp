#include "Particle.h"
#include "D3DDevice.h"

CParticle::CParticle()
{
	m_Position = D3DXVECTOR3( 0, 0, 0 );
	m_Direct   = D3DXVECTOR3( 0, 0, 0 );
	m_Speed	   = 0.01f;
	m_Kill     = false;
}
CException::CException()
{
	m_Size = 100;
	m_VertexBuffer = 0; // указатель на буфер вершин
	m_IndexBuffer  = 0; // указатель на буфер индексов	
	m_TextureExp   = 0; // указатель на текстуру	
	m_LifeTime     = 2000.0f;	
	m_StartTime    = 0;
	m_Kill         = false;	
	m_D3DDevice    = 0;
	m_Particles.resize( m_Size );
	m_Vershin.resize( m_Size * 4 );
	m_Index.resize( m_Size * 6 );
}

void CException::Init( IDirect3DDevice9* D3DDevice )
{		
	if ( D3DDevice )
		m_D3DDevice = D3DDevice;
	else
	{
		Log( "error: No init Explosion" );
		return;	
	}	
	// создаём буфер вершин
	if ( FAILED( m_D3DDevice->CreateVertexBuffer( m_Size * 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_VertexBuffer, 0 ) ) )		
		Log( "error create vertex buffer Explosion" );

	// создаём буфер индексов
	if ( FAILED( m_D3DDevice->CreateIndexBuffer( m_Size * 6 * sizeof( short ), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_IndexBuffer, 0 ) ) )  		
		Log( "error create index buffer Explosion" );

	for ( int i = 0; i < m_Size; ++i )
	{
		//------задаём случайное направление-----------------
		m_Particles[i].m_Direct = D3DXVECTOR3( rand()/float(RAND_MAX)*2-1, 0, rand()/float(RAND_MAX)*2-1 );		
		D3DXVec3Normalize( &m_Particles[i].m_Direct, &m_Particles[i].m_Direct );		
	}
	std::string FileName = "model//Explosion.tga";
	//------загружаем текстуру-----
	if ( FAILED( D3DXCreateTextureFromFile( m_D3DDevice, FileName.c_str(), &m_TextureExp )))	
		Log( "error load texture Exception" );	
	
	Log( "Init Explosion" );
}

void CException::Update( CameraDevice const& Camera )
{
	void *pBV;
	void *pBI;
	D3DXVECTOR3 Vec, Ver, Norm;	

	if ( !m_Particles.empty() )
	{
		for ( int i = 0; i < m_Particles.size(); ++i )
		{
			//-----перемещаем-------------------
			m_Particles[i].m_Position += m_Particles[i].m_Direct * m_Particles[i].m_Speed;

			//-----заполняем данными о вершине-----		
			D3DXVec3Cross( &Norm, &Camera.CameraUp, &Camera.DirX );			
			D3DXVec3Normalize( &Norm, &Norm );
			
			Ver = m_Particles[i].m_Position - Camera.CameraUp + Camera.DirX;		
			m_Vershin[i*4+0] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 1.0f, 1.0f ); // 0

			Ver = m_Particles[i].m_Position - Camera.CameraUp - Camera.DirX;
			m_Vershin[i*4+1] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 0.0f, 1.0f ); //1

			Ver = m_Particles[i].m_Position + Camera.CameraUp - Camera.DirX;
			m_Vershin[i*4+2] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 0.0f, 0.0f ); //2

			Ver = m_Particles[i].m_Position + Camera.CameraUp + Camera.DirX;
			m_Vershin[i*4+3] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 1.0f, 0.0f ); //3

			// заполнение буфера индексов		
			m_Index[i*6+0] = i * 4;
			m_Index[i*6+1] = i * 4 + 1;
			m_Index[i*6+2] = i * 4 + 2;
			m_Index[i*6+3] = i * 4 + 2;
			m_Index[i*6+4] = i * 4 + 3;
			m_Index[i*6+5] = i * 4;		
		}
		//-----Блокирование буфера вершин------------
		if ( FAILED( m_VertexBuffer->Lock( 0, 0, ( void** )&pBV, 0 ) ) ) 
			Log( "error lock vertex buffer Explosion" );
		memcpy( pBV, &m_Vershin[0], sizeof( CVertexFVF ) *  m_Vershin.size() ); // копирование данных о вершинах в буфер вершин
		m_VertexBuffer->Unlock(); // разблокирование	

		// Блокирование буфера индексов
		if ( FAILED( m_IndexBuffer -> Lock( 0, 0, ( void** )&pBI, 0 ) ) ) 
			Log( "error lock index buffer Explosion" );
		memcpy( pBI, &m_Index[0], sizeof( short ) *  m_Index.size() ); // копирование данных о вершинах в буфер вершин
		m_IndexBuffer->Unlock();  // разблокирование	
	}
}

void CException::RenderParticle(  CameraDevice const& Camera, CShader const& Shader )
{	
	char        str[50];
	sprintf(str, "x=%f  y=%f   z=%f", Camera.TargetDir.x, Camera.TargetDir.y, Camera.TargetDir.z );		
	//DrawMyText( m_D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));

	if ( m_StartTime && !m_Kill )
	{		
		D3DXMATRIX MatrixWorld;	

		float      m_Alpha = -1.0f;
		Update( Camera );
		D3DXMatrixTranslation( &MatrixWorld, 1.0f, 1.0f, 1.0f );		
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
		m_D3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6 * m_Size, 0, 2 * m_Size );

		if ( timeGetTime() > m_StartTime + m_LifeTime ) 
			m_Kill = true;		
	}	
}

void CException::SetTime( POINT Point, DWORD Time )
{ 
	m_StartTime = Time; 
	m_Pos = D3DXVECTOR3( Point.y - int( MaxField / 2 ), 0, Point.x - int( MaxField / 2 ) );
	for ( int i = 0; i < m_Particles.size(); ++i )
		m_Particles[i].m_Position = m_Pos;
	Beep(100,100);
}

void CException::Release()
{
	if ( m_IndexBuffer )
		m_IndexBuffer->Release();
	if ( m_VertexBuffer )
		m_VertexBuffer->Release();
	if ( m_TextureExp )
		m_TextureExp->Release();
}

