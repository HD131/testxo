#pragma once
#include "Init.h"

struct CText
{
private:
	IDirect3DVertexBuffer9* m_pVerBuf;
	IDirect3DIndexBuffer9*  m_pIndexBuf;
	IDirect3DDevice9*       m_pD3DDevice;
	IDirect3DTexture9*      m_Texture;	
	IDirect3DTexture9*      m_TextureTarget;
public:
	HRESULT                 Init( IDirect3DDevice9* D3DDevice );
	void                    Render( CShader const& Shader, IDirect3DTexture9* Texture, const D3DXMATRIX&  MatrixWorld, int Num );
	void                    RenderInt( float x, float y, float dist, int Number, int Value, CShader const& Shader );
	void                    RenderImage( CShader const& Shader, float Scale, const D3DXMATRIX&  MatrixWorldTrans );
	IDirect3DVertexBuffer9* GetVertexBuffer(){	return m_pVerBuf;	}
	IDirect3DIndexBuffer9*  GetIndexBuffer() {	return m_pIndexBuf; }
	void                    Release();
	~CText();
};