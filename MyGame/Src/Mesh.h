#pragma once
#include <vector>
#include "CameraDevice.h"

class CMesh3D
{
public:
	ID3DXMesh*  GetMesh()   { return m_pMesh; }
	HRESULT		InitialMesh( LPCSTR Name, IDirect3DDevice9* pD3DDevice );
	void		Release();
	void        RenderMesh( CameraDevice const& Camera, const D3DXMATRIX&  MatrixWorld, CShader const& Shader );
private:
	ID3DXMesh*							m_pMesh;
	IDirect3DDevice9*                   m_pD3DDevice;
	float								m_Alpha;
	D3DXMATRIX							m_MatrixWorld;
	D3DXMATRIX							m_MatrixView;
	D3DXMATRIX							m_MatrixProjection;
	IDirect3DVertexBuffer9*				m_VertexBuffer;
	IDirect3DIndexBuffer9*				m_IndexBuffer;
	DWORD								m_TexturCount; 
	D3DMATERIAL9*						m_pMeshMaterial;
	std::vector<IDirect3DTexture9*>     m_pMeshTextura;
};

