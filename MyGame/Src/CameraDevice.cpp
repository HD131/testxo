#include "Init.h"
#include "CameraDevice.h"

CameraDevice::CameraDevice()
{	
	m_CameraUp				= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	m_TargetDir				= D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	m_DirX					= D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	m_CentrMass.m_Centre	= D3DXVECTOR3( 0.0f, 0.9f, 0.0f );
	m_CentrMass.m_Radius    = 1.5f;
	m_PositionCamera		= D3DXVECTOR3( 0.0f, 1.7f, 0.0f );
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
	D3DXMatrixPerspectiveFovLH(&m_Proj, D3DX_PI / 4, (FLOAT)Width / Height, 0.1f, 3000.0f);
	StepCamera  = 0.2f;
	AngleCamera = 1.0f * D3DX_PI / 180;
	Sensivity   = 300.0f;
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
	if ( Collision( m_pMesh ) )
		m_PositionCamera -= m_TargetDir * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveBack()
{
	Refresh();
	m_PositionCamera += m_TargetDir * -StepCamera;
	if ( Collision( m_pMesh ) )
		m_PositionCamera -= m_TargetDir * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveRight()
{
	Refresh();
	m_PositionCamera += m_DirX * StepCamera;
	if ( Collision( m_pMesh ) )
		m_PositionCamera -= m_DirX * StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void CameraDevice::MoveLeft()
{
	Refresh();
	m_PositionCamera += m_DirX * -StepCamera;
	if ( Collision( m_pMesh ) )
		m_PositionCamera -= m_DirX * -StepCamera;
	D3DXMatrixLookAtLH( &m_View, &m_PositionCamera, &(m_PositionCamera + m_TargetDir), &m_CameraUp );
}

void  CameraDevice::MouseRotateLeft( long Ang )
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

void  CameraDevice::MouseRotateRight( long Ang )
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
void  CameraDevice::MouseRotateUp( long Ang )
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

void  CameraDevice::MouseRotateDown( long Ang )
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

D3DXMATRIX  CameraDevice::MatViewProject()
{
	return m_View * m_Proj;
}
D3DXMATRIX  CameraDevice::MatInverseViewProject()
{
	D3DXMATRIX	InversView;
	D3DXMatrixInverse( &InversView, 0, &m_View );
	return  InversView * m_View * m_Proj;
}

bool PointInTr( D3DXVECTOR3&  v1, D3DXVECTOR3&  v2, D3DXVECTOR3&  v3, D3DXVECTOR3&  n, D3DXVECTOR3&  ip )
{
	// проверяем, находится ли точка пересечения внутри треугольника.
	D3DXVECTOR3 a;
	D3DXVec3Cross( &a, &(v2 - v1), &(ip - v1) );
	if( D3DXVec3Dot( &a, &n ) <= 0) 
		return FALSE; 
	D3DXVec3Cross( &a, &(v3 - v2), &(ip - v2) );
	if( D3DXVec3Dot( &a, &n ) <= 0)
		return FALSE; 
	D3DXVec3Cross( &a, &(v1 - v3), &(ip - v3) );
	if( D3DXVec3Dot( &a, &n ) <= 0)
		return FALSE; 

return true;
} 

bool CameraDevice::Collision( ID3DXMesh* pMesh )
{	
	CVertexFVF* Triangle[3];
	short*       pIndices;
	CVertexFVF*  pVertices;
	
	DWORD m_NumPolygons = pMesh->GetNumFaces();	
		
	pMesh->LockVertexBuffer( D3DLOCK_READONLY,( void** )&pVertices);
	pMesh->LockIndexBuffer(  D3DLOCK_READONLY,( void** )&pIndices );
		
	
	for( DWORD i = 0; i < m_NumPolygons; i++ )
	{
		Triangle[0] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка А i-того треугольника
		Triangle[1] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка B i-того треугольника
		Triangle[2] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка C i-того треугольника

		//тут можем работать с этим треугольником		
		D3DXVECTOR3 n = D3DXVECTOR3( Triangle[0]->nx, Triangle[0]->ny, Triangle[0]->nz );	// нормаль треугольника
		D3DXVECTOR3 V = -n;
		float D = -n.x * Triangle[0]->x - n.y * Triangle[0]->y - n.z * Triangle[0]->z;
		float k = -( n.x * m_PositionCamera.x + n.y * m_PositionCamera.y + n.z * m_PositionCamera.z + D ) / ( n.x * V.x + n.y * V.y + n.z * V.z );
		D3DXVECTOR3 P = D3DXVECTOR3( k * V.x + m_PositionCamera.x, k * V.y + m_PositionCamera.y, k * V.z + m_PositionCamera.z);
		float d = D3DXVec3LengthSq( &(D3DXVECTOR3( m_PositionCamera.x - P.x, m_PositionCamera.y - P.y, m_PositionCamera.z - P.z )) );

		D3DXVECTOR3 A = D3DXVECTOR3( Triangle[0]->x, Triangle[0]->y, Triangle[0]->z );
		D3DXVECTOR3 B = D3DXVECTOR3( Triangle[1]->x, Triangle[1]->y, Triangle[1]->z );
		D3DXVECTOR3 C = D3DXVECTOR3( Triangle[2]->x, Triangle[2]->y, Triangle[2]->z );
		if ( PointInTr( A, B, C, n, P) )
		{
			if ( d < m_CentrMass.m_Radius )
			{
				//Beep(200,40);
				return true;
			}
		}
	}
	pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();
return false;
}

