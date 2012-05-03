#pragma once
#include "Init.h"

class CameraDevice
{
public:
	CameraDevice();
   
	D3DXVECTOR3		m_PositionCamera;
	D3DXVECTOR3		m_CameraUp;	// вверх
	D3DXVECTOR3		m_TargetDir;	// вперед
	D3DXVECTOR3     m_DirX;
  	CSphere         m_CentrMass;
	D3DXMATRIX		m_View;
	D3DXMATRIX		m_Proj;
	FLOAT           StepCamera;
	FLOAT	        AngleCamera;
	FLOAT	        Sensivity;
	ID3DXMesh*      m_pMesh;
	DWORD           m_TimeGravity;

	void  		MoveForv();
	void  		MoveBack();
	void  		MoveRight();
	void  		MoveLeft();
	void  		MouseRotateLeft(  long Ang );
	void  		MouseRotateRight( long Ang );
	void  		MouseRotateUp(    long Ang );
	void  		MouseRotateDown(  long Ang );
	void  		Refresh();	
	void		Gravity();
	D3DXMATRIX  MatViewProject();
	D3DXMATRIX  MatInverseViewProject();
	void        SetMesh( ID3DXMesh* pMesh );
};