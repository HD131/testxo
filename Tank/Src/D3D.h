#pragma once

#include "Class.h"

enum  BLENDING 
{
	BLEND_DEFAULT,
	BLEND_ALPHA,
	BLEND_MUL,
	BLEND_ADD
};

class CD3DGraphic
{
public:	
	CD3DGraphic();
   ~CD3DGraphic();

	HRESULT						InitD3D( HWND hwnd );	
	void						Release();
	static IDirect3DDevice9*	GetDevice()						{ return m_pD3DDevice; }
	static IDirect3D9*			GetDirect3D()					{ return m_pDirect3D;  }
	void						SetBlending( BLENDING Blend );

public:

private:
	static IDirect3DDevice9*    m_pD3DDevice;	
	static IDirect3D9*			m_pDirect3D;				// указатель на Главный интерфейс отвечающий за Direct3D	
	bool						FullScreen;
};

class CTexture
{
public:
	CTexture();
	CTexture( IDirect3DTexture9* pTexture, const std::string& srPathTexture );

	IDirect3DTexture9*  GetTexture()	const	{ return m_pTexture; }
	const std::string&  GetPathTexture() const  { return m_srPathTexture; }

private:
	IDirect3DTexture9*	m_pTexture;
	std::string			m_srPathTexture;
};
