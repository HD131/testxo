#pragma once

#include <windows.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>
#include <stdio.h>
#include <winuser.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>
#include <d3dx9math.h>
#define  DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#define  INITGUID
#include <fstream>
#include <string>
#include <d3dx9mesh.h>

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"dinput8.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")

const UINT Width  = 1024;
const UINT Height = 768;

#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define KEYDOWN(name, key) (name[key]&0x80)

class CameraDevice
{
public:
	CameraDevice();
   
	D3DXVECTOR3		PositionCamera;
	D3DXVECTOR3		CameraUp;	// вверх
	D3DXVECTOR3		TargetDir;	// вперед
	D3DXVECTOR3     DirX;
	D3DXVECTOR3     DirY;
	D3DXVECTOR3     Point;
	D3DXMATRIX		m_View;
	D3DXMATRIX		m_Proj;
	FLOAT           StepCamera;
	FLOAT	        AngleCamera;
	FLOAT	        Sensivity;

	void  MoveForv();
	void  MoveBack();
	void  MoveRight();
	void  MoveLeft();
	void  MouseRotateLeft();
	void  MouseRotateRight();
	void  MouseRotateUp();
	void  MouseRotateDown();
	void  Refresh();
	void  RefreshMirror();
};