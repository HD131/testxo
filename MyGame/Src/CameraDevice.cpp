#include "Init.h"
#include "CameraDevice.h"

CameraDevice::CameraDevice()
{
	PositionCamera	= D3DXVECTOR3( 0, 1, -10 );
	CameraUp		= D3DXVECTOR3( 0, 1, 0 );
	TargetDir		= D3DXVECTOR3( 0, 0, 1 );
	DirX		    = D3DXVECTOR3( 1, 0, 0 );
	Point		    = PositionCamera + TargetDir;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
	D3DXMatrixPerspectiveFovLH(&m_Proj, D3DX_PI / 4, (FLOAT)Width / Height, 0.1f, 3000.0f);
	StepCamera  = 0.05f;
	AngleCamera = 1.0f * D3DX_PI / 180;
	Sensivity   = 0.8f;
}

void CameraDevice::Refresh()
{
	D3DXVec3Normalize( &TargetDir, &TargetDir );

	D3DXVec3Cross(&DirX, &CameraUp, &TargetDir);
	D3DXVec3Normalize( &DirX, &DirX );

	D3DXVec3Cross(&CameraUp, &TargetDir, &DirX);
	D3DXVec3Normalize( &CameraUp, &CameraUp );
}

void CameraDevice::MoveForv()
{
	Refresh();
	PositionCamera += TargetDir * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void CameraDevice::MoveBack()
{
	Refresh();
	PositionCamera += TargetDir * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void CameraDevice::MoveRight()
{
	Refresh();
	PositionCamera += DirX * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void CameraDevice::MoveLeft()
{
	Refresh();
	PositionCamera += DirX * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void  CameraDevice::MouseRotateLeft()
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, -AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &CameraUp, &Rotate );
	CameraUp = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &DirX, &Rotate );
	DirX = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void  CameraDevice::MouseRotateRight()
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &CameraUp, &Rotate );
	CameraUp = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &DirX, &Rotate );
	DirX = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}
void  CameraDevice::MouseRotateUp()
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &DirX, AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

void  CameraDevice::MouseRotateDown()
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &DirX, -AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &(PositionCamera + TargetDir), &CameraUp );
}

