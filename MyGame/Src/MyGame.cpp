#include "Init.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Mesh.h"
#include "Sky.h"
#include "Weapon.h"
#include "Text.h"
#include "Sound.h"
#include <vector>


CD3DDevice   g_Direct3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CText        g_Text;
CMesh3D      g_Mesh[MaxMesh];
CShader      g_Shader[MaxShader];
CameraDevice g_Camera;
CSound       g_Sound;
bool         g_Exit      = false;
bool		 g_Wireframe = false;
CWeapon*     g_Weapon[MaxWeapon];
byte         ActiveWeapon = M16;
IDirect3DTexture9* TexTarget = 0;

void InitWeapon( IDirect3DDevice9* pD3DDevice )
{
	for ( int i = 0; i < MaxWeapon; ++i )
		g_Weapon[i]  = new CWeapon( WeaponNames[i], pD3DDevice );
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
	g_Direct3D.GetD3DDevice()->GetViewport( &ViewPort );

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

void RenderImg( IDirect3DDevice9* m_pD3DDevice, CShader const& Shader, IDirect3DTexture9* Texture, CText& Img, const D3DXMATRIX&  MatrixWorld, int num )
{
	if ( !TexTarget )
		if ( FAILED( D3DXCreateTextureFromFile( m_pD3DDevice, "model\\Shot.png", &TexTarget ) ) )
			Log( "error load target texture" );
	if ( Shader.m_pConstTableVS )
	{		
		Shader.m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_world", &MatrixWorld );		
		Shader.m_pConstTablePS->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );
		Shader.m_pConstTableVS->SetInt(    m_pD3DDevice, "number", num );
	}
	// устанавливаем шейдеры
	m_pD3DDevice->SetVertexShader( Shader.m_pVertexShader );
	m_pD3DDevice->SetPixelShader(  Shader.m_pPixelShader );
	// здесь перерисовка сцены	
	m_pD3DDevice -> SetStreamSource( 0, Img.GetVertexBuffer(), 0, sizeof( CVertexFVF ) ); // св€зь буфера вершин с потоком данных
	m_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливаетс€ формат вершин
	m_pD3DDevice -> SetIndices( Img.GetIndexBuffer() );	
	m_pD3DDevice -> SetTexture( 0, TexTarget );	
	m_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2 );
}

void RenderingDirect3D( IDirect3DDevice9* D3DDevice )
{
	D3DXMATRIX MatrixWorld, MatrixWorldX, MatrixWorldY, MatrixWorldZ;
	g_Camera.SetMesh( g_Mesh[Zona_1].GetMesh() );
	if ( D3DDevice == 0 )
		return;
	//----------------------------------------------режим каркаса-------------------------------
	if ( g_Wireframe )
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float  Ang = timeGetTime() / 200.0f;
	D3DXMATRIX  MatrixView       = g_Camera.m_View;
	D3DXMATRIX  MatrixProjection = g_Camera.m_Proj;	
	// очистка заднего буфера
	D3DDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );
	// начало рендеринга
	D3DDevice->BeginScene(); 
	//------------------------------------------Render Sky----------------------------------------
	g_Sky.RenderSky( g_Camera, g_Shader[Sky] );

	//------------------------------------------Render Zona----------------------------------------	
	D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
	g_Mesh[Zona_1].RenderMesh( g_Camera, MatrixWorld, g_Shader[Diffuse] );

	
	//------------------------------------------Render Weapon----------------------------------------
	if ( g_Weapon[ActiveWeapon]->GetFire() )
	{
		D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
		D3DXMatrixInverse( &MatV, 0, &g_Camera.m_View ); 
		D3DXMatrixScaling( &MatrixWorldSc, 0.1f, 0.1f, 0.1f );
		//D3DXMatrixRotationY( &MatrixWorld, 0 );
		D3DXMatrixTranslation( &MatrixWorldTr, 0.1f, -0.1f, 0.93f );
		MatrixWorld = MatrixWorldSc * MatrixWorldTr * g_Camera.MatInverseViewProject();
		int i = timeGetTime() %10;
		RenderImg( D3DDevice, g_Shader[Text], 0, g_Text, MatrixWorld, i );
		g_Sound.Play();
	}
	g_Weapon[ActiveWeapon]->RenderWeapon( g_Camera, g_Shader[Diffuse] );

	


	//------------------------------------------Render Text----------------------------------------	
	g_Text.RenderInt( -0.95f, -0.9f,  0.07f, g_Weapon[ActiveWeapon]->GetChargerBullet(), 2, g_Shader[Text] ); // вывод количества патронов в магазине
	g_Text.RenderInt(  0.80f, -0.9f,  0.07f, g_Weapon[ActiveWeapon]->GetAmountBullet(),  3, g_Shader[Text] ); // вывод остатка общего количества патронов
	//------------------------------------------Render Target----------------------------------------
	D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
	g_Text.RenderImage( g_Shader[FlatImage], 0.02f, MatrixWorld );
	
	
// 	char        str[50];
// 	sprintf(str, "x=%d  y=%f   z=%f", timeGetTime(), g_Camera.m_DirX.y, g_Camera.m_DirX.z );		
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
		if ( wparam == '1' )
			ActiveWeapon = M16;
		if ( wparam == '2' )
			ActiveWeapon = AK47;
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
		g_Sky.InitialSky( g_Direct3D.GetD3DDevice() );
		g_Text.Init( g_Direct3D.GetD3DDevice() );
		g_DeviceInput.InitialInput( hwnd );
		InitWeapon( g_Direct3D.GetD3DDevice() );
		g_Mesh[Zona_1].InitialMesh( "model\\Zona_1.x", g_Direct3D.GetD3DDevice() );
		
		g_Shader[  Sky  ].LoadShader( "shader\\Sky", g_Direct3D.GetD3DDevice() );
		g_Shader[Diffuse].LoadShader( "shader\\Diffuse", g_Direct3D.GetD3DDevice() );
		g_Shader[  Text ].LoadShader( "shader\\Text", g_Direct3D.GetD3DDevice() );
		g_Shader[FlatImage].LoadShader( "shader\\FlatImage", g_Direct3D.GetD3DDevice() );
		while( !g_Exit )
		{
			g_DeviceInput.ScanInput( &g_Camera, g_Weapon[ActiveWeapon] );				
			RenderingDirect3D( g_Direct3D.GetD3DDevice() );				
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

