#pragma once

#include <windows.h>
#include <time.h>
#include <fstream>

#pragma warning( disable: 4996 )

void Log( const char * Str );

#define DELETE_ONE( x )   if( x ) { delete x; x = 0; }
#define RELEASE_ONE( x )  if( x ) { x->Release();  x = 0; }
#define SAFE_RELEASE( x ) if( x ){ x->release(); x = 0; }

