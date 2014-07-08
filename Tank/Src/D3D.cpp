#include "D3D.h"

IDirect3DDevice9*   CD3DGraphic::m_pD3DDevice	= 0;	
IDirect3D9*			CD3DGraphic::m_pDirect3D	= 0; 

CD3DGraphic::CD3DGraphic() :
	FullScreen( true )
{
}

CD3DGraphic::~CD3DGraphic()
{
}

HRESULT CD3DGraphic::InitD3D( HWND hwnd )
{
	m_pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );

	if( !m_pDirect3D ) // создаётся главный интерфейс
	{
		Log( "Error Initial Direct3D" );
		return E_FAIL;
	}
	else
		Log( "Initial Direct3D" );

	// возвращает параметры дисплея
	D3DDISPLAYMODE Display;				

	if( FAILED( m_pDirect3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Display ) ) ) // получаем текущий формат дисплея
	{
		Log( "Error GetAdapterDisplayMode" );
		return E_FAIL;
	}

	// структура задающая парметры рендеринга 
	D3DPRESENT_PARAMETERS Direct3DParametr = {0};		
	//ZeroMemory( &Direct3DParametr, sizeof( Direct3DParametr ) );
	Direct3DParametr.BackBufferFormat		= D3DFMT_A8R8G8B8;					// формат пикселей
	Direct3DParametr.BackBufferCount		= 1;
	Direct3DParametr.MultiSampleType		= D3DMULTISAMPLE_NONE;
	Direct3DParametr.MultiSampleQuality		= 0;
	Direct3DParametr.Windowed               = FullScreen;						// видео режим окно (или полноэкранный режим)
	Direct3DParametr.SwapEffect             = D3DSWAPEFFECT_DISCARD;			// определяет обмен буферов
	Direct3DParametr.BackBufferFormat       = Display.Format;					// формат поверхности заднего буфера
	Direct3DParametr.EnableAutoDepthStencil = TRUE;								// включаем Z-буфер
	Direct3DParametr.AutoDepthStencilFormat = D3DFMT_D16;						// 16-разрядный буфер глубины
	Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;	// D3DPRESENT_INTERVAL_IMMEDIATE			D3DPRESENT_INTERVAL_DEFAULT

	// полноэкранный режим
	if( !FullScreen )
	{
		Direct3DParametr.BackBufferWidth			= GetSystemMetrics( SM_CXSCREEN );
		Direct3DParametr.BackBufferHeight			= GetSystemMetrics( SM_CYSCREEN );
		Direct3DParametr.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;//Display.RefreshRate;
	}
	
	HRESULT hr = m_pDirect3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Direct3DParametr, &m_pD3DDevice );

	if( FAILED( hr ) ) // создаётся интерфейс устройства
	{
		Log( "Error Create Device Direct3D HARDWARE" );

		hr = m_pDirect3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &Direct3DParametr, &m_pD3DDevice );

		if( FAILED( hr ) )
		{
			Log( "Error Create Device Direct3D SOFTWARE" );
			Release();
			return E_FAIL;
		}
	}
	else
		Log( "Create Device Direct3D" );

	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );				//  режим отсечения включено и происходит по часовой стрелке
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING,			FALSE );					// запрещается работа со светом
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );				// разрешает использовать Z-буфер
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE );						// включает альфа-канал
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA );
	m_pD3DDevice->SetRenderState( D3DRS_AMBIENT,			0xffffffff );

	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );				// фильтрация текстуры для плавности перехода
	m_pD3DDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );				// фильтрация текстуры для плавности перехода
	m_pD3DDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );				// фильтрация текстуры для плавности перехода
	m_pD3DDevice->SetSamplerState( 8, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );				// фильтрация текстуры для плавности перехода
	
	m_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	m_pD3DDevice->SetRenderState( D3DRS_ALPHAREF,        0x01 );
	m_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );


	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	

	return S_OK;
}

void CD3DGraphic::SetBlending( BLENDING Blend )
{
	if( m_pD3DDevice )
	{
		switch ( Blend )
		{
		case BLEND_DEFAULT:
			m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
			break;

		case BLEND_ALPHA:
			m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );
			m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA );
			m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA );
			break;

		case BLEND_MUL:
			m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );
			m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_DESTCOLOR );
			m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ZERO );
			break;

		case BLEND_ADD:
			m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );
			m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_ONE );
			m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ONE );
			break;
		}
	}
}

void CD3DGraphic::Release()
{	
	RELEASE_ONE( m_pD3DDevice );	
	RELEASE_ONE( m_pDirect3D  );	
}
//-------------------------------------------------------------------------------------


