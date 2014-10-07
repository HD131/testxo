#pragma once

#include "..//..//sdk//PhysX_SDK//Include//PxPhysicsAPI.h"
#include "..//..//sdk//dx9//Include//d3dx9math.h"
#include "../../sdk/PhysX_SDK/Include/PxToolkit.h"
#include "../../sdk/PhysX_SDK/Include/physxvisualdebuggersdk/PvdConnection.h"
#include "../../sdk/PhysX_SDK/Include/physxvisualdebuggersdk/PvdConnectionManager.h"
#include "../../sdk/PhysX_SDK/Include/physxvisualdebuggersdk/PvdErrorCodes.h"
#include "../../sdk/PhysX_SDK/Include/pvd/PxVisualDebugger.h"
#include <vector>

//Collision types and flags describing collision interactions of each collision type.
enum
{
	COLLISION_FLAG_GROUND			=	1 << 0,
	COLLISION_FLAG_WHEEL			=	1 << 1,
	COLLISION_FLAG_CHASSIS			=	1 << 2,
	COLLISION_FLAG_OBSTACLE			=	1 << 3,
	COLLISION_FLAG_DRIVABLE_OBSTACLE=	1 << 4,

	COLLISION_FLAG_GROUND_AGAINST	=															COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_WHEEL_AGAINST	=									COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST=	COLLISION_FLAG_GROUND 						 |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

using namespace physx;

enum
{
	SAMPLEVEHICLE_DRIVABLE_SURFACE = 0xffff0000,
	SAMPLEVEHICLE_UNDRIVABLE_SURFACE = 0x0000ffff
};

static PxSceneQueryHitType::Enum SampleVehicleWheelRaycastPreFilter(PxFilterData filterData0,PxFilterData filterData1,const void* constantBlock, PxU32 constantBlockSize,PxSceneQueryFilterFlags& filterFlags)
{
	//filterData0 is the vehicle suspension raycast.
	//filterData1 is the shape potentially hit by the raycast.
	PX_UNUSED(filterFlags);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	PX_UNUSED(filterData0);
	return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxSceneQueryHitType::eNONE : PxSceneQueryHitType::eBLOCK);
}

void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData);
void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData);
void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData);


class SampleVehicleSceneQueryData
{
public:	
	static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels);
	void					free();
	PxBatchQuery*			setUpBatchedSceneQuery(PxScene* scene);
	PxRaycastQueryResult*	getRaycastQueryResultBuffer()														{return mSqResults;}
	PxU32					getRaycastQueryResultBufferSize() const												{return mNumQueries;}
	void					setPreFilterShader(PxBatchQueryPreFilterShader preFilterShader)						{mPreFilterShader=preFilterShader;}
	void					setSpuPreFilterShader(void* spuPreFilterShader, const PxU32 spuPreFilterShaderSize)	{mSpuPreFilterShader=spuPreFilterShader; mSpuPreFilterShaderSize=spuPreFilterShaderSize;}

private:
	PxRaycastQueryResult*	mSqResults;
	PxRaycastHit*			mSqHitBuffer;
	void*					mSpuPreFilterShader;
	PxU32					mSpuPreFilterShaderSize;
	PxU32					mNumQueries;

	PxBatchQueryPreFilterShader mPreFilterShader;

#ifndef PX_X64
	PxU32 mPad[2];
#else
	PxU32 mPad[2];
#endif

	void init()
	{
		mPreFilterShader=SampleVehicleWheelRaycastPreFilter;
		mSpuPreFilterShader=NULL;
		mSpuPreFilterShaderSize=0;
	}

	SampleVehicleSceneQueryData()
	{
		init();
	}

	~SampleVehicleSceneQueryData()
	{
	}
};

class CPhysX
{
public:
													
													~CPhysX();

	static CPhysX *									GetPhysX();
	static void										ReleasePhysics();

public:
	void											Release();
	void											Update( float fDT );
	D3DXVECTOR3										GetPos() const													{ return m_vPos; }
	void											SetForce( bool bForce, const PxVec3& vec )						{ m_bForce = bForce; m_Forvard = vec; }
	void											ConvertToD3D9( D3DMATRIX & dxmat, const physx::PxMat44 & mat );
	physx::PxCooking *								GetCooking() const												{ return m_pCooking; }
	physx::PxPhysics *								GetPhysics() const												{ return m_pPhysics; }
	void											PushActor( PxRigidActor * pActor )								{ m_Actors.push_back( pActor ); }
	void											PushMaterial( PxMaterial * pMaterial )							{ m_Materials.push_back( pMaterial ); }
	bool											AddActorScene( PxRigidActor * pActor );
	void											AddTank( PxVehicleDriveTank * pTank )							{ m_Tanks.push_back( pTank ); }
	PxVehicleWheels *								GetTank();

private:
	bool											InitPhisX();
													CPhysX();

private:	
	static CPhysX *									m_pThis;
	physx::PxPhysics *								m_pPhysics;
	physx::PxScene *								m_pScene;
	physx::PxFoundation *							m_pFoundation;
	physx::PxCooking *								m_pCooking;
	physx::PxRigidStatic *							m_pEarth;
	std::vector< physx::PxMaterial* >				m_Materials;
	PVD::PvdConnection * 							m_pPVD;
	physx::PxProfileZoneManager *					m_pProfileZoneManager;
	physx::PxRigidDynamic *							m_pBox;
	D3DXVECTOR3										m_vPos;
	bool											m_bForce;
	PxVec3											m_Forvard;
	std::vector< PxRigidActor* >					m_Actors;
	physx::PxProfileZone*							m_pProfileZone;
	std::vector< PxVehicleWheels* >					m_Tanks;
	PxVehicleDrivableSurfaceToTireFrictionPairs *	m_pSurfaceTirePairs;
	PxBatchQuery *									m_pSqWheelRaycastBatchQuery;
	SampleVehicleSceneQueryData *					mSqData;
};