#include "Class.h"
#include <iostream>

PxVehiclePadSmoothingData gCarPadSmoothingData=
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL		
		6.0f,	//rise rate eANALOG_INPUT_BRAKE		
		12.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT	
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT	
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL		
		10.0f,	//fall rate eANALOG_INPUT_BRAKE		
		12.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT	
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT	
	}
};

PxF32 gSteerVsForwardSpeedData[2*8]=
{
	0.0f,		0.75f,
	5.0f,		0.75f,
	30.0f,		0.125f,
	120.0f,		0.1f,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32
};
PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData,4);

///////////////////////////////////////////////////////////////////////////////

CShader::CShader() :
	m_pVertexShader( 0 ),
	m_pPixelShader ( 0 ),
	m_pConstTableVS( 0 ),
	m_pConstTablePS( 0 )
{
}

void CShader::Release()
{
	RELEASE_ONE( m_pVertexShader );
	RELEASE_ONE( m_pPixelShader  );
	RELEASE_ONE( m_pConstTableVS );
	RELEASE_ONE( m_pConstTablePS );
}

CShader::~CShader()
{
	Release();
}

CShader* CManagerShader::GetShader( int shader )
{
	std::map< int, CShader* >::iterator iter = m_MapShader.find( shader );
	if( iter != m_MapShader.end() )
		return iter->second;

	return 0;
}

//--------------------------------------------------------------------------------------------------------
CShader* CManagerShader::LoadShader( const char * File )
{	
	IDirect3DDevice9* pD3DDevice = CD3DGraphic::GetDevice();
	if( pD3DDevice && File && strlen( File ) )
	{
		ID3DXBuffer* pErrors        = 0;
		ID3DXBuffer* pShaderBuff    = 0;		
		CShader*     pShader        = new CShader;
		std::string  FileName( File );

		// вертексный шейдер
		std::string FileNameVS = FileName + std::string( ".vsh" );		
		if( SUCCEEDED( D3DXCompileShaderFromFile( FileNameVS.c_str(), 0, 0, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &pShader->m_pConstTableVS ) ) )
		{
			if( pShaderBuff )
			{
				pD3DDevice->CreateVertexShader( (DWORD*)pShaderBuff->GetBufferPointer(), &pShader->m_pVertexShader );
				pShaderBuff->Release();
			}

			// пиксельный шейдер
			std::string FileNamePS = FileName + std::string( ".psh" );
			if( SUCCEEDED( D3DXCompileShaderFromFile( FileNamePS.c_str(), 0, 0, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &pShader->m_pConstTablePS ) ) )
			{
				if( pShaderBuff )
				{
					pD3DDevice->CreatePixelShader( (DWORD*)pShaderBuff->GetBufferPointer(), &pShader->m_pPixelShader );
					pShaderBuff->Release();
				}

				std::string Load = "Load shader " + FileName;
				Log( Load.c_str() );

				return pShader;
			}
			else
			{
				std::string Error = "Error load pixel shader " + FileNameVS;
				Log( Error.c_str() );
			}
		}
		else
		{
			std::string Error = "Error load vertex shader " + FileNameVS;
			Log( Error.c_str() );
		}

		pShader->Release();	
		DELETE_ONE( pShader );					
	}

	Log( "Error D3DDevice Load shader" );	
	return 0;	
}

void CManagerShader::SetShader( int number, CShader* pShader )
{
	if( pShader )
		m_MapShader[ number ] = pShader;
}

void CManagerShader::Release()
{
	for( std::map< int, CShader* >::iterator iter = m_MapShader.begin(), iter_end = m_MapShader.end(); iter != iter_end; ++iter )
	{
		iter->second->Release();
		DELETE_ONE( iter->second );
	}

	m_MapShader.clear();
}

//--------------------------------------------------------------------------------------------------------
CMesh3D::CMesh3D():
	m_pMesh( 0 ),
	m_pD3DDevice( 0 ),
	m_Alpha( 1.f ),
	m_VertexBuffer( 0 ),
	m_IndexBuffer( 0 ),
	m_TexturCount( 0 ),
	m_pMeshMaterial( 0 )	
{
	m_vOffetUV = D3DXVECTOR4( 0.f, 0.f, 0.f, 0.f );
}

HRESULT CMesh3D::InitMesh( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{
	if( !pD3DDevice )
	{
		Log( "Error pointer x-file" );
		return E_FAIL;
	}

	m_pD3DDevice = pD3DDevice;	
	ID3DXBuffer* pMaterialBuffer  = 0;

	if ( FAILED( D3DXLoadMeshFromX( Name, D3DXMESH_SYSTEMMEM, m_pD3DDevice, 0, &pMaterialBuffer, 0, &m_TexturCount, &m_pMesh ) ) )
	{				
		Log( "error load x-file" );
		return E_FAIL;		
	}

	Log( "Load x-file" );
	DWORD i = m_pMesh->GetNumFaces();// — Возвращает количество граней (треугольных ячеек) в сетке.

	m_pMesh->GetVertexBuffer( &m_VertexBuffer );
	m_pMesh->GetIndexBuffer(  &m_IndexBuffer  );

	// Извлекаем свойства материала и названия{имена} структуры
	D3DXMATERIAL * MaterialMesh = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();
	m_pMeshMaterial = new D3DMATERIAL9[ m_TexturCount ];

	std::string FilePath( Name );

	size_t f = FilePath.find_last_of( "\\" );
	if( f != -1 )
		FilePath.erase( f + 1, FilePath.size() );

	for ( DWORD i = 0; i < m_TexturCount; i++ )
	{
		// Копируем материал
		m_pMeshMaterial[i] = MaterialMesh[i].MatD3D;

		// Установить окружающего свет
		m_pMeshMaterial[i].Ambient = m_pMeshMaterial[i].Diffuse;

		// Загружаем текстуру
		if( MaterialMesh[i].pTextureFilename )
		{		
			IDirect3DTexture9* Tex = 0;
			std::string FileName = FilePath + std::string( MaterialMesh[i].pTextureFilename );
			if( FAILED( D3DXCreateTextureFromFileEx( m_pD3DDevice, FileName.c_str(), 0, 0, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0xff000000, 0, 0, &Tex ) ) )
			{
				Log( "error load texture Mesh" );
				m_pMeshTextura.push_back(0);
			}

			// Сохраняем загруженную текстуру
			m_pMeshTextura.push_back(Tex);
		}
		else 
		{
			// Нет текстуры для i-ой подгруппы
			m_pMeshTextura.push_back(0);
		}
	}

	// Уничтожаем буфер материала
	pMaterialBuffer->Release();

	return S_OK;
}

void CMesh3D::RenderMesh( CameraDevice* pCamera, const D3DXMATRIX&  MatrixWorld, const CShader* pShader )
{
	D3DXMATRIX wvp;

	if( m_pMesh && pCamera )
	{
		D3DXMATRIX vp = pCamera->GetMatrixViewProject();
		wvp = MatrixWorld * vp;
		if ( pShader->m_pConstTableVS )
		{
			pShader->m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_mvp",   &wvp );
			pShader->m_pConstTableVS->SetMatrix( m_pD3DDevice, "mat_world", &MatrixWorld );
			pShader->m_pConstTableVS->SetVector( m_pD3DDevice, "vec_light", &g_Light );
			pShader->m_pConstTablePS->SetFloat(  m_pD3DDevice, "diffuse_intensity", g_Diffuse_intensity );	
			pShader->m_pConstTablePS->SetFloat(  m_pD3DDevice, "Alpha", m_Alpha );
			pShader->m_pConstTablePS->SetVector( m_pD3DDevice, "OffsetUV", &m_vOffetUV );

			// устанавливаем шейдеры
			m_pD3DDevice->SetVertexShader( pShader->m_pVertexShader );
			m_pD3DDevice->SetPixelShader( pShader->m_pPixelShader );
			m_pD3DDevice->SetStreamSource( 0, m_VertexBuffer, 0, m_pMesh->GetNumBytesPerVertex() );
			m_pD3DDevice->SetIndices( m_IndexBuffer );
			m_pD3DDevice->SetFVF( m_pMesh->GetFVF() );

			for( DWORD i = 0; i < m_TexturCount; ++i )
			{
				m_pD3DDevice->SetMaterial( &m_pMeshMaterial[i] );
				m_pD3DDevice->SetTexture( 0, m_pMeshTextura[i] );				
			}

			m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_pMesh->GetNumVertices(), 0, m_pMesh->GetNumFaces() );
		}
	}
}

void CMesh3D::Release()
{
	RELEASE_ONE( m_IndexBuffer );
	RELEASE_ONE( m_VertexBuffer );	

	if( m_pMeshMaterial )
		delete[] m_pMeshMaterial;

	RELEASE_ONE( m_pMesh );
}

//-----------------------------------------------------------------------------------
CBullet::CBullet():	
	m_fSpeed( 0.f ),
	m_fTime( 0.f ),
	m_pMeshBullet( 0 ),
	m_fTimeFull( 0.f ),
	m_fLen( 0.f ),
	m_fAngle( 0.f )
{
	m_vPosition  = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_vTargetDir = D3DXVECTOR3( 0.f, 0.f, 1.f );
}

void CBullet::SetReleaseMatrix( const D3DXMATRIX& mat ) 
{ 
	m_MatrixRelease = mat;
	m_vFirstPosition  = D3DXVECTOR3( mat._41,  mat._42, mat._43 );
	m_vPosition       = m_vFirstPosition;

	// сбрасаваем премещение
	m_MatrixRelease._41 = 0;
	m_MatrixRelease._42 = 0;
	m_MatrixRelease._43 = 0;

	D3DXVec3TransformNormal( &m_vTargetDir, &m_vTargetDir, &m_MatrixRelease );
	D3DXVec3Normalize( &m_vTargetDir, &m_vTargetDir );
	m_vTargetDir *= m_fSpeed;

	float a = -4.9f;
	float b = m_vTargetDir.y;
	float c = m_vFirstPosition.y - 0.f;
	float D = b*b - 4*a*c;

	if( D >= 0.f )
	{
		float x1 = (-b + sqrtf( D )) / (2*a);
		float x2 = (-b - sqrtf( D )) / (2*a);

		if( x1 > 0.f )
			m_fTimeFull = x1;
		else if( x2 > 0.f )
			m_fTimeFull = x2;

		m_fLen = m_vFirstPosition.x + m_vTargetDir.x * m_fTimeFull;

		if( m_fTimeFull && m_fSpeed )
		{
			float a1 = acosf( ( m_fLen - m_vFirstPosition.x ) / ( m_fTimeFull * m_fSpeed ) );
			float a2 = D3DX_PI * 0.5f - a1;
			m_fAngle = a1 < a2 ? a1 : a2;
		}
	}
}

void CBullet::Update( float fDT )
{
	if( m_pMeshBullet )
	{
		m_vPosition    = m_vFirstPosition + m_vTargetDir * m_fTime;
 		m_vPosition.y -= m_fTime * m_fTime * 4.9f;
 		m_fTime       += fDT;
	}
}

void CBullet::Render( CameraDevice * pCamera, const CShader* pShader )
{
	if( m_pMeshBullet && pShader )
	{
		D3DXMATRIX MatrixTrans;			
		D3DXMatrixTranslation( &MatrixTrans, m_vPosition.x, m_vPosition.y, m_vPosition.z );		
		m_pMeshBullet->RenderMesh( pCamera, m_MatrixRelease * MatrixTrans , pShader );
	}
}

//--------------------------------------------------------------------------------------------------------
CObject::CObject() :
	m_pObjectParent( 0 ),
	m_fStepMove( 0.005f ),
	m_fStepRotate( 0.5f ),
	m_pMeshObject( 0 ),
	m_pTriangleMesh( 0 ),
	m_bUseMatrix( false )
{	
	D3DXMatrixTranslation( &m_MatrixRelease, 0.f, 0.f, 0.f );
	m_vPosition = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_AngleXYZ  = D3DXVECTOR3( 0.f, 0.f, 0.f );
	m_vOffetUV  = D3DXVECTOR4( 0.f, 0.f, 0.f, 0.f );
}

CObject::~CObject()
{
	SAFE_RELEASE( m_pTriangleMesh );
}

D3DXVECTOR3	CObject::GetAngle()
{ 
	D3DXVECTOR3 vec = m_AngleXYZ;

	if( m_pObjectParent )
		vec += m_pObjectParent->GetAngle();

	return vec; 
}

D3DXVECTOR3	CObject::GetPosition()
{ 
	D3DXVECTOR3 T(0,0,0);
	D3DXVec3TransformCoord( &T, &T, &m_MatrixRelease );
	return T; 
}

D3DXVECTOR3	CObject::GetForward()
{
	D3DXVECTOR3 T( 0, 0, 1 );
	D3DXVec3TransformNormal( &T, &T, &m_MatrixRelease );
	return T; 
}

void CObject::SetChild( CObject* pObj )
{
	if( pObj )		
	{
		pObj->SetParent( this );
		m_ObjectChild.push_back( pObj );
	}
}

void CObject::Update( float fDT )
{
	if( m_pMeshObject )
	{
		if( !m_bUseMatrix )
		{
			D3DXMATRIX MatrixTrans;			
			D3DXMatrixTranslation( &MatrixTrans, m_vPosition.x, m_vPosition.y, m_vPosition.z );

			D3DXMATRIX MatrixRotateX, MatrixRotateY, MatrixRotateZ;
			D3DXMatrixRotationX( &MatrixRotateX, m_AngleXYZ.x );
			D3DXMatrixRotationY( &MatrixRotateY, m_AngleXYZ.y );
			D3DXMatrixRotationZ( &MatrixRotateZ, m_AngleXYZ.z );
			D3DXMATRIX MatrixRotate  = MatrixRotateX * MatrixRotateY * MatrixRotateZ;
			m_MatrixRelease = MatrixRotate * MatrixTrans;

			// если есть предок, то сначала берём его матрицу
			if( m_pObjectParent )
				m_MatrixRelease = m_MatrixRelease * m_pObjectParent->GetReleaseMatrix();
		}		
	}
	
	for( std::vector< CObject* >::iterator iter = m_ObjectChild.begin(); iter != m_ObjectChild.end(); ++iter )
	{
		(*iter)->Update( fDT );
	}	
}

void CObject::Render( CameraDevice* pCamera, const CShader* pShader )
{
	if( IDirect3DDevice9* pD3DDevice = CD3DGraphic::GetDevice() )
	{
		if( m_pMeshObject )
		{
			D3DXVECTOR4	vOffset = m_pMeshObject->GetOffsetUV();
			m_pMeshObject->SetOffsetUV( m_vOffetUV );
			m_pMeshObject->RenderMesh( pCamera, m_MatrixRelease, pShader );
			m_pMeshObject->SetOffsetUV( vOffset );
		}
	}
}

void CObject::SetMesh( CMesh3D* pMesh )
{ 
	if( pMesh )
	{
		m_pMeshObject = pMesh;
		if( ID3DXMesh* Mesh = pMesh->GetMesh() )
		{
			m_BufVertices.clear();			
			DWORD stride = D3DXGetFVFVertexSize( Mesh->GetFVF() );
			BYTE* vbptr  = 0;
			Mesh->LockVertexBuffer( 0, (LPVOID*)&vbptr );
			int ii = -1;
			m_dwNumVertices = Mesh->GetNumVertices();

			for( DWORD i = 0; i < m_dwNumVertices; ++i )
			{
				ii++;
				D3DXVECTOR3* pos = (D3DXVECTOR3*)vbptr;
				m_BufVertices.push_back( pos->x );
				m_BufVertices.push_back( pos->y );
				m_BufVertices.push_back( pos->z );
				vbptr += stride;
			}

			Mesh->UnlockVertexBuffer();

			LPVOID* ppData = 0;
			stride = sizeof( short );
			BYTE* ibptr = 0;
			m_dwNumNumFaces = Mesh->GetNumFaces();
			short* indices = new short[ m_dwNumNumFaces * 3 ];
			m_BufIndices.clear();			

			Mesh->LockIndexBuffer( 0, (LPVOID*)&indices );
			for( DWORD i = 0; i < m_dwNumNumFaces * 3; ++i )
			{
				m_BufIndices.push_back( indices[ i ] );
			}

			Mesh->UnlockIndexBuffer();			
		}
	}
}

bool CObject::CreateTriangleMesh( CPhysX const* pPhysX )
{
	bool bResult = false;
	if( pPhysX && pPhysX->GetPhysics() && !m_BufVertices.empty() && !m_BufIndices.empty() )
	{
		uint NumVerticies = m_BufVertices.size() / 3;
		uint NumTriangles = m_BufIndices.size()  / 3;

		//Create pointer for vertices
		physx::PxVec3* verts = new physx::PxVec3[ NumVerticies ];
		int ii = -1;

		for( uint i = 0; i < NumVerticies; ++i )
		{
			++ii;
			verts[ i ].x = m_BufVertices[   ii ];
			verts[ i ].y = m_BufVertices[ ++ii ];
			verts[ i ].z = m_BufVertices[ ++ii ];
		}

		//Create pointer for indices
		physx::PxU16 *tris = new physx::PxU16[ m_BufIndices.size() ];

		for( uint i = 0; i < m_BufIndices.size(); ++i )		
			tris[ i ] = m_BufIndices[ i ];

		// Build physical model
		physx::PxTriangleMeshDesc TriMeshDesc;

		TriMeshDesc.points.count  = NumVerticies;		
		TriMeshDesc.points.stride = sizeof(physx::PxVec3);
		TriMeshDesc.points.data	  = verts;

		TriMeshDesc.triangles.count  = NumTriangles;
		TriMeshDesc.triangles.stride = 3 * sizeof(physx::PxU16);	
		TriMeshDesc.triangles.data   = tris;
		
		TriMeshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES;// | physx::PxMeshFlag::eFLIPNORMALS;

		PxToolkit::MemoryOutputStream writeBuffer;
		PxCooking* pCooking = pPhysX->GetCooking();

		if( pCooking && TriMeshDesc.isValid() )
			if( pCooking->cookTriangleMesh( TriMeshDesc, writeBuffer ) )
			{
				PxToolkit::MemoryInputData readBuffer( writeBuffer.getData(), writeBuffer.getSize() );
				m_pTriangleMesh = pPhysX->GetPhysics()->createTriangleMesh( readBuffer );
				bResult = true;
			}

		delete[] verts;
		delete[] tris;
	}
	
	return bResult;
}

//------------------------------------------------------------------------------------------

CTank::CTank():
	m_pActor( 0 ),
	m_bMoveForward( false ),
	m_bMoveBack( false ),
	m_bTurnLeft( false ),
	m_bTurnRight( false ),	
	m_pPhysX( 0 ),
	m_fSpeedTank( 0.f )
{
}

void CTank::Update( float fDT )
{
	if( m_pActor )
	{							
		PxMat44 Matrix = physx::PxMat44( physx::PxMat33( m_pActor->getGlobalPose().q ), m_pActor->getGlobalPose().p );
		D3DXMATRIX dxmat;				
		memcpy( &dxmat._11, Matrix.front(), 4 * 4 * sizeof(float) );		

		if( CObject* pBody = GetDetail( BODY ) )
			pBody->SetReleaseMatrix( dxmat, true );			

		if( m_pPhysX )
		{
			if( PxVehicleWheels* pDriveTank = m_pPhysX->GetTank() )
			{
				PxShape* carShapes[ MAX_DETAIL ];
				
				const PxU32 numShapes = m_pActor->getNbShapes();
				m_pActor->getShapes( carShapes, numShapes );
				for( PxU32 i = 0; i < numShapes - 1; ++i )
				{
					const PxTransform& Trans = carShapes[ i ]->getLocalPose();								
					
					PxMat44 MatrixR = physx::PxMat44(physx::PxMat33( Trans.q ), Trans.p );
					D3DXMATRIX dxmatR;
					memcpy( &dxmatR._11, MatrixR.front(), 4 * 4 * sizeof(float) );

					if( CObject* pObj = GetDetail( (EDetailTank)i ) )
						pObj->SetReleaseMatrix( dxmatR * dxmat, true );
				}

				PxVehicleDriveTank&    vehDriveTank = static_cast< PxVehicleDriveTank& >( *pDriveTank );
				PxVehicleDriveDynData& driveDynData = vehDriveTank.mDriveDynData;

				PxReal e = driveDynData.getEngineRotationSpeed();
				m_fSpeedTank = pDriveTank->computeForwardSpeed();

				static char t[ 256 ] = {0};
				sprintf( t, "EngineRotationSpeed=%f, EngineRotationSpeed=%f", m_fSpeedTank, e );
				//SetWindowText( GetForegroundWindow(), t );
				
				PxVehicleDriveTankRawInputData  carRawInputs( PxVehicleDriveTank::eDRIVE_MODEL_STANDARD );

				carRawInputs.setDigitalAccel( m_bMoveForward || m_bMoveBack || m_bTurnLeft || m_bTurnRight );

				carRawInputs.setDigitalLeftBrake( m_bTurnLeft );						// левая  кнопка тормоза
				carRawInputs.setDigitalRightBrake( m_bTurnRight );						// правая кнопка тормоза
				
				carRawInputs.setDigitalLeftThrust( m_bMoveForward || !m_bTurnLeft );
				carRawInputs.setDigitalRightThrust( m_bMoveForward || !m_bTurnRight );

				if( !m_bMoveForward && !m_bMoveBack && !m_bTurnLeft && !m_bTurnRight )
				{
					carRawInputs.setDigitalLeftBrake( true );	// левая  кнопка тормоза
					carRawInputs.setDigitalRightBrake( true );	// правая кнопка тормоза
				}

				//carRawInputs.setGearUp( !m_bMoveForward && m_bMoveBack);
				//carRawInputs.setGearDown( !m_bMoveBack && m_bMoveForward );				

// 				if( m_bMoveBack )
// 				{
// 					carRawInputs.setDigitalLeftBrake( true );	// левая  кнопка тормоза
// 					carRawInputs.setDigitalRightBrake( true );	// правая кнопка тормоза
// 				}

				static PxVehicleKeySmoothingData KeySmoothingData =
				{
					{
						3.f,	//rise rate eANALOG_INPUT_ACCEL		
						3.f,	//rise rate eANALOG_INPUT_BRAKE		
						5.f,	//rise rate eANALOG_INPUT_HANDBRAKE	
						2.f,	//rise rate eANALOG_INPUT_STEER_LEFT	
						2.f,	//rise rate eANALOG_INPUT_STEER_RIGHT	
					},
					{
						5.f,	//fall rate eANALOG_INPUT__ACCEL		
						5.f,	//fall rate eANALOG_INPUT__BRAKE		
						10.f,	//fall rate eANALOG_INPUT__HANDBRAKE	
						5.f,	//fall rate eANALOG_INPUT_STEER_LEFT	
						5.f		//fall rate eANALOG_INPUT_STEER_RIGHT	
					}
				};

				PxVehicleDriveTankSmoothDigitalRawInputsAndSetAnalogInputs( KeySmoothingData, carRawInputs, fDT, vehDriveTank );			
			}
		}
	}

		if( CObject* pTankTrack = GetDetail( TRACK_L ) )					
			pTankTrack->SetOffsetUV( D3DXVECTOR4( 0.f, m_fSpeedTank*3, 0.f, 0.f ) );

		if( CObject* pTankTrack = GetDetail( TRACK_R ) )					
			pTankTrack->SetOffsetUV( D3DXVECTOR4( 0.f, m_fSpeedTank*3, 0.f, 0.f ) );

	for( std::map< EDetailTank, CObject* >::iterator iter = m_ObjectsTank.begin(), iter_end = m_ObjectsTank.end(); iter != iter_end; ++iter )
	{
		CObject* pObj = iter->second;
		pObj->Update( fDT );
	}
}

void CTank::Render( CameraDevice* pCamera, const CShader* pShader )
{
	for( std::map< EDetailTank, CObject* >::iterator iter = m_ObjectsTank.begin(), iter_end = m_ObjectsTank.end(); iter != iter_end; ++iter )
	{
		CObject* pObj = iter->second;
		pObj->Render( pCamera, pShader );
	}
}

CObject* CTank::GetDetail( EDetailTank detail )
{
	std::map< EDetailTank, CObject* >::iterator iter = m_ObjectsTank.find( detail );
	if( iter != m_ObjectsTank.end() )
		return iter->second;

	return 0;
}

void CTank::SetDetail( EDetailTank detail, CObject* pObj )
{
	m_ObjectsTank[ detail ] = pObj;
}

void CTank::MoveForward( bool bForward )
{
	m_bMoveForward = bForward;
}

void CTank::MoveBack( bool bBack )
{
	m_bMoveBack = bBack;
}

void CTank::TurnRight( bool bRight )
{
	m_bTurnRight = bRight;
}

void CTank::TurnLeft( bool bLeft )
{
	m_bTurnLeft = bLeft;
}

void CTank::RotateTurret( float fDT )
{
	if( CObject* pTurret = GetDetail( TURRET ) )
	{
		pTurret->RotateAxisY( fDT );
	}
}

void CTank::RotateGun( float fDT )
{
	if( CObject* pGun = GetDetail( GUN ) )
	{
		pGun->RotateAxisZ( fDT );
	}
}

void CTank::SetPosition( const D3DXVECTOR3& vPos )
{
	if( CObject* pBody = GetDetail( BODY ) )
		pBody->SetPosition( vPos );
}

D3DXVECTOR3 CTank::GetForvard()
{
	D3DXVECTOR3 dir( 0.f, 0.f, 0.f );

	if( CObject* pBody = GetDetail( BODY ) )
		dir = pBody->GetForward();

	return dir;
}

bool CTank::CreateTankActor( CPhysX * pPhysX )
{
// 	CParamTank* pParamTank = new CParamTank;
// 
// 	if( !CLua::LoadParamTank( "", &pParamTank ) )
// 	{
// 
// 	}

	if( CObject* pBody = GetDetail( BODY ) )
	{
		if ( pBody->CreateTriangleMesh( pPhysX ) )
		{
			m_pPhysX = pPhysX;
			pBody->Update( 0.f );
			
			PxTriangleMesh* triangleMesh = pBody->GetTriangleMesh();
			D3DXVECTOR3     Position     = pBody->GetPosition();
			//D3DXComputeBoundingBox(Vertices,  g_pMesh->GetNumVertices(),  FVFVertexSize, &pMin, &pMax);
			//PxRigidDynamic* pRigDynAct   = pPhysX->GetPhysics()->createRigidDynamic( PxTransform( physx::PxVec3( Position.x, Position.y, Position.z ) ) );
// 			PxRigidDynamic* pRigDynAct   = PxCreateDynamic( *pPhysX->GetPhysics(), PxTransform( physx::PxVec3( Position.x, Position.y, Position.z ) ), PxBoxGeometry( 14.f, 4.6f, 6.f ), *pMaterial, 1.f );
// 
// 			if( pRigDynAct && pMaterial && triangleMesh )
// 			{
// 				//pRigDynAct->createShape( PxTriangleMeshGeometry( triangleMesh ), *pMaterial );
// 				
// 				pRigDynAct->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);
// 				pRigDynAct->setActorFlag( PxActorFlag::eVISUALIZATION, true );
// 				pRigDynAct->setAngularDamping( 0.5f );
// 				//pRigDynAct->setMass( 10000.f );
// 				PxRigidBodyExt::updateMassAndInertia( *pRigDynAct, 10.f );
// 				
// 				if( pMaterial )
// 					pPhysX->PushMaterial( pMaterial );
// 				
// 				pPhysX->AddActorScene( pRigDynAct );
// 				m_pActor = pRigDynAct;
// 
// 				return true;
// 			}

			const int    nWheels     = 12;
			PxF32        chassisMass = 1500.f;
			const PxF32  wheelMass   = 50.f;			
			PxF32		 fShift      = -0.85f;
			PxVec3 wheelCentreOffsets[ nWheels ];

			for( int i = 0; i < nWheels/2; ++i )
			{
				wheelCentreOffsets[ i*2   ] = PxVec3( -1.2f, -0.5f,  2.1f + i * fShift );
				wheelCentreOffsets[ i*2+1 ] = PxVec3(  1.2f, -0.5f,  2.1f + i * fShift );				
			}

			// размеры корпуса
			const PxVec3 chassisDims( 2.4f, 1.f, 6.f );// = computeChassisAABBDimensions(chassisConvexMesh);

			// Начало координат находится в центре шасси сетки
			// Установить центр масс будет ниже этой точки
			const PxVec3 chassisCMOffset = PxVec3( 0.f, -chassisDims.y * 0.5f - 0.65f, 0.f );

			PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate( nWheels );
			PxVehicleWheelsSimData& wheelsData = *wheelsSimData;
			PxVehicleDriveSimData4W driveData;
			
			PxVec3 chassisMOI( (chassisDims.y*chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 12.f,
							   (chassisDims.x*chassisDims.x + chassisDims.z * chassisDims.z) * chassisMass / 12.f,
							   (chassisDims.x*chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 12.f);

			// структура шасси
			PxVehicleChassisData chassisData;
			
			chassisData.mMass	  = chassisMass;		// Масса транспортного средства жесткой актер тела
			chassisData.mMOI	  = chassisMOI;			// Момент инерции автомобиля жесткая актер тела.
			chassisData.mCMOffset = chassisCMOffset;	// Центр масс смещение автомобиля жесткая актер тела.

			// Немного настройки здесь.Автомобиль будет иметь более отзывчивым поворот, если мы сведем
			// у-компоненты шасси момента инерции.
			chassisMOI.y *= 0.8f;

			const PxF32 massRear  = 0.5f * chassisMass * ( chassisDims.z - 3 * chassisCMOffset.z ) / chassisDims.z;
			const PxF32 massFront = massRear;

			//Extract the wheel radius and width from the wheel convex meshes
			PxF32 wheelWidths[ nWheels ] = {0.f};
			PxF32 wheelRadii[ nWheels ]  = {0.f};
			for( PxU32 i = 0; i < nWheels; ++i )
			{
				 wheelWidths[ i ] = 0.5f;
				 wheelRadii [ i ] = 0.32f;
			}

			// Теперь вычислим колеса массы и инерции компонентов вокруг оси оси			
			PxF32 wheelMOIs[ nWheels ];
			for( PxU32 i = 0; i < nWheels; ++i )
			{
				wheelMOIs[ i ] = 0.5f * wheelMass * wheelRadii[ i ] * wheelRadii[ i ];
			}

			// Давайте создадим структуру данных колеса теперь с радиусом, массы и МВД
			PxVehicleWheelData wheels[ nWheels ];
			for(PxU32 i = 0; i < nWheels; ++i )
			{
				wheels[ i ].mRadius				= wheelRadii[ i ];		// Радиус блок, который включает в себя колеса металл плюс резиновые шины
				wheels[ i ].mMass				= wheelMass;			// Масса колеса плюс шины
				wheels[ i ].mMOI				= wheelMOIs[ i ];		// Момент инерции колеса
				wheels[ i ].mWidth				= wheelWidths[ i ];		// Максимальная ширина блок, который включает в себя колеса плюс шин
				//wheels[ i ].mMaxHandBrakeTorque = 0.f;					// Отключение стояночного тормоза от передних колес и позволяют для задних колес
				//wheels[ i ].mMaxSteer			= 0.f;					// Включить рулевого управления для передних колес и отключить для передних колес
				//wheels[ i ].mDampingRate		= 1.f;				// Скорость затухания описывает скорость, с которой свободно вращающееся колесо теряет скорость вращения
			}

			//Let's set up the tire data structures now.
			//Put slicks on the front tires and wets on the rear tires.
			PxVehicleTireData tires[ nWheels ];

			for(PxU32 i = 0; i < nWheels; ++i )
			{				
				tires[ i ].mType = 1;			// тип сцепления шин с поверхностью
			}

			// Структура данных подвески
			PxVehicleSuspensionData susps[ nWheels ];

			for( PxU32 i = 0; i < nWheels; i++ )
			{
				susps[ i ].mMaxCompression	 = 0.03f;				// Максимальное сжатие пружинной подвески
				susps[ i ].mMaxDroop		 = 0.03f;				// Максимальное удлинение пружинной подвески
				susps[ i ].mSpringStrength	 = 20000.f;	// пружинная сила подвески блока
				susps[ i ].mSpringDamperRate = 500.f;
				susps[ i ].mSprungMass		 = chassisMass / nWheels;	// Масса транспортного средства, которая поддерживается пружинная подвеска, указанных в кг.
			}
			
			PxVec3 suspTravelDirections[ nWheels ];
			PxVec3 wheelCentreCMOffsets[ nWheels ];
			PxVec3 suspForceAppCMOffsets[ nWheels ];
			PxVec3 tireForceAppCMOffsets[ nWheels ];

			for( PxU32 i = 0 ; i < nWheels; ++i )
			{
				wheelCentreCMOffsets [ i ] = wheelCentreOffsets[ i ] - chassisCMOffset;
				suspForceAppCMOffsets[ i ] = PxVec3( wheelCentreCMOffsets[ i ].x, -0.3f, wheelCentreCMOffsets[ i ].z );
				tireForceAppCMOffsets[ i ] = PxVec3( wheelCentreCMOffsets[ i ].x, -0.3f, wheelCentreCMOffsets[ i ].z );
				suspTravelDirections [ i ] = PxVec3( 0, -1, 0 );	// направление подвески
			}

			// Теперь добавьте колеса, шины и подвеска данных
			for( PxU32 i = 0; i < nWheels; ++i )
			{
				wheelsData.setWheelData( i, wheels[ i ] );								// установить данные колеса
				wheelsData.setTireData( i, tires[ i ] );								// Установите шину данных колеса
				wheelsData.setSuspensionData( i, susps[ i ] );							// Установите подвеску данные колеса
				wheelsData.setSuspTravelDirection( i, suspTravelDirections[ i ] );		// Установить направление движения подвески колес
				wheelsData.setWheelCentreOffset( i, wheelCentreCMOffsets[ i ] );		// Установить смещение от центра жесткой тело массой в центре колеса
				wheelsData.setSuspForceAppPointOffset( i, suspForceAppCMOffsets[ i ] );	// Установить приложение точкой подвески силу подвески колес
				wheelsData.setTireForceAppPointOffset( i, tireForceAppCMOffsets[ i ] );	// Установить приложение точку шин силу шинах колес
			}

			//Diff
			PxVehicleDifferential4WData diff;
			diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
			driveData.setDiffData( diff );

			//Engine
			PxVehicleEngineData engine;
			engine.mPeakTorque								= 300.f;	// максимальная скорость вращения двигателя
			engine.mMaxOmega								= 400.f;	// Максимальный крутящий момент доступен обратиться к двигателю
			engine.mDampingRateFullThrottle					= 0.15f;	// скорость затухания двигатель при полностью открытой дроссельной заслонке	
			engine.mDampingRateZeroThrottleClutchEngaged	= 8.f;		// скорость затухания двигатель при нулевой газ при включении сцепления
			engine.mDampingRateZeroThrottleClutchDisengaged	= 0.35f;	// Краткие скорость затухания двигатель при нулевой газ при выключенном сцеплении (на нейтральной передаче)

			driveData.setEngineData( engine );

			//Gears
			PxVehicleGearsData gears;
			gears.mSwitchTime = 0.5f;
			driveData.setGearsData( gears );

			// Прочность сцепления
			PxVehicleClutchData clutch;
			clutch.mStrength = PxVehicleGearsData::eMAX_NUM_GEAR_RATIOS;
			driveData.setClutchData( clutch );

			//Ackermann steer accuracy
			PxVehicleAckermannGeometryData ackermann;
			ackermann.mAccuracy		  = 0.1f;
			ackermann.mAxleSeparation = wheelCentreOffsets[ 0 ].z - wheelCentreOffsets[ nWheels - 1 ].z;	// Расстояние между центром передней оси и центром задней оси
			ackermann.mFrontWidth	  = wheelCentreOffsets[ 0 ].x - wheelCentreOffsets[ 1		    ].x;	// Расстояние между центральной точке два передних колеса
			ackermann.mRearWidth	  = wheelCentreOffsets[ nWheels - 2 ].x - wheelCentreOffsets[ nWheels - 1 ].x;	// Расстояние между центральной точке два задних колеса
			driveData.setAckermannGeometryData(ackermann);			
			
			PxTriangleMesh* pTriangleMesh = 0;
			D3DXVECTOR3     vPosition;
			if( CObject* pRoller = GetDetail( WHEEL_LEFT_1ST ) )
			{
				if ( pRoller->CreateTriangleMesh( pPhysX ) )
				{
					pRoller->Update( 0.f );					
					pTriangleMesh = pRoller->GetTriangleMesh();
					Position      = pRoller->GetPosition();
				}
			}

			// Нам нужно добавить колеса столкновения форм, их местный позы, материал для колес, и моделирование фильтра для колес
			PxTriangleMeshGeometry WheelGeom( pTriangleMesh );
			
			PxGeometry* wheelGeometries[ nWheels ] = {0};
			PxTransform wheelLocalPoses[ nWheels ];

			for( PxU32 i = 0; i < nWheels; ++i )
			{
				wheelGeometries[ i ] = &WheelGeom;
				wheelLocalPoses[ i ] = PxTransform::createIdentity();
			}
			
			PxMaterial* pMaterial = pPhysX->GetPhysics()->createMaterial( 0.5f, 0.5f, 0.1f );    //коэффициенты трения скольжения и покоя(Dynamic friction,Static friction), коэффициент упругости
			const PxMaterial& wheelMaterial	= *pMaterial;
			PxFilterData wheelCollFilterData;

			wheelCollFilterData.word0 = COLLISION_FLAG_WHEEL;
			wheelCollFilterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;

			// Нам нужно добавить шасси столкновения форм, их местный позы, материала для шасси и моделирования фильтр для шасси.
			//PxBoxGeometry chassisConvexGeom( 1.5f, 0.3f, 4.f );
			PxBoxGeometry chassisConvexGeom( chassisDims.x/2, chassisDims.y/2, chassisDims.z/2 );

			const PxGeometry* chassisGeoms	    = &chassisConvexGeom;
			const PxTransform chassisLocalPoses = PxTransform::createIdentity();
			const PxMaterial& chassisMaterial	= *pMaterial;

			PxFilterData chassisCollFilterData;
			chassisCollFilterData.word0 = COLLISION_FLAG_CHASSIS;
			chassisCollFilterData.word1 = COLLISION_FLAG_CHASSIS_AGAINST;

			// Создание фильтра запроса данных для автомобилей, чтобы машины не пытайтесь ездить на себя.
			PxFilterData vehQryFilterData;			
			SampleVehicleSetupVehicleShapeQueryFilterData( &vehQryFilterData );

			PxRigidDynamic* vehActor = pPhysX->GetPhysics()->createRigidDynamic( PxTransform::createIdentity() );

			//Add all the wheel shapes to the actor.
			for( PxU32 i = 0; i < nWheels; ++i )
			{
				PxShape* wheelShape=vehActor->createShape( *wheelGeometries[ i ], wheelMaterial );
				wheelShape->setQueryFilterData( vehQryFilterData );
				wheelShape->setSimulationFilterData( wheelCollFilterData );
				wheelShape->setLocalPose( wheelLocalPoses[ i ] );
				wheelShape->setFlag( PxShapeFlag::eSIMULATION_SHAPE, true );
			}

			//Add the chassis shapes to the actor			
			PxShape* chassisShape = vehActor->createShape( *chassisGeoms, chassisMaterial );
			chassisShape->setQueryFilterData( vehQryFilterData );
			chassisShape->setSimulationFilterData( chassisCollFilterData );
			chassisShape->setLocalPose( PxTransform( physx::PxVec3( 0, 0, 0 ) ) );
			

			vehActor->setMass( chassisData.mMass );
			vehActor->setMassSpaceInertiaTensor( chassisData.mMOI );
			vehActor->setCMassLocalPose( PxTransform( chassisData.mCMOffset, PxQuat::createIdentity() ) );
			vehActor->setGlobalPose( PxTransform( physx::PxVec3( 0, 8, 0 ), PxQuat::createIdentity() ) );

			PxVehicleDriveTank* pTank = PxVehicleDriveTank::allocate( nWheels );
 			
 			pTank->setup( pPhysX->GetPhysics(), vehActor, *wheelsSimData, driveData, nWheels );			
			pPhysX->AddActorScene( vehActor );
			m_pActor = vehActor;
			pPhysX->AddTank( pTank );

			//Free the sim data because we don't need that any more.
			wheelsSimData->free();
			//pTank->setDriveModel( PxVehicleDriveTank::eDRIVE_MODEL_SPECIAL );
			pTank->setToRestState();			
			pTank->mDriveDynData.setUseAutoGears( true );

			return true;
		}
	}

	return false;
}

CTank::~CTank()
{
}