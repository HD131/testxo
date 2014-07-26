#pragma once

#include <windows.h>
#include <time.h>
#include <fstream>

#pragma warning( disable: 4996 )

void Log( const char * szFormat, ... );

#define DELETE_ONE( x )   if( x ) { delete x; x = 0; }
#define RELEASE_ONE( x )  if( x ) { x->Release();  x = 0; }
#define SAFE_RELEASE( x ) if( x ){ x->release(); x = 0; }

bool Utf8ToAnsi( const char * str_src, std::string * str_dst, int max_length_dst );
bool Utf8ToAnsi( std::string & str_src, std::string & str_dst );

bool AnsiToUtf8( const char * str_src, std::string & str_dst );
bool AnsiToUtf8( std::string & str_src, std::string * str_dst );

bool Utf8ToAnsiW( const std::string & str_src, std::wstring & str_dst );
bool AnsiToUtf8W( const std::wstring & str_src, std::string & srDst );

