#include "Weapon.h"

CAutomatic_M16::~CAutomatic_M16()
{
	m_Mesh.Release();
}

CAutomatic_M16::CAutomatic_M16()
{
	m_AmountBullet   = 100;  
	m_ChargerBullet  = 20; 
	m_Damage         = 10;		    
	m_NameWeapon     = M16;	
}

CAutomatic_AK47::CAutomatic_AK47()
{
	m_AmountBullet   = 200;  
	m_ChargerBullet  = 30; 
	m_Damage         = 15;		    
	m_NameWeapon     = M16;	
}