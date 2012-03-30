#include "CameraDevice.h"

CameraDevice::CameraDevice()
{
	PositionCamera	= D3DXVECTOR3( 0, 25, 0 );
	CameraUp		= D3DXVECTOR3( 0, 0, 1 );
	TargetDir		= D3DXVECTOR3( 0, -1, 0 );
	DirX		    = D3DXVECTOR3( 1, 0, 0 );
	Point		    = D3DXVECTOR3( 0, 0, 0 );
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
	D3DXMatrixPerspectiveFovLH(&m_Proj, D3DX_PI / 4, (FLOAT)Width / Height, 1.0f, 3000.0f);
	StepCamera  = 0.8f;
	AngleCamera = 1.0f * D3DX_PI / 180;
	Sensivity   = 0.8f;
}

void CameraDevice::Refresh()
{
	TargetDir = Point - PositionCamera;
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
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void CameraDevice::MoveBack()
{
	Refresh();
	PositionCamera += TargetDir * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void CameraDevice::MoveRight()
{
	Refresh();
	PositionCamera += DirX * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void CameraDevice::MoveLeft()
{
	Refresh();
	PositionCamera += DirX * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void  CameraDevice::MouseRotateLeft()
{
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, -AngleCamera / Sensivity);
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void  CameraDevice::MouseRotateRight()
{
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, AngleCamera / Sensivity);
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}
void  CameraDevice::MouseRotateUp()
{
	D3DXVec3Normalize( &TargetDir, &TargetDir );
	D3DXVec3Normalize( &CameraUp, &CameraUp );
	D3DXVec3Cross(&DirX, &TargetDir, &CameraUp);
	D3DXVec3Normalize( &DirX, &DirX );
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &DirX, -AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	DirX = D3DXVECTOR3( DirX.x, 0, DirX.z );
	D3DXVec3Cross(&CameraUp, &DirX, &TargetDir);
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void  CameraDevice::MouseRotateDown()
{

	D3DXVec3Normalize( &TargetDir, &TargetDir );
	D3DXVec3Normalize( &CameraUp, &CameraUp );
	D3DXVec3Cross(&DirX, &TargetDir, &CameraUp);
	D3DXVec3Normalize( &DirX, &DirX );
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &DirX, AngleCamera / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &TargetDir, &Rotate );
	TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	DirX = D3DXVECTOR3( DirX.x, 0, DirX.z );
	D3DXVec3Cross(&CameraUp, &DirX, &TargetDir);
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}

void CameraDevice::RefreshMirror()
{
	D3DXMatrixLookAtLH( &m_View, &PositionCamera, &Point, &CameraUp );
}