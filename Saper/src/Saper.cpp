#include "D3DDevice.h"
#include "CameraDevice.h"
#include "InputDevice.h"

using namespace std;

extern IDirect3DDevice9* g_pD3DDevice;

const D3DXVECTOR4  Light( 0.0f, 1.0f, 0.0f, 1.0f );
bool               g_Wireframe       = false;
const float        Diffuse_intensity = 1.0f;


struct CSky
{
	IDirect3DVertexBuffer9* m_pVerBufSky;
	IDirect3DIndexBuffer9*  m_pBufIndexSky;
	HRESULT                 InitialSky();
	void                    Release();
};

class CMesh3D
{
public:
	ID3DXMesh*              m_pMesh;
	D3DMATERIAL9*           m_pMeshMaterial;
	IDirect3DTexture9**     m_pMeshTextura;
	DWORD                   m_TexturCount; 
	IDirect3DVertexBuffer9* m_VertexBuffer;
	IDirect3DIndexBuffer9*  m_IndexBuffer;
	DWORD 					m_SizeFVF;
	float                   m_Alpha;
	HRESULT                 InitialMesh( LPCSTR Name, FILE* FileLog );
	void					Release();
	void                    DrawMyMesh();
	void					SetMatrixWorld( D3DXMATRIX  Matrix );
	void					SetMatrixView( D3DXMATRIX  Matrix );
	void					SetMatrixProjection( D3DXMATRIX  Matrix );
private:
	D3DXMATRIX              m_MatrixWorld;
	D3DXMATRIX              m_MatrixView;
	D3DXMATRIX              m_MatrixProjection;
};

const int    MaxField = 10;
CD3DDevice   g_DeviceD3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CMesh3D      g_MeshA;
CMesh3D      g_MeshS;
CMesh3D      g_MeshX;
CMesh3D      g_MeshO;
CMesh3D		 g_MeshWin;
CMesh3D		 g_MeshLost;
CMesh3D      g_MeshStalemate;
CameraDevice g_Camera;
CCell        g_Cell[MaxField][MaxField];
bool         g_Exit = false;

void DrawMyText( IDirect3DDevice9* g_pD3DDevice, char* StrokaTexta, int x, int y, int x1, int y1, D3DCOLOR MyColor )
{
	RECT  Rec;
	HFONT hFont;
	ID3DXFont* pFont = 0; 
	hFont = CreateFont(30, 10, 0, 0, FW_NORMAL, FALSE, FALSE, 0, 1, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial");
	Rec.left   = x;
	Rec.top    = y;
	Rec.right  = x1;
	Rec.bottom = y1;
	D3DXCreateFont( g_pD3DDevice, 30, 10, FW_NORMAL, 0, FALSE, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial", &pFont );
	pFont->DrawText(0, StrokaTexta, -1, &Rec, DT_WORDBREAK, MyColor);
	if (pFont)
		pFont->Release();
}

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

int GameOver()
{
	return -1;
}

void RenderingDirect3D( CCell* Cell )
{	
	const D3DXVECTOR4 Scale( tan( D3DX_PI / 8 * (FLOAT)Height / Width), tan( D3DX_PI / 8 * (FLOAT)Height / Width  ), 1.0f, 1.0f );
	//----------------------------------------------режим каркаса-------------------------------
	if ( g_Wireframe )
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float const Angle = timeGetTime() / 2000.0f;

	const D3DXMATRIX MatrixView       = g_Camera.m_View;
	const D3DXMATRIX MatrixProjection = g_Camera.m_Proj;

	if ( g_pD3DDevice == 0 )
		return;

	g_pD3DDevice -> Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);// очистка заднего буфера
	g_pD3DDevice -> BeginScene(); // начало рендеринга

	//------------------------------------------Render Sky----------------------------------------
	g_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, false );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ); 
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ); 
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP ); 
	D3DXMATRIX MatrixWorld;
	D3DXMatrixTranslation( &MatrixWorld, 1.0f, 1.0f, 1.0f );
	D3DXMATRIX tmp = MatrixWorld * MatrixView * MatrixProjection;
	if ( g_DeviceD3D.m_pConstTableVS[Sky] )
	{
		g_DeviceD3D.m_pConstTableVS[Sky] -> SetMatrix( g_pD3DDevice, "mat_mvp",   &tmp );
		g_DeviceD3D.m_pConstTableVS[Sky] -> SetVector( g_pD3DDevice, "vec_light", &Light );
		g_DeviceD3D.m_pConstTableVS[Sky] -> SetVector( g_pD3DDevice, "scale",     &Scale );
		g_DeviceD3D.m_pConstTableVS[Sky] -> SetMatrix( g_pD3DDevice, "mat_view",  &MatrixView );
	}
	// здесь перерисовка сцены	
	g_pD3DDevice -> SetStreamSource(0, g_Sky.m_pVerBufSky, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
	g_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
	g_pD3DDevice -> SetIndices( g_Sky.m_pBufIndexSky );
	g_pD3DDevice -> SetTexture( 0, g_DeviceD3D.m_CubeTexture );
	// устанавливаем шейдеры
	g_pD3DDevice -> SetVertexShader( g_DeviceD3D.m_pVertexShader[Sky] );
	g_pD3DDevice -> SetPixelShader(  g_DeviceD3D.m_pPixelShader [Sky] );
	// вывод примитивов
	g_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2 );

	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, true );
	//------------------------------------------Render Mesh----------------------------------------
	D3DXMATRIX MatrixWorldX,MatrixWorldY;
	int t = ( MaxField - 1) / 2;
	//------------------Ограда--------------
	//----------
	D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
	D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( -1 - t ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh();

	D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
	D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( - t +  MaxField) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh();

	D3DXMatrixRotationY( &MatrixWorldY, 3.14f );
	D3DXMatrixTranslation( &MatrixWorldX, (- t +  MaxField ), 0, ( -1 - t ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh();

	D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
	D3DXMatrixTranslation( &MatrixWorldX, (- t +  MaxField ), 0, ( - t +  MaxField ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh();
	for ( int x = 1; x < MaxField - 1; ++x )
	{
		D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
		D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( x - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh();

		D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
		D3DXMatrixTranslation( &MatrixWorldX, ( - t +  MaxField ), 0, ( x - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh();

		D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
		D3DXMatrixTranslation( &MatrixWorldX, ( x - t ), 0, ( -1 - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh();

		D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
		D3DXMatrixTranslation( &MatrixWorldX, ( x - t ), 0, ( - t +  MaxField ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh();
	}
	//-------------------------------------------------------------------------
	for ( int y = 0; y < MaxField; ++y )
		for ( int x = 0; x < MaxField; ++x )
		{
			if ( Cell[x*MaxField+y].m_Value == 1 )
			{
				//--------------------X-------------------
				D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_MeshX.SetMatrixWorld( MatrixWorld );
				g_MeshX.SetMatrixView( MatrixView );
				g_MeshX.SetMatrixProjection( MatrixProjection );
				g_MeshX.m_Alpha = 1.0f;
				g_MeshX.DrawMyMesh();
			}
			if ( Cell[x*MaxField+y].m_Value == 0 )
			{		
				//--------------------O-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -Angle + 3.14f );
				D3DXMatrixTranslation( &MatrixWorldX, ( x * 16 - 16 ), ( 16 - y * 16  ), 0 );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_MeshO.SetMatrixWorld( MatrixWorld );
				g_MeshO.SetMatrixView( MatrixView );
				g_MeshO.SetMatrixProjection( MatrixProjection );
				g_MeshO.m_Alpha = 1.0f;
				g_MeshO.DrawMyMesh();
			}
		}
		POINT P = PickObject( &Cell[0] );
		char  str[50];
		sprintf(str, "%d          %d", P.x, P.y);		
		//DrawMyText(g_pD3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	
		if ( P.x >= 0)
		{
			if ( Cell[P.x*MaxField+P.y].m_Value == 1 )
			{
				//--------------------X-------------------
				D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( P.y - t ), 0.2, ( P.x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_MeshX.SetMatrixWorld( MatrixWorld );
				g_MeshX.SetMatrixView( MatrixView );
				g_MeshX.SetMatrixProjection( MatrixProjection );
				g_MeshX.m_Alpha = 1.0f;
				g_MeshX.DrawMyMesh();
			}
		}

		switch ( GameOver() )
		{
		case 1:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshWin.SetMatrixWorld( MatrixWorld );
			g_MeshWin.SetMatrixView( g_Camera.m_View );
			g_MeshWin.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshWin.m_Alpha = 1.0f;
			g_MeshWin.DrawMyMesh();
			break;
		case 2:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshLost.SetMatrixWorld( MatrixWorld );
			g_MeshLost.SetMatrixView( g_Camera.m_View );
			g_MeshLost.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshLost.m_Alpha = 1.0f;
			g_MeshLost.DrawMyMesh();
			break;
		case 3:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshStalemate.SetMatrixWorld( MatrixWorld );
			g_MeshStalemate.SetMatrixView( g_Camera.m_View );
			g_MeshStalemate.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshStalemate.m_Alpha = 1.0f;
			g_MeshStalemate.DrawMyMesh();
			break;
		}

		g_pD3DDevice -> EndScene();
		g_pD3DDevice -> Present( 0, 0, 0, 0 ); // вывод содержимого заднего буфера в окно
}

void CheckPC( lua_State* m_luaVM, bool* Check, CCell* Cell )
{
	Beep(150, 50); 
	lua_getglobal( m_luaVM, "IO" );
	lua_newtable( m_luaVM );//создать таблицу, поместить ее на вершину стэка
	for (int y = 0; y < MaxField; ++y)
		for (int x = 0; x < MaxField; ++x) 
		{
			lua_pushnumber( m_luaVM,  y * MaxField + x + 1 );               //кладем в стэк число (key)
			lua_pushnumber( m_luaVM,  Cell[x*MaxField+y].m_Value );//добавляем значение ключа (m_Value)
			lua_settable  ( m_luaVM, -3 );              //добавить к таблице пару ключ-значение: table[key] = m_Value		
		}
		if ( lua_pcall( m_luaVM, 1, 2, 0 ) )
		{
			//if ( FileLog ) 				fprintf( FileLog, lua_tostring( m_luaVM, -1 ) );
			lua_pop( m_luaVM, 1 );
		}	
		int y = lua_tonumber( m_luaVM, -1 );
		int x = lua_tonumber( m_luaVM, -2 );


		if ( Cell[x*MaxField+y].m_Value == 10 )
			Cell[x*MaxField+y].m_Value = 0;
		*Check = false;		
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

struct CFps
{
	int			 m_count;
	int          m_fps;
	int          m_last_tick;
	int			 m_this_tick;
	int			 Fps();

	CFps(): m_count(0)
	{	}
};

int CFps::Fps()
{	
	m_this_tick = GetTickCount();
	if ( m_this_tick - m_last_tick >= 1000 )
	{
		m_last_tick = m_this_tick;
		m_fps       = m_count;
		m_count     = 0;
	}
	else m_count++;
	return m_fps;
}


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine, int       nCmdShow)
{	
	MSG			 msg;
	WNDCLASS	 w;	
	CFps         g_fps;	
	bool         Check = false;

	// Запись лога в файл 
	FILE *g_FileLog = fopen( "log.txt", "w" );
	CLuaScript   g_Lua( g_FileLog );

	memset(&w,0,sizeof(WNDCLASS));
	w.style         = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc   = WndProc;
	w.hInstance     = hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	w.lpszClassName = "My Class";
	w.hIcon         = LoadIcon( 0, IDI_QUESTION );//стандартная иконка приложения Win API 	
	w.hCursor       = LoadCursor( 0, "aero_link_l.cur" );
	RegisterClass(&w);
	HWND hwnd = CreateWindow( "My Class", "Сапёр", WS_SYSMENU | WS_MINIMIZEBOX,
							  250, 150, Width, Height, 0, 0, hInstance, 0 );	
	ShowWindow( hwnd, nCmdShow );
	ZeroMemory( &msg, sizeof( msg ) );
	/*
	HWND i;
	i=LoadImage(0,"cursor1.cur",IMAGE_CURSOR,0,0,LR_LOADFROMFILE);
	if (i==0) ShowMessage("Ошибка загрузки курсора!");
	else
	{
		Screen->Cursors[1]=i;
		Form1->Cursor=1;
	}*/

	if ( SUCCEEDED( g_DeviceD3D.IntialDirect3D( hwnd, g_FileLog) ) )
	{	
		if ( SUCCEEDED( g_DeviceD3D.LoadTexture( g_FileLog ) ) )
		{			
			for (int y = 0; y < MaxField; ++y)
				for (int x = 0; x < MaxField; ++x)
				{
					g_Cell[x][y].SetCenter(  y - int((MaxField-1)/2) , 0 ,x - int((MaxField-1)/2));	
					g_Cell[x][y].m_Value = 1;
				}
			g_MeshS.InitialMesh( "model//Stena.x", g_FileLog );
			g_MeshA.InitialMesh( "model//Angle.x", g_FileLog );
			g_MeshO.InitialMesh( "model//O.x", g_FileLog );
			g_MeshX.InitialMesh( "model//Box.x", g_FileLog );	
			g_MeshWin.InitialMesh( "model//Win.x", g_FileLog );	
			g_MeshLost.InitialMesh( "model//Lost.x", g_FileLog );
			g_MeshStalemate.InitialMesh( "model//Stalemate.x", g_FileLog );
			g_Sky.InitialSky();
			g_DeviceInput.InitialInput( hwnd, g_FileLog );					
			g_DeviceD3D.InitialShader();
			g_fps.m_last_tick = GetTickCount();
			while( !g_Exit )
			{
				g_DeviceInput.ScanInput( &g_Camera, &Check, &g_Cell[0][0] );
				if ( Check )
					CheckPC( g_Lua.m_luaVM , &Check, &g_Cell[0][0]);
				RenderingDirect3D( &g_Cell[0][0] );				
				if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage(  &msg );
				}
			}						
		}
	}	
	g_MeshStalemate.Release();
	g_MeshLost.Release();
	g_MeshWin.Release();
	g_MeshS.Release();
	g_MeshX.Release();
	g_MeshO.Release();	
	g_Sky.Release(); 
	g_DeviceInput.Release();
	g_DeviceD3D.Release();
	if ( g_FileLog )
		fclose( g_FileLog );
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------




HRESULT CSky::InitialSky()
{
	void *pBV;
	void *pBI;

	m_pVerBufSky   = 0; // указатель на буфер вершин
	m_pBufIndexSky = 0; // указатель на буфер вершин

	CVertexFVF SkyVershin[] =
	{			
		{ 1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // 0
		{-1.0f, -1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f}, // 1	
		{-1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // 2		
		{ 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f}, // 3
		// X        Y     Z    nx    ny    nz     tu    tv
	};
	const unsigned short SkyIndex[] =
	{
		0,1,2,    2,3,0,		
	};
	if ( FAILED( g_pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, // создаём буфер вершин
		D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBufSky, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pVerBufSky -> Lock( 0, sizeof( SkyVershin ), ( void** )&pBV, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBV, SkyVershin, sizeof( SkyVershin ) ); // копирование данных о вершинах в буфер вершин
	m_pVerBufSky -> Unlock(); // разблокирование

	if ( FAILED( g_pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,         // создаём буфер вершин
		D3DPOOL_DEFAULT, &m_pBufIndexSky, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pBufIndexSky -> Lock( 0, sizeof( SkyIndex ), ( void** )&pBI, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBI, SkyIndex, sizeof( SkyIndex ) ); // копирование данных о вершинах в буфер вершин
	m_pBufIndexSky -> Unlock(); // разблокирование	

	return S_OK;
}

void CSky::Release()
{
	if ( m_pBufIndexSky )
		m_pBufIndexSky -> Release();
	if ( m_pVerBufSky )
		m_pVerBufSky -> Release();
}

HRESULT CMesh3D::InitialMesh(LPCSTR Name, FILE *FileLog )
{
	m_pMesh         = 0;
	m_pMeshMaterial = 0;
	m_pMeshTextura  = 0;
	m_SizeFVF       = 0;
	m_Alpha         = 1.0f;	
	ID3DXBuffer *pMaterialBuffer  = 0;
	if ( FAILED( D3DXLoadMeshFromX( Name, D3DXMESH_SYSTEMMEM, g_pD3DDevice, 0, &pMaterialBuffer, 0, &m_TexturCount, &m_pMesh ) ) )
	{
		if ( m_pMesh == 0 )
		{		
			if ( FileLog ) 
				fprintf( FileLog, "error load x file '%s'\n", Name );
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
	D3DXMATERIAL *D3DXMeshMaterial = (D3DXMATERIAL *)pMaterialBuffer->GetBufferPointer();
	m_pMeshMaterial  = new D3DMATERIAL9[m_TexturCount];
	m_pMeshTextura   = new IDirect3DTexture9*[m_TexturCount];
	for ( DWORD i = 0; i < m_TexturCount; i++ )
	{
		// Копируем материал
		m_pMeshMaterial[i] = D3DXMeshMaterial[i].MatD3D;
		// Установить окружающего свет
		m_pMeshMaterial[i].Ambient = m_pMeshMaterial[i].Diffuse;
		// Загружаем текстуру
		string FileName = string( "model//" ) + string( D3DXMeshMaterial[i].pTextureFilename );
		if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, FileName.c_str(), &m_pMeshTextura[i] )))
		{
			fprintf( FileLog, "error load texture '%s'\n", D3DXMeshMaterial[i].pTextureFilename );
			m_pMeshTextura[i] = 0;
		}
	}
	// Уничтожаем буфер материала
	pMaterialBuffer->Release();

	return S_OK;
}

void CMesh3D::SetMatrixWorld(D3DXMATRIX Matrix)
{
	m_MatrixWorld = Matrix;
}

void CMesh3D::SetMatrixView(D3DXMATRIX Matrix)
{
	m_MatrixView = Matrix;
}

void CMesh3D::SetMatrixProjection(D3DXMATRIX Matrix)
{
	m_MatrixProjection = Matrix;
}
void CMesh3D::DrawMyMesh()
{
	D3DXMATRIX  wvp;
	if ( m_pMesh )
	{
		wvp = m_MatrixWorld * m_MatrixView * m_MatrixProjection;
		if ( g_DeviceD3D.m_pConstTableVS[Diffuse] )
		{
			g_DeviceD3D.m_pConstTableVS[Diffuse] -> SetMatrix( g_pD3DDevice, "mat_mvp",   &wvp );
			g_DeviceD3D.m_pConstTableVS[Diffuse] -> SetMatrix( g_pD3DDevice, "mat_world", &m_MatrixWorld );
			g_DeviceD3D.m_pConstTableVS[Diffuse] -> SetVector( g_pD3DDevice, "vec_light", &Light );
			g_DeviceD3D.m_pConstTablePS[Diffuse] -> SetFloat(  g_pD3DDevice, "diffuse_intensity", Diffuse_intensity );	
			g_DeviceD3D.m_pConstTablePS[Diffuse] -> SetFloat(  g_pD3DDevice, "Alpha", m_Alpha );	
		}
		// устанавливаем шейдеры
		g_pD3DDevice->SetVertexShader( g_DeviceD3D.m_pVertexShader[Diffuse] );
		g_pD3DDevice->SetPixelShader(  g_DeviceD3D.m_pPixelShader [Diffuse] );

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
	if ( m_pMeshTextura )
	{
		for ( int i = 1; i < m_TexturCount; ++i )
		{
			if ( m_pMeshTextura[i] )
				m_pMeshTextura[i] -> Release();
		}
		delete []m_pMeshTextura;
	}
	if ( m_pMesh )
		m_pMesh -> Release();
}