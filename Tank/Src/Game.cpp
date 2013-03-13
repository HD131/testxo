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
			m_pCamera->SetPosition( D3DXVECTOR3( -70.f, 20.f, 0.f ) );
			m_Camers[ 1 ] = m_pCamera;

			m_pCamera = new CameraDevice;
			m_pCamera->SetPosition( D3DXVECTOR3( -0.f, 5.f, -10.f ) );
			m_Camers[ 2 ] = m_pCamera;

			m_ShaderManager.SetShader( 0, m_ShaderManager.LoadShader( "shader\\Diffuse" ) );
			m_ShaderManager.SetShader( 1, m_ShaderManager.LoadShader( "shader\\Sky" ) );
			m_Sky.InitialSky( CD3DGraphic::GetDevice() );

			CMesh3D* mesh = new CMesh3D;
			if( SUCCEEDED( mesh->InitMesh( "model\\Tank\\Grass1.x", CD3DGraphic::GetDevice() ) ) )
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
			if( SUCCEEDED( meshBasa->InitMesh( "model\\Tank\\Basa.x", CD3DGraphic::GetDevice() ) ) )
			{
				m_Mesh.push_back( meshBasa );

				CObject* pMeshB = new CObject;
				pMeshB->SetMesh( meshBasa );							
				pMeshB->SetPosition( D3DXVECTOR3( 0.f, 3.f, 0.f ) );
				m_Objects[ "Basa" ] = pMeshB;

				CMesh3D* meshHead = new CMesh3D;
				if( SUCCEEDED( meshHead->InitMesh( "model\\Tank\\Head.x", CD3DGraphic::GetDevice() ) ) )
				{
					m_Mesh.push_back( meshHead );

					CObject* pMeshH = new CObject;					
					pMeshB->SetChild( pMeshH );
					pMeshH->SetMesh( meshHead );								
					pMeshH->SetPosition( D3DXVECTOR3( 0.f, 0.5f, 0.f ) );					
					m_Objects[ "Turret" ] = pMeshH;

					CMesh3D* meshPushka = new CMesh3D;
					if( SUCCEEDED( meshPushka->InitMesh( "model\\Tank\\Pushka.x", CD3DGraphic::GetDevice() ) ) )
					{
						m_Mesh.push_back( meshPushka );

						CObject* pMeshP = new CObject;						
						pMeshH->SetChild( pMeshP );
						pMeshP->SetMesh( meshPushka );										
						pMeshP->SetPosition( D3DXVECTOR3( 0.f, 0.5f, 0.f ) );
						m_Objects[ "Gun" ] = pMeshP;						

						CMesh3D* meshTrack = new CMesh3D;
						if( SUCCEEDED( meshTrack->InitMesh( "model\\Tank\\Tracks.x", CD3DGraphic::GetDevice() ) ) )
						{
							m_Mesh.push_back( meshTrack );

							CObject* pMeshTrackL = new CObject;						
							pMeshB->SetChild( pMeshTrackL );
							pMeshTrackL->SetMesh( meshTrack );											
							pMeshTrackL->SetPosition( D3DXVECTOR3( 1.6f, -0.7f, -0.4f ) );
							m_Objects[ "TrackL" ] = pMeshTrackL;

							CObject* pMeshTrackR = new CObject;						
							pMeshB->SetChild( pMeshTrackR );
							pMeshTrackR->SetMesh( meshTrack );											
							pMeshTrackR->SetPosition( D3DXVECTOR3( -1.6f, -0.7f, -0.4f ) );
							m_Objects[ "TrackR" ] = pMeshTrackR;

							CMesh3D* meshRoller = new CMesh3D;
							if( SUCCEEDED( meshRoller->InitMesh( "model\\Tank\\Roller.x", CD3DGraphic::GetDevice() ) ) )
							{
								m_Mesh.push_back( meshRoller );

								CObject* pMeshRollerL = new CObject;						
								pMeshB->SetChild( pMeshRollerL );
								pMeshRollerL->SetMesh( meshRoller );											
								pMeshRollerL->SetPosition( D3DXVECTOR3( 1.6f, -1.f, -1.5f ) );
								pMeshRollerL->SetStepRotate( 10.f );
								m_Objects[ "RollerL1" ] = pMeshRollerL;

								CObject* pMeshRollerR = new CObject;						
								pMeshB->SetChild( pMeshRollerR );
								pMeshRollerR->SetMesh( meshRoller );											
								pMeshRollerR->SetPosition( D3DXVECTOR3( -1.6f, -1.f, -1.5f ) );
								pMeshRollerR->SetStepRotate( 10.f );
								m_Objects[ "RollerL2" ] = pMeshRollerR;							

								CTank* pTank = new CTank;
								pTank->SetDetail( BODY,     pMeshB );
								pTank->SetDetail( TURRET,   pMeshH );
								pTank->SetDetail( GUN,      pMeshP );
								pTank->SetDetail( TRACK_L,  pMeshTrackL );
								pTank->SetDetail( TRACK_R,  pMeshTrackR );
								pTank->SetDetail( Roller_L, pMeshRollerL );
								pTank->SetDetail( Roller_R, pMeshRollerR );

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

			(*iter)->Update( fDT, m_pPhysX );
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
		if( m_DeviceInput->PressKey( DIK_LEFTARROW ) )
			m_pMyTank->RotateBody( fDT );

		// поворот корпуса направо
		if( m_DeviceInput->PressKey( DIK_RIGHTARROW ) )
			m_pMyTank->RotateBody( -fDT );

		// ход вперёд
		if( m_DeviceInput->PressKey( DIK_UPARROW ) )
		{
			m_pMyTank->MoveForward( fDT, m_pPhysX );

			if( m_pPhysX )
			{
				D3DXVECTOR3 dir = m_pMyTank->GetForvard();
				m_pPhysX->SetForce( true, PxVec3( dir.x, dir.y, dir.z ) );
			}
		}
		else if( m_pPhysX )
			m_pPhysX->SetForce( false, PxVec3( 0, 0, 0 ) );

		// ход назад
		if( m_DeviceInput->PressKey( DIK_DOWNARROW ) )
			m_pMyTank->MoveForward( -fDT, m_pPhysX );

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
				D3DXVECTOR3	vPos1 = vPos  + T * 35.0f;	
				m_Camers[ 1 ]->SetPosition( D3DXVECTOR3(vPos1.x, vPos1.y, vPos1.z ) );
				
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
		
		m_Sky.RenderSky( m_pCamera, m_ShaderManager.GetShader( 1 ) );

		if( CObject* pEarth = GetObject( "Earth" ) )
			pEarth->Render( m_pCamera, m_ShaderManager.GetShader( 0 ) );

// 		D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
// 		for( std::vector< CMesh3D* >::iterator iter = m_Mesh.begin(); iter != m_Mesh.end(); ++iter )
// 		{
// 			(*iter)->RenderMesh( m_pCamera, MatrixWorld, m_ShaderManager.GetShader( 0 ) );
// 			break;
// 		}

		for( std::vector< CBullet* >::iterator iter = m_Bullet.begin(); iter != m_Bullet.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( 0 ) );			
		}

		for( std::vector< CTank* >::iterator iter = m_Tanks.begin(); iter != m_Tanks.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( 0 ) );
		}
		
		for( std::vector< CParticles* >::iterator iter = m_Particles.begin(); iter != m_Particles.end(); ++iter )
		{
			(*iter)->Render( m_pCamera, m_ShaderManager.GetShader( 0 ) );
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


