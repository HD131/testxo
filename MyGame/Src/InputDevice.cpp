#include "InputDevice.h"
#include "Init.h"

bool				Pressed = false;

HRESULT CInputDevice::InitialInput( HWND hwnd )
{	
	m_pInput    = 0;
	m_pKeyboard = 0;
	m_pMouse    = 0;

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize		= sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize	= sizeof( DIPROPHEADER );
	dipdw.diph.dwObj		= 0;
	dipdw.diph.dwHow		= DIPH_DEVICE;
	dipdw.dwData			= 32;

	if (FAILED(DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInput, 0 )))
		return E_FAIL;
	Log( "Initial DirectInput8" );

	if FAILED(m_pInput -> CreateDevice(GUID_SysKeyboard, &m_pKeyboard, 0 )) //создание устройства клавиатура
		return E_FAIL;
	if FAILED(m_pKeyboard -> SetDataFormat(&c_dfDIKeyboard))
		return E_FAIL;
	if FAILED(m_pKeyboard -> SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))
		return E_FAIL;
	if( FAILED(m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
		return E_FAIL;
	if FAILED(m_pKeyboard -> Acquire())
		return E_FAIL;

	if FAILED(m_pInput -> CreateDevice(GUID_SysMouse, &m_pMouse, 0)) // создание устройства мышь
		return E_FAIL;	
	if FAILED(m_pMouse -> SetDataFormat(&c_dfDIMouse2))
		return E_FAIL;	
	if FAILED(m_pMouse -> SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))
		return E_FAIL;	
	if FAILED(m_pMouse -> Acquire())
		return E_FAIL;

return S_OK;
}

bool CInputDevice::ScanInput( CameraDevice *m_Camera, CWeapon* pWeapon )
{	
	char     keyboard[256];     
	LONG     dx, dy, dz;

	if FAILED( m_pKeyboard -> GetDeviceState(sizeof(keyboard), (LPVOID)&keyboard) )
	{
		m_pKeyboard -> Acquire();
		return FALSE;
	}

	if ( KEYDOWN(keyboard, DIK_RIGHT) || KEYDOWN(keyboard, DIK_D))
		m_Camera->MoveRight();
	if ( KEYDOWN(keyboard, DIK_LEFT) || KEYDOWN(keyboard, DIK_A))
		m_Camera->MoveLeft();
	if ( KEYDOWN(keyboard, DIK_UP) || KEYDOWN(keyboard, DIK_W))     
		m_Camera->MoveForv();
	if ( KEYDOWN(keyboard, DIK_DOWN) || KEYDOWN(keyboard, DIK_S))
		m_Camera->MoveBack();
	if ( KEYDOWN(keyboard, DIK_R ) )
		pWeapon->Recharge();

	if FAILED( m_pMouse -> GetDeviceState( sizeof( CMouseState ), (LPVOID)&m_Mouse ) )
		m_pMouse -> Acquire();

	dx = m_Mouse.m_lX;
	dy = m_Mouse.m_lY;
	dz = m_Mouse.m_lZ;
	
	if ( (dx < 0) )  
		m_Camera->MouseRotateLeft( dx );
	if ( (dx > 0) )  
		m_Camera->MouseRotateRight( dx );
	if ( (dy > 0) )  
		m_Camera->MouseRotateUp( dy );
	if ( (dy < 0) )  
		m_Camera->MouseRotateDown( dy );
	
	if ( m_Mouse.m_rgbButtons[LEFT_BUTTON]&0x80 )
	{
		 if ( !Pressed )		 
			 Pressed = true;
		 pWeapon->Fire();
	}
	 else		 
		 if ( Pressed )
		 {
			 Pressed = false;
		 }
	
return TRUE;
}

void CInputDevice::Release()
{
	if (m_pInput)
	{
		if (m_pKeyboard)
		{
			if (m_pKeyboard)
				m_pKeyboard -> Unacquire();
			if (m_pKeyboard)
				m_pKeyboard -> Release();
			m_pKeyboard = 0;
		}
		if (m_pMouse)
		{
			if (m_pMouse)
				m_pMouse -> Unacquire();
			if (m_pMouse)
				m_pMouse -> Release();
			m_pMouse = 0;
		}
		if (m_pInput)
			m_pInput -> Release();
		m_pInput = 0;
	}
}