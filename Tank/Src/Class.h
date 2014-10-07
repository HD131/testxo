#pragma once

#define  INITGUID
#include "..//..//sdk//dx9//Include//d3d9.h"
#include "..//..//sdk//dx9//Include//d3dx9.h"
#include "Help.h"
#include <map>
#include <vector>
#include <string>
#include "CameraDevice.h"
#include "D3D.h"
#include "PhysX.h"
#include "ShaderManager.h"

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
typedef unsigned int uint;

const D3DXVECTOR4 g_Light       = D3DXVECTOR4( 0.f, 10000.f, 0.f, 1.f );
const float       g_Diffuse_intensity = 1.0f;

//---------------------------------------------------------
struct CVertex
{
	FLOAT  x,  y,  z;
	FLOAT nx, ny, nz;
	FLOAT  u,  v;

	CVertex() :
	x(0.f), y(0.f), z(0.f), nx(0.f), ny(0.f), nz(0.f), u(0.f), v(0.f)
	{}

	CVertex( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V ) : 
	x(X), y(Y), z(Z), nx(NX), ny(NY), nz(NZ), u(U), v(V)
	{}
};

//---------------------------------------------------------
class CDirection
{
	D3DXVECTOR3	m_vForward;
	D3DXVECTOR3	m_vUp;
	D3DXVECTOR3	m_vRight;

public:
	D3DXVECTOR3 GetForward()const 								{ return m_vForward; } 
	D3DXVECTOR3 GetUp()const 									{ return m_vUp;		 }
	D3DXVECTOR3 GetRight()const 								{ return m_vRight;	 }
	void		SetForward( const D3DXVECTOR3& vec )			{ m_vForward = vec;	 } 
	void		SetUp( const D3DXVECTOR3& vec )					{ m_vUp      = vec;	 }
	void		SetRight( const D3DXVECTOR3& vec )				{ m_vRight   = vec;	 }

	void		RefreshAxis()
	{
		D3DXVec3Normalize( &m_vForward, &m_vForward );

		D3DXVec3Cross( &m_vRight, &m_vUp, &m_vForward );
		D3DXVec3Normalize( &m_vRight, &m_vRight );

		D3DXVec3Cross( &m_vUp, &m_vForward, &m_vRight );
		D3DXVec3Normalize( &m_vUp, &m_vUp );
	}

	void RotateUp( float Ang )
	{
		RefreshAxis();
		D3DXMATRIX Rotate;
		D3DXMatrixRotationAxis( &Rotate, &m_vUp, Ang );
		D3DXVECTOR4 T;
		D3DXVec3Transform( &T, &m_vForward, &Rotate );
		m_vForward = D3DXVECTOR3( T.x, T.y, T.z );
		RefreshAxis();		
	}

	CDirection()
	{
		m_vForward  = D3DXVECTOR3( 1.f, 0.f, 0.f );
		m_vUp       = D3DXVECTOR3( 0.f, 1.f, 0.f );
		m_vRight    = D3DXVECTOR3( 0.f, 0.f, -1.f );
	}
};

//---------------------------------------------------------
class CShader;

class CMesh3D
{
public:	
				CMesh3D();

	ID3DXMesh * GetMesh()																						{ return m_pMesh; }
	D3DXVECTOR4	GetOffsetUV()																					{ return m_vOffetUV; }
	HRESULT		InitMesh( LPCSTR Name, IDirect3DDevice9 * pD3DDevice );
	void		Release();
	void        RenderMesh( CameraDevice * pCamera, const D3DXMATRIX &  MatrixWorld, const CShader * pShader );
	void		SetOffsetUV( const D3DXVECTOR4 & vec )															{ m_vOffetUV = vec; }

private:
	ID3DXMesh *							m_pMesh;
	IDirect3DDevice9 *                  m_pD3DDevice;
	float								m_Alpha;	
	IDirect3DVertexBuffer9 *			m_VertexBuffer;
	IDirect3DIndexBuffer9 *				m_IndexBuffer;
	DWORD								m_TexturCount; 
	D3DMATERIAL9 *						m_pMeshMaterial;
	std::vector< IDirect3DTexture9* >    m_pMeshTextura;
	D3DXVECTOR4							m_vOffetUV;			
};

//---------------------------------------------------------
class CBullet
{
public:
	CBullet();	

	D3DXVECTOR3			GetPosition()													{ return m_vPosition;}
	void				Update( float fDT );	
	void				SetMesh( CMesh3D * pMesh )										{ m_pMeshBullet = pMesh; }		
	void				SetSpeed( float fSpeed )										{ m_fSpeed		= fSpeed;}
	void				SetReleaseMatrix( const D3DXMATRIX& mat );	
	void				Render( CameraDevice * pCamera, const CShader * pShader );
	bool				IsDown()														{ return m_vPosition.y < 0.f ? true : false; }

private:
	D3DXVECTOR3			m_vPosition;
	D3DXVECTOR3			m_vFirstPosition;
	float				m_fSpeed;
	D3DXVECTOR3			m_vTargetDir;
	float				m_fTime;
	CMesh3D *			m_pMeshBullet;	
	D3DXMATRIX			m_MatrixRelease;
	float				m_fTimeFull;
	float				m_fLen;
	float				m_fAngle;
};

enum EDetailTank
{	
	WHEEL_LEFT_1ST  = 0,
	WHEEL_RIGHT_1ST,
	WHEEL_LEFT_2ST,
	WHEEL_RIGHT_2ST,
	WHEEL_LEFT_3ST,
	WHEEL_RIGHT_3ST,
	WHEEL_LEFT_4ST,
	WHEEL_RIGHT_4ST,
	WHEEL_LEFT_5ST,
	WHEEL_RIGHT_5ST,
	WHEEL_LEFT_6ST,
	WHEEL_RIGHT_6ST,
	WHEEL_LEFT_7ST,
	WHEEL_RIGHT_7ST,
	WHEEL_LEFT_8ST,
	WHEEL_RIGHT_8ST,		
	BODY,
	TURRET,
	GUN,
	TRACK_L,
	TRACK_R,
	MAX_DETAIL
};

class TankData
{
public:
									TankData();

public:
	std::string						m_srPathLoad;
	int								m_nNumberWheels;
	float							m_fMassTank;
	float							m_fMassWheel;
	float							m_WheelWidths;
	float							m_WheelRadius;
	physx::PxVec3					m_vWheelMaterial;
	std::vector< physx::PxVec3 >	m_vecWheelCenter;
	physx::PxVec3					m_vSuspensionTravelDirections;
	float							m_SuspensionMaxCompression;
	float							m_SuspensionMaxDroop;
	float							m_SuspensionSpringStrength;
	float							m_SuspensionSpringDamperRate;
	float							m_EnginePeakTorque;
	float							m_EngineMaxOmega;
	float							m_EngineDampingRateFullThrottle;
	float							m_EngineDampingRateZeroThrottleClutchEngaged;
	float							m_EngineDampingRateZeroThrottleClutchDisengaged;
	physx::PxVec3					m_vSizeBody;
};

class GameObject;

class CTank
{
public:
	CTank();
	~CTank();

	GameObject *							GetDetail( EDetailTank detail );
	void									SetDetail( EDetailTank detail, GameObject * pObj );
	void									MoveForward( bool bForward );
	void									MoveBack( bool bBack );
	void									TurnLeft( bool bLeft );									// поворот корпуса
	void									TurnRight( bool bRight );								// поворот корпуса
	void									RotateTurret( float fDT );								// поворот башни
	void									RotateGun( float fDT );									// поворот пушки
	void									Update( float fDT );
	void									Render( CameraDevice * pCamera, const CShader * pShader );
	void									SetPosition( const D3DXVECTOR3& vPos );
	D3DXVECTOR3 							GetForvard();
	bool									CreateTankActor( CPhysX * pPhysX );
	bool									LoadData( const std::string & srPath );

private:
	std::map< EDetailTank, GameObject* >	m_ObjectsTank;
	TankData *								m_pTankData;
	PxRigidDynamic*							m_pActor;
	bool									m_bMoveForward;
	bool									m_bMoveBack;
	bool									m_bTurnLeft;
	bool									m_bTurnRight;
	float									m_fSpeedTank;
};