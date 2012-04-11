#pragma once
#include "Mesh.h"
#include "Init.h"

enum Weapon { M16, AK47, MaxWeapon };

class CWeapon
{
protected:
	unsigned short 		m_AmountBullet;    // количество пуль
	unsigned short 		m_ChargerBullet;   // сколько в обойме
	unsigned short 		m_Damage;		  // урон от пули
  	Weapon         		m_NameWeapon;
	CMesh3D        		m_Mesh;
	D3DXMATRIX     		m_MatrixWorld;
	IDirect3DDevice9*   m_pD3DDevice;
public:
 	virtual Weapon GetWeaponType() = 0;
	virtual void   Recharge()      = 0;  // перезарядка
	virtual void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader ) = 0;
	virtual short  GetChargerBullet() = 0;
	//virtual ~CWeapon();

};

class CAutomatic_M16 : public CWeapon
{
public:
	CAutomatic_M16( LPCSTR Name, IDirect3DDevice9* pD3DDevice );
   ~CAutomatic_M16();
    void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader );
	void   Recharge();
	short  GetChargerBullet();
	Weapon GetWeaponType()
	{
		return 	m_NameWeapon; 
	}

};

class CAutomatic_AK47 : public CWeapon
{
public:
	CAutomatic_AK47( LPCSTR Name, IDirect3DDevice9* pD3DDevice );
   ~CAutomatic_AK47();
    void   RenderWeapon( CameraDevice const& Camera, CShader const& Shader );
	void   Recharge();
	short  GetChargerBullet();
	Weapon GetWeaponType()
	{
		return 	m_NameWeapon; 
	}

};