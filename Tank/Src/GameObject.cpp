#include "GameObject.h"
#include "PhysX.h"

GameObject::GameObject() :
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

GameObject::~GameObject()
{
	SAFE_RELEASE( m_pTriangleMesh );
}

D3DXVECTOR3	GameObject::GetAngle()
{ 
	D3DXVECTOR3 vec = m_AngleXYZ;

	if( m_pObjectParent )
		vec += m_pObjectParent->GetAngle();

	return vec; 
}

D3DXVECTOR3	GameObject::GetPosition()
{ 
	D3DXVECTOR3 T(0,0,0);
	D3DXVec3TransformCoord( &T, &T, &m_MatrixRelease );
	return T; 
}

D3DXVECTOR3	GameObject::GetForward()
{
	D3DXVECTOR3 T( 0, 0, 1 );
	D3DXVec3TransformNormal( &T, &T, &m_MatrixRelease );
	return T; 
}

void GameObject::SetChild( GameObject* pObj )
{
	if( pObj )		
	{
		pObj->SetParent( this );
		m_ObjectChild.push_back( pObj );
	}
}

void GameObject::Update( float fDT )
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

	for( std::list< GameObject* >::iterator iter = m_ObjectChild.begin(); iter != m_ObjectChild.end(); ++iter )
	{
		(*iter)->Update( fDT );
	}	
}

void GameObject::Render( CameraDevice* pCamera, const CShader* pShader )
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

void GameObject::SetMesh( CMesh3D* pMesh )
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

bool GameObject::CreateTriangleMesh( CPhysX const * pPhysX )
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
