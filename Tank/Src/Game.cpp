#include "Game.h"

HWND		CGame::m_hWnd		= 0;
HINSTANCE	CGame::m_hInstance	= 0;
bool		CGame::m_bEndGame   = false;
CPhysX*		CGame::m_pPhysX		= 0;

CGame::CGame() :
	m_nWidth( 1024 ),
	m_nHeight( 768 ),
	m_DeviceInput( 0 ),
	m_pMyTank( 0 )	
{
}

CGame::~CGame()
{
}

CObject* CGame::GetObject( std::string srName )
{
	std::map< std::string, CObject* >::iterator iter = m_Objects.find( srName );
	if( iter != m_Objects.end() )
		return iter->second;

	return 0;
}
C3DModel* g_pModel;
HWND CGame::Init( HINSTANCE hInstance, WNDPROC pWndProc )
{
	WNDCLASS w; 
	ZeroMemory( &w, sizeof( WNDCLASS ) );
	Log( "Create window game" );	
	
	w.style         = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc   = pWndProc;
	w.hInstance     = hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	w.lpszClassName = "MyTank";
	w.hIcon         = LoadIcon( 0, IDI_QUESTION );	
	
	RegisterClass( &w );

	if( m_hWnd = CreateWindow( "MyTank", "Tank", WS_SYSMENU | WS_MINIMIZEBOX, 250, 150, m_nWidth, m_nHeight, 0, 0, hInstance, 0 ) )
	{
		m_hInstance = hInstance;
		if( SUCCEEDED( m_D3D.InitD3D( m_hWnd ) ) )
		{
			m_pPhysX = new CPhysX;

			if( !m_pPhysX->InitPhisX() )
			{
				DELETE_ONE( m_pPhysX );
				return 0;
			}

			m_pCamera = new CameraDevice;
			m_pCamera->SetPosition( D3DXVECTOR3( 0.f, 0.f, -20.f ) );
			m_Camers[ 1 ] = m_pCamera;

			m_pCamera = new CameraDevice;
			m_pCamera->SetPosition( D3DXVECTOR3( -0.f, 5.f, -10.f ) );
			m_Camers[ 2 ] = m_pCamera;

			m_ShaderManager.SetShader( DIFFUSE, m_ShaderManager.LoadShader( "shader\\Diffuse" ) );
			m_ShaderManager.SetShader( Sky, m_ShaderManager.LoadShader( "shader\\Sky" ) );
			m_ShaderManager.SetShader( DIFFUSE_NORMAL_SPECULAR, m_ShaderManager.LoadShader( "shader\\DNS" ) );
			m_Sky.InitialSky( CD3DGraphic::GetDevice() );

			g_pModel = new C3DModel;
			g_pModel->Parse( "model\\t-34\\tank.BLG" );

			CMesh3D* mesh = new CMesh3D;
			if( SUCCEEDED( mesh->InitMesh( "model\\Tank\\Grass4.x", CD3DGraphic::GetDevice() ) ) )
			{
				m_Mesh.push_back( mesh );
				CObject* pEarth = new CObject;
				pEarth->SetMesh( mesh );							
				pEarth->SetPosition( D3DXVECTOR3( 0.f, 0.f, 0.f ) );

				if ( pEarth->CreateTriangleMesh( m_pPhysX ) )
				{
					PxMaterial*     pMaterial    = m_pPhysX->GetPhysics()->createMaterial( 0.5f, 0.5f, 0.2f );    //коэффициенты трения скольжения и покоя(Dynamic friction,Static friction), коэффициент упругости
					PxTriangleMesh* triangleMesh = pEarth->GetTriangleMesh();
					PxRigidStatic*  pEarth       = m_pPhysX->GetPhysics()->createRigidStatic( PxTransform( physx::PxVec3( 0, 0.f, 0 ) ) );

					if( pEarth && pMaterial && triangleMesh )
					{						
						if( PxShape* pShape = pEarth->createShape( PxTriangleMeshGeometry( triangleMesh ), *pMaterial ) )
						{
							PxFilterData simFilterData;
							simFilterData.word0 = COLLISION_FLAG_GROUND;
							simFilterData.word1 = COLLISION_FLAG_GROUND_AGAINST;
							PxFilterData qryFilterData;
							SampleVehicleSetupDrivableShapeQueryFilterData(&qryFilterData);							
							pShape->setSimulationFilterData(simFilterData);
							pShape->setQueryFilterData(qryFilterData);
							pShape->setFlag( PxShapeFlag::eSIMULATION_SHAPE, true );
						}
						
						if( pMaterial )
							m_pPhysX->PushMaterial( pMaterial );
						
						m_pPhysX->AddActorScene( pEarth );
					}
				}


// 				int nSize = sizeof(PxHeightFieldSample) * 128 * 128;
// 				PxHeightFieldSample* samples = new PxHeightFieldSample[ nSize ];
// 				ZeroMemory( samples, nSize );
// 
// 				BYTE* raw = new BYTE[ nSize ];
// 				ZeroMemory( raw, nSize );
// 				if( FILE* pFile = fopen( "model\\Tank\\terrain.raw", "rb" ) )
// 				{
// 					fread( raw, 1, nSize, pFile );
// 				}				
// 
// 				for( int i =0; i < nSize; ++i )
// 				{
// 					samples[ i ].height = 0;//raw[ i ];
// 				}
// 					
// 				//memset( samples, 1, nSize );
// 
// 				PxHeightFieldDesc heightFieldDesc;
// 				heightFieldDesc.format             = PxHeightFieldFormat::eS16_TM;
// 				heightFieldDesc.nbColumns          = 128;
// 				heightFieldDesc.nbRows             = 128;
// 				heightFieldDesc.samples.data       = samples;
// 				heightFieldDesc.samples.stride     = sizeof(PxHeightFieldSample);
// 
// 				PxMaterial* pMaterial = m_pPhysX->GetPhysics()->createMaterial( 0.5f, 0.5f, 0.1f );
// 
// 				if( PxHeightField* heightField = m_pPhysX->GetPhysics()->createHeightField( heightFieldDesc ) )
// 					if( PxRigidActor* heightFieldActor = m_pPhysX->GetPhysics()->createRigidStatic( PxTransform( physx::PxVec3( -200, 0, -200 ) ) ) )
// 					{
// 						PxShape* shape = heightFieldActor->createShape(PxHeightFieldGeometry(heightField, PxMeshGeometryFlags(), 0.05f, 3.f, 3.f ), *pMaterial );
// 						
// 						// add actor to the scene
// 						m_pPhysX->AddActorScene( heightFieldActor );
// 					}

				m_Objects[ "Earth" ] = pEarth;
			}
			else 
				delete mesh;

			mesh = new CMesh3D;
			if( SUCCEEDED( mesh->InitMesh( "model\\Tank\\Bullet.x", CD3DGraphic::GetDevice() ) ) )
				m_Mesh.push_back( mesh );
			else 
				delete mesh;

			CMesh3D* meshBasa = new CMesh3D;
			if( SUCCEEDED( meshBasa->InitMesh( "model\\t-34\\basa_t34.x", CD3DGraphic::GetDevice() ) ) )
			{
				m_Mesh.push_back( meshBasa );

				CObject* pMeshB = new CObject;
				pMeshB->SetMesh( meshBasa );							
				pMeshB->SetPosition( D3DXVECTOR3( 0.f, 3.f, 0.f ) );
				m_Objects[ "Basa" ] = pMeshB;

				CMesh3D* meshHead = new CMesh3D;
				if( SUCCEEDED( meshHead->InitMesh( "model\\t-34\\tower_t34.x", CD3DGraphic::GetDevice() ) ) )
				{
					m_Mesh.push_back( meshHead );

					CObject* pMeshH = new CObject;					
					pMeshB->SetChild( pMeshH );
					pMeshH->SetMesh( meshHead );								
					pMeshH->SetPosition( D3DXVECTOR3( 0.f, 0.4f, 0.5f ) );					
					m_Objects[ "Turret" ] = pMeshH;

					CMesh3D* meshPushka = new CMesh3D;
					if( SUCCEEDED( meshPushka->InitMesh( "model\\t-34\\gun_t34.x", CD3DGraphic::GetDevice() ) ) )
					{
						m_Mesh.push_back( meshPushka );

						CObject* pMeshP = new CObject;						
						pMeshH->SetChild( pMeshP );
						pMeshP->SetMesh( meshPushka );										
						pMeshP->SetPosition( D3DXVECTOR3( 0.1f, 0.5f, 1.1f ) );
						m_Objects[ "Gun" ] = pMeshP;						

						CMesh3D* meshTrack = new CMesh3D;
						if( SUCCEEDED( meshTrack->InitMesh( "model\\t-34\\track_t34.x", CD3DGraphic::GetDevice() ) ) )
						{
							m_Mesh.push_back( meshTrack );

							CObject* pMeshTrackL = new CObject;						
							pMeshB->SetChild( pMeshTrackL );
							pMeshTrackL->SetMesh( meshTrack );											
							pMeshTrackL->SetPosition( D3DXVECTOR3( 1.2f, -0.48f, -0.1f ) );
							m_Objects[ "TrackL" ] = pMeshTrackL;

							CObject* pMeshTrackR = new CObject;						
							pMeshB->SetChild( pMeshTrackR );
							pMeshTrackR->SetMesh( meshTrack );											
							pMeshTrackR->SetPosition( D3DXVECTOR3( -1.2f, -0.48f, -0.1f ) );
							m_Objects[ "TrackR" ] = pMeshTrackR;

							CMesh3D* meshRoller = new CMesh3D;
							if( SUCCEEDED( meshRoller->InitMesh( "model\\t-34\\roller_1.x", CD3DGraphic::GetDevice() ) ) )
							{
								m_Mesh.push_back( meshRoller );

								CObject* pMeshRollerL1 = new CObject;						
								pMeshB->SetChild( pMeshRollerL1 );
								pMeshRollerL1->SetMesh( meshRoller );								
								m_Objects[ "RollerL1" ] = pMeshRollerL1;

								CObject* pMeshRollerR1 = new CObject;						
								pMeshB->SetChild( pMeshRollerR1 );
								pMeshRollerR1->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR1;

								CObject* pMeshRollerL2 = new CObject;						
								pMeshB->SetChild( pMeshRollerL2 );
								pMeshRollerL2->SetMesh( meshRoller );								
								m_Objects[ "RollerL2" ] = pMeshRollerL2;

								CObject* pMeshRollerR2 = new CObject;						
								pMeshB->SetChild( pMeshRollerR2 );
								pMeshRollerR2->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR2;

								CObject* pMeshRollerL3 = new CObject;						
								pMeshB->SetChild( pMeshRollerL3 );
								pMeshRollerL3->SetMesh( meshRoller );								
								m_Objects[ "RollerL1" ] = pMeshRollerL3;

								CObject* pMeshRollerR3 = new CObject;						
								pMeshB->SetChild( pMeshRollerR3 );
								pMeshRollerR3->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR3;

								CObject* pMeshRollerL4 = new CObject;						
								pMeshB->SetChild( pMeshRollerL4 );
								pMeshRollerL4->SetMesh( meshRoller );								
								m_Objects[ "RollerL2" ] = pMeshRollerL4;

								CObject* pMeshRollerR4 = new CObject;						
								pMeshB->SetChild( pMeshRollerR4 );
								pMeshRollerR4->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR4;

								CObject* pMeshRollerL5 = new CObject;						
								pMeshB->SetChild( pMeshRollerL5 );
								pMeshRollerL5->SetMesh( meshRoller );								
								m_Objects[ "RollerL1" ] = pMeshRollerL5;

								CObject* pMeshRollerR5 = new CObject;						
								pMeshB->SetChild( pMeshRollerR5 );
								pMeshRollerR5->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR5;

								CObject* pMeshRollerL6 = new CObject;						
								pMeshB->SetChild( pMeshRollerL6 );
								pMeshRollerL6->SetMesh( meshRoller );								
								m_Objects[ "RollerL2" ] = pMeshRollerL6;

								CObject* pMeshRollerR6 = new CObject;						
								pMeshB->SetChild( pMeshRollerR6 );
								pMeshRollerR6->SetMesh( meshRoller );								
								m_Objects[ "RollerR1" ] = pMeshRollerR6;

								CTank* pTank = new CTank;
								pTank->SetDetail( BODY,     pMeshB );
								pTank->SetDetail( TURRET,   pMeshH );
								pTank->SetDetail( GUN,      pMeshP );
								pTank->SetDetail( TRACK_L,  pMeshTrackL );
								pTank->SetDetail( TRACK_R,  pMeshTrackR );
								pTank->SetDetail( WHEEL_LEFT_1ST,  pMeshRollerL1 );
								pTank->SetDetail( WHEEL_RIGHT_1ST, pMeshRollerR1 );
								pTank->SetDetail( WHEEL_LEFT_2ST,  pMeshRollerL2 );
								pTank->SetDetail( WHEEL_RIGHT_2ST, pMeshRollerR2 );
								pTank->SetDetail( WHEEL_LEFT_3ST,  pMeshRollerL3 );
								pTank->SetDetail( WHEEL_RIGHT_3ST, pMeshRollerR3 );
								pTank->SetDetail( WHEEL_LEFT_4ST,  pMeshRollerL4 );
								pTank->SetDetail( WHEEL_RIGHT_4ST, pMeshRollerR4 );
								pTank->SetDetail( WHEEL_LEFT_5ST,  pMeshRollerL5 );
								pTank->SetDetail( WHEEL_RIGHT_5ST, pMeshRollerR5 );
								pTank->SetDetail( WHEEL_LEFT_6ST,  pMeshRollerL6 );
								pTank->SetDetail( WHEEL_RIGHT_6ST, pMeshRollerR6 );


								pTank->CreateTankActor( m_pPhysX );

								m_Tanks.push_back( pTank );
								m_pMyTank = pTank;
							}
							else 
								delete meshRoller;							
						}
						else 
							delete meshTrack;
					}
					else 
						delete meshPushka;
				}
				else 
					delete meshHead;
			}
			else 
				delete meshBasa;			

			return m_hWnd;
		}

		return 0;
	}

	Log( "Error create window game" );
	return false;
}

bool CGame::InitInputDevice()
{
	m_DeviceInput = new CInputDevice;

	if( FAILED( m_DeviceInput->InitInputDevice( m_hWnd, m_hInstance ) ) )
	{
		m_DeviceInput->Release();
		DELETE_ONE( m_DeviceInput );
		return false;
	}

	return true;
}

void CGame::Update( float fDT )
{
	if( m_pPhysX )
	{
		m_pPhysX->Update( fDT );		
	}

	if( CObject* pEarth = GetObject( "Earth" ) )
		pEarth->Update( fDT );	


	if( m_DeviceInput )
	{
		m_DeviceInput->ScanInput( m_pCamera );

		for( std::vector< CTank* >::iterator iter = m_Tanks.begin(); iter != m_Tanks.end(); ++iter )
		{
			
			if( m_pPhysX )
				(*iter)->SetPosition( m_pPhysX->GetPos() );			

			(*iter)->Update( fDT );
		}

		for( std::vector< CBullet* >::iterator iter = m_Bullet.begin(); iter != m_Bullet.end(); ++iter )
		{
			(*iter)->Update( fDT );
		}

		for( std::vector< CBullet* >::iterator iter = m_Bullet.begin(); iter != m_Bullet.end(); ++iter )
		{
			CBullet* pBullet = *iter;
			if( pBullet->IsDown() )
			{
				CParticles* pTemp = new CParticles;
				pTemp->SetPosition( pBullet->GetPosition() );
				pTemp->SetSpeed( 12.f );
				pTemp->Init( CD3DGraphic::GetDevice() );
				m_Particles.push_back( pTemp );

				delete pBullet;
				m_Bullet.erase( iter );
				break;
			}
		}

		for( std::vector< CParticles* >::iterator iter = m_Particles.begin(); iter != m_Particles.end(); ++iter )
		{
			CParticles* pTemp = *iter;
			pTemp->Update( fDT, m_pCamera );

			if( pTemp->IsKill() )
			{
				delete pTemp;
				m_Particles.erase( iter );
				break;
			}
		}
	}

	if( m_pMyTank && m_DeviceInput )
	{			
		if( m_DeviceInput->PressKey( DIK_1 ) )
			m_pCamera = m_Camers[ 1 ];

		if( m_DeviceInput->PressKey( DIK_2 ) )
			m_pCamera = m_Camers[ 2 ];

		// поворот башни налево
		if( m_DeviceInput->PressKey( DIK_Q ) )
			m_pMyTank->RotateTurret( fDT );

		// поворот башни направо
		if( m_DeviceInput->PressKey( DIK_E ) )
			m_pMyTank->RotateTurret( -fDT );

		// поднять пушку
		if( m_DeviceInput->PressKey( DIK_Z ) )
			m_pMyTank->RotateGun( -fDT );

		// опустить пушку
		if( m_DeviceInput->PressKey( DIK_C ) )
			m_pMyTank->RotateGun( fDT );

		// поворот корпуса налево
		m_pMyTank->TurnLeft( m_DeviceInput->PressKey( DIK_LEFTARROW ) );

		// поворот корпуса направо
		m_pMyTank->TurnRight( m_DeviceInput->PressKey( DIK_RIGHTARROW ) );

		// ход вперёд		
		m_pMyTank->MoveForward( m_DeviceInput->PressKey( DIK_UPARROW ) );

		// ход назад
		m_pMyTank->MoveBack( m_DeviceInput->PressKey( DIK_DOWNARROW ) );

		// выстрел из пушки
		if( m_DeviceInput->KeyDown( DIK_SPACE ) )
		{				
			CBullet* bullet = new CBullet;
			bullet->SetMesh( m_Mesh[ 1 ] );
			bullet->SetSpeed( 800.f );

			if( CObject* pTankGun = GetObject( "Gun" ) )
				bullet->SetReleaseMatrix( pTankGun->GetReleaseMatrix() );

			m_Bullet.push_back( bullet );
		}

		if( m_pCamera )
		{
			if( CObject* pTankHead = GetObject( "Turret" ) )
			{					
				D3DXVECTOR3 T = pTankHead->GetForward();
				m_Camers[ 1 ]->SetForvard( T );				
				
				D3DXMATRIX mat   = pTankHead->GetReleaseMatrix();
				D3DXVECTOR3	vPos( mat._41, mat._42, mat._43 );
				D3DXVECTOR3	vPos1 = vPos  - T * 15.0f;	
				m_Camers[ 1 ]->SetPosition( D3DXVECTOR3(vPos1.x, vPos1.y + 4.f, vPos1.z ) );
				
				m_pCamera->Update( fDT );					
			}
		}
	}	
}

void CGame::RenderingScene()
{
	if( IDirect3DDevice9* pD3DDevice = CD3DGraphic::GetDevice() )
	{
		D3DXMATRIX MatrixWorld;		
		float      Ang = timeGetTime() / 200.f;		

		pD3DDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0 ), 1.f, 0 );	// очистка заднего буфера	
		pD3DDevice->BeginScene(); // начало рендеринга	
		
		m_Sky.RenderSky( m_pCamera, m_ShaderManager.GetShader( Sky ) );

		if( CObject* pEarth = GetObject( "Earth" ) )
			pEarth->Render( m_pCamera, m_ShaderManager.GetShader( DIFFUSE ) );

 		D3DXMatrixTranslation( &MatrixWorld, 3.f, 0.8f, 0.f );
		g_pModel->RenderMesh( m_pCamera, MatrixWorld, m_ShaderManager.GetShader( DIFFUSE_NORMAL_SPECULAR ) );
// 		for( std::vector< CMesh3D* >::iterator iter = m_Mesh.begin(); iter != m_Mesh.end(); ++iter )
// 		{
// 			(*iter)->RenderMesh( m_pCamera, MatrixWorld, m_ShaderManager.GetShader( 0 ) );
// 			break;
// 		}

		for( std::vector< CBullet* >::iterator iter = m_Bullet.begin(); iter != m_Bullet.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( DIFFUSE ) );			
		}

		for( std::vector< CTank* >::iterator iter = m_Tanks.begin(); iter != m_Tanks.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( DIFFUSE ) );
		}
		
		for( std::vector< CParticles* >::iterator iter = m_Particles.begin(); iter != m_Particles.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( DIFFUSE ) );
		}		

		pD3DDevice->EndScene();
		pD3DDevice->Present( 0, 0, 0, 0 ); // вывод содержимого заднего буфера в окно
	}
}

void CGame::Release()
{
	for( std::map< std::string, CObject* >::iterator iter = m_Objects.begin(); iter != m_Objects.end(); ++iter )
	{
		DELETE_ONE( iter->second );
	}

	for( std::vector< CTank* >::iterator iter = m_Tanks.begin(); iter != m_Tanks.end(); ++iter )
	{
		DELETE_ONE( *iter );
	}

	for( std::vector< CMesh3D* >::iterator iter = m_Mesh.begin(); iter != m_Mesh.end(); ++iter )
	{
		(*iter)->Release();
		delete *iter;
	}	

	if( m_DeviceInput )
	{
		m_DeviceInput->Release();
		DELETE_ONE( m_DeviceInput );
	}

	for( std::map< int, CameraDevice* >::iterator iter = m_Camers.begin(); iter != m_Camers.end(); ++iter )
	{
		DELETE_ONE( iter->second );
	}

	DELETE_ONE( m_pPhysX );

	m_Objects.clear();
	m_Camers.clear();
	m_Mesh.clear();
	m_Sky.Release();
	m_ShaderManager.Release();
	m_D3D.Release();	
}


