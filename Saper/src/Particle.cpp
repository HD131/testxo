#include "Particle.h"
#include "D3DDevice.h"

CParticle::CParticle()
{
	m_Position = D3DXVECTOR3( 0, 0, 0 );
	m_Direct   = D3DXVECTOR3( 0, 0, 0 );
	m_Speed	   = 0.1f;
	m_Kill     = false;
}
CException::CException()
{
	m_Size = 1;
	m_VertexBuffer = 0; // указатель на буфер вершин
	m_IndexBuffer  = 0; // указатель на буфер индексов	
	m_TextureExp   = 0; // указатель на текстуру	
	m_LifeTime     = 1000.0f;	
	m_StartTime    = 0;
	m_Kill         = false;
	m_Particles.resize( m_Size );
	m_Vershin.resize( m_Size * 4 );
	m_Index.resize( m_Size * 6 );
}

void CException::Init( IDirect3DDevice9* D3DDevice, FILE* FileLog )
{
	if ( FileLog )
		m_FileLog = FileLog;
	else 
		return;
	if ( D3DDevice )
		m_D3DDevice = D3DDevice;
	else
	{
		fprintf( m_FileLog, "error: No init Explosion\n" );
		return;	
	}	
	// создаём буфер вершин
	if ( FAILED( m_D3DDevice->CreateVertexBuffer( m_Size * 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_VertexBuffer, 0 ) ) )		
		fprintf( m_FileLog, "error create vertex buffer Explosion\n" );
	// создаём буфер индексов
	if ( FAILED( m_D3DDevice->CreateIndexBuffer( m_Size * 6 * sizeof( short ), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_IndexBuffer, 0 ) ) )  		
		fprintf( m_FileLog, "error create index buffer Explosion\n" );

	for ( int i = 0; i < m_Size; ++i )
	{
		//------задаём случайное направление-----------------
		m_Particles[i].m_Direct = D3DXVECTOR3( rand()/float(RAND_MAX)*2-1, 0, rand()/float(RAND_MAX)*2-1 );		
		D3DXVec3Normalize( &m_Particles[i].m_Direct, &m_Particles[i].m_Direct );
		fprintf( m_FileLog, "Dir   x=%f y=%f z=%f\n", m_Particles[i].m_Direct.x, m_Particles[i].m_Direct.y, m_Particles[i].m_Direct.z );
	}

	std::string FileName = "model//Explosion.jpg";
	if ( FAILED( D3DXCreateTextureFromFile( m_D3DDevice, FileName.c_str(), &m_TextureExp )))	
		fprintf( m_FileLog, "error load texture '%s'\n",  FileName.c_str() );	
	
	fprintf( m_FileLog, "Init Explosion\n" );
}

void CException::Update( CameraDevice const& Camera )
{
	void *pBV;
	void *pBI;

	for ( int i = 0; i < m_Size; ++i )
	{
		fprintf( m_FileLog, "Camera.CameraUp   x=%f y=%f z=%f\n", Camera.CameraUp.x, Camera.CameraUp.y, Camera.CameraUp.z );
		fprintf( m_FileLog, "Camera.DirX       x=%f y=%f z=%f\n", Camera.DirX.x, Camera.DirX.y, Camera.DirX.z );
		//-----перемещаем-------------------
		//m_Particles[i].m_Position += m_Particles[i].m_Direct * m_Particles[i].m_Speed;

		//-----заполняем данными о вершине-----
		D3DXVECTOR3 Vec, Ver, Norm;	
		//D3DXVec3Cross( &Norm, &Camera.DirX, &Camera.CameraUp );
		Norm = D3DXVECTOR3( 0, 1, 0 );
		D3DXVec3Normalize( &Norm, &Norm );
		fprintf( m_FileLog, "Norm   x=%f y=%f z=%f\n", Norm.x, Norm.y, Norm.z );

		Vec = Camera.DirX - Camera.CameraUp;
		D3DXVec3Normalize( &Vec, &Vec );
		Ver = m_Particles[i].m_Position + Vec * sqrtf(2);		
		m_Vershin[i*4+0] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 1.0f, 1.0f ); // 0
		fprintf( m_FileLog, "Ver0   x=%f y=%f z=%f\n", Ver.x, Ver.y, Ver.z );

		Vec = -Camera.DirX - Camera.CameraUp;
		D3DXVec3Normalize( &Vec, &Vec );
		Ver = m_Particles[i].m_Position + Vec * sqrtf(2);
		m_Vershin[i*4+1] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 0.0f, 1.0f ); //1
		fprintf( m_FileLog, "Ver1   x=%f y=%f z=%f\n", Ver.x, Ver.y, Ver.z );

		Vec = -Camera.DirX + Camera.CameraUp;
		D3DXVec3Normalize( &Vec, &Vec );
		Ver = m_Particles[i].m_Position + Vec * sqrtf(2);
		m_Vershin[i*4+2] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 0.0f, 0.0f ); //2
		fprintf( m_FileLog, "Ver2   x=%f y=%f z=%f\n", Ver.x, Ver.y, Ver.z );

		Vec = Camera.DirX + Camera.CameraUp;
		D3DXVec3Normalize( &Vec, &Vec );
		Ver = m_Particles[i].m_Position + Vec * sqrtf(2);
		m_Vershin[i*4+3] = CVertexFVF( Ver.x, Ver.y, Ver.z, Norm.x,  Norm.y, Norm.z, 1.0f, 0.0f ); //3	
		fprintf( m_FileLog, "Ver3   x=%f y=%f z=%f\n", Ver.x, Ver.y, Ver.z );

		// заполнение буфера индексов		
		m_Index[i*6+0] = i * 4;
		m_Index[i*6+1] = i * 4 + 1;
		m_Index[i*6+2] = i * 4 + 2;
		m_Index[i*6+3] = i * 4 + 2;
		m_Index[i*6+4] = i * 4 + 3;
		m_Index[i*6+5] = i * 4;		
	}
	//-----Блокирование буфера вершин------------
	if ( FAILED( m_VertexBuffer->Lock( 0, sizeof( m_Vershin ), ( void** )&pBV, 0 ) ) ) 
		fprintf( m_FileLog, "error lock vertex buffer Explosion\n" );
	memcpy( pBV, &m_Vershin, sizeof( m_Vershin ) ); // копирование данных о вершинах в буфер вершин
	m_VertexBuffer->Unlock(); // разблокирование	

	// Блокирование буфера индексов
	if ( FAILED( m_IndexBuffer -> Lock( 0, sizeof( m_Index ), ( void** )&pBI, 0 ) ) ) 
		fprintf( m_FileLog, "error lock index buffer Explosion\n" );
	memcpy( pBI, &m_Index, sizeof( m_Index ) ); // копирование данных о вершинах в буфер вершин
	m_IndexBuffer->Unlock();  // разблокирование	
}

bool CException::RenderParticle(  CameraDevice const& Camera, CShader const& Shader )
{ 
	char        str[50];
	sprintf(str, "Start=%d   End=%d   Time=%d", m_StartTime, m_StartTime + m_LifeTime, timeGetTime());		
	DrawMyText( m_D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));

	Update( Camera );
	if ( m_StartTime )
	{		
		D3DXMATRIX MatrixWorld,MatZ;	
		float      m_Alpha = 1.0f;
		
		D3DXMatrixRotationX( &MatrixWorld, 0.0f );
		//D3DXMatrixTranslation( &MatrixWorld, m_Position.x, m_Position.y, m_Position.z );
		//MatrixWorld = MatZ * MatrixWorld;
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
return false;
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


/*/----------------------------------------------

void CException::Init( IDirect3DDevice9* pD3DDevice, FILE* FileLog )
{
	D3DDevice = pD3DDevice;
	m_Particles.resize( m_Size );	

	for( int i = 0; i < m_Particles.size(); ++i )	
		m_Particles[i].Initial( D3DDevice, FileLog );	
}

void CException::RenderParticle( CameraDevice const& Camera, CShader const& Shader )
{
  	char        str[50];
  	sprintf(str, "%d", m_Particles.size() );		
 	DrawMyText( D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	
	if ( m_Particles.empty() )
		return;
	for( int i = 0; i < m_Particles.size(); ++i )
	{
		
			m_Particles[i].m_StartTime = m_Time;
			if ( m_Particles[i].Render( Camera, Shader ) )
			{
				m_Particles[i].Release();
				m_Particles.erase( m_Particles.begin() + i );
		    }		
	}
};*/