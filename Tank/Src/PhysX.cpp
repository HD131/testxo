#include "PhysX.h"
#include "Help.h"

#define CHECK_MSG(exp, msg) (!!(exp) || (physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, msg), 0) )
#define PX_FREE(x)              physx::shdfnd::Allocator().deallocate(x)
#define PX_ALLOC(n, name)		 physx::shdfnd::NamedAllocator(name).allocate(n, __FILE__, __LINE__)

void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	//CHECK_MSG( 0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_DRIVABLE_SURFACE;
}

void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	//CHECK_MSG( 0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData)
{
	//CHECK_MSG(0==qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

SampleVehicleSceneQueryData* SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels)
{
	const PxU32 size = sizeof(SampleVehicleSceneQueryData) + sizeof(PxRaycastQueryResult)*maxNumWheels + sizeof(PxRaycastHit)*maxNumWheels;
	SampleVehicleSceneQueryData* sqData = new SampleVehicleSceneQueryData[ size ];//(SampleVehicleSceneQueryData*)PX_ALLOC(size, PX_DEBUG_EXP("PxVehicleNWSceneQueryData"));
	sqData->init();
	PxU8* ptr = (PxU8*) sqData;
	ptr += sizeof(SampleVehicleSceneQueryData);
	sqData->mSqResults = (PxRaycastQueryResult*)ptr;
	ptr +=  sizeof(PxRaycastQueryResult)*maxNumWheels;
	sqData->mSqHitBuffer = (PxRaycastHit*)ptr;
	ptr += sizeof(PxRaycastHit)*maxNumWheels;
	sqData->mNumQueries = maxNumWheels;
	return sqData;
}

void SampleVehicleSceneQueryData::free()
{
	//delete[] sqData;//PX_FREE( this );
}

PxBatchQuery* SampleVehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene)
{
	PxBatchQueryDesc sqDesc;

	sqDesc.userRaycastResultBuffer	= mSqResults;
	sqDesc.userRaycastHitBuffer		= mSqHitBuffer;
	sqDesc.raycastHitBufferSize		= mNumQueries;
	sqDesc.preFilterShader			= mPreFilterShader;
	sqDesc.spuPreFilterShader		= mSpuPreFilterShader;
	sqDesc.spuPreFilterShaderSize	= mSpuPreFilterShaderSize;

	return scene->createBatchQuery(sqDesc);
}

//--------------------------------------------------------------------------------------------------------//

PxDefaultAllocator gDefaultAllocatorCallback;

//Tire model friction for each combination of drivable surface type and tire type.
static PxF32 gTireFrictionMultipliers[4][4] =
{
	//WETS	SLICKS	ICE		MUD
	{0.95f,	0.95f,	0.95f,	0.95f},		//MUD
	{1.10f,	1.15f,	1.10f,	1.10f},		//TARMAC
	{0.70f,	0.70f,	0.70f,	0.70f},		//ICE
	{0.80f,	0.80f,	0.80f,	0.80f}		//GRASS
};

CPhysX::CPhysX():
	m_pPhysics( 0 ),
	m_pScene( 0 ),
	m_pFoundation( 0 ),
	m_pCooking( 0 ),
	m_Materials( 0 ),
	m_pEarth( 0 ),
	m_pPVD( 0 ),
	m_pProfileZoneManager( 0 ),
	m_pBox( 0 ),
	m_pProfileZone( 0 ),
	m_pSurfaceTirePairs( 0 ),
	m_pSqWheelRaycastBatchQuery( 0 ),
	mSqData( 0 )
{
	m_Forvard = PxVec3( 0, 0, 0 );
}

CPhysX::~CPhysX()
{
}

PxVehicleWheels* CPhysX::GetTank()
{ 
	if( !m_Tanks.empty() ) 
		return m_Tanks[ 0 ];
	
	return 0;
}

bool CPhysX::InitPhisX()
{
	static PxDefaultErrorCallback	m_DefaultErrorCallback;
	//static PxDefaultAllocator		m_DefaultAllocatorCallback;
	PxAllocatorCallback* allocator = &gDefaultAllocatorCallback;

	//create a physx foundation and the physics itself
	if( m_pFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, *allocator, m_DefaultErrorCallback ) )
	{		
		m_pProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager( m_pFoundation );

		if( !m_pProfileZoneManager )
			Log( "PxProfileZoneManager::createProfileZoneManager failed!" );

		m_pPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pProfileZoneManager );

		if( !m_pPhysics ) 
		{
			Release();
			Log( "Error creating PhysX device." );
			return false;
		}		

		PxInitVehicleSDK( *m_pPhysics );		

		if( !m_pProfileZone )
			m_pProfileZone = &physx::PxProfileZone::createProfileZone( &(*m_pFoundation), "SampleProfileZone" );

		if( m_pProfileZone )
			m_pProfileZoneManager->addProfileZone( *m_pProfileZone );

		if( !PxInitExtensions( *m_pPhysics ) )
		{
			Log( "PxInitExtensions failed!" );
			Release();
			return false;
		}

		m_pCooking = PxCreateCooking( PX_PHYSICS_VERSION, *m_pFoundation, PxCookingParams() );
		if( !m_pCooking )
			Log("PxCreateCooking failed!" );

		//PVD::PvdConnection* pConnection( physx::PxVisualDebuggerExt::connect( m_pPhysics->getPvdConnectionManager(), "localhost", 5425, 100, true ) );
		PxVisualDebuggerConnectionFlags theConnectionFlags( PxVisualDebuggerExt::getAllConnectionFlags() );		

		if( m_pPhysics->getVisualDebugger() )
		{
			m_pPhysics->getVisualDebugger()->setVisualizeConstraints(true);
			m_pPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlags::eTRANSMIT_CONTACTS, true);
		}
		
		// PhysX Visual Debugger		
		if( m_pPhysics->getPvdConnectionManager() )
		{
			PxVisualDebuggerConnectionFlags theConnectionFlags( PxVisualDebuggerExt::getAllConnectionFlags() );
			m_pPVD = PxVisualDebuggerExt::createConnection( m_pPhysics->getPvdConnectionManager(), "127.0.0.1", 5425, 10, theConnectionFlags );		
		}


		//Create scene description
		PxSceneDesc sceneDesc( m_pPhysics->getTolerancesScale() );
		
		sceneDesc.gravity = PxVec3( 0.f, -9.8f, 0.f );
		sceneDesc.flags  |= PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
		sceneDesc.flags  |= PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
		sceneDesc.flags  |= PxSceneFlag::eENABLE_KINEMATIC_PAIRS;

		if( !sceneDesc.cpuDispatcher ) 
		{
			PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
			if( !mCpuDispatcher )
				Log( "PxDefaultCpuDispatcherCreate failed!" );

			sceneDesc.cpuDispatcher = mCpuDispatcher;
		}

	 	if( !sceneDesc.filterShader )
	 		sceneDesc.filterShader = PxDefaultSimulationFilterShader;

		//Create the scene
		m_pScene = m_pPhysics->createScene( sceneDesc );
		if( !m_pScene )
		{
			Log( "FATAL: createScene failed!" );
			Release();
			return false;
		}

		m_pScene->setVisualizationParameter( PxVisualizationParameter::eSCALE,             1.f );
		m_pScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_SHAPES,  1.f );
		m_pScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_STATIC,  1.f );
		m_pScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f );
		m_pScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_EDGES ,  1.f );

		PxMaterial* pMaterial = m_pPhysics->createMaterial( 0.5f, 0.5f, 0.1f );    //коэффициенты трения скольжения и покоя(Dynamic friction,Static friction), коэффициент упругости
		if( pMaterial )
			m_Materials.push_back( pMaterial );
		else
			Log( "CreateMaterial failed!");

		PxF32 restitutions[4] = {0.2f, 0.2f, 0.2f, 0.2f};
		PxF32 staticFrictions[4] = {0.5f, 0.5f, 0.5f, 0.5f};
		PxF32 dynamicFrictions[4] = {0.5f, 0.5f, 0.5f, 0.5f};
		PxMaterial* mStandardMaterials[4];
		PxVehicleDrivableSurfaceType mVehicleDrivableSurfaceTypes[4];

		for(PxU32 i=0;i<4;i++) 
		{
			//Create a new material.
			mStandardMaterials[i] = m_pPhysics->createMaterial(staticFrictions[i], dynamicFrictions[i], restitutions[i]);			

			//Set up the drivable surface type that will be used for the new material.
			mVehicleDrivableSurfaceTypes[i].mType = i;
		}

		mSqData = SampleVehicleSceneQueryData::allocate( 1 * 4 );

		//Set up the friction values arising from combinations of tire type and surface type.		
		m_pSurfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::create( 4, 4, (const PxMaterial**)mStandardMaterials, mVehicleDrivableSurfaceTypes );
		for(PxU32 i=0;i<4;i++)
		{
			for(PxU32 j=0;j<4;j++)
			{
				m_pSurfaceTirePairs->setTypePairFriction( i, j, gTireFrictionMultipliers[ i ][ j ] );
			}
		}

// 		PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(12);
// 		PxVehicleDriveSimData4W driveSimData;
// 		setupTankSimData(wheelsSimData,driveSimData);
// 		PxVehicleDriveTank* tank = PxVehicleDriveTank::allocate(12);
// 		PxRigidDynamic* vehActor=createVehicleActor12W();
// 		tank->setup(&physics,vehActor,*wheelsSimData,tankDriveSimData,12);


// 		PxMaterial* pMaterial = m_pPhysics->createMaterial( 0.5f, 0.5f, 0.1f );    //коэффициенты трения скольжения и покоя(Dynamic friction,Static friction), коэффициент упругости
// 		if( pMaterial )
// 			m_Materials.push_back( pMaterial );
// 		else
// 			Log( "CreateMaterial failed!");
// 
// 		m_pEarth = PxCreatePlane( *m_pPhysics, PxPlane( PxVec3( 0, 1, 0 ), 0 ), *pMaterial );
// 		if (!m_pEarth)
// 			Log( "create shape failed!" );
// 
// 		m_pScene->addActor( *m_pEarth );
// 		m_Actors.push_back( m_pEarth );
		
	

// 		pMaterial = m_pPhysics->createMaterial( 0.1f, 0.1f, 0.1f ); 
// 		if( pMaterial )
// 			m_Materials.push_back( pMaterial );
// 		else
// 			Log( "CreateMaterial failed!");
// 
// 		if( m_pBox = PxCreateDynamic( *m_pPhysics, PxTransform( PxVec3( 0, 30, 0 ) ), PxBoxGeometry( PxVec3( 5, 4.6f, 5 ) ), *pMaterial, 1.f ) )
// 		{
// 			m_pBox->setActorFlag(PxActorFlag::eVISUALIZATION, true);
// 			m_pBox->setAngularDamping(0.5f);
// 			//m_pBox->setMass( 10000.f );
// 			m_pBox->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false );
// 			PxRigidBodyExt::updateMassAndInertia( *m_pBox, 10000.f );
// 			
// 			//m_pScene->addActor( *m_pBox );
// 			//m_Actors.push_back( m_pBox );
// 		}

		return true;
	}

	return false;
}

void CPhysX::Update( float fDT )
{
	static float fTime = 0.f;
	const static float fStepSize = 1.f / 60.f;

	fTime += fDT;

	if( fTime >= fStepSize )
	{
		fTime = 0.f;
		//Create a scene query if we haven't already done so.
		if( !m_pSqWheelRaycastBatchQuery && mSqData )
			m_pSqWheelRaycastBatchQuery = mSqData->setUpBatchedSceneQuery( m_pScene );
		
		//Raycasts.
		

		if( m_pScene )
		{
			if( !m_Tanks.empty() && mSqData )
			{
				PxVehicleSuspensionRaycasts( m_pSqWheelRaycastBatchQuery, m_Tanks.size(), (PxVehicleWheels**)&m_Tanks[ 0 ], mSqData->getRaycastQueryResultBufferSize(), mSqData->getRaycastQueryResultBuffer() );
				PxVehicleUpdates( fStepSize, m_pScene->getGravity(), *m_pSurfaceTirePairs, m_Tanks.size(), (PxVehicleWheels**)&m_Tanks[ 0 ] );
			}

			m_pScene->simulate( fStepSize );
			m_pScene->fetchResults( true );

// 			PxTransform temp = m_pBox->getGlobalPose();
// 			m_vPos = D3DXVECTOR3( temp.p.x, temp.p.y, temp.p.z );
// 
// 			if( m_bForce )
// 			{
// 				PxVec3 dir = m_Forvard;
// 				m_pBox->addForce( dir * 30.f, PxForceMode::eACCELERATION );
// 			}

			
// 			PxTransform pos1, pos2;
// 			PxRigidActor* pActor1 = m_Actors[1];
// 			PxRigidActor* pActor2 = m_Actors[0];
// 			PxBounds3 b1;
// 			PxBounds3 b2;
// 			PxVec3 c1, c2;
// 			if( pActor1->isRigidDynamic() )
// 			{
// 				PxRigidDynamic* pStat = reinterpret_cast<PxRigidDynamic*>( pActor1 );
// 				if( pStat )
// 					pos1 = pStat->getGlobalPose();
// 
// 				b1 = pActor1->getWorldBounds();
// 				c1 = b1.getCenter();
// 			}
// 
// 			if( pActor2->isRigidStatic() )
// 			{
// 				PxRigidStatic* pStat = reinterpret_cast<PxRigidStatic*>( pActor2 );
// 				if( pStat )
// 				{
// 					pos2 = pStat->getGlobalPose();
// 					//pStat->setGlobalPose( PxTransform(PxVec3( 0, 0.f + fDT, 0 ) ) );
// 				}
// 
// 				b2 = pActor2->getWorldBounds();
// 				c2 = b2.getCenter();
// 			}

			int a = 0;

			//D3DXMATRIX mPose; // матрица мира физ. объекта

			// получаем значение матрицы у актора.
			//m_pBox->getGlobalPose().getColumnMajor44(reinterpret_cast<NxF32*>(&mPose));
		}
	}
}

void CPhysX::ConvertToD3D9( D3DMATRIX &dxmat, const physx::PxMat44 &mat )
{
	PxMat44 mat44 = mat.getTranspose();
	memcpy( &dxmat._11, mat44.front(), 4 * 4 * sizeof(float) );
}

bool CPhysX::AddActorScene( PxRigidActor* pActor )
{
	if( pActor && m_pScene )
	{
		m_pScene->addActor( *pActor );
		PushActor( pActor );
		return true;
	}

	return false;
}

void CPhysX::Release()
{
	SAFE_RELEASE( m_pBox );
	SAFE_RELEASE( m_pPVD );
	SAFE_RELEASE( m_pEarth );

	for( std::vector< PxMaterial* >::iterator iter = m_Materials.begin(); iter != m_Materials.end(); ++iter )
	{
		(*iter)->release();
	}

// 	for( std::vector< PxVehicleWheels* >::iterator iter = m_Tanks.begin(); iter != m_Tanks.end(); ++iter )
// 	{		
// 		(*iter)->free();
// 	}

	SAFE_RELEASE( m_pScene );
	SAFE_RELEASE( m_pCooking );	
	SAFE_RELEASE( m_pProfileZone);
	PxCloseVehicleSDK();
	SAFE_RELEASE( m_pPhysics );	
	SAFE_RELEASE( m_pProfileZoneManager );
	SAFE_RELEASE( m_pFoundation );
}