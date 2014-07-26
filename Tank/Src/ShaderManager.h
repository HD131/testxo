#pragma once

#include "Precompile.h"

//---------------------------------------------------------
class CShader
{
public:
								CShader();
								~CShader();

	bool						LoadVertexShader( const std::string & srPath );
	bool						LoadPixelShader( const std::string & srPath );
	bool						IsReload( const std::string & srPath );
	void						Release();

public:
	IDirect3DVertexShader9 *	m_pVertexShader;
	IDirect3DPixelShader9 *		m_pPixelShader;	
	ID3DXConstantTable *		m_pConstTableVS;
	ID3DXConstantTable *		m_pConstTablePS;
	std::string					m_srPathVertexShader;
	std::string					m_srPathPixelShader;
};

enum ESHADER 
{ 
	DIFFUSE = 0,
	Sky,	
	Text,
	FlatImage,
	DIFFUSE_NORMAL_SPECULAR,
	MAX_SHASER
};

class CShaderManager
{
public:	
							CShaderManager();
							~CShaderManager();

	CShader *				GetShader( ESHADER NameShader );
	CShader *				LoadShader( const std::string & srPath );
	bool					IsReload( const std::string & srPath ) const;
	void					Release();
	void					SetShader( ESHADER NameShader, CShader * pShader );	

private:
	std::vector< CShader* >	m_Shader;
};