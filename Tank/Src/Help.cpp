#include "Help.h"

#define MAX_STR_LENGTH 4096

void Log( const char * szFormat, ... )
{
//#ifdef DEBUG
	FILE * pLogFile = 0;
	errno_t err = fopen_s( &pLogFile, "log.txt", "a" );

	if( !err )
	{
		va_list ap;
		va_start( ap, szFormat );
		vfprintf( pLogFile, szFormat, ap );
		va_end( ap );
		fprintf( pLogFile, "\n");
		fclose( pLogFile );
	}
//#endif
}

bool Utf8ToAnsi( std::string & str_src, std::string & str_dst )
{
	if( str_src.empty() )
	{
		str_dst.clear();
		return true;
	}

	Utf8ToAnsi( str_src.c_str(), &str_dst, 512 );
	return true;
}

bool Utf8ToAnsi( const char * str_src, std::string *str_dst, int max_length_dst )
{
	if( ! str_src || ! str_dst || max_length_dst <= 0 )
		return false;

	WCHAR swStr[ MAX_STR_LENGTH ] = L"";
	MultiByteToWideChar( CP_UTF8, 0, str_src, strlen( str_src ), swStr, MAX_STR_LENGTH );
	swStr[ MAX_STR_LENGTH - 1 ] = 0;
	char sss[ MAX_STR_LENGTH ] = "";
	WideCharToMultiByte( CP_ACP, 0, swStr, wcslen( swStr ), sss, MAX_STR_LENGTH, NULL, NULL );
	sss[ MAX_STR_LENGTH - 1 ] = 0;
	*str_dst = sss;
	return true;
}

bool AnsiToUtf8( const char * str_src, std::string &str_dst )
{
	if( !str_src )
		return false;

	WCHAR wData[ MAX_STR_LENGTH ] = L"";
	int count = MultiByteToWideChar( CP_ACP, 0, str_src, strlen(str_src), wData, MAX_STR_LENGTH );

	if(count<=0)
		return false;

	wData[ count ] = 0;

	char utfData[ MAX_STR_LENGTH ] = "";
	count = WideCharToMultiByte( CP_UTF8, 0, wData, wcslen( wData ), utfData, MAX_STR_LENGTH, NULL, NULL );

	if(count<=0)
		return false;

	utfData[ count ] = 0;
	str_dst = utfData;
	return true;
}

bool Utf8ToAnsiW( const std::string & str_src, std::wstring & str_dst )
{
	WCHAR swStr[ MAX_STR_LENGTH ] = L"";
	MultiByteToWideChar( CP_UTF8, 0, str_src.c_str(), str_src.size(), swStr, MAX_STR_LENGTH );
	swStr[ MAX_STR_LENGTH - 1 ] = 0;
	str_dst = swStr;
	return true;
}

bool AnsiToUtf8W( const std::wstring & str_src, std::string & srDst )
{
	char utfData[ MAX_STR_LENGTH ] = "";
	WideCharToMultiByte( CP_UTF8, 0, str_src.c_str(), str_src.length(), utfData, MAX_STR_LENGTH, NULL, NULL );
	utfData[ MAX_STR_LENGTH - 1 ] = 0;
	srDst = utfData;
	return true;
}

bool AnsiToUtf8( std::string & str_src, std::string * str_dst )
{
	WCHAR wData[ MAX_STR_LENGTH ] = L"";
	MultiByteToWideChar( CP_ACP, 0, str_src.c_str(), str_src.size(), wData, MAX_STR_LENGTH );
	wData[ MAX_STR_LENGTH - 1 ] = 0;
	char utfData[ MAX_STR_LENGTH ] = "";
	WideCharToMultiByte( CP_UTF8, 0, wData, wcslen( wData ), utfData, MAX_STR_LENGTH, NULL, NULL );
	utfData[ MAX_STR_LENGTH - 1 ] = 0;
	*str_dst = utfData;
	return true;
}
