#pragma once

#include "D3DDevice.h"
#include "CameraDevice.h"
#include <vector>

struct CParticle
{
	D3DXVECTOR3     m_Position;
	D3DXVECTOR3     m_Direct;
	float           m_Speed;
	bool            m_Kill;
public:
	CParticle();
};

class CException
{
	IDirect3DVertexBuffer9* 	m_VertexBuffer;
	IDirect3DIndexBuffer9*  	m_IndexBuffer;
	IDirect3DTexture9*      	m_TextureExp;
	std::vector<CParticle>  	m_Particles;
	int					    	m_Size;
	DWORD                   	m_LifeTime;
	DWORD                   	m_StartTime;	
	IDirect3DDevice9*       	m_D3DDevice;
	std::vector<CVertexFVF> 	m_Vershin;
	std::vector<unsigned short> m_Index;
	FILE*                       m_FileLog;
	bool                        m_Kill;
public:
	CException();
	void Init( IDirect3DDevice9* D3DDevice, FILE* FileLog );
	void Update( CameraDevice const& Camera );
	bool RenderParticle( CameraDevice const& Camera, CShader const& Shader );
	void SetTime( DWORD Time )
	{ 
		Time = m_StartTime; 
		Beep(100,100);
	}
	void Release();
};