#pragma once

#include "PhysX.h"
#include "Class.h"
#include <list>

class CameraDevice;
class CShader;
class CMesh3D;
class CDirection;

class GameObject
{
public:
											GameObject();
											~GameObject();

	bool									CreateTriangleMesh( CPhysX const * pPhysX );	// Создает объект треугольников
	D3DXVECTOR3								GetAngle();
	D3DXVECTOR3								GetPosition();
	D3DXMATRIX								GetReleaseMatrix() const										{ return m_MatrixRelease; }
	D3DXVECTOR3								GetForward();
	PxTriangleMesh*							GetTriangleMesh()const											{ return m_pTriangleMesh; }
	void									SetForward( float fDelta )										{ m_vPosition  += m_fStepMove * m_Direction.GetForward() * fDelta; }	
	void									RotateAxisX( float fDelta )										{ m_AngleXYZ.x -= m_fStepRotate * fDelta; m_Direction.RotateUp( -m_fStepRotate * fDelta ); }
	void									RotateAxisY( float fDelta )										{ m_AngleXYZ.y -= m_fStepRotate * fDelta; m_Direction.RotateUp( -m_fStepRotate * fDelta ); }
	void									RotateAxisZ( float fDelta )										{ m_AngleXYZ.z -= m_fStepRotate * fDelta; m_Direction.RotateUp( -m_fStepRotate * fDelta ); }
	void									Render( CameraDevice * pCamera, const CShader * pShader );
	void									SetMesh( CMesh3D * pMesh );
	void									SetParent( GameObject *	pParent )								{ m_pObjectParent = pParent; }
	void									SetPosition( const D3DXVECTOR3& pos )							{ m_vPosition  = pos;		 }	
	void									SetAngleAxisX( float fAng )										{ m_AngleXYZ.x = fAng;		 }
	void									SetAngleAxisY( float fAng )										{ m_AngleXYZ.y = fAng;		 }
	void									SetAngleAxisZ( float fAng )										{ m_AngleXYZ.z = fAng;		 }
	void									SetAngleAxis( const D3DXVECTOR3 & vAng )						{ m_AngleXYZ   = vAng;		 }
	void									SetOffsetUV( const D3DXVECTOR4 & vec )							{ m_vOffetUV  += vec * m_fStepMove * 0.04f;			 }
	void									SetChild( GameObject * pObj );
	void									SetStepMove( float fDelta )										{ m_fStepMove   = fDelta; }
	void									SetStepRotate( float fDelta )									{ m_fStepRotate = fDelta; }
	void									SetReleaseMatrix( const D3DXMATRIX & mat, bool bUse = true )	{ m_MatrixRelease = mat; m_bUseMatrix = bUse;	}
	void									Update( float fDT );	

private:
	GameObject *							m_pObjectParent;
	std::list< GameObject* >				m_ObjectChild;
	D3DXMATRIX								m_MatrixRelease;	
	D3DXVECTOR3								m_vPosition;	
	CDirection								m_Direction;
	float									m_fStepMove;
	float									m_fStepRotate;
	D3DXVECTOR3								m_AngleXYZ;
	CMesh3D *								m_pMeshObject;
	D3DXVECTOR4								m_vOffetUV;
	std::vector< float >					m_BufVertices;
	std::vector< short >					m_BufIndices;
	DWORD									m_dwNumVertices;
	DWORD									m_dwNumNumFaces;
	PxTriangleMesh *						m_pTriangleMesh;
	bool									m_bUseMatrix;
};
