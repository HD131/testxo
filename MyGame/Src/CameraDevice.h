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

	void  		MoveForv();
	void  		MoveBack();
	void  		MoveRight();
	void  		MoveLeft();
	void  		MouseRotateLeft(  long Ang );
	void  		MouseRotateRight( long Ang );
	void  		MouseRotateUp(    long Ang );
	void  		MouseRotateDown(  long Ang );
	void  		Refresh();
	D3DXMATRIX  MatViewProject()
	{
		return m_View * m_Proj;
	}
	D3DXMATRIX  MatInverseViewProject()
	{
		D3DXMATRIX	InversView;
		D3DXMatrixInverse( &InversView, 0, &m_View );
		return  InversView * m_View * m_Proj;
	}
};