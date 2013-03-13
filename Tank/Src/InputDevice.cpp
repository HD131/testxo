#include "InputDevice.h"

bool				Pressed = false;

CInputDevice::CInputDevice() :
	m_pInput( 0 ),
	m_pKeyboard( 0 ),
	m_pMouse( 0 )
{
	ZeroMemory( m_Keyboard,     sizeof(char) * MAX_KEYS );
	ZeroMemory( m_KeyboardLast, sizeof(char) * MAX_KEYS );
}

HRESULT CInputDevice::InitInputDevice( HWND hwnd, HINSTANCE hinst )
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize		= sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize	= sizeof( DIPROPHEADER );
	dipdw.diph.dwObj		= 0;
	dipdw.diph.dwHow		= DIPH_DEVICE;
	dipdw.dwData			= 32;

	HRESULT hr = E_FAIL;

	if( FAILED( DirectInput8Create( GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInput, 0 ) ) )
	{
		Log( "ERROR Create DirectInput8" );
		return E_FAIL;
	}

	Log( "Initial DirectInput8" );

	if( SUCCEEDED( m_pInput->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, 0 ) ) )								// создание устройства клавиатура
		if( SUCCEEDED( m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )										// задаёт формат получаемых данных от устройства
			if( SUCCEEDED( m_pKeyboard->SetCooperativeLevel( hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) ) )	// уровень кооперации
				if( SUCCEEDED( m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
					if( SUCCEEDED( m_pKeyboard->Acquire() ) )
					{
						Log( "Create Keyboard Device" );
						hr = S_OK;
					}

	if( hr != S_OK )
	{
		Log( "ERROR Create Keyboard" );
		return hr;
	}

	hr = E_FAIL;

	if( SUCCEEDED( m_pInput->CreateDevice( GUID_SysMouse, &m_pMouse, 0 ) ) )// создание устройства мышь
		if( SUCCEEDED( m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
			if( SUCCEEDED( m_pMouse->SetCooperativeLevel( hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) ) )
				if( SUCCEEDED( m_pMouse->Acquire() ) )
				{
					Log( "Create Mouse Device" );
					hr = S_OK;
				}

	if( hr != S_OK )
		Log( "ERROR Create Mouse" );

	return hr;
}

bool CInputDevice::PressKey( const byte Button ) const
{
	return KEYDOWN( m_Keyboard, Button ) ? true : false;
}

// Нажата ли кнопка
bool CInputDevice::KeyDown( const byte Button ) const
{
	return ( ( m_Keyboard[ Button ] & 0x80) && ( ~m_KeyboardLast[ Button ] & 0x80 ) );
}

// Отжатие кнопки
bool CInputDevice::KeyUp( const byte Button ) const
{
	return ( ( ~m_Keyboard[ Button ] & 0x80 ) && ( m_KeyboardLast[ Button ] & 0x80 ) );
}

bool CInputDevice::ScanInput( CameraDevice *m_Camera )
{	
	if( m_pKeyboard && m_Camera && m_pMouse )
	{
		memcpy( m_KeyboardLast, m_Keyboard, sizeof( char ) * MAX_KEYS );
		ZeroMemory( m_Keyboard, sizeof(m_Keyboard) );  
		LONG     dx, dy, dz;

		if( FAILED( m_pKeyboard->GetDeviceState( sizeof(m_Keyboard), (LPVOID)&m_Keyboard ) ) )
		{
			m_pKeyboard->Acquire();
			return false;
		}

		if( KEYDOWN(m_Keyboard, DIK_D) )
			m_Camera->MoveRight();

		if( KEYDOWN(m_Keyboard, DIK_A) )
			m_Camera->MoveLeft();

		if( KEYDOWN(m_Keyboard, DIK_W) )     
			m_Camera->MoveForv();

		if( KEYDOWN(m_Keyboard, DIK_S) )
			m_Camera->MoveBack();		

		if( FAILED( m_pMouse->GetDeviceState( sizeof( CMouseState ), (LPVOID)&m_Mouse ) ) )
			m_pMouse->Acquire();

		dx = m_Mouse.m_lX;
		dy = m_Mouse.m_lY;
		dz = m_Mouse.m_lZ;
		
		if( dx < 0 )  
			m_Camera->MouseRotateLeft( dx );

		if( dx > 0 )  
			m_Camera->MouseRotateRight( dx );

		if( dy > 0 )  
			m_Camera->MouseRotateUp( dy );

		if( dy < 0 )  
			m_Camera->MouseRotateDown( dy );
		
		if( m_Mouse.m_rgbButtons[ LEFT_BUTTON ] & 0x80 )
		{
			 if( !Pressed )		 
				 Pressed = true;
		}
		else if( Pressed )
			Pressed = false;
		
		return true;
	}

	return false;
}

void CInputDevice::Release()
{
	if( m_pInput )
	{
		if( m_pKeyboard )
		{
			if( m_pKeyboard )
				m_pKeyboard->Unacquire();
			
			m_pKeyboard->Release();
			m_pKeyboard = 0;
		}

		if( m_pMouse )
		{
			if( m_pMouse )
				m_pMouse->Unacquire();
			
			m_pMouse->Release();
			m_pMouse = 0;
		}
		
		m_pInput->Release();
		m_pInput = 0;
	}
}

CInputDevice::~CInputDevice()
{
}