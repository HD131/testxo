#include "TextureManager.h"

TEXTURE LoadTexture( const std::string & srPath )
{
	if( CTextureManager * pMng = CTextureManager::GetMng() )
		return pMng->LoadTexture( srPath );

	return 0;
}

void FreeTexture( TEXTURE Tex )
{
	if( CTextureManager * pMng = CTextureManager::GetMng() )
		return pMng->FreeTexture( Tex );
}

IDirect3DTexture9 * ConvertToD3DXTex( TEXTURE Tex )
{
	IDirect3DTexture9 * pD3DXTex = 0;

	if( CTexItem * pItem = reinterpret_cast< CTexItem* >( Tex ) )
		pD3DXTex = pItem->m_pD3DTexture;

	return pD3DXTex;
}

void SetTexture( DWORD Stage, TEXTURE Tex )
{
	if( CTextureManager * pMng = CTextureManager::GetMng() )
		pMng->SetTexture( Stage, Tex );
}

/*--------------------------------------------------*/
/*													*/
/*--------------------------------------------------*/
CTexItem::CTexItem():
	m_pD3DTexture( 0 ),
	m_nRefCount( 1 )
{
}

CTexItem::CTexItem( IDirect3DTexture9 * pTex, const std::string & srPath ):
	m_pD3DTexture( pTex ),
	m_srPath( srPath ),
	m_nRefCount( 1 )
{
}

CTexItem::CTexItem( const std::string & srPath ):
	m_pD3DTexture( 0 ),
	m_srPath( srPath ),
	m_nRefCount( 1 )
{
}

void CTexItem::Release()
{
	if( m_pD3DTexture )
	{
		m_pD3DTexture->Release();
		m_pD3DTexture = 0;
	}
}

CTexItem::~CTexItem()
{
	Release();
}

/*--------------------------------------------------*/
/*													*/
/*--------------------------------------------------*/
CTextureManager * CTextureManager::m_pTexMng = 0;

CTextureManager::CTextureManager():
	m_pD3DDevice( 0 )
{
}

CTextureManager * CTextureManager::GetMng()
{
	if( !m_pTexMng )
		m_pTexMng = new CTextureManager;

	return m_pTexMng;
}

void CTextureManager::Create( IDirect3DDevice9 * pD3DDevice )
{
	if( CTextureManager * pMng = GetMng() )
		pMng->Init( pD3DDevice );	
}

void CTextureManager::Init( IDirect3DDevice9 * pD3DDevice )
{
	m_pD3DDevice = pD3DDevice;
}

TEXTURE	CTextureManager::LoadTexture( const std::string & srPath )
{
	TEXTURE pTex = 0;

	if( !srPath.empty() && m_pD3DDevice )
	{
		for ( std::list< CTexItem* >::iterator iter = m_Textures.begin(), iter_end = m_Textures.end(); iter != iter_end; ++iter )
		{
			CTexItem * pItem = *iter;

			if( !_stricmp( srPath.c_str(), pItem->m_srPath.c_str() ) )
			{
				++pItem->m_nRefCount;
				return reinterpret_cast< TEXTURE >( pItem );
			}
		}

		CTexItem * pItem = new CTexItem( srPath );
		
		if( FAILED( D3DXCreateTextureFromFileEx( m_pD3DDevice, srPath.c_str(), 0, 0, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, 0, &pItem->m_pD3DTexture ) ) )
		{
			std::string szError = std::string( "error load texture Mesh" ) + srPath;
			Log( szError.c_str() );				
		}

		m_Textures.push_back( pItem );
		pTex = reinterpret_cast< TEXTURE >( pItem );

	}

	return pTex;
}

void CTextureManager::FreeTexture( TEXTURE Tex )
{
	if( CTexItem * pItem = reinterpret_cast< CTexItem* >( Tex ) )
	{
		--pItem->m_nRefCount;

		if( !pItem->m_nRefCount )
		{
			for ( std::list< CTexItem* >::iterator iter = m_Textures.begin(), iter_end = m_Textures.end(); iter != iter_end; ++iter )
			{
				CTexItem * pTexItem = *iter;

				if( pTexItem == pItem )
				{
					m_Textures.erase( iter );
					break;
				}
			}

			pItem->Release();
			delete pItem;
		}
	}
}

void CTextureManager::SetTexture( DWORD Stage, TEXTURE Tex )
{
	if( m_pD3DDevice )
		if( CTexItem * pItem = reinterpret_cast< CTexItem* >( Tex ) )
			m_pD3DDevice->SetTexture( Stage, pItem->m_pD3DTexture );
}

void CTextureManager::Release()
{
	if( m_pTexMng )
	{
		delete m_pTexMng;
		m_pTexMng = 0;
	}
}

void CTextureManager::Free()
{
	for( std::list< CTexItem* >::iterator iter = m_Textures.begin(), iter_end = m_Textures.end(); iter != iter_end; ++iter )
	{
		CTexItem * pTexItem = *iter;

		pTexItem->Release();
		delete pTexItem;
	}

	m_Textures.clear();
	m_pD3DDevice = 0;
}

CTextureManager::~CTextureManager()
{
	Free();
}
