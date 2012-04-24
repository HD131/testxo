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
		if ( i > m_AmountBullet)
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
			m_Sound.Play(0);
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

void CWeapon::RenderWeapon( CameraDevice const& Camera, CShader const& Shader )
{
	float sc = 0.003f;
	D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXMatrixScaling( &MatrixWorldSc, sc, sc, sc );
	D3DXMatrixTranslation( &MatrixWorldTr, 0.1f, -0.15f, 0.55f );
	m_MatrixWorld =  MatrixWorldSc * MatrixWorldTr * MatV ;
	 
	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );	
}
