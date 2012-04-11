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
const int  MaxMine				= 13;
const D3DXVECTOR4 g_Light       = D3DXVECTOR4( 0.0f, 1.0f, -1.0f, 1.0f );
const float	g_Diffuse_intensity = 1.0f;

enum  NameShader { Sky , Diffuse, Text, FlatImage, MaxShader };
enum  Mesh { Pers, MaxMesh };
enum  Game_State { STATE_PLAY, STATE_WIN, STATE_LOST };
enum  BLEND { BLEND_DEFAULT, BLEND_ALPHA, BLEND_MUL, BLEND_ADD };


#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define KEYDOWN(name, key) (name[key]&0x80)

void Log( char* Str );

class CD3DDevice
{
public:
	IDirect3D9*			    m_pDirect3D; // указатель на Главный интерфейс отвечающий за Direct3D	
	IDirect3DPixelShader9*  m_pPixelShader [MaxShader];
	IDirect3DVertexShader9* m_pVertexShader[MaxShader];
	ID3DXConstantTable*     m_pConstTableVS[MaxShader];
	ID3DXConstantTable*     m_pConstTablePS[MaxShader];	
	HRESULT                 IntialDirect3D( HWND hwnd );
	void				    Release();
};

class CShader
{
public:
	IDirect3DPixelShader9*  m_pPixelShader;
	IDirect3DVertexShader9* m_pVertexShader;
	ID3DXConstantTable*     m_pConstTableVS;
	ID3DXConstantTable*     m_pConstTablePS;	
	HRESULT LoadShader( std::string FileName, IDirect3DDevice9* pD3DDevice );
	void	Release();
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
	FLOAT x,   y,  z;
	FLOAT nx, ny, nz;
	FLOAT u, v;
	CVertexFVF()
	{	}
	CVertexFVF( float X, float Y, float Z, float NX, float NY, float NZ, float U, float V ) : x(X), y(Y), z(Z), nx(NX), ny(NY), nz(NZ), u(U), v(V)
	{	}
};

void DrawMyText( IDirect3DDevice9* g_pD3DDevice, char* StrokaTexta, int x, int y, int x1, int y1, D3DCOLOR MyColor );
void Blending( BLEND Blend, IDirect3DDevice9* D3DDevice );


