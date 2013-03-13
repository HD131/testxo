#pragma once

#include "d3d9.h"
#include "d3dx9.h"

class CameraDevice
{
public:
	CameraDevice();

	void  				MoveForv();
	void  				MoveBack();
	void  				MoveRight();
	void  				MoveLeft();
	void  				MouseRotateLeft( long Ang );
	void  				MouseRotateRight( long Ang );
	void  				MouseRotateUp( long Ang );
	void  				MouseRotateDown( long Ang );
	void  				Refresh();	
	void				Gravity();
	const D3DXMATRIX	GetMatrixViewProject();
	const D3DXMATRIX&	GetMatrixView()	const					{ return m_View;		}
	const D3DXMATRIX&	GetMatrixProject()const					{ return m_Proj;		}
	const D3DXVECTOR3&	GetForvard()const						{ return m_TargetDir;	}
	const D3DXVECTOR3&	GetUp()const							{ return m_CameraUp;	}
	const D3DXVECTOR3&	GetRight()const							{ return m_DirX;		}
	D3DXVECTOR3			GetPosition()const						{ return m_PositionCamera;}

	D3DXMATRIX			MatInverseViewProject();
	void				SetMesh( ID3DXMesh* pMesh );
	void				SetPosition( const D3DXVECTOR3& vec )	{ m_PositionCamera = vec; }
	void				SetForvard( const D3DXVECTOR3& vec )	{ m_TargetDir      = vec; }
	static float		GetAspect()								{ return m_fAspect;		  }
	void				Update( float fDT );

private:
	D3DXVECTOR3		m_PositionCamera;
	D3DXVECTOR3		m_CameraUp;				// вверх
	D3DXVECTOR3		m_TargetDir;			// вперед
	D3DXVECTOR3     m_DirX;  				// вправо
	D3DXMATRIX		m_View;
	D3DXMATRIX		m_Proj;
	FLOAT           StepCamera;
	FLOAT	        AngleCamera;
	FLOAT	        Sensivity;
	ID3DXMesh*      m_pMesh;
	DWORD           m_TimeGravity;
	static float	m_fAspect;
};