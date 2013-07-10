#pragma once

#include "Help.h"
#include <fstream>
#include "PhysX.h"

struct CParamTank
{
	int								m_nNumberWheels;									
	float							m_fMassTank;									
	float							m_fMassWheel;									
	float							m_WheelWidths;										
	float							m_WheelRadius;										
	physx::PxVec3					m_vWheelMaterial;									
	std::vector< physx::PxVec3 >	m_vecWheelCenter;
	physx::PxVec3					m_vSuspensionTravelDirections;  					
	float							m_SuspensionMaxCompression; 						
	float							m_SuspensionMaxDroop;		 						
	float							m_SuspensionSpringStrength; 						
	float							m_SuspensionSpringDamperRate; 
	float							m_EnginePeakTorque;								
	float							m_EngineMaxOmega;									
	float							m_EngineDampingRateFullThrottle;					
	float							m_EngineDampingRateZeroThrottleClutchEngaged;		
	float							m_EngineDampingRateZeroThrottleClutchDisengaged;
	physx::PxVec3					m_vSizeBody;	
};

extern "C"
{
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}

class CLua
{
public:
	CLua();
	~CLua();

	bool          lua_dobuffer( lua_State* Lua, void const* Buffer, int Size );
	static bool	  LoadParamTank( const std::string& srPathScript, CParamTank** pParamTank );

public:
	lua_State*    m_LuaVM;
	void*		  m_FileBuffer;		
};