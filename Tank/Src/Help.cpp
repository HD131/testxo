#include "Help.h"

void Log( const char * Str )
{
	FILE *FileLog = fopen( "log.txt", "a" );
	if( FileLog )
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		fprintf( FileLog, "|%d:%d:%d| ", st.wHour, st.wMinute, st.wSecond );
		fprintf( FileLog, Str );
		fprintf( FileLog, "\n" );
		fclose(  FileLog );
	}
}
