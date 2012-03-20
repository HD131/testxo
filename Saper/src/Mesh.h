#pragma once
#include <vector>

class CMesh3D
{
public:
	ID3DXMesh*							m_pMesh;	
	float								m_Alpha;
	HRESULT								InitialMesh( LPCSTR Name, FILE* FileLog );
	void								Release();
	void								DrawMyMesh( ID3DXConstantTable** pConstTableVS, ID3DXConstantTable** pConstTablePS, 
													IDirect3DVertexShader9** VertexShader, IDirect3DPixelShader9** PixelShader );
	void								SetMatrixWorld( const D3DXMATRIX& Matrix );
	void								SetMatrixView( const D3DXMATRIX& Matrix );
	void								SetMatrixProjection( const D3DXMATRIX& Matrix );
private:
	D3DXMATRIX							m_MatrixWorld;
	D3DXMATRIX							m_MatrixView;
	D3DXMATRIX							m_MatrixProjection;
	IDirect3DVertexBuffer9*				m_VertexBuffer;
	IDirect3DIndexBuffer9*				m_IndexBuffer;
	DWORD 								m_SizeFVF;
	DWORD								m_TexturCount; 
	D3DMATERIAL9*						m_pMeshMaterial;
	std::vector<IDirect3DTexture9*>     m_pMeshTextura;
};