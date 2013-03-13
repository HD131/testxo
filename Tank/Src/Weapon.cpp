#include "Weapon.h"
#include "Init.h"



CWeapon::CWeapon( std::string NameWeapon, IDirect3DDevice9* pD3DDevice )
{	
	std::string FileIni = "model\\" + NameWeapon + ".ini";

	m_AmountBullet     = atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "AmountBullet") ); 
	m_MaxChargerBullet = atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "MaxChargerBullet") ); 
	m_ChargerBullet    = atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "ChargerBullet") ); 
	m_Damage           = atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "Damage") ); 
	m_RateOfFire       = atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "RateOfFire") ); 
	m_LastTimeFire     = 0;
	m_NameWeapon       = (Weapon) atoi( ReadIniFile( FileIni.c_str(), NameWeapon.c_str(), "NameWeapon") );
	m_pD3DDevice       = pD3DDevice;
	m_Fire             = false;

	std::string Name = "model\\" + NameWeapon + ".x";
	m_Mesh.InitialMesh( Name.c_str(), pD3DDevice );
}

char* CWeapon::ReadIniFile( const char* filename, const char* section, const char* key )
{
	char *out = new char[512];
	GetPrivateProfileString( (LPCSTR)section, (LPCSTR)key, 0, out, 200, (LPCSTR)filename );

return out;
} 

void CWeapon::Recharge()
{
	if ( ( m_AmountBullet ) && ( m_ChargerBullet != m_MaxChargerBullet ) )
	{
		int i = m_MaxChargerBullet - m_ChargerBullet;
		if ( i > (int)m_AmountBullet)
			i = m_AmountBullet;
		m_ChargerBullet += i;
		m_AmountBullet  -= i;
		m_Sound.Play(1);
	}
}

void CWeapon::Fire() 
{
	if ( !m_ChargerBullet )
		SetEndFire();
	if ( ( m_LastTimeFire == 0 ) || ( m_LastTimeFire + m_RateOfFire < timeGetTime() ) )
		if ( m_ChargerBullet )
		{
			--m_ChargerBullet;
			m_Fire = true;
			//m_Sound.Play(0);
			m_LastTimeFire = timeGetTime();
		}	
}

void CWeapon::SetEndFire()
{
	m_LastTimeFire = 0;
	m_Fire         = false;
}

CWeapon::~CWeapon()
{
	if ( m_Mesh.GetMesh() )
		m_Mesh.Release();
}

void CWeapon::RenderWeapon( CameraDevice const& Camera, CCShader const& Shader )
{
	float sc = 0.003f;
	D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXMatrixScaling( &MatrixWorldSc, sc, sc, sc );
	D3DXMatrixTranslation( &MatrixWorldTr, 0.1f, -0.15f, 0.55f );
	m_MatrixWorld =  MatrixWorldSc * MatrixWorldTr * MatV ;
	 
	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );	
}

bool   CWeapon::Hit( HWND hwnd, ID3DXMesh* pMesh, CameraDevice const& Camera, D3DXVECTOR3& Point, const CSphere& sphera )
{
	bool		 Result = false;

	D3DXVECTOR3 Direction = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

	D3DXMATRIX MatV;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXVECTOR3 PosCamera = D3DXVECTOR3( MatV._41, MatV._42, MatV._43 ); //   извлечь координаты камеры из матрицы вида	
	D3DXVec3TransformNormal( &Direction, &Direction, &MatV );
	D3DXVec3Normalize( &Direction, &Direction );

	
	//----------перечение со сферой-------------------
	D3DXVECTOR3 v =  PosCamera - sphera.m_Centre;
	float b = 2.0f * D3DXVec3Dot( &Direction, &v );
	float c = D3DXVec3Dot( &v, &v ) - sphera.m_Radius * sphera.m_Radius ;
	// Находим дискриминант
	float Discr = ( b * b ) - ( 4.0f * c );			
	if ( Discr >= 0.0f )
	{
		Discr = sqrtf(Discr);
		float s0 = ( -b + Discr ) / 2.0f;
		float s1 = ( -b - Discr ) / 2.0f;
		// Если есть решение >= 0, луч пересекает сферу
		if ( ( s0 >= 0.0f ) && ( s1 >= 0.0f ) )
		{
			Point = D3DXVECTOR3( sphera.m_Centre.x, sphera.m_Centre.y, sphera.m_Centre.z );
			Result = true;			
		}
	}
	//---------------------------------------------------------------------
	/*
	CVertexFVF*  Triangle[3];
	short*       pIndices;
	CVertexFVF*  pVertices;
	

	DWORD m_NumPolygons = pMesh->GetNumFaces();	

	pMesh->LockVertexBuffer( D3DLOCK_READONLY,( void** )&pVertices);
	pMesh->LockIndexBuffer(  D3DLOCK_READONLY,( void** )&pIndices );

  	float MinY =  1000000.0f;
  	float MaxY = -1000000.0f;	

	for( DWORD i = 0; i < m_NumPolygons; i++ )
	{
		

		Triangle[0] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка А i-того треугольника
		Triangle[1] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка B i-того треугольника
		Triangle[2] = (CVertexFVF*)( pVertices + ( *pIndices++ ) );//точка C i-того треугольника
		
		
		if ( Triangle[0]->y > MaxY )
			MaxY = Triangle[0]->y;
		if ( Triangle[0]->y < MinY )
			MinY = Triangle[0]->y;

		if ( Triangle[1]->y > MaxY )
			MaxY = Triangle[1]->y;
		if ( Triangle[1]->y < MinY )
			MinY = Triangle[1]->y;

		if ( Triangle[2]->y > MaxY )
			MaxY = Triangle[2]->y;
		if ( Triangle[2]->y < MinY )
			MinY = Triangle[2]->y;

		//тут можем работать с этим треугольником		
		D3DXVECTOR3 Normal = D3DXVECTOR3( Triangle[0]->nx, Triangle[0]->ny, Triangle[0]->nz );	// нормаль треугольника
		D3DXVec3Normalize( &Normal, &Normal );
		//D3DXVECTOR3 V = -Normal;
		float D = -Normal.x * Triangle[0]->x - Normal.y * Triangle[0]->y - Normal.z * Triangle[0]->z;
		float k = -( Normal.x * PosCamera.x + Normal.y * PosCamera.y + Normal.z * PosCamera.z + D ) / 
				   ( Normal.x * Direction.x + Normal.y * Direction.y + Normal.z * Direction.z );
		D3DXVECTOR3 P = D3DXVECTOR3( k * Direction.x + PosCamera.x, k * Direction.y + PosCamera.y, k * Direction.z + PosCamera.z);
		// расстояние от камеры до точки столкновения 
		float Dist = D3DXVec3Length( &(D3DXVECTOR3( PosCamera.x - P.x, PosCamera.y - P.y, PosCamera.z - P.z ) ) );

		D3DXVECTOR3 V1 = D3DXVECTOR3( Triangle[0]->x, Triangle[0]->y, Triangle[0]->z );
		D3DXVECTOR3 V2 = D3DXVECTOR3( Triangle[1]->x, Triangle[1]->y, Triangle[1]->z );
		D3DXVECTOR3 V3 = D3DXVECTOR3( Triangle[2]->x, Triangle[2]->y, Triangle[2]->z );

		if ( PointInTr( V1, V2, V3, Normal, P ) )
		{
			Point = P;
			Result = true;
		}
			
	}
	pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();
	Point = D3DXVECTOR3( MaxY, MinY, 0 );*/
	
return Result;
}