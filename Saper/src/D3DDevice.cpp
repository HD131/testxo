#include "D3DDevice.h"


IDirect3DDevice9* g_pD3DDevice  = 0; //Наше устройство


HRESULT CD3DDevice::InitialShader()
{
	LPD3DXBUFFER pErrors        = 0;
	LPD3DXBUFFER pShaderBuff    = 0;
	m_Light                     = D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 1.0f );
	m_Diffuse_intensity         = 1.0f;

	for (int i = 0; i < MaxShader; ++i)
	{	
		m_pVertexShader[i] = 0;
		m_pPixelShader[i]  = 0;
		m_pConstTableVS[i] = 0;
		m_pConstTablePS[i] = 0;
	}
	//-------------------------------SkyShader----------------------------
	// вертексный шейдер
	D3DXCompileShaderFromFile( "shader//Sky.vsh", 0, 0, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
								&pShaderBuff, &pErrors, &m_pConstTableVS[Sky] );
	if ( pShaderBuff )
	{
		g_pD3DDevice->CreateVertexShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pVertexShader[Sky]);
		pShaderBuff -> Release();
	}
	// пиксельный шейдер
	D3DXCompileShaderFromFile( "shader//Sky.psh", 0, 0, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
								&pShaderBuff, &pErrors, &m_pConstTablePS[Sky] );
	if ( pShaderBuff )
	{
		g_pD3DDevice->CreatePixelShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pPixelShader[Sky]);
		pShaderBuff -> Release();
	}
	//-------------------------------Diffuse----------------------------
	// вертексный шейдер
	D3DXCompileShaderFromFile( "shader//Diffuse.vsh", 0, 0, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
								&pShaderBuff, &pErrors, &m_pConstTableVS[Diffuse] );
	if ( pShaderBuff )
	{
		g_pD3DDevice->CreateVertexShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pVertexShader[Diffuse]);
		pShaderBuff -> Release();
	}
	// пиксельный шейдер
	D3DXCompileShaderFromFile( "shader//Diffuse.psh", 0, 0, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3,
								&pShaderBuff, &pErrors, &m_pConstTablePS[Diffuse] );
	if ( pShaderBuff )
	{
		g_pD3DDevice->CreatePixelShader(( DWORD* )pShaderBuff->GetBufferPointer(), &m_pPixelShader[Diffuse]);
		pShaderBuff -> Release();
	}
	return S_OK;
}

HRESULT CD3DDevice::IntialDirect3D( HWND hwnd , FILE* FileLog)
{
	m_pDirect3D  = 0;
	g_pD3DDevice = 0;
	D3DPRESENT_PARAMETERS Direct3DParametr; // структура задающая парметры рендеринга 
	D3DDISPLAYMODE        Display; // возвращает параметры дисплея

	if ( ( m_pDirect3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == 0 ) // создаётся главный интерфейс
		return E_FAIL;	
	if ( FileLog ) 
		fprintf( FileLog, "Initial Direct3D\n" );
	if ( FAILED( m_pDirect3D -> GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Display ) ) ) // получаем текущий формат дисплея
		return E_FAIL;

	ZeroMemory( &Direct3DParametr, sizeof( Direct3DParametr ) );
	Direct3DParametr.Windowed               = TRUE;					 // видео режим окно (или полноэкранный режим)
	Direct3DParametr.SwapEffect             = D3DSWAPEFFECT_DISCARD; // определяет обмен буферов
	Direct3DParametr.BackBufferFormat       = Display.Format;		 // формат поверхности заднего буфера
	Direct3DParametr.EnableAutoDepthStencil = TRUE;					 // включаем Z-буфер
	Direct3DParametr.AutoDepthStencilFormat = D3DFMT_D16;
	Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_DEFAULT; 

	if ( FAILED( m_pDirect3D -> CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
											  &Direct3DParametr, &g_pD3DDevice ) ) ) // создаётся интерфейс устройства
		return E_FAIL;
	if ( FileLog ) 
		fprintf( FileLog, "Initial CreateDevice Direct3D\n" );
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

HRESULT	CD3DDevice::LoadTexture( FILE* FileLog )
{	
	m_CubeTexture = 0;

	if ( FAILED( D3DXCreateCubeTextureFromFileEx( g_pD3DDevice, "model//sky_cube_mipmap.dds", D3DX_DEFAULT, D3DX_FROM_FILE, 0, 
		                                          D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, 0, 0, &m_CubeTexture )))
		if ( FileLog ) 
			fprintf( FileLog, "error load sky texture\n" );
	return S_OK;
}

void CD3DDevice::Release()
{
	for (int i = 0; i < MaxShader; ++i)
	{	
		if ( m_pVertexShader[i] )
			m_pVertexShader[i] -> Release();
		if ( m_pPixelShader[i] )
			m_pPixelShader[i] -> Release();
		if ( m_pConstTableVS[i] )
			m_pConstTableVS[i] -> Release();
		if ( m_pConstTablePS[i] )
			m_pConstTablePS[i] -> Release();
	}
	if ( m_CubeTexture )
		m_CubeTexture -> Release();
	if ( g_pD3DDevice )
		g_pD3DDevice -> Release();
	if ( m_pDirect3D )
		m_pDirect3D -> Release();	
};

bool CLuaScript::lua_dobuffer( lua_State* Lua, void const* Buffer, int Size )
{
	if ( !Size )
		return true;
	if ( luaL_loadbuffer( Lua, (char const*)Buffer, Size, 0 ) )
	{
		char const* ErrorMsg = lua_tostring( Lua, -1 );
		lua_pop( Lua, 1 );
		if ( m_FileLog ) 
			fprintf( m_FileLog, "%s\n",ErrorMsg );
		return false;
	}

	if ( lua_pcall( Lua, 0, LUA_MULTRET, 0 ) )
	{
		char const* ErrorMsg = lua_tostring( Lua, -1 );
		lua_pop( Lua, 1 );
		if ( m_FileLog ) 
			fprintf( m_FileLog, "%s\n", ErrorMsg );
		return false;
	}
	if ( m_FileLog ) 
		fprintf( m_FileLog, "Initial Script \n" );	
	return true;
}

CLuaScript::CLuaScript( FILE* FileLog )
{
	m_FileBuffer = 0;
	m_FileSize   = 0;
	m_FileLog    = FileLog;
	FILE* const FO = fopen( "CheckComputer.lua", "rb" );
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
	if ( m_luaVM == 0 ) 
		if ( m_FileLog ) 
			fprintf( m_FileLog, "Error Initializing lua\n" );

	// инициализация стандартных библиотечных функции lua
	luaopen_base  ( m_luaVM );
	luaopen_table ( m_luaVM );
	luaopen_string( m_luaVM );
	luaopen_math  ( m_luaVM );
	luaopen_os    ( m_luaVM );


	lua_dobuffer( m_luaVM, m_FileBuffer, m_FileSize );
}

CLuaScript::~CLuaScript()
{

	free( m_FileBuffer );
	lua_close( m_luaVM );
}
