#include "Lua.h"

#pragma warning( disable: 4996 )

#include <vector>

LuaScript *		LuaScript::m_pThis			= 0;
lua_State *		LuaScript::m_LuaState		= 0;
char *			LuaScript::m_pFileBuffer	= 0;

LuaScript::LuaScript() 
{
	m_LuaState = lua_open();

	if( m_LuaState )
	{
		// инициализация стандартных библиотечных функции lua
		luaopen_base( m_LuaState );
		luaopen_table( m_LuaState );
		luaopen_string( m_LuaState );
		luaopen_math( m_LuaState );
		luaopen_os( m_LuaState );	
	}
	else
		Log( "Error Initializing Lua" );
}

lua_State * LuaScript::RunScript( const std::string & srPathScript )
{
	if( srPathScript.empty() )
		return 0;

	if( !m_pThis )
		m_pThis = new LuaScript;

	if( m_pThis && m_LuaState )
	{
		
		FILE *       pFile    = 0;
		fopen_s( &pFile, srPathScript.c_str(), "rb" );

		if( pFile )
		{
			fseek( pFile, 0, SEEK_END );						
			unsigned int nFileSize = ftell( pFile );					
			m_pFileBuffer = new char[ nFileSize ];			
			fseek( pFile, 0, SEEK_SET );					
			fread( m_pFileBuffer, 1, nFileSize, pFile );		
			fclose( pFile );

			if( m_pThis->LuaDoBuffer( m_LuaState, m_pFileBuffer, nFileSize ) )
				return m_LuaState;
		}
	}
	
	return 0;
}

bool LuaScript::LuaDoBuffer( lua_State* Lua, void const * Buffer, int Size )
{
	//return true;
// 	if ( !Size )
// 		return true;
//
// 	if ( luaL_loadbuffer( Lua, (char const*)Buffer, Size, 0 ) )
// 	{
// 		char const* ErrorMsg = lua_tostring( Lua, -1 );
// 		lua_pop( Lua, 1 );
// 		Log( ErrorMsg );
// 		return false;
// 	}

	int nRes = luaL_loadfile( Lua, "InitShader.lua" );

	if( nRes )
		return false;

	if ( lua_pcall( Lua, 0, LUA_MULTRET, 0 ) )
	{
		char const* ErrorMsg = lua_tostring( Lua, -1 );
		lua_pop( Lua, 1 );
		Log( ErrorMsg );
		return false;
	}

	std::vector<std::string > vec;
	lua_getglobal( Lua, "tank" );

	int n = lua_gettop( Lua );//получаем количество параметров переданных функции

    for (int i = 1; i <= n; ++i)
    {
        if( lua_isnumber( Lua, i ) )//проверяем тип i-го параметра
            printf( "result[%d] = %f\n", i, lua_tonumber( Lua, i ) );//если это вещественное число выводим его на экран
        else if( lua_isstring( Lua, i ) )//проверка на строку
            printf( "result[%d] = %s\n", i, lua_tostring( Lua, i ) );
        else if( lua_istable( Lua, i ))//если таблица
        {
			lua_pushnil( Lua );//кладем на вершину стека NULL

            while( lua_next( Lua, i ) )
            {
                if( lua_type( Lua, -2 ) == LUA_TNUMBER )
                    printf ( "key = %f", lua_tonumber( Lua, -2 ) );
				else if( lua_type( Lua, -2 ) == LUA_TSTRING )
				{
                    //vec.push_back( lua_tostring( Lua, -2 ) );
				}

                //print value
                if( lua_type( Lua, -1 ) == LUA_TNUMBER )
                    printf( "\tvalue = %f\n", lua_tonumber( Lua, -1 ) );
				else if( lua_type( Lua, -1 ) == LUA_TSTRING )
				{
					const char* str = lua_tostring( Lua, -1 );
					if( str )
						vec.push_back( str );
				}

                //удаляем значение value из стэка, что-бы на следующей итерации
                //ключ использовался для нахождения следующей пары значений из таблицы
                lua_pop( Lua, 1 );
            }
        }
        else if( lua_isuserdata( Lua, i ) ) //пользовательский тип данных
        {
            void* udata = lua_touserdata( Lua, i );
            printf( "lua userdata %06", udata );
        }
    }

	lua_getglobal( Lua, "NumberWheels" );
	int mass = (int)lua_tonumber( Lua, -1 );
	lua_pop( Lua, 1 );

	mass = 0;
	GetValueParam( "NumberWheels", &mass );

	lua_getglobal( Lua, "wheele" );
	if( !lua_istable( Lua,-1) )
	{
		//lua_close( Lua );
		//exit(0);
	}

// 	int tIndex = lua_gettop( Lua );
// 	lua_pushnil( Lua ); // first key
//
// 	while( lua_next( Lua, tIndex ) != 0 )
// 	{
// 		//cout<<"key has type="<<lua_typename(Lua, lua_type(Lua, -2));
// 		vec.push_back( lua_tostring( Lua, -1 ) );
// 		lua_pop( Lua, 1 );
// 	}

	Log( "Initial Script" );
	return true;
}

template< typename T >
bool LuaScript::GetValueParam( const std::string & srParam, T * pValue )
{
	if( !pValue || !m_LuaState )
		return false;

	lua_getglobal( m_LuaState, srParam.c_str() );

	if( lua_isnil( m_LuaState, -1 ) )
	{
		lua_pop( m_LuaState, 1 );
		return false;
	}

	*pValue = GetLuaValue < T >( srParam );
	lua_pop( m_LuaState, 1 );
	return true;
}

template <>
bool LuaScript::GetLuaValue< bool >( const std::string & variableName )
{
	return lua_toboolean( m_LuaState, -1 ) ? true : false;
}

template <>
float LuaScript::GetLuaValue< float >( const std::string & variableName )
{
	if( !lua_isnumber( m_LuaState, -1 ) )
		return 0.f;

	return (float)lua_tonumber( m_LuaState, -1 );
}

template <>
int LuaScript::GetLuaValue<int>( const std::string & variableName )
{
	if(!lua_isnumber( m_LuaState, -1 ) )
		return 0;

	return (int)lua_tonumber( m_LuaState, -1 );
}

template <>
std::string LuaScript::GetLuaValue< std::string >( const std::string & variableName )
{
	std::string s;

	if( lua_isstring( m_LuaState, -1 ) )
		s = std::string( lua_tostring( m_LuaState, -1 ) );

	return s;
}

void LuaScript::Clear()
{
	if( m_pFileBuffer )
	{
		delete [] m_pFileBuffer;
		m_pFileBuffer = 0;
	}
}

void LuaScript::ReleaseLua()
{
	if( m_pThis )
	{
		delete m_pThis;
		m_pThis = 0;
	}
}

LuaScript::~LuaScript()
{
	Clear();	
	lua_close( m_LuaState );
}
/*
bool LuaScript::LoadParamTank( const std::string& srPathScript, CParamTank** pParamTank )
{
	bool bResult = false;

	if( pParamTank || srPathScript.empty() )
	{
		if( lua_State* pLua = lua_open() )
		{
			// инициализация стандартных библиотечных функции lua
			luaopen_base( pLua );
			luaopen_table( pLua );
			luaopen_string( pLua );
			luaopen_math( pLua );
			luaopen_os( pLua );

			luaL_loadfile( pLua, srPathScript.c_str() );

			if( !lua_pcall( pLua, 0, LUA_MULTRET, 0 ) )
			{
			}
			else
			{
				char const* ErrorMsg = lua_tostring( pLua, -1 );
				lua_pop( pLua, 1 );
				Log( ErrorMsg );
			}

			lua_close( pLua );
		}
		else
			Log( "Error Initializing Lua" );
	}

	return bResult;
}
*/