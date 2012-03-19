#include "D3DDevice.h"
#include "CameraDevice.h"
#include "InputDevice.h"
#include "Field.h"
#include "Mesh.h"
#include <vector>

extern IDirect3DDevice9* g_pD3DDevice;

struct CSky
{
	IDirect3DVertexBuffer9* m_pVerBufSky;
	IDirect3DIndexBuffer9*  m_pBufIndexSky;
	HRESULT                 InitialSky();
	void                    Release();
};


CD3DDevice   g_DeviceD3D;
CInputDevice g_DeviceInput;
CSky         g_Sky;
CMesh3D      g_MeshA;
CMesh3D      g_Mesh[11];
CMesh3D      g_MeshS;
CMesh3D		 g_MeshWin;
CMesh3D		 g_MeshLost;
CMesh3D      g_MeshStalemate;
CameraDevice g_Camera;
CField       g_Field( MaxField );
bool         g_Exit      = false;
bool		 g_Wireframe = false;


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

void RenderingDirect3D( CCell* Cell, int* Field )
{	
	const D3DXVECTOR4 Scale( tan( D3DX_PI / 8 * (FLOAT)Height / Width), tan( D3DX_PI / 8 * (FLOAT)Height / Width  ), 1.0f, 1.0f );
	//----------------------------------------------режим каркаса-------------------------------
	if ( g_Wireframe )
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------	
	float const Angle = timeGetTime() / 2000.0f;

	 D3DXMATRIX MatrixView       = g_Camera.m_View;
	 D3DXMATRIX MatrixProjection = g_Camera.m_Proj;

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
		g_DeviceD3D.m_pConstTableVS[Sky]->SetMatrix( g_pD3DDevice, "mat_mvp",   &tmp );
		g_DeviceD3D.m_pConstTableVS[Sky]->SetVector( g_pD3DDevice, "vec_light", &g_Light );
		g_DeviceD3D.m_pConstTableVS[Sky]->SetVector( g_pD3DDevice, "scale",     &Scale );
		g_DeviceD3D.m_pConstTableVS[Sky]->SetMatrix( g_pD3DDevice, "mat_view",  &MatrixView );
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
	D3DXMATRIX MatrixWorldX,MatrixWorldY,MatrixWorldZ;
	int t = ( MaxField - 1) / 2;
	//------------------Ограда--------------
	//----------
	D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
	D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( -1 - t ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

	D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
	D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( - t +  MaxField) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

	D3DXMatrixRotationY( &MatrixWorldY, 3.14f );
	D3DXMatrixTranslation( &MatrixWorldX, (- t +  MaxField ), 0, ( -1 - t ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

	D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
	D3DXMatrixTranslation( &MatrixWorldX, (- t +  MaxField ), 0, ( - t +  MaxField ) );
	MatrixWorld = MatrixWorldY * MatrixWorldX;
	g_MeshA.SetMatrixWorld( MatrixWorld );
	g_MeshA.SetMatrixView( MatrixView );
	g_MeshA.SetMatrixProjection( MatrixProjection );
	g_MeshA.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
	for ( int x = 1; x < MaxField - 1; ++x )
	{
		D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
		D3DXMatrixTranslation( &MatrixWorldX, ( -1 - t ), 0, ( x - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

		D3DXMatrixRotationY( &MatrixWorldY, 1.57f );
		D3DXMatrixTranslation( &MatrixWorldX, ( - t +  MaxField ), 0, ( x - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

		D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
		D3DXMatrixTranslation( &MatrixWorldX, ( x - t ), 0, ( -1 - t ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);

		D3DXMatrixRotationY( &MatrixWorldY, 0.0f );
		D3DXMatrixTranslation( &MatrixWorldX, ( x - t ), 0, ( - t +  MaxField ) );
		MatrixWorld = MatrixWorldY * MatrixWorldX;
		g_MeshS.SetMatrixWorld( MatrixWorld );
		g_MeshS.SetMatrixView( MatrixView );
		g_MeshS.SetMatrixProjection( MatrixProjection );
		g_MeshS.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
	}
	
	//-------------------------------------------------------------------------
	for ( int y = 0; y < MaxField; ++y )
		for ( int x = 0; x < MaxField; ++x )
		{
			if ( ( Cell[x*MaxField+y].m_Value == Mine ) && ( Field[x*MaxField+y] == -1 ) )
			{
				//--------------------Mine-------------------
				D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_Mesh[10].SetMatrixWorld( MatrixWorld );
				g_Mesh[10].SetMatrixView( MatrixView );
				g_Mesh[10].SetMatrixProjection( MatrixProjection );
				g_Mesh[10].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( Field[x*MaxField+y] == Empty ) 
			{		
				//--------------------Empty-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[0].SetMatrixWorld( MatrixWorld );
				g_Mesh[0].SetMatrixView( MatrixView );
				g_Mesh[0].SetMatrixProjection( MatrixProjection );
				g_Mesh[0].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( Field[x*MaxField+y] == Flag ) 
			{		
				//--------------------Flag-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[9].SetMatrixWorld( MatrixWorld );
				g_Mesh[9].SetMatrixView( MatrixView );
				g_Mesh[9].SetMatrixProjection( MatrixProjection );
				g_Mesh[9].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 1 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------1-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[1].SetMatrixWorld( MatrixWorld );
				g_Mesh[1].SetMatrixView( MatrixView );
				g_Mesh[1].SetMatrixProjection( MatrixProjection );
				g_Mesh[1].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 2 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------2-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[2].SetMatrixWorld( MatrixWorld );
				g_Mesh[2].SetMatrixView( MatrixView );
				g_Mesh[2].SetMatrixProjection( MatrixProjection );
				g_Mesh[2].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 3 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------3-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[3].SetMatrixWorld( MatrixWorld );
				g_Mesh[3].SetMatrixView( MatrixView );
				g_Mesh[3].SetMatrixProjection( MatrixProjection );
				g_Mesh[3].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 4 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------4-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[4].SetMatrixWorld( MatrixWorld );
				g_Mesh[4].SetMatrixView( MatrixView );
				g_Mesh[4].SetMatrixProjection( MatrixProjection );
				g_Mesh[4].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 5 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------5-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixRotationX(   &MatrixWorldZ, -Angle );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldZ * MatrixWorldX;
				g_Mesh[5].SetMatrixWorld( MatrixWorld );
				g_Mesh[5].SetMatrixView( MatrixView );
				g_Mesh[5].SetMatrixProjection( MatrixProjection );
				g_Mesh[5].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 6 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------6-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[6].SetMatrixWorld( MatrixWorld );
				g_Mesh[6].SetMatrixView( MatrixView );
				g_Mesh[6].SetMatrixProjection( MatrixProjection );
				g_Mesh[6].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 7 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------7-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[7].SetMatrixWorld( MatrixWorld );
				g_Mesh[7].SetMatrixView( MatrixView );
				g_Mesh[7].SetMatrixProjection( MatrixProjection );
				g_Mesh[7].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
			if ( ( Cell[x*MaxField+y].m_Value == 8 ) && ( Field[x*MaxField+y] == -1 ) )
			{		
				//--------------------8-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( y - t ), 0, ( x - t ) );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_Mesh[8].SetMatrixWorld( MatrixWorld );
				g_Mesh[8].SetMatrixView( MatrixView );
				g_Mesh[8].SetMatrixProjection( MatrixProjection );
				g_Mesh[8].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
		}
		POINT P = PickObject( &Cell[0] );
		char  str[50];
		sprintf(str, "%d          %d", P.x, P.y);		
		//DrawMyText(g_pD3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));	
		if ( P.x >= 0)
		{
			if ( Field[P.x*MaxField+P.y] == Empty )
			{
				//--------------------X-------------------
				D3DXMatrixRotationY( &MatrixWorldY, -1.57f );
				D3DXMatrixTranslation( &MatrixWorldX, ( P.y - t ), 0.2, ( P.x - t ) );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_Mesh[0].SetMatrixWorld( MatrixWorld );
				g_Mesh[0].SetMatrixView( MatrixView );
				g_Mesh[0].SetMatrixProjection( MatrixProjection );
				g_Mesh[0].DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			}
		}

		switch ( GameOver() )
		{
		case 1:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshWin.SetMatrixWorld( MatrixWorld );
			g_MeshWin.SetMatrixView( g_Camera.m_View );
			g_MeshWin.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshWin.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			break;
		case 2:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshLost.SetMatrixWorld( MatrixWorld );
			g_MeshLost.SetMatrixView( g_Camera.m_View );
			g_MeshLost.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshLost.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			break;
		case 3:
			D3DXMatrixTranslation( &MatrixWorld, 0, 0, -7 );		
			g_MeshStalemate.SetMatrixWorld( MatrixWorld );
			g_MeshStalemate.SetMatrixView( g_Camera.m_View );
			g_MeshStalemate.SetMatrixProjection( g_Camera.m_Proj );
			g_MeshStalemate.DrawMyMesh(g_DeviceD3D.m_pConstTableVS, g_DeviceD3D.m_pConstTablePS, g_DeviceD3D.m_pVertexShader, g_DeviceD3D.m_pPixelShader);
			break;
		}

		g_pD3DDevice -> EndScene();
		g_pD3DDevice -> Present( 0, 0, 0, 0 ); // вывод содержимого заднего буфера в окно
}

void CheckPC( lua_State* m_luaVM, CCell* Cell )
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

		//if ( Cell[x*MaxField+y].m_Value == 10 )			Cell[x*MaxField+y].m_Value = 0;			
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
	int          m_Count;
	int          m_FPS;
	int          m_LastTick;
	int          m_ThisTick;
	int			 Fps();

	CFps(): m_Count(0)
	{	}
};

int CFps::Fps()
{	
	m_ThisTick = GetTickCount();
	if ( m_ThisTick - m_LastTick >= 1000 )
	{
		m_LastTick = m_ThisTick;
		m_FPS      = m_Count;
		m_Count    = 0;
	}
	else m_Count++;
return m_FPS;
}


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine, int       nCmdShow)
{	
	MSG			 Msg;
	WNDCLASS	 w;	
	CFps         g_Fieldps;		

	// Запись лога в файл 
	FILE *FileLog = fopen( "log.txt", "w" );
	CLuaScript   LuaVM( FileLog );

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
			
			g_MeshS.InitialMesh( "model//Stena.x", FileLog );
			g_MeshA.InitialMesh( "model//Angle.x", FileLog );
			g_Mesh[0].InitialMesh( "model//Empty.x", FileLog );
			g_Mesh[1].InitialMesh( "model//1.x", FileLog );
			g_Mesh[2].InitialMesh( "model//2.x", FileLog );
			g_Mesh[3].InitialMesh( "model//3.x", FileLog );
			g_Mesh[4].InitialMesh( "model//4.x", FileLog );
			g_Mesh[5].InitialMesh( "model//5.x", FileLog );
			g_Mesh[6].InitialMesh( "model//6.x", FileLog );
			g_Mesh[7].InitialMesh( "model//7.x", FileLog );
			g_Mesh[8].InitialMesh( "model//8.x", FileLog );
			g_Mesh[9].InitialMesh( "model//Flag.x", FileLog );
			g_Mesh[10].InitialMesh( "model//Mine.x", FileLog );	
			g_MeshWin.InitialMesh( "model//Win.x", FileLog );	
			g_MeshLost.InitialMesh( "model//Lost.x", FileLog );
			g_MeshStalemate.InitialMesh( "model//Stalemate.x", FileLog );
			g_Sky.InitialSky();
			g_DeviceInput.InitialInput( hwnd, FileLog );					
			g_DeviceD3D.InitialShader();
			g_Fieldps.m_LastTick = GetTickCount();
			while( !g_Exit )
			{
				g_DeviceInput.ScanInput( &g_Camera, &g_Field.m_Cell[0], &g_Field.m_Field[0] );				
				RenderingDirect3D( &g_Field.m_Cell[0], &g_Field.m_Field[0]);				
				if ( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &Msg );
					DispatchMessage(  &Msg );
				}
			}						
		}
	}	
	g_MeshStalemate.Release();
	g_MeshLost.Release();
	g_MeshWin.Release();
	g_MeshS.Release();
	for (int i = 0; i < 11; ++i)
		if ( g_Mesh[i].m_pMesh )
			g_Mesh[i].Release();
		
	g_Sky.Release(); 
	g_DeviceInput.Release();
	g_DeviceD3D.Release();
	if ( FileLog )
		fclose( FileLog );
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

