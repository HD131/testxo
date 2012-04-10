#pragma once
#include "Init.h"

class CameraDevice
{
public:
	CameraDevice();
   
	D3DXVECTOR3		PositionCamera;
	D3DXVECTOR3		CameraUp;	// вверх
	D3DXVECTOR3		TargetDir;	// вперед
	D3DXVECTOR3     DirX;
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