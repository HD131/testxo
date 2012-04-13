#include "Weapon.h"
#include "Init.h"

void CWeapon::Recharge()
{
	if ( m_AmountBullet )
	{
		int i = m_MaxChargerBullet - m_ChargerBullet;
		if ( i > m_AmountBullet)
			i = m_AmountBullet;
		m_ChargerBullet += i;
		m_AmountBullet  -= i;
	}
}

void CWeapon::Fire() 
{
	if ( ( m_LastTimeFire == 0 ) || ( m_LastTimeFire + m_RateOfFire < timeGetTime() ) )
		if ( m_ChargerBullet )
		{
			--m_ChargerBullet;
			m_Fire = true;
			Beep(150,40);
			m_LastTimeFire = timeGetTime();
		}
}

bool CWeapon::GetFire()
{
	return m_Fire;
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

CAutomatic_M16::CAutomatic_M16( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{	
	m_AmountBullet     = 100; 
	m_MaxChargerBullet = 20;
	m_ChargerBullet    = m_MaxChargerBullet; 
	m_Damage           = 10;
	m_RateOfFire       = 70;
	m_LastTimeFire     = 0;
	m_NameWeapon       = M16;
	m_pD3DDevice       = pD3DDevice;
	m_Fire             = false;
	m_Mesh.InitialMesh( Name, pD3DDevice );
}
void CAutomatic_M16::RenderWeapon( CameraDevice const& Camera, CShader const& Shader )
{
	float sc = 0.003f;
	D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXMatrixScaling( &MatrixWorldSc, sc, sc, sc );
	D3DXMatrixTranslation( &MatrixWorldTr, 0.1f, -0.15f, 0.55f );
	m_MatrixWorld =  MatrixWorldSc * MatrixWorldTr * MatV ;
	 
	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );	
}

void CAutomatic_M16::Fire()
{	
	CWeapon::Fire();
}

void CAutomatic_M16::Recharge()
{
	CWeapon::Recharge();
}

CAutomatic_M16::~CAutomatic_M16()
{
	CWeapon::~CWeapon();
}
//-------------------------------------------------------------------------------------------------------

CAutomatic_AK47::CAutomatic_AK47( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{
	m_AmountBullet     = 200;
	m_MaxChargerBullet = 30;
	m_ChargerBullet    = m_MaxChargerBullet;
	m_Damage           = 15;
	m_RateOfFire       = 100;
	m_LastTimeFire     = 0;
	m_NameWeapon       = AK47;
	m_pD3DDevice       = pD3DDevice;
	m_Fire             = false;
	m_Mesh.InitialMesh( Name, pD3DDevice );
}

void CAutomatic_AK47::RenderWeapon( CameraDevice const& Camera, CShader const& Shader )
{
	float sc = 0.003f;
	D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXMatrixScaling( &MatrixWorldSc, sc, sc, sc );
	D3DXMatrixTranslation( &MatrixWorldTr, 0.1f, -0.15f, 0.55f );
	m_MatrixWorld =  MatrixWorldSc * MatrixWorldTr * MatV ;

	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );	
}

void CAutomatic_AK47::Fire()
{
	CWeapon::Fire();
}

void CAutomatic_AK47::Recharge()
{
	CWeapon::Recharge();
}

CAutomatic_AK47::~CAutomatic_AK47()
{
	CWeapon::~CWeapon();
}