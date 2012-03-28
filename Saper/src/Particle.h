#pragma once

#include "D3DDevice.h"
#include "CameraDevice.h"
#include <vector>

class CParticle
{
	IDirect3DVertexBuffer9* m_VertexBuffer;
	IDirect3DIndexBuffer9*  m_IndexBuffer;
	IDirect3DTexture9*      m_TextureExp;
	D3DXVECTOR3             m_Position;
	D3DXVECTOR3             m_Direct;
	float                   m_Speed;
	DWORD                   m_LifeTime;	
	bool					m_Kill;
	IDirect3DDevice9*       m_D3DDevice;
public:
	DWORD                   m_StartTime;
	void  Initial( IDirect3DDevice9* D3DDevice, FILE* FileLog );
	bool  Render( CameraDevice const& Camera, CShader const& Shader );
	bool  Kill();
	void  Release();
};

class CException
{
	std::vector<CParticle> m_Part;
	int					   m_Size;
	DWORD                  m_Time;
	IDirect3DDevice9*      D3DDevice;

public:
	void Init( IDirect3DDevice9* D3DDevice, FILE* FileLog );
	void RenderParticle( CameraDevice const& Camera, CShader const& Shader );
	void SetTime( DWORD Time )
	{ 
		m_Time = Time; 
		Beep(100,100);
	}
};