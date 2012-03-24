#include "D3DDevice.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Field.h"
#include "Mesh.h"
#include <vector>




struct CSky
{
	IDirect3DVertexBuffer9* m_pVerBufSky;
	IDirect3DIndexBuffer9*  m_pBufIndexSky;
	HRESULT                 InitialSky( IDirect3DDevice9* pD3DDevice );
	void                    Release();
};

CD3DDevice        g_DeviceD3D;
CShader			  g_Shader;
//IDirect3DDevice9* g_pD3DDevice = g_DeviceD3D.GetDeviceD3D();
CInputDevice g_DeviceInput;
CSky         g_Sky;
CMesh3D      g_MeshA;
CMesh3D      g_Mesh[MaxMesh];
CMesh3D      g_MeshS;
CMesh3D		 g_MeshWin;
CMesh3D		 g_MeshLost;

CameraDevice g_Camera;
CField       g_Field( MaxField );
bool         g_Exit      = false;
bool		 g_Wireframe = false;


void DrawMyText( IDirect3DDevice9* g_pD3DDevice, char* StrokaTexta, int x, int y, int x1, int y1, D3DCOLOR MyColor )
{
	RECT       Rec;	
	ID3DXFont* pFont = 0;

	HFONT hFont = CreateFont( 30, 10, 0, 0, FW_NORMAL, FALSE, FALSE, 0, 1, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial" );
	Rec.left   = x;
	Rec.top    = y;
	Rec.right  = x1;
	Rec.bottom = y1;
	D3DXCreateFont( g_pD3DDevice, 30, 10, FW_NORMAL, 0, FALSE, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial", &pFont );
	pFont->DrawText( 0, StrokaTexta, -1, &Rec, DT_WORDBREAK, MyColor );
	if ( pFont )
		pFont->Release();
	DeleteObject(hFont);
}

POINT PickObject( IDirect3DDevice9* pD3DDevice, CCell* Cell )
{
	POINT        Point;
	D3DVIEWPORT9 ViewPort;
	RECT         ClientRec;

	GetClientRect ( GetForegroundWindow(), &ClientRec);
	ClientToScreen( GetForegroundWindow(), (LPPOINT)&ClientRec);
	GetCursorPos( &Point );
	int x = Point.x - ClientRec.left;
	int y = Point.y - ClientRec.top;
	pD3DDevice->GetViewport( &ViewPort );

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

void ShowBomb( CCell* Cell, int* Field )
{
	for (int y = 0; y < MaxField; ++y)
		for (int x = 0; x < MaxField; ++x)
		{
			if ( ( Cell[x*MaxField+y].m_Value == Mine) )
				Field[x*MaxField+y] = OpenCell;			
		}
}

Game_State GameOverCheck( CCell* Cell, int* Field )
{
	for (int y = 0; y < MaxField; ++y)
		for (int x = 0; x < MaxField; ++x)
		{
			if ( Field[x*MaxField+y] == OpenCell && Cell[x*MaxField+y].m_Value == Mine )
			{
				ShowBomb( Cell, Field );
				return STATE_LOST;
			}
		}
	int Count = 0;
	for (int y = 0; y < MaxField; ++y)
		for (int x = 0; x < MaxField; ++x)		
			if ( Field[x*MaxField+y] == Flag && Cell[x*MaxField+y].m_Value == Mine )
				++Count;
	if ( MaxMine == Count )			
		return STATE_WIN;	
		
return  STATE_PLAY;
}

void RenderFence()
{	
	g_Mesh[ Angle ].RenderMesh( g_Camera, -1, -1, -1.57f );
	g_Mesh[ Angle ].RenderMesh( g_Camera, -1, MaxField, 3.14f );
	g_Mesh[ Angle ].RenderMesh( g_Camera, MaxField, -1, 0.0f );
	g_Mesh[ Angle ].RenderMesh( g_Camera, MaxField, MaxField, 1.57f );
	for ( int x = 1; x < MaxField - 1; ++x )
	{		
		g_Mesh[ Stena ].RenderMesh( g_Camera, -1, x, 0.0f );
		g_Mesh[ Stena ].RenderMesh( g_Camera, MaxField, x, 0.0f );
		g_Mesh[ Stena ].RenderMesh( g_Camera, x, -1, 1.57f );
		g_Mesh[ Stena ].RenderMesh( g_Camera, x, MaxField, 1.57f );
	}
}

void RenderingDirect3D( IDirect3DDevice9* pD3DDevice, CCell* Cell, int* Field )
{	
	const D3DXVECTOR4 Scale( tan( D3DX_PI / 8 * (FLOAT)Height / Width), tan( D3DX_PI / 8 * (FLOAT)Height / Width  ), 1.0f, 1.0f );

	if ( !pD3DDevice )
		return;
	//----------------------------------------------режим каркаса-------------------------------
	if ( g_Wireframe )
		pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float const Ang = timeGetTime() / 2000.0f;

	 D3DXMATRIX MatrixView       = g_Camera.m_View;
	 D3DXMATRIX MatrixProjection = g_Camera.m_Proj;

	

	pD3DDevice -> Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);// очистка заднего буфера
	pD3DDevice -> BeginScene(); // начало рендеринга

	//------------------------------------------Render Sky----------------------------------------
	pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, false );
	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ); 
	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ); 
	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP ); 
	D3DXMATRIX MatrixWorld;
	D3DXMatrixTranslation( &MatrixWorld, 1.0f, 1.0f, 1.0f );
	D3DXMATRIX tmp = MatrixWorld * MatrixView * MatrixProjection;
	if ( g_Shader.m_pConstTableVS[Sky] )
	{
		g_Shader.m_pConstTableVS[Sky]->SetMatrix( pD3DDevice, "mat_mvp",   &tmp );
		g_Shader.m_pConstTableVS[Sky]->SetVector( pD3DDevice, "vec_light", &g_Light );
		g_Shader.m_pConstTableVS[Sky]->SetVector( pD3DDevice, "scale",     &Scale );
		g_Shader.m_pConstTableVS[Sky]->SetMatrix( pD3DDevice, "mat_view",  &MatrixView );
	}
	// здесь перерисовка сцены	
	pD3DDevice -> SetStreamSource(0, g_Sky.m_pVerBufSky, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
	pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
	pD3DDevice -> SetIndices( g_Sky.m_pBufIndexSky );
	pD3DDevice -> SetTexture( 0, g_DeviceD3D.m_CubeTexture );
	// устанавливаем шейдеры
	pD3DDevice -> SetVertexShader( g_Shader.m_pVertexShader[Sky] );
	pD3DDevice -> SetPixelShader(  g_Shader.m_pPixelShader [Sky] );
	// вывод примитивов
	pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2 );

	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, true );
	//------------------------------------------Render Mesh----------------------------------------
	D3DXMATRIX MatrixWorldX,MatrixWorldY,MatrixWorldZ;
	int t = ( MaxField - 1) / 2;
	//-----------ограда---------------
	RenderFence();
	//-------------------------------------------------------------------------
	for ( int y = 0; y < MaxField; ++y )
		for ( int x = 0; x < MaxField; ++x )
		{				
			if ( Field[x*MaxField+y] == Empty )					
				g_Mesh[ Empty ].RenderMesh( g_Camera, x, y, -1.57f );			
			if ( Field[x*MaxField+y] == Flag ) 
				g_Mesh[ Flag ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == One )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ One ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Two )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Two ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Three ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Three ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Four )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Four ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Five )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Five ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Six )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Six ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Seven ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Seven ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Eight ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Eight ].RenderMesh( g_Camera, x, y, -1.57f );
			if ( ( Cell[x*MaxField+y].m_Value == Mine )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Mine ].RenderMesh( g_Camera, x, y, -1.57f );
		}
		POINT P = PickObject( pD3DDevice, &Cell[0] );		
		if ( P.x >= 0)
		{
			if ( Field[P.x*MaxField+P.y] == Empty )
			{
				//--------------------X-------------------
				D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( P.y - t ), 0.2, ( P.x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_Mesh[Empty].SetMatrixWorld( MatrixWorld );
				g_Mesh[Empty].SetMatrixView( MatrixView );
				g_Mesh[Empty].SetMatrixProjection( MatrixProjection );
				g_Mesh[Empty].DrawMyMesh(g_Shader.m_pConstTableVS, g_Shader.m_pConstTablePS, g_Shader.m_pVertexShader, g_Shader.m_pPixelShader);
			}
		}

		switch ( GameOverCheck( Cell, Field ) )
		{
		case STATE_WIN:
			D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
			D3DXMatrixRotationZ( &MatrixWorld, 1.57f );
			D3DXMatrixScaling( &MatrixWorldX, 0.2f, 0.2f, 0.2f );
			MatrixWorld = MatrixWorldY * MatrixWorld * MatrixWorldX;
			D3DXMatrixTranslation( &MatrixWorldY, MaxField - 2, 0, 1 );
			MatrixWorld = MatrixWorld * MatrixWorldY;		
			g_MeshWin.SetMatrixWorld( MatrixWorld );
			g_MeshWin.SetMatrixView( g_Camera.m_View );
			g_MeshWin.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshWin.DrawMyMesh(g_Shader.m_pConstTableVS, g_Shader.m_pConstTablePS, g_Shader.m_pVertexShader, g_Shader.m_pPixelShader);
			break;
		case STATE_LOST:
			D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
			D3DXMatrixRotationZ( &MatrixWorld, 1.57f );
			D3DXMatrixScaling( &MatrixWorldX, 0.2f, 0.2f, 0.2f );
			MatrixWorld = MatrixWorldY * MatrixWorld * MatrixWorldX;
			D3DXMatrixTranslation( &MatrixWorldY, MaxField - 2, 0, 0.5f );
			MatrixWorld = MatrixWorld * MatrixWorldY;
			g_MeshLost.SetMatrixWorld( MatrixWorld );
			g_MeshLost.SetMatrixView( g_Camera.m_View );
			g_MeshLost.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshLost.DrawMyMesh(g_Shader.m_pConstTableVS, g_Shader.m_pConstTablePS, g_Shader.m_pVertexShader, g_Shader.m_pPixelShader);
			break;
		}
		//-------------------CountMine-----------------------------------------
		if ( GameOverCheck( Cell, Field ) == STATE_PLAY )
		{
			int flag = 0;
			for ( int y = 0; y < MaxField; ++y )
				for ( int x = 0; x < MaxField; ++x )
				{
					if ( Field[x*MaxField+y] == Flag ) 
						++flag;
				}
			flag = MaxMine - flag;
			int Units = flag % 10;
			int Tens  = (flag - Units)/10;						
			g_Mesh[ Units ].RenderMesh( g_Camera, t + 0.5f, 1 - t, -1.57f );
			g_Mesh[ Tens ].RenderMesh( g_Camera, t, 1 - t, -1.57f  );
			
		}
// 		char  str[50];
// 		sprintf(str, "%d:%d", Tens,Units);		
// 		DrawMyText(g_pD3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	

		pD3DDevice -> EndScene();
		pD3DDevice -> Present( 0, 0, 0, 0 ); // вывод содержимого заднего буфера в окно
}

void ClearField( CCell* Cell, int* Field, int x, int y )
{	
	Field[x*MaxField+y] = OpenCell;
	for (int b = y-1; b < y+2; ++b)
		for (int a = x-1; a < x+2; ++a)
			if ( (a >= 0) && (b >= 0) && (a < MaxField) && (b < MaxField) && ( Field[a*MaxField+b] == Empty ) )
			{				
				if ( ( Cell[a*MaxField+b].m_Value == Empty ) )
					ClearField(  Cell, Field, a, b);
				Field[a*MaxField+b] = OpenCell;				
			}
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

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine, int       nCmdShow)
{	
	MSG			 Msg;
	WNDCLASS	 w;			

	// Запись лога в файл 
	FILE *FileLog = fopen( "log.txt", "w" );	

	memset( &w, 0, sizeof(WNDCLASS) );
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
	ZeroMemory( &Msg, sizeof( MSG ) );
	/*
	HWND i;
	i=LoadImage(0,"cursor1.cur",IMAGE_CURSOR,0,0,LR_LOADFROMFILE);
	if (i==0) ShowMessage("Ошибка загрузки курсора!");
	else
	{
		Screen->Cursors[1]=i;
		Form1->Cursor=1;
	}*/

	if ( SUCCEEDED( g_DeviceD3D.IntialDirect3D( hwnd, FileLog) ) )
	{	
		if ( SUCCEEDED( g_DeviceD3D.LoadTexture( FileLog ) ) )
		{				 
			
			
			g_Mesh[Zero].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//0.x", FileLog );
			g_Mesh[One].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//1.x", FileLog );
			g_Mesh[Two].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//2.x", FileLog );
			g_Mesh[Three].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//3.x", FileLog );
			g_Mesh[Four].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//4.x", FileLog );
			g_Mesh[Five].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//5.x", FileLog );
			g_Mesh[Six].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//6.x", FileLog );
			g_Mesh[Seven].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//7.x", FileLog );
			g_Mesh[Eight].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//8.x", FileLog );
			g_Mesh[Nine].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//9.x", FileLog );
			g_Mesh[Empty].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//Empty.x", FileLog );
			g_Mesh[Flag].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//Flag.x", FileLog );
			g_Mesh[Mine].InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//Mine.x", FileLog );
			g_Mesh[Stena].InitialMesh( g_DeviceD3D.GetDeviceD3D(),   "model//Stena.x", FileLog );
			g_Mesh[Angle].InitialMesh( g_DeviceD3D.GetDeviceD3D(),   "model//Angle.x", FileLog );
			g_MeshWin.InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//Win.x", FileLog );	
			g_MeshLost.InitialMesh( g_DeviceD3D.GetDeviceD3D(), "model//Lost.x", FileLog );
			
			g_Sky.InitialSky( g_DeviceD3D.GetDeviceD3D() );
			g_DeviceInput.InitialInput( hwnd, FileLog );					
			g_Shader.InitialShader( g_DeviceD3D.GetDeviceD3D() );
			
			while( !g_Exit )
			{
				g_DeviceInput.ScanInput( &g_Camera, &g_Field.m_Cell[0], &g_Field.m_Field[0] );				
				RenderingDirect3D( g_DeviceD3D.GetDeviceD3D(), &g_Field.m_Cell[0], &g_Field.m_Field[0]);				
				if ( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &Msg );
					DispatchMessage(  &Msg );
				}
			}						
		}
	}	
	
	g_MeshLost.Release();
	g_MeshWin.Release();
	g_MeshS.Release();
	for (int i = 0; i < MaxMesh; ++i)
		if ( g_Mesh[i].GetMesh() )
			g_Mesh[i].Release();
	g_Shader.Release();	
	g_Sky.Release(); 
	g_DeviceInput.Release();
	g_DeviceD3D.Release();
	if ( FileLog )
		fclose( FileLog );
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------




HRESULT CSky::InitialSky( IDirect3DDevice9* pD3DDevice )
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
	if ( !pD3DDevice )
		return E_FAIL;
	if ( FAILED( pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, // создаём буфер вершин
		D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBufSky, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pVerBufSky -> Lock( 0, sizeof( SkyVershin ), ( void** )&pBV, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBV, SkyVershin, sizeof( SkyVershin ) ); // копирование данных о вершинах в буфер вершин
	m_pVerBufSky -> Unlock(); // разблокирование

	if ( FAILED( pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,         // создаём буфер вершин
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

