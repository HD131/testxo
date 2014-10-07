#pragma once

#include "Help.h"
#include <fstream>
#include "PhysX.h"

extern "C"
{
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}

class LuaScript
{
public:
	
	~LuaScript();

	static lua_State *				RunScript( const std::string & srPathScript );
	static void						Clear();
	static void						ReleaseLua();

	template< typename T >
	bool							GetValueParam( const std::string & srParam, T * pValue );
	template<typename T>
	T								GetLuaValue( const std::string & variableName )									{ return 0; }

private:
									LuaScript();
	bool							LuaDoBuffer( lua_State * Lua, void const * Buffer, int Size );	

private:
	static LuaScript *				m_pThis;
	static lua_State *				m_LuaState;
	static char *					m_pFileBuffer;
};