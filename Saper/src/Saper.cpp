#include "D3DDevice.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Field.h"
#include "Mesh.h"
#include "Particle.h"
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

CD3DDevice   g_Direct3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CMesh3D      g_MeshA;
CMesh3D      g_Mesh[MaxMesh];
CMesh3D      g_MeshS;
CMesh3D		 g_MeshWin;
CMesh3D		 g_MeshLost;
CShader      g_Diffuse;
CShader      g_ShaderSky;
CameraDevice g_Camera;
CField       g_Field( MaxField );
bool         g_Exit      = false;
bool		 g_Wireframe = false;
CException   g_Exception;



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
	D3DXVECTOR3 PosView = D3DXVECTOR3( MatV._41, MatV._42, MatV._43 ); //   ������� ���������� ������ �� ������� ����	
	D3DXVec3TransformNormal( &Direction, &Direction, &MatV );
	D3DXVec3Normalize( &Direction, &Direction );
	//----------------------------���������� ����������� �� �������----------------------------------------
	POINT NumObject[MaxField*MaxField];
	int Count = -1;
	for ( int ArrY = 0; ArrY < MaxField; ++ArrY )
		for ( int ArrX = 0; ArrX < MaxField; ++ArrX )
		{	
			D3DXVECTOR3 v =  PosView - Cell[ArrX*MaxField+ArrY].m_Centr;
			float b = 2.0f * D3DXVec3Dot( &Direction, &v );
			float c = D3DXVec3Dot( &v, &v ) - Cell[ArrX*MaxField+ArrY].m_Radius * Cell[ArrX*MaxField+ArrY].m_Radius ;
			// ������� ������������
			float Discr = ( b * b ) - ( 4.0f * c );			
			if ( Discr >= 0.0f )
			{
				Discr = sqrtf(Discr);
				float s0 = ( -b + Discr ) / 2.0f;
				float s1 = ( -b - Discr ) / 2.0f;
				// ���� ���� ������� >= 0, ��� ���������� �����
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
	int Empt  = 0;
	for (int y = 0; y < MaxField; ++y)
		for (int x = 0; x < MaxField; ++x)
		{
			if ( Field[x*MaxField+y] == Flag && Cell[x*MaxField+y].m_Value == Mine )
				++Count;
			if ( Field[x*MaxField+y] == Empty )
				++Empt;
		}
	if ( ( MaxMine == Count ) && ( Empt == 0 ) )		
		return STATE_WIN;	
		
return  STATE_PLAY;
}

D3DXMATRIX MatWorld( float x, float y, float z, float Ang )
{
	D3DXMATRIX MatrixWorld, MatrixWorldY, MatrixWorldX;
	int t = ( MaxField - 1) / 2;
	D3DXMatrixRotationY(   &MatrixWorldY, Ang );
	D3DXMatrixTranslation( &MatrixWorldX, ( x - t ), y, ( z - t ) );
	D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);

return MatrixWorld;
}

void RenderFence()
{
	g_Mesh[ Angle ].RenderMesh( g_Camera, MatWorld( -1, 0, -1, -D3DX_PI / 2 ), g_Diffuse );
	g_Mesh[ Angle ].RenderMesh( g_Camera, MatWorld( -1, 0, MaxField, 0 ), g_Diffuse );
	g_Mesh[ Angle ].RenderMesh( g_Camera, MatWorld( MaxField, 0, -1, D3DX_PI ), g_Diffuse );
	g_Mesh[ Angle ].RenderMesh( g_Camera, MatWorld( MaxField, 0, MaxField, D3DX_PI / 2 ), g_Diffuse );
	for ( int x = 1; x < MaxField - 1; ++x )
	{		
		g_Mesh[ Stena ].RenderMesh( g_Camera, MatWorld( -1, 0, x, D3DX_PI / 2 ), g_Diffuse );
		g_Mesh[ Stena ].RenderMesh( g_Camera, MatWorld( MaxField, 0, x, D3DX_PI / 2 ), g_Diffuse );
		g_Mesh[ Stena ].RenderMesh( g_Camera, MatWorld( x, 0, -1, 0 ), g_Diffuse );
		g_Mesh[ Stena ].RenderMesh( g_Camera, MatWorld( x, 0, MaxField, 0 ), g_Diffuse );
	}
}

void RenderingDirect3D( CCell* Cell, int* Field, IDirect3DDevice9* D3DDevice )
{
	if ( D3DDevice == 0 )
		return;
	//----------------------------------------------����� �������-------------------------------
	if ( g_Wireframe )
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		D3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float const Ang = timeGetTime() / 2000.0f;
	D3DXMATRIX  MatrixView       = g_Camera.m_View;
	D3DXMATRIX  MatrixProjection = g_Camera.m_Proj;	
	// ������� ������� ������
	D3DDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );
	// ������ ����������
	D3DDevice->BeginScene(); 
	//------------------------------------------Render Sky----------------------------------------
	g_Sky.RenderSky( g_Camera, g_ShaderSky );
	//------------------------------------------Render Mesh----------------------------------------
	D3DXMATRIX MatrixWorld, MatrixWorldX, MatrixWorldY, MatrixWorldZ;
	int t = ( MaxField - 1) / 2;
	//-----------������---------------
	RenderFence();
	//-------------------------------------------------------------------------
	for ( int y = 0; y < MaxField; ++y )
		for ( int x = 0; x < MaxField; ++x )
		{				
			if ( Field[x*MaxField+y] == Empty )					
				g_Mesh[ Empty ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );			
			if ( Field[x*MaxField+y] == Flag ) 
				g_Mesh[ Flag ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == One )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ One ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Two )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Two ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Three ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Three ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Four )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Four ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Five )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Five ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Six )   && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Six ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Seven ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Seven ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Eight ) && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Eight ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
			if ( ( Cell[x*MaxField+y].m_Value == Mine )  && ( Field[x*MaxField+y] == OpenCell ) )
				g_Mesh[ Mine ].RenderMesh( g_Camera, MatWorld( x, 0, y, 0 ), g_Diffuse );	
		}
		POINT P = PickObject( &Cell[0] );		
		if ( P.x >= 0)
		{
			if ( Field[P.x*MaxField+P.y] == Empty )
			{				
				D3DXMatrixRotationY( &MatrixWorldY, 0 );
				D3DXMatrixTranslation( &MatrixWorldX, ( P.x - t ), 0.2, ( P.y - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_Mesh[Empty].RenderMesh( g_Camera, MatrixWorld, g_Diffuse );								
			}
		}

		switch ( GameOverCheck( Cell, Field ) )
		{
		case STATE_WIN:
			D3DXMatrixRotationX( &MatrixWorldY, D3DX_PI / 2 );
			D3DXMatrixScaling( &MatrixWorldX, 0.2f, 0.2f, 0.2f );
			MatrixWorld = MatrixWorldY * MatrixWorldX;
			D3DXMatrixTranslation( &MatrixWorldY, 1.0f, 0.0f, -MaxField + 3 );
			MatrixWorld = MatrixWorld * MatrixWorldY;	
			g_MeshWin.RenderMesh( g_Camera, MatrixWorld, g_Diffuse );
			break;
		case STATE_LOST:
			D3DXMatrixRotationX( &MatrixWorldY, D3DX_PI / 2 );
			D3DXMatrixScaling( &MatrixWorldX, 0.2f, 0.2f, 0.2f );
  			MatrixWorld = MatrixWorldY * MatrixWorldX;
  			D3DXMatrixTranslation( &MatrixWorldY, 0.5f, 0.0f, -MaxField + 3 );
  			MatrixWorld = MatrixWorld * MatrixWorldY;
			g_MeshLost.RenderMesh( g_Camera, MatrixWorld, g_Diffuse );
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
			if ( Units < 0 )
				Tens = 0;
			if ( Units < 0 )
				Tens = 0;
			g_Mesh[ Units ].RenderMesh( g_Camera, MatWorld(  t + 0.8f, 0.0f, MaxField + 2, 0 ), g_Diffuse );
			g_Mesh[ Tens  ].RenderMesh( g_Camera, MatWorld(  t + 0.2f, 0.0f, MaxField + 2, 0 ), g_Diffuse );
			
		}
		//-----------------Particle--------------------
		g_Exception.RenderParticle( g_Camera, g_Diffuse );

		char        str[50];
		sprintf(str, "x=%f  y=%f   z=%f", g_Camera.CameraUp.x, g_Camera.CameraUp.y, g_Camera.CameraUp.z );		
		//DrawMyText( D3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));

		D3DDevice -> EndScene();
		D3DDevice -> Present( 0, 0, 0, 0 ); // ����� ����������� ������� ������ � ����
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
	w.hIcon         = LoadIcon( 0, IDI_QUESTION );//����������� ������ ���������� Win API 	
	w.hCursor       = LoadCursor( 0, "aero_link_l.cur" );
	RegisterClass(&w);
	HWND hwnd = CreateWindow( "My Class", "����", WS_SYSMENU | WS_MINIMIZEBOX,
							  250, 150, Width, Height, 0, 0, hInstance, 0 );	
	ShowWindow( hwnd, nCmdShow );
	ZeroMemory( &Msg, sizeof( MSG ) );
	
	if ( SUCCEEDED( g_Direct3D.IntialDirect3D( hwnd ) ) )
	{
		g_Mesh[Zero].InitialMesh( "model\\0.x" );
		g_Mesh[One].InitialMesh( "model\\1.x" );
		g_Mesh[Two].InitialMesh( "model\\2.x" );
		g_Mesh[Three].InitialMesh( "model\\3.x" );
		g_Mesh[Four].InitialMesh( "model\\4.x" );
		g_Mesh[Five].InitialMesh( "model\\5.x" );
		g_Mesh[Six].InitialMesh( "model\\6.x" );
		g_Mesh[Seven].InitialMesh( "model\\7.x" );
		g_Mesh[Eight].InitialMesh( "model\\8.x" );
		g_Mesh[Nine].InitialMesh( "model\\9.x" );
		g_Mesh[Empty].InitialMesh( "model\\Empty.x" );
		g_Mesh[Flag].InitialMesh( "model\\Flag.x" );
		g_Mesh[Mine].InitialMesh( "model\\Mine.x" );
		g_Mesh[Stena].InitialMesh(   "model\\Stena.x" );
		g_Mesh[Angle].InitialMesh(   "model\\Angle.x" );
		g_MeshWin.InitialMesh( "model\\Win.x" );	
		g_MeshLost.InitialMesh( "model\\Lost.x" );
		g_Exception.Init( g_pD3DDevice );
		g_Sky.InitialSky( g_pD3DDevice );
		g_DeviceInput.InitialInput( hwnd );					
		g_Diffuse.LoadShader( "shader\\Diffuse", g_pD3DDevice );
		g_ShaderSky.LoadShader( "shader\\Sky", g_pD3DDevice );
		
		while( !g_Exit )
		{
			g_DeviceInput.ScanInput( &g_Camera, &g_Field.m_Cell[0], &g_Field.m_Field[0] );				
			RenderingDirect3D( &g_Field.m_Cell[0], &g_Field.m_Field[0], g_pD3DDevice );				
			if ( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &Msg );
				DispatchMessage(  &Msg );
			}
		}		
	}
	g_Exception.Release();
	g_ShaderSky.Release();
	g_Diffuse.Release();
	g_MeshLost.Release();
	g_MeshWin.Release();
	g_MeshS.Release();
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




HRESULT CSky:: InitialSky( IDirect3DDevice9* D3DDevice )
{
	void *pBV;
	void *pBI;
	if ( !D3DDevice )
	{
		Log( "error init sky " );
		return E_FAIL;
	}
	m_pD3DDevice = D3DDevice;
	m_pVerBufSky   = 0; // ��������� �� ����� ������
	m_pBufIndexSky = 0; // ��������� �� ����� ������

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
	if ( FAILED( m_pD3DDevice -> CreateVertexBuffer( 4 * sizeof( CVertexFVF ), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBufSky, 0 ) ) ) // ������ ����� ������		
		return E_FAIL;
	if ( FAILED( m_pVerBufSky -> Lock( 0, sizeof( SkyVershin ), ( void** )&pBV, 0 ) ) ) // ������������
		return E_FAIL; 
	memcpy( pBV, SkyVershin, sizeof( SkyVershin ) ); // ����������� ������ � �������� � ����� ������
	m_pVerBufSky -> Unlock(); // ���������������

	if ( FAILED( m_pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,         // ������ ����� ������
		D3DPOOL_DEFAULT, &m_pBufIndexSky, 0 ) ) )
		return E_FAIL;
	if ( FAILED( m_pBufIndexSky -> Lock( 0, sizeof( SkyIndex ), ( void** )&pBI, 0 ) ) ) // ������������
		return E_FAIL; 
	memcpy( pBI, SkyIndex, sizeof( SkyIndex ) ); // ����������� ������ � �������� � ����� ������
	m_pBufIndexSky -> Unlock(); // ���������������	

	m_CubeTexture = 0;
	if ( FAILED( D3DXCreateCubeTextureFromFileEx( m_pD3DDevice, "model\\sky_cube_mipmap.dds", D3DX_DEFAULT, D3DX_FROM_FILE, 0, 
		D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, 0, 0, &m_CubeTexture )))
		Log( "error load sky texture" );

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
	// ����� ����������� �����	
	m_pD3DDevice -> SetStreamSource(0, m_pVerBufSky, 0, sizeof( CVertexFVF ) ); // ����� ������ ������ � ������� ������
	m_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // ��������������� ������ ������
	m_pD3DDevice -> SetIndices( m_pBufIndexSky );
	m_pD3DDevice -> SetTexture( 0, m_CubeTexture );
	// ������������� �������
	m_pD3DDevice -> SetVertexShader( Shader.m_pVertexShader );
	m_pD3DDevice -> SetPixelShader(  Shader.m_pPixelShader  );
	// ����� ����������
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

