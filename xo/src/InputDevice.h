#pragma once
#include "CameraDevice.h"

struct CMouseState
{
	LONG     lX;
	LONG     lY;
	LONG     lZ;
	BYTE     rgbButtons[8];
	CMouseState():lX(0),lY(0), lZ(0)
	{	}	
};

class CInputDevice
{
public:
	LPDIRECTINPUT8			pInput;
	LPDIRECTINPUTDEVICE8    pKeyboard;
	LPDIRECTINPUTDEVICE8    pMouse;
	CMouseState             mouse;	
	HRESULT                 InitialInput( HWND hwnd, FILE *m_FileLog );
	bool                    ScanInput( CameraDevice *m_Camera , bool *Check );
	void					Release();
};