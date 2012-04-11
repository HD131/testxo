#include "Weapon.h"
#include "Init.h"

CAutomatic_M16::CAutomatic_M16( LPCSTR Name, IDirect3DDevice9* pD3DDevice )
{	
	m_AmountBullet   = 100;  
	m_ChargerBullet  = 20; 
	m_Damage         = 10;		    
	m_NameWeapon     = M16;	
	m_pD3DDevice     = pD3DDevice;
	m_Mesh.InitialMesh( Name, pD3DDevice );
}
void CAutomatic_M16::RenderWeapon( CameraDevice const& Camera, CShader const& Shader )
{
	float sc = 0.008f;
	D3DXMATRIX MatV, MatrixWorldTr, MatrixWorldSc;
	D3DXMatrixInverse( &MatV, 0, &Camera.m_View ); 
	D3DXMatrixScaling( &MatrixWorldSc, sc, sc, sc );
	D3DXMatrixTranslation( &MatrixWorldTr, 0.3f, -0.3f, 1.2f );
	m_MatrixWorld =  MatrixWorldSc * MatrixWorldTr * MatV ;
	 
	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );	
}

short  CAutomatic_M16::GetChargerBullet()
{
	return m_ChargerBullet;
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
	m_Mesh.InitialMesh( Name, pD3DDevice );
}

void CAutomatic_AK47::RenderWeapon( CameraDevice const& Camera, CShader const& Shader )
{
	float sc = 0.01f;
	D3DXMatrixScaling( &m_MatrixWorld, sc, sc, sc );
	D3DXMATRIX  MatrixWorldTr;
	//D3DXMatrixTranslation( &MatrixWorldTr, Camera.PositionCamera.x+0.3f, Camera.PositionCamera.y-0.5f, Camera.PositionCamera.z+1.5f );
	D3DXMatrixTranslation( &MatrixWorldTr, 0, 0, 0 );
	m_MatrixWorld = m_MatrixWorld * MatrixWorldTr;
	m_Mesh.RenderMesh( Camera, m_MatrixWorld, Shader );

}

short  CAutomatic_AK47::GetChargerBullet()
{
	return m_ChargerBullet;
}

void CAutomatic_AK47::Recharge()
{

}

CAutomatic_AK47::~CAutomatic_AK47()
{
	m_Mesh.Release();
}