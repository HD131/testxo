#pragma once

#include <windows.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>
#include <stdio.h>
#include <winuser.h>
#include "../../sdk/dx9/Include/d3d9.h"
#include "../../sdk/dx9/Include/d3dx9.h"
#include "../../sdk/dx9/Include/d3dx9core.h"
#include "../../sdk/dx9/Include/d3dx9math.h"
#define  DIRECTINPUT_VERSION  0x0800
#include "../../sdk/dx9/Include/dinput.h"
#include "../../sdk/dx9/Include/d3dx9mesh.h"
#define  INITGUID
#include <fstream>
#include <string>

const UINT Width				= 1024;
const UINT Height				= 768;
const int  MaxField				= 10;
const int  MaxMine				= 20;
const D3DXVECTOR4 g_Light       = D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 1.0f );
const float	g_Diffuse_intensity = 1.0f;

enum  NameShader { Sky , Diffuse, MaxShader };
enum  Values { Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Empty, Flag, Mine, Stena, Angle, MaxMesh, OpenCell };
enum  Game_State { STATE_PLAY, STATE_WIN, STATE_LOST };


#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define KEYDOWN(name, key) (name[key]&0x80)

class CD3DDevice
{
public:
	IDirect3D9*			    m_pDirect3D; // указатель на Главный интерфейс отвечающий за Direct3D	
	IDirect3DCubeTexture9*  m_CubeTexture;	
	IDirect3DPixelShader9*  m_pPixelShader [MaxShader];
	IDirect3DVertexShader9* m_pVertexShader[MaxShader];
	ID3DXConstantTable*     m_pConstTableVS[MaxShader];
	ID3DXConstantTable*     m_pConstTablePS[MaxShader];	
	HRESULT                 IntialDirect3D( HWND hwnd, FILE *FileLog );
	HRESULT                 InitialShader();
	HRESULT				    LoadTexture( FILE *FileLog );
	void				    Release();
};

struct CCell
{
	float       m_Radius;
	D3DXVECTOR3 m_Centr;	
	int         m_Value;
	CCell()
	{		
		m_Value  = 0;
		m_Radius = 0.5f;			
		m_Centr  = D3DXVECTOR3( 0, 0, 0 );			
	}
	void SetCenter( float x, float y, float z)
	{
		D3DXMATRIX MatrixWorld;
		D3DXMatrixTranslation( &MatrixWorld, 0, 0, 0 );
		m_Centr = D3DXVECTOR3( x, y, z );
		D3DXVec3TransformNormal( &m_Centr, &m_Centr, &MatrixWorld );
	}
};

struct CVertexFVF
{
	FLOAT X,   Y,  Z;
	FLOAT nx, ny, nz;
	FLOAT tu, tv;
};



