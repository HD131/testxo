#include "Init.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Mesh.h"
#include <vector>

extern IDirect3DDevice9* g_pD3DDevice;

struct CSky
{
	IDirect3DVertexBuffer9* m_pVerBufSky;
	IDirect3DIndexBuffer9*  m_pBufIndexSky;
	IDirect3DCubeTexture9*  m_CubeTexture;	
	IDirect3DDevice9*       m_pD3DDevice;
	HRESULT                 InitialSky( IDirect3DDevice9* D3DDevice );
	void                    RenderSky( CameraDevice const& Camera, CShader const& Shader );
	void                    Release();
};

struct CText
{
	struct CVertexPT
	{
		FLOAT x,   y,  z;
		FLOAT u, v;
		CVertexPT()
		{	}
		CVertexPT( float X, float Y, float Z, float U, float V ) : x(X), y(Y), z(Z), u(U), v(V)
		{	}
	};
	IDirect3DVertexBuffer9* m_pVerBuf;
	IDirect3DIndexBuffer9*  m_pIndexBuf;
	IDirect3DDevice9*       m_pD3DDevice;
	IDirect3DTexture9*      m_Texture;	
	HRESULT                 Init( IDirect3DDevice9* D3DDevice );
	void                    Render( CShader const& Shader, const D3DXMATRIX&  MatrixWorldTrans, int Num );
	void                    RenderInt( int Number, CShader const& Shader );
	void                    Release();
   ~CText();
};

CD3DDevice   g_Direct3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CText        g_Text;
CMesh3D      g_MeshA;
CMesh3D      g_Mesh[MaxMesh];
CMesh3D      g_MeshS;
CMesh3D		 g_MeshWin;
CMesh3D		 g_MeshLost;
CShader      g_Shader[MaxShader];
CameraDevice g_Camera;
bool         g_Exit      = false;
bool		 g_Wireframe = false;




POINT PickObject( CCell* Cell )
{
	POINT        Point;
	D3DVIEWPORT9 ViewPort;
	RECT         ClientRec;

	GetClientRect ( GetForegroundWindow(), &ClientRec);
	ClientToScreen( GetForegroundWindow(), (LPPOINT)&ClientRec);
	GetCursorPos( &Point );
	int x = Point.x - ClientRec.left;
	int y = Point.y - ClientRec.top;
	g_pD3DDevice->GetViewport( &ViewPort );

	float px = (  2.0f * x / ViewPort.Width  - 1.0f) / g_Camera.m_Proj._11;
	float py = ( -2.0f * y / ViewPort.Height + 1.0f) / g_Camera.m_Proj._22;	

	D3DXVECTOR3 Direction = D3DXVECTOR3( px, py, 1.0f );

	D3DXMATRIX MatV;
	D3DXMatrixInverse( &MatV, 0, &g_Camera.m_View ); 
	D3DXVECTOR3 PosView = D3DXVECTOR3( MatV._41, MatV._42, MatV._43 ); //   извлечь координаты камеры из матрицы вида	
	D3DXVec3TransformNormal( &Direction, &Direction, &MatV );
	D3DXVec3Normalize( &Direction, &Direction );
	//----------------------------Нахождение пересечение со сферами----------------------------------------
	POINT NumObject[MaxField*MaxField];
	int Count = -1;
	for ( int ArrY = 0; ArrY < MaxField; ++ArrY )
		for ( int ArrX = 0; ArrX < MaxField; ++ArrX )
		{	
			D3DXVECTOR3 v =  PosView - Cell[ArrX*MaxField+ArrY].m_Centr;
			float b = 2.0f * D3DXVec3Dot( &Direction, &v );
			float c = D3DXVec3Dot( &v, &v ) - Cell[ArrX*MaxField+ArrY].m_Radius * Cell[ArrX*MaxField+ArrY].m_Radius ;
			// Находим дискриминант
			float Discr = ( b * b ) - ( 4.0f * c );			
			if ( Discr >= 0.0f )
			{
				Discr = sqrtf(Discr);
				float s0 = ( -b + Discr ) / 2.0f;
				float s1 = ( -b - Discr ) / 2.0f;
				// Если есть решение >= 0, луч пересекает сферу
				if ( ( s0 >= 0.0f ) && ( s1 >= 0.0f ) )
				{
					++Count;
					NumObject[Count].x = ArrX;
					NumObject[Count].y = ArrY;					
				}
			}
		}
		float Dist = 100000.f;
		if ( Count < 0 )
		{
			NumObject[0].x = -1;
			NumObject[0].y = -1;
			return NumObject[0];
		}
		if ( Count == 0 )	
			return NumObject[0];
		if ( Count > 0 )
		{
			for ( int i = 0; i < Count; ++i)
			{
				D3DXVECTOR3 T = PosView - Cell[NumObject[i].x * MaxField + NumObject[i].y].m_Centr;
				float DistVec = D3DXVec3LengthSq( &T );
				if ( DistVec < Dist )
				{
					NumObject[0].x = NumObject[i].x;
					NumObject[0].y = NumObject[i].y;
				}
			}
		}	
		return NumObject[0];
}



void RenderingDirect3D( IDirect3DDevice9* D3DDevice )
{
	D3DXMATRIX MatrixWorld, MatrixWorldX, MatrixWorldY, MatrixWorldZ;

	if ( D3DDevice == 0 )
		return;
	//----------------------------------------------режим каркаса-------------------------------
	if ( g_Wireframe )
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float  Ang = timeGetTime() / 2000.0f;
	D3DXMATRIX  MatrixView       = g_Camera.m_View;
	D3DXMATRIX  MatrixProjection = g_Camera.m_Proj;	
	// очистка заднего буфера
	D3DDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );
	// начало рендеринга
	D3DDevice->BeginScene(); 
	//------------------------------------------Render Sky----------------------------------------
	g_Sky.RenderSky( g_Camera, g_Shader[Sky] );
	//-------------------------------------- 
	float sc = 0.01f;
	D3DXMatrixScaling( &MatrixWorld, sc, sc, sc );
	g_Mesh[Pers].RenderMesh( g_Camera, MatrixWorld, g_Shader[Diffuse] );
	//------------------------------------------Render Text----------------------------------------
	int a = timeGetTime() % 100000;
	g_Text.RenderInt( a, g_Shader[Text] );
	

	char        str[50];
	sprintf(str, "x=%f  y=%f   z=%f", g_Camera.DirX.x, g_Camera.DirX.y, g_Camera.DirX.z );		
	//DrawMyText( D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));

	D3DDevice -> EndScene();
	D3DDevice -> Present( 0, 0, 0, 0 ); // вывод содержимого заднего буфера в окно
}

LONG WINAPI WndProc( HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam )
{	
	switch ( Message )
	{
	case WM_CLOSE:
		g_Exit = true;
		break;
	case WM_KEYDOWN:
		if ( wparam == VK_ESCAPE )
			g_Exit = true;		
		if ( wparam == VK_F4 )
			g_Wireframe = !g_Wireframe;
		break;
	}
	return DefWindowProc( hwnd, Message, wparam, lparam );
}  

void Init( int& F, int& M )
{
	FILE *File = fopen( "Init.txt", "r" );

	while ( !feof( File ) )
	{
		fscanf( File,"%d",&F);
		fscanf( File,"%d",&M);
	}

	fclose(  File );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine, int       nCmdShow)
{	
	MSG			 Msg;
	WNDCLASS	 w;	

	remove( "log.txt" );
	Log("Begin");	
	//Init( MaxField, MaxMine );
	memset( &w, 0, sizeof(WNDCLASS) );
	w.style         = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc   = WndProc;
	w.hInstance     = hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	w.lpszClassName = "My Class";
	w.hIcon         = LoadIcon( 0, IDI_QUESTION );//стандартная иконка приложения Win API 	
	w.hCursor       = LoadCursor( 0, "aero_link_l.cur" );
	RegisterClass(&w);
	HWND hwnd = CreateWindow( "My Class", "MyGame", WS_SYSMENU | WS_MINIMIZEBOX,
		250, 150, Width, Height, 0, 0, hInstance, 0 );	
	ShowWindow( hwnd, nCmdShow );
	ZeroMemory( &Msg, sizeof( MSG ) );

	if ( SUCCEEDED( g_Direct3D.IntialDirect3D( hwnd ) ) )
	{		
		g_Sky.InitialSky( g_pD3DDevice );
		g_Text.Init( g_pD3DDevice );
		g_DeviceInput.InitialInput( hwnd );	
		g_Mesh[Pers].InitialMesh( "model\\M16.x" );
		
		g_Shader[  Sky  ].LoadShader( "shader\\Sky", g_pD3DDevice );
		g_Shader[Diffuse].LoadShader( "shader\\Diffuse", g_pD3DDevice );
		g_Shader[  Text ].LoadShader( "shader\\Text", g_pD3DDevice );

		while( !g_Exit )
		{
			g_DeviceInput.ScanInput( &g_Camera );				
			RenderingDirect3D( g_pD3DDevice );				
			if ( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &Msg );
				DispatchMessage(  &Msg );
			}
		}		
	}
	for (int i = 0; i < MaxShader; ++i)
		g_Shader[i].Release();

	for (int i = 0; i < MaxMesh; ++i)
		if ( g_Mesh[i].GetMesh() )
			g_Mesh[i].Release();	
	g_Sky.Release(); 
	g_DeviceInput.Release();
	g_Direct3D.Release();

	Log("End");
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------

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

	CVertexPT Vershin[4];

	Vershin[0] = CVertexPT(  1.0f, -1.0f, 0.0f, 1.0f, 1.0f ); // 0
	Vershin[1] = CVertexPT( -1.0f, -1.0f, 0.0f, 0.0f, 1.0f ); // 1	
	Vershin[2] = CVertexPT( -1.0f,  1.0f, 0.0f, 0.0f, 0.0f ); // 2		
	Vershin[3] = CVertexPT(  1.0f,  1.0f, 0.0f, 1.0f, 0.0f ); // 3
	                       // X      Y     Z     tu    tv

	const unsigned short Index[] =
	{
		0,1,2,    2,3,0,		
	};
	
	if ( FAILED( m_pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexPT ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBuf, 0 ) ) ) // создаём буфер вершин		
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
		Log( "error load target texture" );

	Log( "Init Text " );
return S_OK;
}

void CText::RenderInt( int Number, CShader const& Shader )
{
	D3DXMATRIX MatrixWorldTrans; 
	char str[20];
	itoa( Number, str, 10);
	for ( int i = 0; i < strlen(str); ++i )
	{
		D3DXMatrixTranslation( &MatrixWorldTrans, -0.95f + i * 0.07f, -0.9f, 0 );		
		Render( Shader, MatrixWorldTrans, int(str[i]) - 48 );
	}

}

void CText::Render( CShader const& Shader, const D3DXMATRIX&  MatrixWorldTrans, int Num )
{
	D3DXMATRIX   MatrixWorld, 
		         MatrixWorldScal;
	D3DVIEWPORT9 ViewPort;

	m_pD3DDevice->SetRenderState(  D3DRS_ZENABLE, false );
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ); 
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ); 
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );

	m_pD3DDevice->GetViewport( &ViewPort );

	float Scale = 0.05f;
	D3DXMatrixScaling( &MatrixWorldScal, Scale, Scale * float(ViewPort.Width) / float(ViewPort.Height), Scale );	
	MatrixWorld = MatrixWorldScal * MatrixWorldTrans;	
	if ( Shader.m_pConstTableVS )
	{		
		Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_world", &MatrixWorld );		
		Shader.m_pConstTablePS->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );
		Shader.m_pConstTableVS->SetInt(  m_pD3DDevice, "number", Num );
	}
	// устанавливаем шейдеры
	m_pD3DDevice->SetVertexShader( Shader.m_pVertexShader );
	m_pD3DDevice->SetPixelShader(  Shader.m_pPixelShader );
	// здесь перерисовка сцены	
	m_pD3DDevice -> SetStreamSource(0, m_pVerBuf, 0, sizeof( CVertexPT ) ); // связь буфера вершин с потоком данных
	m_pD3DDevice -> SetFVF( D3DFVF_XYZ | D3DFVF_TEX1 ); // устанавливается формат вершин
	m_pD3DDevice -> SetIndices( m_pIndexBuf );	
	m_pD3DDevice -> SetTexture( 0, m_Texture );	
	m_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2 );

	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	m_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, true );
}

void CText::Release()
{
	if ( m_Texture )
		m_Texture->Release();
	if ( m_pIndexBuf )
		m_pIndexBuf->Release();
	if ( m_pVerBuf )
		m_pVerBuf->Release();	
}

CText::~CText()
{
	Release();
}
