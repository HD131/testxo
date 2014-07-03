#include "CameraDevice.h"

float CameraDevice::m_fAspect = 1.333333f;

CameraDevice::CameraDevice() :	
	StepCamera( 0.07f ),
	Sensivity( 300.f ),
	m_TimeGravity( 0 )
{	
	m_CameraUp				= D3DXVECTOR3( 0.f, 1.f, 0.f );
	m_TargetDir				= D3DXVECTOR3( 0.f, 0.f, 1.f );
	m_DirX					= D3DXVECTOR3( 1.f, 0.f, 0.f );	
	m_PositionCamera		= D3DXVECTOR3( 0.f, 0.f, 0.f );
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
	D3DXMatrixPerspectiveFovLH( &m_Proj, D3DX_PI / 4, m_fAspect, 0.1f, 20000.f );	
	AngleCamera   = 1.f * D3DX_PI / 180.f;
	Refresh();
}

void CameraDevice::SetMesh( ID3DXMesh* pMesh )
{
	m_pMesh = pMesh;
}

void CameraDevice::Refresh()
{
	D3DXVec3Normalize( &m_TargetDir, &m_TargetDir );

	D3DXVec3Cross(&m_DirX, &m_CameraUp, &m_TargetDir);
	D3DXVec3Normalize( &m_DirX, &m_DirX );

	D3DXVec3Cross(&m_CameraUp, &m_TargetDir, &m_DirX);
	D3DXVec3Normalize( &m_CameraUp, &m_CameraUp );	
}

void CameraDevice::MoveForv()
{
	Refresh();
	m_PositionCamera += m_TargetDir * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveBack()
{
	Refresh();
	m_PositionCamera += m_TargetDir * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveRight()
{
	Refresh();
	m_PositionCamera += m_DirX * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveLeft()
{
	Refresh();
	m_PositionCamera += m_DirX * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MouseRotateLeft( long Ang )
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, (float)Ang / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &m_TargetDir, &Rotate );
	m_TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &m_CameraUp, &Rotate );
	m_CameraUp = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &m_DirX, &Rotate );
	m_DirX = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MouseRotateRight( long Ang )
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationY( &Rotate, (float)Ang / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &m_TargetDir, &Rotate );
	m_TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &m_CameraUp, &Rotate );
	m_CameraUp = D3DXVECTOR3( T.x, T.y, T.z );
	D3DXVec3Transform( &T, &m_DirX, &Rotate );
	m_DirX = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MouseRotateUp( long Ang )
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &m_DirX, (float)Ang / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &m_TargetDir, &Rotate );
	m_TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MouseRotateDown( long Ang )
{
	Refresh();
	D3DXMATRIX Rotate;
	D3DXMatrixRotationAxis( &Rotate, &m_DirX, (float)Ang / Sensivity );
	D3DXVECTOR4 T;
	D3DXVec3Transform( &T, &m_TargetDir, &Rotate );
	m_TargetDir = D3DXVECTOR3( T.x, T.y, T.z );
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

const D3DXMATRIX CameraDevice::GetMatrixViewProject()
{
	return m_View * m_Proj;	
}

D3DXMATRIX  CameraDevice::MatInverseViewProject()
{
	D3DXMATRIX	InversView;
	D3DXMatrixInverse( &InversView, 0, &m_View );
	return  InversView * m_View * m_Proj;
}

void CameraDevice::Gravity()
{
// 	DWORD t = timeGetTime();
// 	if ( m_TimeGravity )
// 		m_TimeGravity = t - m_TimeGravity;
// 
// 	float s = 4.8f * m_TimeGravity * m_TimeGravity / 2000.f + 0.01f;
// 	m_PositionCamera = D3DXVECTOR3( m_PositionCamera.x, m_PositionCamera.y - s, m_PositionCamera.z );
// 
// 	if ( !Collision( m_pMesh,  m_PositionCamera, m_CentrMass.m_Radius ) )
// 		m_TimeGravity = t;
// 	else 
// 		m_TimeGravity = 0;
// 
// 	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::Update( float fDT )
{
	Refresh();
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}



