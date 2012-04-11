#include "Init.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Mesh.h"
#include "Sky.h"
#include "Weapon.h"
#include "Text.h"
#include <vector>

extern IDirect3DDevice9* g_pD3DDevice;



CD3DDevice   g_Direct3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CText        g_Text;
CMesh3D      g_Mesh[MaxMesh];
CShader      g_Shader[MaxShader];
CameraDevice g_Camera;
bool         g_Exit      = false;
bool		 g_Wireframe = false;
CWeapon*     g_Weapon[MaxWeapon];
byte         Avtomat = M16;

void InitWeapon( IDirect3DDevice9* pD3DDevice )
{
	g_Weapon[M16]  = new CAutomatic_M16(  "model\\M16.x", pD3DDevice );
	g_Weapon[AK47] = new CAutomatic_AK47( "model\\AK47.x", pD3DDevice );
}

void DeleteWeapon()
{
	for ( int i = 0; i < MaxWeapon; ++i )
		delete g_Weapon[i];
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
	//----------------------------Ќахождение пересечение со сферами----------------------------------------
	POINT NumObject[MaxField*MaxField];
	int Count = -1;
	for ( int ArrY = 0; ArrY < MaxField; ++ArrY )
		for ( int ArrX = 0; ArrX < MaxField; ++ArrX )
		{	
			D3DXVECTOR3 v =  PosView - Cell[ArrX*MaxField+ArrY].m_Centr;
			float b = 2.0f * D3DXVec3Dot( &Direction, &v );
			float c = D3DXVec3Dot( &v, &v ) - Cell[ArrX*MaxField+ArrY].m_Radius * Cell[ArrX*MaxField+ArrY].m_Radius ;
			// Ќаходим дискриминант
			float Discr = ( b * b ) - ( 4.0f * c );			
			if ( Discr >= 0.0f )
			{
				Discr = sqrtf(Discr);
				float s0 = ( -b + Discr ) / 2.0f;
				float s1 = ( -b - Discr ) / 2.0f;
				// ≈сли есть решение >= 0, луч пересекает сферу
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
	//------------------------------------------Render Weapon----------------------------------------	
	//g_Mesh[Pers].RenderMesh( g_Camera, MatrixWorld, g_Shader[Diffuse] );	
	g_Weapon[Avtomat]->RenderWeapon( g_Camera, g_Shader[Diffuse] );
	//------------------------------------------Render Text----------------------------------------
	//int a = timeGetTime() % 100000;
	g_Text.RenderInt( g_Weapon[Avtomat]->GetChargerBullet(), g_Shader[Text] );
	//------------------------------------------Render Target----------------------------------------
	D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
	g_Text.RenderImage( g_Shader[FlatImage], 0.02f, MatrixWorld );
	

// 	char        str[50];
// 	sprintf(str, "x=%f  y=%f   z=%f", g_Camera.DirX.x, g_Camera.DirX.y, g_Camera.DirX.z );		
// 	DrawMyText( D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));

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
		if ( wparam == VK_F6 )
			Avtomat = M16;
		if ( wparam == VK_F5 )
			Avtomat = AK47;
		break;
	}
	return DefWindowProc( hwnd, Message, wparam, lparam );
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
	w.hIcon         = LoadIcon( 0, IDI_QUESTION );//стандартна€ иконка приложени€ Win API 	
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
		InitWeapon( g_pD3DDevice );
		//g_Mesh[Pers].InitialMesh( "model\\M16.x", g_pD3DDevice );
		
		g_Shader[  Sky  ].LoadShader( "shader\\Sky", g_pD3DDevice );
		g_Shader[Diffuse].LoadShader( "shader\\Diffuse", g_pD3DDevice );
		g_Shader[  Text ].LoadShader( "shader\\Text", g_pD3DDevice );
		g_Shader[FlatImage].LoadShader( "shader\\FlatImage", g_pD3DDevice );
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
	DeleteWeapon();
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
