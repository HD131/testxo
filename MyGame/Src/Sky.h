#pragma once
#include "CameraDevice.h"

struct CSky
{
	IDirect3DVertexBuffer9* m_pVerBufSky;
	IDirect3DIndexBuffer9*  m_pBufIndexSky;
	IDirect3DCubeTexture9*  m_CubeTexture;	
	IDirect3DDevice9*       m_pD3DDevice;
	HRESULT                 InitialSky( IDirect3DDevice9* D3DDevice );
	void                    RenderSky( CameraDevice const& Camera, CShader const& Shader );
	void                    Release();
};