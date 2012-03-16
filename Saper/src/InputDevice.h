#pragma once
#include "CameraDevice.h"

struct CMouseState
{
	LONG     m_lX;
	LONG     m_lY;
	LONG     m_lZ;
	BYTE     m_rgbButtons[8];
	CMouseState():m_lX(0), m_lY(0), m_lZ(0)
	{	}	
};

class CInputDevice
{
public:
	LPDIRECTINPUT8			m_pInput;
	LPDIRECTINPUTDEVICE8    m_pKeyboard;
	LPDIRECTINPUTDEVICE8    m_pMouse;
	CMouseState             m_Mouse;	
	HRESULT                 InitialInput( HWND hwnd, FILE* FileLog );
	bool                    ScanInput( CameraDevice* m_Camera , CCell* Cell );
	void					Release();
};