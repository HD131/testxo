#pragma once

#include "Precompile.h"

typedef DWORD_PTR TEXTURE;

TEXTURE				LoadTexture( const std::string & srPath );
void				FreeTexture( TEXTURE Tex );
IDirect3DTexture9 * ConvertToD3DXTex( TEXTURE Tex );
void				SetTexture( DWORD Stage, TEXTURE Tex );

/*--------------------------------------------------*/
/*													*/
/*--------------------------------------------------*/
class CTexItem
{
public:
						CTexItem();
	explicit			CTexItem( IDirect3DTexture9 * pTex, const std::string & srPath );
	explicit			CTexItem( const std::string & srPath );
						~CTexItem();

	void				AddRef()																	{ ++m_nRefCount; }
	void				DelRef()																	{ m_nRefCount = ( m_nRefCount > 0 ) ? m_nRefCount - 1 : 0; }
	unsigned int		GetRefCount() const															{ return m_nRefCount; }
	IDirect3DTexture9 * GetPtrTex() const															{ return m_pD3DTexture; }
	const std::string & GetPathTex() const															{ return m_srPath; }
	bool				Load( const std::string & srPath, IDirect3DDevice9 * pD3DDevice );
	bool				IsReload( const std::string & srPath, IDirect3DDevice9 * pD3DDevice );
	void				Release();

private:
	IDirect3DTexture9 * m_pD3DTexture;
	std::string			m_srPath;
	unsigned int		m_nRefCount;
};

/*--------------------------------------------------*/
/*													*/
/*--------------------------------------------------*/
class CTextureManager
{
public:
	static CTextureManager *	GetMng();
	static void					Create( IDirect3DDevice9 * pD3DDevice );
	static void					Release();

	TEXTURE						LoadTexture( const std::string & srPath );
	void						FreeTexture( TEXTURE Tex );

	void						SetTexture( DWORD Stage, TEXTURE Tex );
	bool						IsReload( const std::string & srPath );

								~CTextureManager();

private:
								CTextureManager();
	void						Init( IDirect3DDevice9 * pD3DDevice );
	void						Free();

private:
	static CTextureManager *	m_pTexMng;
	IDirect3DDevice9 *			m_pD3DDevice; 
	std::list< CTexItem* >		m_Textures;
};