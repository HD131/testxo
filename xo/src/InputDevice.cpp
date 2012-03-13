#include "InputDevice.h"

int    GameOver();
POINT PickObject( CCell *m_Cell );


HRESULT CInputDevice::InitialInput( HWND hwnd, FILE *m_FileLog )
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
	if ( m_FileLog ) 
		fprintf( m_FileLog, "Initial DirectInput8\n" );

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

bool CInputDevice::ScanInput( CameraDevice *m_Camera, bool *Check, CCell *m_Cell )
{	
	char     keyboard[256];     
	LONG     dx, dy, dz;

	if FAILED( pKeyboard -> GetDeviceState(sizeof(keyboard), (LPVOID)&keyboard) )
	{
		pKeyboard -> Acquire();
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

	if FAILED( pMouse -> GetDeviceState( sizeof( CMouseState ), (LPVOID)&mouse ) )
	{
		pMouse -> Acquire();
		return FALSE;
	}
	dx = mouse.lX;
	dy = mouse.lY;
	dz = mouse.lZ;

	if ( mouse.rgbButtons[LEFT_BUTTON]&0x80 )
	{
		POINT Point = PickObject( &m_Cell[0]);
		if ( ( Point.x >= 0 ) && ( m_Cell[Point.x*3+Point.y].Value > 1 ) && ( GameOver() < 0 ) )
		{
			m_Cell[Point.x*3+Point.y].Value = 1;
			
			if ( GameOver() > 0 )
				return TRUE;
			*Check = true;					
		}
	}
	return TRUE;
}

void CInputDevice::Release()
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