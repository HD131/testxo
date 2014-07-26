#include "ShaderManager.h"

CShader::CShader():
	m_pVertexShader( 0 ),
	m_pPixelShader ( 0 ),
	m_pConstTableVS( 0 ),
	m_pConstTablePS( 0 )
{
}

bool CShader::LoadVertexShader( const std::string & srPath )
{
	RELEASE_ONE( m_pVertexShader );
	RELEASE_ONE( m_pConstTableVS );
	IDirect3DDevice9 * pD3DDevice  = CD3DGraphic::GetDevice();
	ID3DXBuffer *      pErrors     = 0;
	ID3DXBuffer *      pShaderBuff = 0;

	// вершинный шейдер	
	HRESULT hr = D3DXCompileShaderFromFile( srPath.c_str(), 0, 0, "main", "vs_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &m_pConstTableVS );

	if( hr == S_OK )
	{
		m_srPathVertexShader = srPath;

		if( pShaderBuff )
		{
			if( FAILED( pD3DDevice->CreateVertexShader( (DWORD*)pShaderBuff->GetBufferPointer(), &m_pVertexShader ) ) )
			{
			}			
		}

		if( pErrors )
			Log( (LPCSTR)pErrors->GetBufferPointer() );		
	}
	else
	{
		std::string Error = "Don't compile vertex shader: " + srPath + "\n";
		Log( Error.c_str() );
	}

	RELEASE_ONE( pShaderBuff );
	RELEASE_ONE( pErrors );

	return m_pVertexShader ? true : false;
}

bool CShader::LoadPixelShader( const std::string & srPath )
{
	RELEASE_ONE( m_pPixelShader );
	RELEASE_ONE( m_pConstTablePS );
	IDirect3DDevice9 * pD3DDevice  = CD3DGraphic::GetDevice();
	ID3DXBuffer *      pErrors     = 0;
	ID3DXBuffer *      pShaderBuff = 0;

	// пиксельный шейдер	
	HRESULT hr = D3DXCompileShaderFromFile( srPath.c_str(), 0, 0, "main", "ps_2_0", D3DXSHADER_OPTIMIZATION_LEVEL3, &pShaderBuff, &pErrors, &m_pConstTablePS );

	if( hr == S_OK )
	{
		m_srPathPixelShader = srPath;

		if( pShaderBuff )
		{
			if( FAILED( pD3DDevice->CreatePixelShader( (DWORD*)pShaderBuff->GetBufferPointer(), &m_pPixelShader ) ) )
			{
			}
		}

		if( pErrors )
			Log( (LPCSTR)pErrors->GetBufferPointer() );
	}
	else
	{
		std::string Error = "Don't compile pixel shader: " + srPath + "\n";
		Log( Error.c_str() );
	}

	RELEASE_ONE( pShaderBuff );
	RELEASE_ONE( pErrors );

	return m_pPixelShader ? true : false;
}

bool CShader::IsReload( const std::string & srPath )
{
	if( m_srPathVertexShader == srPath )
	{
		LoadVertexShader( srPath );
		return true;
	}

	if( m_srPathPixelShader == srPath )
	{
		LoadPixelShader( srPath );
		return true;
	}

	return false;
}

void CShader::Release()
{
	RELEASE_ONE( m_pVertexShader );
	RELEASE_ONE( m_pPixelShader  );
	RELEASE_ONE( m_pConstTableVS );
	RELEASE_ONE( m_pConstTablePS );
}

CShader::~CShader()
{
	Release();
}

/*---------------------------------------------------------------------*/

CShaderManager::CShaderManager()
{
	m_Shader.resize( MAX_SHASER );
}

CShader * CShaderManager::GetShader( ESHADER NameShader )
{
	return m_Shader[ NameShader ];
}

CShader * CShaderManager::LoadShader( const std::string & srPath )
{
	CShader * pShader = 0;

	if( !srPath.empty() )
	{		
		pShader = new CShader;

		// вертексный шейдер
		std::string srFileNameVS = srPath + ".vsh";

		if( pShader->LoadVertexShader( srFileNameVS ) )
		{
			// пиксельный шейдер
			std::string srFileNamePS = srPath + ".psh";
			
			if( !pShader->LoadPixelShader( srFileNamePS ) )
			{
				pShader->Release();	
				DELETE_ONE( pShader );	
			}
		}
		else
		{
			pShader->Release();	
			DELETE_ONE( pShader );	
		}						
	}		

	return pShader;	
}

void CShaderManager::SetShader( ESHADER NameShader, CShader * pShader )
{
	m_Shader[ NameShader ] = pShader;
}

bool CShaderManager::IsReload( const std::string & srPath ) const
{
	for( std::vector< CShader* >::const_iterator iter = m_Shader.begin(), iter_end = m_Shader.end(); iter != iter_end; ++iter )
	{
		CShader * pShader = *iter;

		if( pShader && pShader->IsReload( srPath ) )
			return true;
	}

	return false;
}

void CShaderManager::Release()
{
	for( std::vector< CShader* >::iterator iter = m_Shader.begin(), iter_end = m_Shader.end(); iter != iter_end; ++iter )
	{
		if( CShader * pShader = *iter )
		{
			pShader->Release();
			DELETE_ONE( pShader );
		}
	}

	m_Shader.clear();
}

CShaderManager::~CShaderManager()
{
	Release();
}
