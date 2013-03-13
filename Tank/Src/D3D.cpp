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
	D3DPRESENT_PARAMETERS Direct3DParametr;		// структура задающая парметры рендеринга 
	D3DDISPLAYMODE        Display;				// возвращает параметры дисплея

	if( ( m_pDirect3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == 0 ) // создаётся главный интерфейс
	{
		Log( "Error Initial Direct3D" );
		return E_FAIL;
	}
	else
		Log( "Initial Direct3D" );

	if( FAILED( m_pDirect3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Display ) ) ) // получаем текущий формат дисплея
	{
		Log( "Error GetAdapterDisplayMode" );
		return E_FAIL;
	}

	ZeroMemory( &Direct3DParametr, sizeof( Direct3DParametr ) );
	Direct3DParametr.Windowed               = FullScreen;					// видео режим окно (или полноэкранный режим)
	Direct3DParametr.SwapEffect             = D3DSWAPEFFECT_DISCARD;		// определяет обмен буферов
	Direct3DParametr.BackBufferFormat       = Display.Format;				// формат поверхности заднего буфера
	Direct3DParametr.EnableAutoDepthStencil = TRUE;							// включаем Z-буфер
	Direct3DParametr.AutoDepthStencilFormat = D3DFMT_D16;					// 16-разрядный буфер глубины
	Direct3DParametr.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;// D3DPRESENT_INTERVAL_IMMEDIATE			D3DPRESENT_INTERVAL_DEFAULT

	// полноэкранный режим
	if( !FullScreen )
	{
		Direct3DParametr.BackBufferWidth			= GetSystemMetrics( SM_CXSCREEN );
		Direct3DParametr.BackBufferHeight			= GetSystemMetrics( SM_CYSCREEN );
		Direct3DParametr.BackBufferCount			= 3;
		Direct3DParametr.FullScreen_RefreshRateInHz = Display.RefreshRate;
	}
	
	if( FAILED( m_pDirect3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Direct3DParametr, &m_pD3DDevice ) ) ) // создаётся интерфейс устройства
	{
		Log( "Error Create Device Direct3D" );
		Release();
		return E_FAIL;
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


