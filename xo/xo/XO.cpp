#include "CameraDevice.h"

extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}


using namespace std;

IDirect3DDevice9	   *g_pD3DDevice       = NULL; //Наше устройство
IDirect3DCubeTexture9  *CubeTexture;
IDirect3DTexture9      *pTextura001;

enum         NameShader { Sky , Diffuse, Mirror };
bool         g_Exit=false;
int          Field[3][3];
D3DXVECTOR4  Light( 0.0f, 1.0f, -1.0f, 1.0f );
D3DXMATRIX   MatrixView;
D3DXMATRIX   MatrixProjection;
bool         Wireframe = false;
FLOAT        Diffuse_intensity = 1.0f;
CameraDevice Camera;

struct CSphereObject
{
	float       Radius;
	D3DXVECTOR3 Centr;
    CSphereObject():Radius(5.0f), Centr(D3DXVECTOR3(0,0,0))
	{	 }
};

CSphereObject g_Sphere[3][3];

struct CVertexFVF
{
	FLOAT X,   Y,  Z;
	FLOAT nx, ny, nz;
	FLOAT tu, tv;
};

struct CMouseState
{
	LONG     lX;
	LONG     lY;
	LONG     lZ;
	BYTE     rgbButtons[8];
	CMouseState()
	{
		lX = 0;
		lY = 0;
		lZ = 0;
	}
};

struct CLuaScript
{
	lua_State    *m_luaVM;
	void         *m_FileBuffer;
	unsigned int  m_FileSize;
	bool          lua_dobuffer( lua_State* Lua, void const* Buffer, int Size );
	CLuaScript();
   ~CLuaScript();
};

bool CLuaScript::lua_dobuffer( lua_State* Lua, void const* Buffer, int Size )
{
	if ( !Size )
		return true;

	if ( luaL_loadbuffer( Lua, (char const*)Buffer, Size, 0 ) )
	{
		lua_pop( Lua, 1 );
		return false;
	}

	if ( lua_pcall( Lua, 0, LUA_MULTRET, 0 ) )
	{
		lua_pop( Lua, 1 );
		return false;
	}

	return true;
}

CLuaScript::CLuaScript()
{
	m_FileBuffer = 0;
	m_FileSize   = 0;
	FILE* const FO = fopen( "Rotate.lua", "rb" );
	if ( FO )
	{
		fseek(FO,0,SEEK_END);			// устанавливает указатель на конец файла
		m_FileSize   = ftell(FO);			// возвращает количество байт от начала до указателя         
		m_FileBuffer = malloc( m_FileSize );// возвращает указатель на захваченную память размером m_FileSize
		fseek(FO,0,SEEK_SET);			// устанавливает указатель на начало файла
		fread( m_FileBuffer, 1, m_FileSize, FO );// читает и записывает в память по 1 байту
		fclose(FO);
	}
	m_luaVM = lua_open();
	if ( m_luaVM == NULL ) 
		printf("Error Initializing lua\n");
	lua_dobuffer( m_luaVM, m_FileBuffer, m_FileSize );
	// инициализация стандартных библиотечных функции lua
	luaopen_base  ( m_luaVM );
	luaopen_table ( m_luaVM );
	luaopen_string( m_luaVM );
	luaopen_math  ( m_luaVM );
	luaopen_os    ( m_luaVM );
}

CLuaScript::~CLuaScript()
{

	free( m_FileBuffer );
	lua_close( m_luaVM );
}

CLuaScript CLuaScript;

class CD3DDevice
{
public:
	IDirect3D9			   *m_pDirect3D; // указатель на Главный интерфейс отвечающий за Direct3D
	IDirect3DTexture9      *pTextura002;
	IDirect3DTexture9      *m_pTexturaSky;
	IDirect3DTexture9	   *m_Texture;
	HRESULT                 IntialDirect3D(HWND hwnd);	
	HRESULT				    LoadTexture();
	void				    Release();
};

CD3DDevice   g_DeviceD3D;

HRESULT CD3DDevice::IntialDirect3D( HWND hwnd )
{
	m_pDirect3D  = NULL;
	g_pD3DDevice = NULL;
	D3DPRESENT_PARAMETERS Direct3DParametr; // структура задающая парметры рендеринга 
	D3DDISPLAYMODE        Display; // возвращает параметры дисплея

	if ( ( m_pDirect3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == NULL ) // создаётся главный интерфейс
		return E_FAIL;	
	if ( FAILED( m_pDirect3D -> GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Display ) ) ) // получаем текущий формат дисплея
		return E_FAIL;

	ZeroMemory( &Direct3DParametr, sizeof( Direct3DParametr ) );
	Direct3DParametr.Windowed               = TRUE;					 // видео режим окно (или полноэкранный режим)
	Direct3DParametr.SwapEffect             = D3DSWAPEFFECT_DISCARD; // определяет обмен буферов
	Direct3DParametr.BackBufferFormat       = Display.Format;		 // формат поверхности заднего буфера
	Direct3DParametr.EnableAutoDepthStencil = TRUE;					 // включаем Z-буфер
	Direct3DParametr.AutoDepthStencilFormat = D3DFMT_D16;
	Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_DEFAULT; // 60fps
	//Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE; //максимальное fps
	/*/---------------------------полноэкранный режим--------------------
	Direct3DParametr.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
	Direct3DParametr.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	Direct3DParametr.BackBufferCount  = 3;
	Direct3DParametr.FullScreen_RefreshRateInHz = Display.RefreshRate;
	//------------------------------------------------------------------*/
	if ( FAILED( m_pDirect3D -> CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
		                                      &Direct3DParametr, &g_pD3DDevice ) ) ) // создаётся интерфейс устройства
		return E_FAIL;	
	g_pD3DDevice -> SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );				//  режим отсечения включено и происходит по часовой стрелке
	g_pD3DDevice -> SetRenderState( D3DRS_LIGHTING, FALSE );					// запрещается работа со светом
	g_pD3DDevice -> SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );				// разрешает использовать Z-буфер
	g_pD3DDevice -> SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );            // включает альфа-канал
	g_pD3DDevice -> SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	g_pD3DDevice -> SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	g_pD3DDevice -> SetRenderState( D3DRS_AMBIENT, 0xffffffff );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR ); // фильтрация текстуры для плавности перехода
	
	return S_OK;
}

HRESULT	CD3DDevice::LoadTexture()
{
	pTextura001   = NULL;
	pTextura002   = NULL;
	m_pTexturaSky = NULL;
	m_Texture     = NULL;
	CubeTexture   = NULL;
	if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, "Textures/TexturaUV.jpg", &pTextura001 )))
		MessageBox( NULL, "Не удалось загрузить текстуру ", "", MB_OK );
	if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, "Textures/bricks003.jpg", &pTextura002 )))
		MessageBox( NULL, "Не удалось загрузить текстуру ", "", MB_OK );		
	if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, "Textures/Sky.jpg", &m_pTexturaSky )))
		MessageBox( NULL, "Не удалось загрузить текстуру ", "", MB_OK );
	if ( FAILED( D3DXCreateCubeTextureFromFileEx( g_pD3DDevice, "Textures/sky_cube_mipmap.dds", D3DX_DEFAULT, D3DX_FROM_FILE, 0, 
											      D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, 
												  D3DX_FILTER_NONE, 0, 0, 0, &CubeTexture )))
		MessageBox( NULL, "Не удалось загрузить текстуру SkyCube", "", MB_OK );
	return S_OK;
}

void CD3DDevice::Release()
{
	if ( CubeTexture != NULL )
		CubeTexture -> Release();
	if ( m_pTexturaSky != NULL )
		m_pTexturaSky -> Release();	
	if ( m_Texture   != NULL )
		m_Texture   -> Release();	
	if ( pTextura002 != NULL )
		pTextura002 -> Release();
	if ( pTextura001 != NULL )
		pTextura001 -> Release();
	if ( g_pD3DDevice != NULL )
		g_pD3DDevice -> Release();
	if ( m_pDirect3D != NULL )
		m_pDirect3D -> Release();
};

struct CShader
{
	IDirect3DPixelShader9  *pPixelShader [3];
	IDirect3DVertexShader9 *pVertexShader[3];
	ID3DXConstantTable     *pConstTableVS[3];
	ID3DXConstantTable     *pConstTablePS[3];
	int						m_CountShader;
	HRESULT                 InitialShader();
	~CShader();
};
CShader g_Shader;

HRESULT CShader::InitialShader()
{
	LPD3DXBUFFER pErrors        = NULL;
	LPD3DXBUFFER pShaderBuff    = NULL;
	m_CountShader = 2;
	for (int i = 0; i < m_CountShader; ++i)
	{	
		pVertexShader[i] = NULL;
		pPixelShader[i]  = NULL;
		pConstTableVS[i] = NULL;
		pConstTablePS[i] = NULL;
	}
	//-------------------------------SkyShader----------------------------
	// вертексный шейдер
	D3DXCompileShaderFromFile( "Sky.vsh", NULL, NULL, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
		&pShaderBuff, &pErrors, &pConstTableVS[Sky] );
	g_pD3DDevice->CreateVertexShader(( DWORD* )pShaderBuff->GetBufferPointer(), &pVertexShader[Sky]);
	pShaderBuff -> Release();
	// пиксельный шейдер
	D3DXCompileShaderFromFile( "Sky.psh", NULL, NULL, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
		&pShaderBuff, &pErrors, &pConstTablePS[Sky] );
	g_pD3DDevice->CreatePixelShader(( DWORD* )pShaderBuff->GetBufferPointer(), &pPixelShader[Sky]);
	pShaderBuff -> Release();
	//-------------------------------Diffuse----------------------------
	// вертексный шейдер
	D3DXCompileShaderFromFile( "Diffuse.vsh", NULL, NULL, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
		&pShaderBuff, &pErrors, &pConstTableVS[Diffuse] );
	g_pD3DDevice->CreateVertexShader(( DWORD* )pShaderBuff->GetBufferPointer(), &pVertexShader[Diffuse]);
	pShaderBuff -> Release();
	// пиксельный шейдер
	D3DXCompileShaderFromFile( "Diffuse.psh", NULL, NULL, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
		&pShaderBuff, &pErrors, &pConstTablePS[Diffuse] );
	g_pD3DDevice->CreatePixelShader(( DWORD* )pShaderBuff->GetBufferPointer(), &pPixelShader[Diffuse]);
	pShaderBuff -> Release();

	return S_OK;
}

CShader::~CShader()
{
	for (int i = 0; i < m_CountShader; ++i)
	{	
		if (pVertexShader[i] != NULL)
			pVertexShader[i] -> Release();
		if (pPixelShader[i] != NULL)
			pPixelShader[i] -> Release();
		if (pConstTableVS[i] != NULL)
			pConstTableVS[i] -> Release();
		if (pConstTablePS[i] != NULL)
			pConstTablePS[i] -> Release();
	}
}

class CInputDevice
{
public:
	LPDIRECTINPUT8			pInput;
	LPDIRECTINPUTDEVICE8    pKeyboard;
	LPDIRECTINPUTDEVICE8    pMouse;
	CMouseState             mouse;
	~CInputDevice();
	HRESULT                 InitialInput(HWND hwnd);
	bool                    ScanInput();
};

CInputDevice DeviceInput;


void DrawMyText(IDirect3DDevice9 *g_pD3DDevice, char* StrokaTexta, int x, int y, int x1, int y1, D3DCOLOR MyColor)
{
	RECT  Rec;
	HFONT hFont;
	ID3DXFont *pFont = NULL; 
	hFont = CreateFont(30, 10, 0, 0, FW_NORMAL, FALSE, FALSE, 0, 1, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial");
	Rec.left   = x;
	Rec.top    = y;
	Rec.right  = x1;
	Rec.bottom = y1;
	D3DXCreateFont( g_pD3DDevice, 30, 10, FW_NORMAL, 0, FALSE, 0, 0, 0, DEFAULT_PITCH | FF_MODERN, "Arial", &pFont );
	pFont->DrawText(NULL, StrokaTexta, -1, &Rec, DT_WORDBREAK, MyColor);
	if (pFont != NULL)
		pFont -> Release();
}

struct CSky
{
	IDirect3DVertexBuffer9 *m_pVerBufSky;
	IDirect3DIndexBuffer9  *m_pBufIndexSky;
	HRESULT                 InitialSky();
	~CSky();
};

CSky g_Sky;

HRESULT CSky::InitialSky()
{
	void *pBV;
	void *pBI;

	m_pVerBufSky   = NULL; // указатель на буфер вершин
	m_pBufIndexSky = NULL; // указатель на буфер вершин

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
		                                             D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVerBufSky, NULL ) ) )
		return E_FAIL;
	if ( FAILED( m_pVerBufSky -> Lock( 0, sizeof( SkyVershin ), ( void** )&pBV, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBV, SkyVershin, sizeof( SkyVershin ) ); // копирование данных о вершинах в буфер вершин
	m_pVerBufSky -> Unlock(); // разблокирование

	if ( FAILED( g_pD3DDevice -> CreateIndexBuffer( 6 * sizeof( short ), 0, D3DFMT_INDEX16,         // создаём буфер вершин
		                                            D3DPOOL_DEFAULT, &m_pBufIndexSky, NULL ) ) )
		return E_FAIL;
	if ( FAILED( m_pBufIndexSky -> Lock( 0, sizeof( SkyIndex ), ( void** )&pBI, 0 ) ) ) // Блокирование
		return E_FAIL; 
	memcpy( pBI, SkyIndex, sizeof( SkyIndex ) ); // копирование данных о вершинах в буфер вершин
	m_pBufIndexSky -> Unlock(); // разблокирование	

	return S_OK;
}

CSky::~CSky()
{
	if ( m_pBufIndexSky != NULL )
		m_pBufIndexSky -> Release();
	if ( m_pVerBufSky != NULL )
		m_pVerBufSky -> Release();
}

class CMesh3D
{
public:
	ID3DXMesh              *m_pMesh;
	D3DMATERIAL9           *m_pMeshMaterial;
	IDirect3DTexture9     **m_pMeshTextura;
	DWORD                   m_TexturCount; 
	IDirect3DVertexBuffer9 *m_VertexBuffer;
	IDirect3DIndexBuffer9  *m_IndexBuffer;
	DWORD 					m_SizeFVF;
	float                   m_Alpha;
	HRESULT                 InitialMesh(LPCSTR Name);
	void					Release();
	void                    DrawMyMesh();
	void					SetMatrixWorld(D3DXMATRIX * Matrix);
	void					SetMatrixView(D3DXMATRIX * Matrix);
	void					SetMatrixProjection(D3DXMATRIX * Matrix);
private:
	D3DXMATRIX              m_MatrixWorld;
	D3DXMATRIX              m_MatrixView;
	D3DXMATRIX              m_MatrixProjection;
};

CMesh3D  g_MeshS;
CMesh3D  g_MeshX;
CMesh3D  g_MeshO;

HRESULT CMesh3D::InitialMesh(LPCSTR Name)
{
	m_pMesh         = NULL;
	m_pMeshMaterial = NULL;
	m_pMeshTextura  = NULL;
	m_SizeFVF       = 0;
	m_Alpha         = 1.0f;	
	ID3DXBuffer *pMaterialBuffer  = NULL;
	if (FAILED(D3DXLoadMeshFromX(Name, D3DXMESH_SYSTEMMEM, g_pD3DDevice, NULL, &pMaterialBuffer, NULL, &m_TexturCount, &m_pMesh)))
	{
		MessageBox(NULL, "Не удалось загрузить X-file", "", MB_OK);
		return E_FAIL;
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
		if ( FAILED( D3DXCreateTextureFromFile( g_pD3DDevice, D3DXMeshMaterial[i].pTextureFilename, &m_pMeshTextura[i] )))
		{
			//MessageBox(NULL, "Не удалось загрузить текстуры на модель", "", MB_OK);
			m_pMeshTextura[i] = NULL;
		}
	}
	// Уничтожаем буфер материала
	pMaterialBuffer->Release();

	return S_OK;
}

void CMesh3D::SetMatrixWorld(D3DXMATRIX * Matrix)
{
	m_MatrixWorld = *Matrix;
}

void CMesh3D::SetMatrixView(D3DXMATRIX * Matrix)
{
	m_MatrixView = *Matrix;
}

void CMesh3D::SetMatrixProjection(D3DXMATRIX * Matrix)
{
	m_MatrixProjection = *Matrix;
}
void CMesh3D::DrawMyMesh()
{
	D3DXMATRIX  wvp;
	
	wvp = m_MatrixWorld * m_MatrixView * m_MatrixProjection;

	g_Shader.pConstTableVS[Diffuse] -> SetMatrix( g_pD3DDevice, "mat_mvp",   &wvp );
	g_Shader.pConstTableVS[Diffuse] -> SetMatrix( g_pD3DDevice, "mat_world", &m_MatrixWorld );
	g_Shader.pConstTableVS[Diffuse] -> SetVector( g_pD3DDevice, "vec_light", &Light );
	g_Shader.pConstTablePS[Diffuse] -> SetFloat(  g_pD3DDevice, "diffuse_intensity", Diffuse_intensity );	
	g_Shader.pConstTablePS[Diffuse] -> SetFloat(  g_pD3DDevice, "Alpha", m_Alpha );	
	
	// устанавливаем шейдеры
	g_pD3DDevice->SetVertexShader( g_Shader.pVertexShader[Diffuse] );
	g_pD3DDevice->SetPixelShader(  g_Shader.pPixelShader [Diffuse] );
	
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

void CMesh3D::Release()
{
	if ( m_IndexBuffer != NULL )
		m_IndexBuffer->Release();
	if ( m_VertexBuffer != NULL )
		m_VertexBuffer->Release();
	if ( m_pMeshMaterial != NULL )
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
	if ( m_pMesh != NULL )
		m_pMesh -> Release();
}

bool CalcPickingRay( HWND hwnd , D3DXMATRIX *MatrixView, D3DXMATRIX *MatrixProjection, int ArrX, int ArrY)
{
	float px = 0.0f;
	float py = 0.0f;
	D3DVIEWPORT9 ViewPort;
	RECT ClientRec;
	POINT PosMouse;
	//GetWindowRect(hwnd,&rect);
	GetClientRect(hwnd, &ClientRec);
	ClientToScreen(hwnd, (LPPOINT)&ClientRec);
	GetCursorPos( &PosMouse );
	int x = PosMouse.x - ClientRec.left;
	int y = PosMouse.y - ClientRec.top;
	g_pD3DDevice->GetViewport( &ViewPort );
	
	px = (  2.0f * x / ViewPort.Width  - 1.0f) / MatrixProjection->_11;
	py = ( -2.0f * y / ViewPort.Height + 1.0f) / MatrixProjection->_22;	
	
	D3DXVECTOR3 Direction = D3DXVECTOR3( px, py, 1.0f );

	D3DXMATRIX MatV;
	D3DXMatrixInverse( &MatV, NULL, MatrixView ); 
	D3DXVECTOR3 PosView = D3DXVECTOR3( MatV._41, MatV._42, MatV._43 ); //   извлечь координаты камеры из матрицы вида	
	D3DXVec3TransformNormal( &Direction, &Direction, &MatV );
	D3DXVec3Normalize( &Direction, &Direction );
	

	D3DXVECTOR3 v =  PosView - g_Sphere[ArrX][ArrY].Centr;
	float b = 2.0f * D3DXVec3Dot( &Direction, &v );
	float c = D3DXVec3Dot( &v, &v ) - g_Sphere[ArrX][ArrY].Radius * g_Sphere[ArrX][ArrY].Radius ;
	// Находим дискриминант
	float discriminant = (b * b) - (4.0f * c);
	// Проверяем на мнимые числа
	if ( discriminant < 0.0f )
		return false;
	discriminant = sqrtf(discriminant);
	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;
	// Если есть решение >= 0, луч пересекает сферу
	if ( ( s0 >= 0.0f ) || ( s1 >= 0.0f ) )
		return true;
	
	return false;
}

void RenderingDirect3D(HWND hwnd)
{
	D3DXMATRIX  MatrixWorld, MatrixWorldX, MatrixWorldY, MatrixWorldZ;
	D3DXMATRIX  tmp;
	char        str[50];
	D3DXVECTOR4 Scale( tan( D3DX_PI / 8 * (FLOAT)Height / Width), tan( D3DX_PI / 8 * (FLOAT)Height / Width  ), 1.0f, 1.0f );
	//----------------------------------------------режим каркаса-------------------------------
	if ( Wireframe )
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		g_pD3DDevice -> SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
	//------------------------------------------------------------------------------------------

	UINT  Time  = timeGetTime() % 20000;
	FLOAT Angle = Time * (2.0f * D3DX_PI) / 20000.0f;
	
	MatrixView       = Camera.m_View;
	MatrixProjection = Camera.m_Proj;

	if ( g_pD3DDevice == NULL )
		return;
	
	g_pD3DDevice -> Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);// очистка заднего буфера
	g_pD3DDevice -> BeginScene(); // начало рендеринга

	//------------------------------------------Render Sky----------------------------------------
	g_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, false );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ); // фильтрация текстуры для плавности перехода
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ); // фильтрация текстуры для плавности перехода
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP ); // фильтрация текстуры для плавности перехода

	D3DXMatrixTranslation( &MatrixWorld, 1.0f, 1.0f, 1.0f );
	tmp = MatrixWorld * MatrixView * MatrixProjection;
	g_Shader.pConstTableVS[Sky] -> SetMatrix( g_pD3DDevice, "mat_mvp",   &tmp );
	g_Shader.pConstTableVS[Sky] -> SetVector( g_pD3DDevice, "vec_light", &Light );
	g_Shader.pConstTableVS[Sky] -> SetVector( g_pD3DDevice, "scale",     &Scale );
	g_Shader.pConstTablePS[Sky] -> SetMatrix( g_pD3DDevice, "mat_view",  &MatrixView );
	// здесь перерисовка сцены	
	g_pD3DDevice -> SetStreamSource(0, g_Sky.m_pVerBufSky, 0, sizeof( CVertexFVF ) ); // связь буфера вершин с потоком данных
	g_pD3DDevice -> SetFVF( D3DFVF_CUSTOMVERTEX ); // устанавливается формат вершин
	g_pD3DDevice -> SetIndices( g_Sky.m_pBufIndexSky );
	g_pD3DDevice -> SetTexture( 0, CubeTexture );
	// устанавливаем шейдеры
	g_pD3DDevice -> SetVertexShader( g_Shader.pVertexShader[Sky] );
	g_pD3DDevice -> SetPixelShader(  g_Shader.pPixelShader [Sky] );
	// вывод примитивов
	//g_pD3DDevice -> DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2 );

	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetSamplerState( 0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	g_pD3DDevice -> SetRenderState(  D3DRS_ZENABLE, true );
	//------------------------------------------Render Mesh----------------------------------------
	
	//------------------Setka--------------
	D3DXMatrixRotationY(   &MatrixWorld, 0 );
	g_MeshS.SetMatrixWorld( &MatrixWorld );
	g_MeshS.SetMatrixView(  &MatrixView );
	g_MeshS.SetMatrixProjection( &MatrixProjection );
	g_MeshS.DrawMyMesh();
	for ( int y = 0; y < 3; ++y )
		for ( int x = 0; x < 3; ++x )
		{
			if ( Field[x][y] == 1 )
			{
				//--------------------X-------------------
				D3DXMatrixRotationX(   &MatrixWorldY, Angle );
				D3DXMatrixTranslation( &MatrixWorldX, ( x * 16 - 16 ), ( 16 - y * 16 ), 0 );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_MeshX.SetMatrixWorld( &MatrixWorld );
				g_MeshX.SetMatrixView( &MatrixView );
				g_MeshX.SetMatrixProjection( &MatrixProjection );
				g_MeshX.m_Alpha = 1.0f;
				g_MeshX.DrawMyMesh();
			}
			if ( Field[x][y] == 0 )
			{		
				//--------------------O-------------------
				D3DXMatrixRotationY(   &MatrixWorldY, -Angle );
				D3DXMatrixTranslation( &MatrixWorldX, ( x * 16 - 16 ), ( 16 - y * 16  ), 0 );
				D3DXMatrixMultiply(&MatrixWorld, &MatrixWorldY, &MatrixWorldX);
				g_MeshO.SetMatrixWorld( &MatrixWorld );
				g_MeshO.SetMatrixView( &MatrixView );
				g_MeshO.SetMatrixProjection( &MatrixProjection );
				g_MeshO.m_Alpha = 1.0f;
				g_MeshO.DrawMyMesh();
			}	
			if ( ( Field[x][y] == 10 ) && ( CalcPickingRay( hwnd, &MatrixView, &MatrixProjection, x, y ) ) )
			{
				//--------------------X-------------------
				D3DXMatrixRotationX(   &MatrixWorldY, Angle );
				D3DXMatrixTranslation( &MatrixWorldX, ( x * 16 - 16 ), ( 16 - y * 16 ), 0 );
				MatrixWorld = MatrixWorldY * MatrixWorldX;
				g_MeshX.SetMatrixWorld( &MatrixWorld );
				g_MeshX.SetMatrixView( &MatrixView );
				g_MeshX.SetMatrixProjection( &MatrixProjection );
				g_MeshX.m_Alpha = 0.3f;
				g_MeshX.DrawMyMesh();
			}
		}
		sprintf(str, "%d", (int)CalcPickingRay( hwnd, &MatrixView, &MatrixProjection, 2, 0 ));
		if ( CalcPickingRay( hwnd, &MatrixView, &MatrixProjection, 2, 0 ) )
		    DrawMyText(g_pD3DDevice, str, 10, 10, 500, 700, D3DCOLOR_ARGB(250, 250, 250,50));
	//------------------------------------------LuaScript----------------------------------------
		/*
	lua_getglobal( CLuaScript.m_luaVM, "Update" );
	if ( lua_pcall( CLuaScript.m_luaVM, 0, 0, 0 ) && lua_tostring( CLuaScript.m_luaVM, -1 ) )
		lua_pop( CLuaScript.m_luaVM, 1 );

	lua_getglobal( CLuaScript.m_luaVM, "blablabla" );
	Diffuse_intensity	=	(float)lua_tonumber( CLuaScript.m_luaVM, -1 );
	lua_pop( CLuaScript.m_luaVM, 1 );

	lua_getglobal( CLuaScript.m_luaVM, "Rotate" );
	if ( lua_pcall( CLuaScript.m_luaVM, 0, 0, 0 ) && lua_tostring( CLuaScript.m_luaVM, -1 ) )
		lua_pop( CLuaScript.m_luaVM, 1 );

	lua_getglobal( CLuaScript.m_luaVM, "Ang" );
	Angle	=	(float)lua_tonumber( CLuaScript.m_luaVM, -1 );
	lua_pop( CLuaScript.m_luaVM, 1 );
	*/
	
	

	g_pD3DDevice -> EndScene();
	g_pD3DDevice -> Present(NULL, NULL, NULL, NULL); // вывод содержимого заднего буфера в окно
}

LONG WINAPI WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam)
{	
	switch (Message)
	{
	case WM_CLOSE:
		g_Exit = true;
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
			g_Exit = true;
		break;
	}
	return DefWindowProc(hwnd, Message, wparam, lparam);
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





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine, int       nCmdShow)
{
	HWND		hwnd;
	MSG			msg;
	WNDCLASS	w;	
	CFps        g_fps;
	D3DVIEWPORT9 vp;

	memset(&w,0,sizeof(WNDCLASS));
	w.style         = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc   = WndProc;
	w.hInstance     = hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	w.lpszClassName = "My Class";
	w.hIcon         = LoadIcon(NULL,IDI_QUESTION);//стандартная иконка приложения Win API 	
	RegisterClass(&w);
	hwnd = CreateWindow("My Class", "Тест", WS_SYSMENU | WS_MINIMIZEBOX,
		                250, 150, Width+6, Height+28, NULL, NULL, hInstance, NULL);	
	char str[8];
	//memset(Field,0,sizeof(int)*9);
	//ZeroMemory(Field, sizeof(Field));
	srand(1000);
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
		{
			Field[x][y] = 10; //rand() % 2;	
			g_Sphere[x][y].Centr = D3DXVECTOR3( ( x * 16 - 16 ), ( 16 - y * 16 ), 0 );
		}
		Field[2][0] = 10;
	if ( SUCCEEDED(g_DeviceD3D.IntialDirect3D(hwnd) ) )
	{	
		if ( SUCCEEDED( g_DeviceD3D.LoadTexture() ) )
		{			
			ShowWindow(hwnd,nCmdShow);
			ZeroMemory(&msg, sizeof(msg));
			g_MeshS.InitialMesh("Setka.x");
			g_MeshO.InitialMesh("O.x");
			g_MeshX.InitialMesh("X.x");	
			g_Sky.InitialSky();
			DeviceInput.InitialInput(hwnd);					
			g_Shader.InitialShader();
			g_fps.m_last_tick = GetTickCount();
			while( !g_Exit )
			{						
				g_pD3DDevice -> GetViewport(&vp);
				//sprintf(str, "FPS=%d", g_fps.Fps());
				//SetWindowText(hwnd,str);
				DeviceInput.ScanInput();
				RenderingDirect3D( hwnd );
				if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage(  &msg );
				}
			}						
		}
	}	
	g_MeshS.Release();
	g_MeshX.Release();
	g_MeshO.Release();
	g_DeviceD3D.Release();
	return 0;
}








HRESULT CInputDevice::InitialInput(HWND hwnd)
{	
	pInput    = NULL;
	pKeyboard = NULL;
	pMouse    = NULL;
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize		= sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize	= sizeof( DIPROPHEADER );
	dipdw.diph.dwObj		= 0;
	dipdw.diph.dwHow		= DIPH_DEVICE;
	dipdw.dwData			= 32;
	if (FAILED(DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pInput, NULL)))
		return E_FAIL;

	if FAILED(pInput -> CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)) //создание устройства клавиатура
		return E_FAIL;
	if FAILED(pKeyboard -> SetDataFormat(&c_dfDIKeyboard))
		return E_FAIL;
	if FAILED(pKeyboard -> SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))
		return E_FAIL;
	if( FAILED(pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
		return E_FAIL;
	if FAILED(pKeyboard -> Acquire())
		return E_FAIL;

	if FAILED(pInput -> CreateDevice(GUID_SysMouse, &pMouse, NULL)) // создание устройства мышь
		return E_FAIL;	
	if FAILED(pMouse -> SetDataFormat(&c_dfDIMouse2))
		return E_FAIL;	
	if FAILED(pMouse -> SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))
		return E_FAIL;	
	if FAILED(pMouse -> Acquire())
		return E_FAIL;

	return S_OK;
}

bool CInputDevice::ScanInput()
{	
	char     keyboard[256];     
	LONG     dx, dy, dz;

	if FAILED( pKeyboard -> GetDeviceState(sizeof(keyboard), (LPVOID)&keyboard) )
	{
		pKeyboard -> Acquire();
		return FALSE;
	}
	if ( KEYDOWN(keyboard, DIK_F4) )
		Wireframe = !Wireframe;
	if ( KEYDOWN(keyboard, DIK_RIGHT) || KEYDOWN(keyboard, DIK_D))
		Camera.MoveRight();
	if ( KEYDOWN(keyboard, DIK_LEFT) || KEYDOWN(keyboard, DIK_A))
		Camera.MoveLeft();
	if ( KEYDOWN(keyboard, DIK_UP) || KEYDOWN(keyboard, DIK_W))     
		Camera.MoveForv();
	if ( KEYDOWN(keyboard, DIK_DOWN) || KEYDOWN(keyboard, DIK_S))
		Camera.MoveBack();

	if FAILED( pMouse -> GetDeviceState( sizeof( CMouseState ), (LPVOID)&mouse ) )
	{
		pMouse -> Acquire();
		return FALSE;
	}
	dx = mouse.lX;
	dy = mouse.lY;
	dz = mouse.lZ;

	if (mouse.rgbButtons[LEFT_BUTTON]&0x80)
		Camera.MoveBack();
	/*
	if ( (dx < 0) )  
		Camera.MouseRotateLeft();
	if ( (dx > 0) )  
		Camera.MouseRotateRight();
	if ( (dy > 0) )  
		Camera.MouseRotateUp();
	if ( (dy < 0) )  
		Camera.MouseRotateDown();
	if ( (dz > 0) )  
		Camera.MoveForv();
	if ( (dz < 0) )  
		Camera.MoveBack();
	*/
	return TRUE;
}

CInputDevice::~CInputDevice()
{
	if (pInput)
	{
		if (pKeyboard)
		{
			if (pKeyboard != NULL)
				pKeyboard -> Unacquire();
			if (pKeyboard != NULL)
				pKeyboard -> Release();
			pKeyboard = NULL;
		}
		if (pMouse)
		{
			if (pMouse != NULL)
				pMouse -> Unacquire();
			if (pMouse != NULL)
				pMouse -> Release();
			pMouse = NULL;
		}
		if (pInput != NULL)
			pInput -> Release();
		pInput = NULL;
	}
}
