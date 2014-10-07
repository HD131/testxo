#pragma once

#include "Class.h"
#include <map>
#include <vector>
#include <string>
#include "Help.h"
#include "D3D.h"
#include "Lua.h"
#include "Sky.h"
#include "InputDevice.h"
#include "Particle.h"
#include "PhysX.h"
#include "3DModel.h"

class CGame
{
public:
													CGame();
													~CGame();

	static HWND										GetHWND()											{ return m_hWnd;	  }
	static HINSTANCE								GetHINSTANCE()										{ return m_hInstance; }
	const CameraDevice *							GetCamera()	const									{ return m_pCamera;	  }
	GameObject *									GetObject( std::string srName );
	HWND											Init( HINSTANCE hInstance, WNDPROC pWndProc );
	bool											InitInputDevice();
	void											Release();
	void											RenderingScene();
	void											Update( float fDT );	
	
public:
	static bool										m_bEndGame;

private:
	static HWND										m_hWnd;
	static HINSTANCE								m_hInstance;	
	int												m_nWidth;
	int												m_nHeight;
	CD3DGraphic										m_D3D;
	CShaderManager									m_ShaderManager;
	CameraDevice* 									m_pCamera;
	CSky         									m_Sky;
	std::vector< CMesh3D* >							m_Mesh;
	CInputDevice*									m_DeviceInput;
	std::map< std::string, GameObject* >			m_Objects;
	std::vector< CTank* >							m_Tanks;
	std::vector< CBullet* >							m_Bullet;
	std::vector< CParticles* >						m_Particles;
	std::map< int, CameraDevice* >					m_Camers;
	CTank *											m_pMyTank;
	uintptr_t										m_pThreadChange;
};