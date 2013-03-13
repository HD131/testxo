#pragma once

#include "Help.h"
#include <fstream>

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

public:
	lua_State*    m_LuaVM;
	void*		  m_FileBuffer;		
};