#pragma once
#include "Mesh.h"
#include "Init.h"

enum                 Weapon          {  M16,   AK47,   MaxWeapon };
std::string  const   WeaponNames[] = { "M16", "AK47" };


class CWeapon
{
private:
	unsigned int  		 m_AmountBullet;       // количество пуль
	unsigned int  		 m_ChargerBullet;      // сколько в обойме
	unsigned int         m_MaxChargerBullet;   // максимально сколько в обойме
	unsigned int 		 m_Damage;		       // урон от пули
	DWORD                m_RateOfFire;		   // время между пулями в милисекундах	
	DWORD                m_LastTimeFire;
  	Weapon         		 m_NameWeapon;
	CMesh3D        		 m_Mesh;
	bool                 m_Fire;
	D3DXMATRIX     		 m_MatrixWorld;
	IDirect3DDevice9*    m_pD3DDevice;
public:
	CWeapon( std::string NameWeapon, IDirect3DDevice9* pD3DDevice );
	char*  ReadIniFile( const char *filename, const char *section, const char *key );
 	Weapon GetWeaponType()	    {	return m_NameWeapon; 	};
	void   SetEndFire();
	void   Fire();
	bool   GetFire()			{	return m_Fire;			};
	void   Recharge();
	void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader );
	int    GetChargerBullet()	{	return m_ChargerBullet;	};
	int    GetAmountBullet()	{	return m_AmountBullet;	};
   ~CWeapon();

};
