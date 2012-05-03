#include "Init.h"


//IDirect3DDevice9* g_pD3DDevice  = 0; //Наше устройство


HRESULT CShader::LoadShader( std::string FileName, IDirect3DDevice9* pD3DDevice )
{
	if ( pD3DDevice )
	{
	LPD3DXBUFFER pErrors        = 0;
	LPD3DXBUFFER pShaderBuff    = 0;

	m_pVertexShader = 0;
	m_pPixelShader  = 0;
	m_pConstTableVS = 0;
	m_pConstTablePS = 0;	

	HRESULT hr;
	// вертексный шейдер
	std::string FileNameVS = FileName + std::string( ".vsh" );
	hr = D3DXCompileShaderFromFile( FileNameVS.c_str(), 0, 0, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &m_pConstTableVS );
	if ( hr != S_OK )
		Log( "error load vertex shader" );
	if ( pShaderBuff )
	{
		pD3DDevice->CreateVertexShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pVertexShader );
		pShaderBuff -> Release();
	}
	// пиксельный шейдер
	std::string FileNamePS = FileName + std::string( ".psh" );
	hr = D3DXCompileShaderFromFile( FileNamePS.c_str(), 0, 0, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &m_pConstTablePS );
	if ( hr != S_OK )
		Log( "error load pixel shader" );
	if ( pShaderBuff )
	{
		pD3DDevice->CreatePixelShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pPixelShader );
		pShaderBuff -> Release();
	}
	Log( "Load shader " );
	return S_OK;
	}
	else
	{
		Log( "Error pD3DDevice Load shader " );
		return E_FAIL;
	}
}

void CShader::Release()
{		
	if ( m_pVertexShader )
		m_pVertexShader->Release();
	if ( m_pPixelShader )
		m_pPixelShader->Release();
	if ( m_pConstTableVS )
		m_pConstTableVS->Release();
	if ( m_pConstTablePS )
		m_pConstTablePS->Release();
}

HRESULT CD3DDevice::IntialDirect3D( HWND hwnd )
{
	m_pDirect3D  = 0;
	m_pD3DDevice = 0;
	D3DPRESENT_PARAMETERS Direct3DParametr; // структура задающая парметры рендеринга 
	D3DDISPLAYMODE        Display; // возвращает параметры дисплея

	if ( ( m_pDirect3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == 0 ) // создаётся главный интерфейс
		return E_FAIL;	
	Log( "Initial Direct3D" );
	if ( FAILED( m_pDirect3D -> GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Display ) ) ) // получаем текущий формат дисплея
		return E_FAIL;

	ZeroMemory( &Direct3DParametr, sizeof( Direct3DParametr ) );
	Direct3DParametr.Windowed               = TRUE;					 // видео режим окно (или полноэкранный режим)
	Direct3DParametr.SwapEffect             = D3DSWAPEFFECT_DISCARD; // определяет обмен буферов
	Direct3DParametr.BackBufferFormat       = Display.Format;		 // формат поверхности заднего буфера
	Direct3DParametr.EnableAutoDepthStencil = TRUE;					 // включаем Z-буфер
	Direct3DParametr.AutoDepthStencilFormat = D3DFMT_D16;
	Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_DEFAULT; 
	/*/---------------------------полноэкранный режим--------------------
	Direct3DParametr.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
	Direct3DParametr.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	Direct3DParametr.BackBufferCount  = 3;
	Direct3DParametr.FullScreen_RefreshRateInHz = Display.RefreshRate;
	//------------------------------------------------------------------*/
	if ( FAILED( m_pDirect3D -> CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Direct3DParametr, &m_pD3DDevice ) ) ) // создаётся интерфейс устройства
		return E_FAIL;
	Log( "Initial CreateDevice Direct3D" );
	m_pD3DDevice -> SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );				//  режим отсечения включено и происходит по часовой стрелке
	m_pD3DDevice -> SetRenderState( D3DRS_LIGHTING, FALSE );					// запрещается работа со светом
	m_pD3DDevice -> SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );				// разрешает использовать Z-буфер
	m_pD3DDevice -> SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );            // включает альфа-канал
	m_pD3DDevice -> SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pD3DDevice -> SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pD3DDevice -> SetRenderState( D3DRS_AMBIENT, 0xffffffff );
	m_pD3DDevice -> SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR ); // фильтрация текстуры для плавности перехода

	return S_OK;
}

void CD3DDevice::Release()
{	
	if ( m_pD3DDevice )
		m_pD3DDevice -> Release();
	if ( m_pDirect3D )
		m_pDirect3D -> Release();	
};

void DrawMyText( IDirect3DDevice9* pD3DDevice, char* StrokaTexta, int x, int y, int x1, int y1, D3DCOLOR MyColor )
{
	RECT  Rec;
	HFONT hFont;
	ID3DXFont* pFont = 0; 
	hFont = CreateFont(30, 10, 0, 0, FW_NORMAL, FALSE, FALSE, 0, 1, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial");
	Rec.left   = x;
	Rec.top    = y;
	Rec.right  = x1;
	Rec.bottom = y1;
	D3DXCreateFont( pD3DDevice, 30, 10, FW_NORMAL, 0, FALSE, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial", &pFont );
	pFont->DrawText(0, StrokaTexta, -1, &Rec, DT_WORDBREAK, MyColor);
	if (pFont)
		pFont->Release();
}

void Blending( BLEND Blend, IDirect3DDevice9* D3DDevice )
{
	switch ( Blend )
	{
	case BLEND_DEFAULT:
		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );
		break;

	case BLEND_ALPHA:
		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
		D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		break;

	case BLEND_MUL:
		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
		D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		break;

	case BLEND_ADD:
		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
		D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;
	}
}

void Log( char* Str )
{
	FILE *FileLog = fopen( "log.txt", "a" );
	SYSTEMTIME st;
	GetLocalTime(&st);
	fprintf( FileLog, "|%d:%d:%d| ", st.wHour, st.wMinute, st.wSecond );
	fprintf( FileLog, Str );
	fprintf( FileLog, "\n" );
	fclose(  FileLog );
}

bool PointInTr( D3DXVECTOR3&  v1, D3DXVECTOR3&  v2, D3DXVECTOR3&  v3, D3DXVECTOR3&  n, D3DXVECTOR3&  ip )
{
	// проверяем, находится ли точка пересечения внутри треугольника.
	D3DXVECTOR3 a;
	D3DXVec3Cross( &a, &(v2 - v1), &(ip - v1) );
	if( D3DXVec3Dot( &a, &n ) <= 0) 
		return FALSE; 
	D3DXVec3Cross( &a, &(v3 - v2), &(ip - v2) );
	if( D3DXVec3Dot( &a, &n ) <= 0)
		return FALSE; 
	D3DXVec3Cross( &a, &(v1 - v3), &(ip - v3) );
	if( D3DXVec3Dot( &a, &n ) <= 0)
		return FALSE; 

	return true;
} 

bool Collision( ID3DXMesh* pMesh, D3DXVECTOR3& Pos, float Radius )
{
	bool Result = false;

	if ( pMesh )
	{
		CVertexFVF*  Triangle[3];
		short*       pIndices;
		CVertexFVF*  pVertices;		

		DWORD m_NumPolygons = pMesh->GetNumFaces();	

		pMesh->LockVertexBuffer( D3DLOCK_READONLY,( void** )&pVertices);
		pMesh->LockIndexBuffer(  D3DLOCK_READONLY,( void** )&pIndices );


		for( DWORD i = 0; i < m_NumPolygons; i++ )
		{
			Triangle[0] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка А i-того треугольника
			Triangle[1] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка B i-того треугольника
			Triangle[2] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка C i-того треугольника

			//тут можем работать с этим треугольником		
			D3DXVECTOR3 Normal = D3DXVECTOR3( Triangle[0]->nx, Triangle[0]->ny, Triangle[0]->nz );	// нормаль треугольника
			D3DXVec3Normalize( &Normal, &Normal );
			D3DXVECTOR3 V = -Normal;
			float D = -Normal.x * Triangle[0]->x - Normal.y * Triangle[0]->y - Normal.z * Triangle[0]->z;
			float k = -( Normal.x * Pos.x + Normal.y * Pos.y + Normal.z * Pos.z + D ) / ( Normal.x * V.x + Normal.y * V.y + Normal.z * V.z );
			D3DXVECTOR3 P = D3DXVECTOR3( k * V.x + Pos.x, k * V.y + Pos.y, k * V.z + Pos.z);
			// расстояние от камеры до точки столкновения 
			float Dist = D3DXVec3Length( &(D3DXVECTOR3( Pos.x - P.x, Pos.y - P.y, Pos.z - P.z ) ) );

			D3DXVECTOR3 V1 = D3DXVECTOR3( Triangle[0]->x, Triangle[0]->y, Triangle[0]->z );
			D3DXVECTOR3 V2 = D3DXVECTOR3( Triangle[1]->x, Triangle[1]->y, Triangle[1]->z );
			D3DXVECTOR3 V3 = D3DXVECTOR3( Triangle[2]->x, Triangle[2]->y, Triangle[2]->z );

			if ( PointInTr( V1, V2, V3, Normal, P ) )		
				if ( Dist < Radius )
				{
					Pos += Normal * ( Radius - Dist );
					Result = true;
				}
		}
		pMesh->UnlockIndexBuffer();
		pMesh->UnlockVertexBuffer();
	}
return Result;
}

