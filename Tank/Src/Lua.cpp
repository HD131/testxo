#include "Lua.h"

#pragma warning( disable: 4996 )

CLua::CLua() :
	m_FileBuffer( 0 )
{
	unsigned int FileSize = 0;
	FILE* const FO = fopen( "InitShader.lua", "rb" );
	if( FO )
	{
		fseek( FO, 0,SEEK_END );					// устанавливает указатель на конец файла
		FileSize   = ftell(FO);					// возвращает количество байт от начала до указателя         
		m_FileBuffer = malloc( FileSize );		// возвращает указатель на захваченную память размером m_FileSize
		fseek( FO, 0, SEEK_SET );					// устанавливает указатель на начало файла
		fread( m_FileBuffer, 1, FileSize, FO );	// читает и записывает в память по 1 байту
		fclose( FO );
	}

	m_LuaVM = lua_open();

	if( !m_LuaVM ) 
		Log( "Error Initializing Lua" );

	// инициализация стандартных библиотечных функции lua
	luaopen_base( m_LuaVM );
	luaopen_table( m_LuaVM );
	luaopen_string( m_LuaVM );
	luaopen_math( m_LuaVM );
	luaopen_os( m_LuaVM );

	lua_dobuffer( m_LuaVM, m_FileBuffer, FileSize );
}

bool CLua::lua_dobuffer( lua_State* Lua, void const* Buffer, int Size )
{
	if ( !Size )
		return true;

	if ( luaL_loadbuffer( Lua, (char const*)Buffer, Size, 0 ) )
	{
		char const* ErrorMsg = lua_tostring( Lua, -1 );
		lua_pop( Lua, 1 );
		Log( ErrorMsg );
		return false;
	}

	if ( lua_pcall( Lua, 0, LUA_MULTRET, 0 ) )
	{
		char const* ErrorMsg = lua_tostring( Lua, -1 );
		lua_pop( Lua, 1 );
		Log( ErrorMsg );
		return false;
	}
	
	const char *c = lua_tostring( Lua, -1 );

	Log( "Initial Script" );	
	return true;
}

CLua::~CLua()
{
	free( m_FileBuffer );
	lua_close( m_LuaVM );
}