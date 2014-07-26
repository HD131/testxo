#pragma once

#include "D3D.h"
#include "CameraDevice.h"
#include <vector>
#include "Class.h"

class CShader;

struct CEmmiter
{
	D3DXVECTOR3     m_Position;
	D3DXVECTOR3     m_Direct;
	float           m_Speed;
	bool            m_Kill;

	CEmmiter();
};

class CParticles
{
public:
	CParticles();
	~CParticles();

	void Init( IDirect3DDevice9* D3DDevice );
	bool IsKill()													{ return m_bKill;     }
	void Update( float fDT, CameraDevice* pCamera );
	void Render( CameraDevice* pCamera, const CShader* Shader );
	void SetTime( DWORD Time );
	void SetSpeed( float fTime )									{ m_fSpeed   = fTime; }
	void SetPosition( const D3DXVECTOR3& vec )						{ m_Position = vec;   }
	void Release();

private:
	IDirect3DVertexBuffer9* 	m_VertexBuffer;
	IDirect3DIndexBuffer9*  	m_IndexBuffer;
	IDirect3DTexture9*      	m_TextureExp;
	std::vector< CEmmiter >  	m_Particles;
	int					    	m_Size;
	float                   	m_fLifeTime;
	float                   	m_fStartTime;	
	IDirect3DDevice9*       	m_D3DDevice;
	std::vector< CVertex > 		m_Vershin;
	std::vector<unsigned short> m_Index;	
	bool                        m_bKill;
	D3DXVECTOR3                 m_Position;
	float						m_fSpeed;
	float						m_fScale;
};