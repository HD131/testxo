#pragma once

#include <windows.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>
#include <stdio.h>
#include <winuser.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>
#include <d3dx9math.h>
#define  DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#define  INITGUID
#include <fstream>
#include <string>
#include <d3dx9mesh.h>

extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}
/*
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"dinput8.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"winmm.lib")
*/
const UINT Width  = 1024;
const UINT Height = 768;

#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define KEYDOWN(name, key) (name[key]&0x80)

class CD3DDevice
{
public:
	IDirect3D9			   *m_pDirect3D; // указатель на Главный интерфейс отвечающий за Direct3D
	IDirect3DTexture9      *pTextura002;
	IDirect3DTexture9      *m_pTexturaSky;
	IDirect3DTexture9	   *m_Texture;
	IDirect3DCubeTexture9  *m_CubeTexture;
	HRESULT                 IntialDirect3D( HWND hwnd, FILE *m_FileLog );	
	HRESULT				    LoadTexture( FILE *m_FileLog );
	void				    Release();
};

struct CCell
{
	float       Radius;
	D3DXVECTOR3 Centr;	
	int         Value;
	CCell()
	{		
		Value  = 10;
		Radius = 5.0f;			
		Centr  = D3DXVECTOR3( 0, 0, 0 );			
	}
	void SetCenter( float x, float y, float z)
	{
		D3DXMATRIX MatrixWorld;
		D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
		Centr = D3DXVECTOR3( x, y, z );
		D3DXVec3TransformNormal( &Centr, &Centr, &MatrixWorld );
	}
};

struct CVertexFVF
{
	FLOAT X,   Y,  Z;
	FLOAT nx, ny, nz;
	FLOAT tu, tv;
};

struct CLuaScript
{
	lua_State    *m_luaVM;
	void         *m_FileBuffer;
	unsigned int  m_FileSize;
	FILE         *m_FileLog; 
	bool          lua_dobuffer( lua_State* Lua, void const* Buffer, int Size );
	CLuaScript( FILE *FileLog );
	~CLuaScript();
};

