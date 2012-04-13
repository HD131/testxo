#pragma once
#include "Mesh.h"
#include "Init.h"

enum Weapon { M16, AK47, MaxWeapon };

class CWeapon
{
protected:
	unsigned int  		 m_AmountBullet;       // количество пуль
	unsigned int  		 m_ChargerBullet;      // сколько в обойме
	unsigned int         m_MaxChargerBullet;   // максимально сколько в обойме
	unsigned int 		 m_Damage;		       // урон от пули
	DWORD                m_RateOfFire;		   // скорострельность	
	DWORD                m_LastTimeFire;
  	Weapon         		 m_NameWeapon;
	CMesh3D        		 m_Mesh;
	bool                 m_Fire;
	D3DXMATRIX     		 m_MatrixWorld;
	IDirect3DDevice9*    m_pD3DDevice;
public:
 	virtual Weapon GetWeaponType() = 0;	
	virtual void   SetEndFire()    = 0;
	virtual void   Fire()          = 0;
	virtual bool   GetFire()       = 0;
	virtual void   Recharge()      = 0;  // перезарядка
	virtual void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader ) = 0;
	virtual int    GetChargerBullet() = 0;
	virtual int    GetAmountBullet()  = 0;
	virtual       ~CWeapon();

};

class CAutomatic_M16 : public CWeapon
{
public:
	CAutomatic_M16( LPCSTR Name, IDirect3DDevice9* pD3DDevice );
   ~CAutomatic_M16();
    void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader );
	void   Recharge();	
	void   SetEndFire()			{	CWeapon::SetEndFire();	}
	void   Fire();
	bool   GetFire()			{	return m_Fire;			};
	int    GetChargerBullet()	{	return m_ChargerBullet;	};
	int    GetAmountBullet()	{	return m_AmountBullet;	};
	Weapon GetWeaponType()	    {	return m_NameWeapon; 	};
};

class CAutomatic_AK47 : public CWeapon
{
public:
	CAutomatic_AK47( LPCSTR Name, IDirect3DDevice9* pD3DDevice );
   ~CAutomatic_AK47();
    void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader );
	void   Recharge();	
	void   SetEndFire()			{	CWeapon::SetEndFire();	}
	void   Fire();
	bool   GetFire()			{	return m_Fire;			};
	int    GetChargerBullet()	{	return m_ChargerBullet;	};
	int    GetAmountBullet()	{	return m_AmountBullet;	};
	Weapon GetWeaponType()	    {	return m_NameWeapon; 	};
};