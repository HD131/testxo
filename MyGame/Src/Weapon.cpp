#include "Weapon.h"
#include "Init.h"

CAutomatic_M16::CAutomatic_M16( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{	
	m_AmountBullet   = 100;  
	m_ChargerBullet  = 20; 
	m_Damage         = 10;		    
	m_NameWeapon     = M16;	
	m_Mesh.InitialMesh( Name, pD3DDevice );
}
void CAutomatic_M16::RenderWeapon( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader )
{
	m_Mesh.RenderMesh( Camera, MatrixWorld, Shader );
	
}

void CAutomatic_M16::Recharge()
{

}

CAutomatic_M16::~CAutomatic_M16()
{
	m_Mesh.Release();
}
//-------------------------------------------------------------------------------------------------------

CAutomatic_AK47::CAutomatic_AK47( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{
	m_AmountBullet   = 200;  
	m_ChargerBullet  = 30; 
	m_Damage         = 15;		    
	m_NameWeapon     = AK47;

}

void CAutomatic_AK47::RenderWeapon( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader )
{
	m_Mesh.RenderMesh( Camera, MatrixWorld, Shader );

}

void CAutomatic_AK47::Recharge()
{

}

CAutomatic_AK47::~CAutomatic_AK47()
{
	m_Mesh.Release();
}