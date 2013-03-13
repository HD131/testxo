#pragma once

#include "D3D.h"
#include "CameraDevice.h"
#define  DIRECTINPUT_VERSION  0x0800
#include "dinput.h"
#include "Help.h"

#pragma comment( lib,"dinput8.lib" )

#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define MAX_KEYS      256
#define KEYDOWN( name, key ) ( name[ key ] & 0x80 )

struct CMouseState
{
	LONG     m_lX;
	LONG     m_lY;
	LONG     m_lZ;
	BYTE     m_rgbButtons[ 8 ];

	CMouseState():
		m_lX(0),
		m_lY(0),
		m_lZ(0)
	{
	}	
};

class CInputDevice
{
public:

	CInputDevice();
	~CInputDevice();
	
	HRESULT                 InitInputDevice( HWND hwnd, HINSTANCE hinst );
	bool                    ScanInput( CameraDevice *m_Camera );
	void					Release();
	bool					PressKey( const byte Button ) const;
	bool					KeyDown( const byte Button ) const;
	bool					KeyUp( const byte Button ) const;

private:
	char					m_Keyboard[ MAX_KEYS ];
	char					m_KeyboardLast[ MAX_KEYS ];
	LPDIRECTINPUT8			m_pInput;
	LPDIRECTINPUTDEVICE8    m_pKeyboard;
	LPDIRECTINPUTDEVICE8    m_pMouse;
	CMouseState             m_Mouse;
};