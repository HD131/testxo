#pragma once
#include <vector>

class CameraDevice;

class CMesh3D
{
public:
	ID3DXMesh*  GetMesh()   { return m_pMesh; }
	HRESULT		InitialMesh( LPCSTR Name, FILE* FileLog );
	void		Release();
	void		SetMatrixWorld( const D3DXMATRIX& Matrix );
	void		SetMatrixView( const D3DXMATRIX& Matrix );
	void		SetMatrixProjection( const D3DXMATRIX& Matrix );
	void        RenderMesh( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader );
private:
	ID3DXMesh*							m_pMesh;	
	float								m_Alpha;
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