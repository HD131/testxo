#include "String.h"
#include <string.h>
#include <assert.h>

#pragma warning( disable : 4996 )
#define SIZE_STRING 32

void String::Allocate( const size_t& nSize )
{
	if( nSize >= m_nAllocator )
	{
		if( m_pString )
		{
			delete [] m_pString;
			m_pString = 0;
		}

		while( nSize >= m_nAllocator )
			m_nAllocator *= 2;

		m_pString = new char[ m_nAllocator ];
		memset( m_pString, 0, sizeof( char ) * m_nAllocator );
	}
	else
	{
		if( !m_pString )
			m_pString = new char[ m_nAllocator ];

		memset( m_pString, 0, sizeof( char ) * m_nAllocator );
	}
}

String::String( const char * szStr ):
	m_pString( 0 ),
	m_nLenght( 0 ),
	m_nAllocator( SIZE_STRING )
{
	if( szStr )
	{
		m_nLenght = strlen( szStr );
		Allocate( m_nLenght );
		strcpy( m_pString, szStr );
	}
	else
		Allocate( m_nLenght );
}

String::String( char Char ):
	m_pString( 0 ),
	m_nLenght( 1 ),
	m_nAllocator( SIZE_STRING )
{
	Allocate( m_nLenght );
	m_pString[ 0 ]= Char;
}

String::String( const String& Str ):
	m_pString( 0 ),
	m_nLenght( 0 ),
	m_nAllocator( SIZE_STRING )
{
	m_nLenght = Str.Size();
	Allocate( m_nLenght );
	strcpy( m_pString, Str.CStr() );
}

String String::operator + ( const String& str ) const
{
	String temp( m_pString );
	temp += str;
	return temp;
}

String& String::operator = ( const String & str )
{
	// проверка на самоприсваивание
	if( this != &str )
	{
		m_nLenght = str.Size();
		Allocate( m_nLenght );
		strcpy( m_pString, str.CStr() );
	}

	return *this;
}

String& String::operator = ( const char * str )
{
	assert( str );

	// проверка на самоприсваивание
	if( m_pString != str )
	{
		m_nLenght = strlen( str );
		Allocate( m_nLenght );
		strcpy( m_pString, str );
	}

	return *this;
}

char& String::operator [] ( size_t n )
{
	assert( n < m_nLenght );
	return m_pString[ n ];
}

const char& String::operator [] ( size_t n ) const
{
	assert( n < m_nLenght );
	return m_pString[ n ];
}

String&	String::operator += ( const String & str )
{
	m_nLenght += str.Size();

	if( m_nLenght >= m_nAllocator )
	{
		if( m_pString )
		{
			delete [] m_pString;
			m_pString = 0;
		}

		while( m_nLenght >= m_nAllocator )
			m_nAllocator *= 2;

		m_pString = new char[ m_nAllocator ];
		memset( m_pString, 0, sizeof( char ) * m_nAllocator );
	}

	strcat( m_pString, str.CStr() );
	return *this;
}

String& String::operator += ( const char * str )
{
	if( str )
	{
		m_nLenght += strlen( str );

		if( m_nLenght >= m_nAllocator )
		{
			if( m_pString )
			{
				delete [] m_pString;
				m_pString = 0;
			}

			while( m_nLenght >= m_nAllocator )
				m_nAllocator *= 2;

			m_pString = new char[ m_nAllocator ];
			memset( m_pString, 0, sizeof( char ) * m_nAllocator );
		}

		strcat( m_pString, str );
	}

	return *this;
}

bool operator == ( const String& str1, const String& str2 )
{
	if ( str1.m_nLenght != str2.m_nLenght )
		return false;

	return strcmp( str1.m_pString, str2.m_pString ) ? false : true;
}

bool operator == ( const String &str1, const char * str2 )
{
	return strcmp( str1.m_pString, str2 ) ? false : true;
}

bool operator == ( const char * str1, const String &str2 )
{
	return strcmp( str1, str2.m_pString ) ? false : true;
}

bool operator != ( const String& str1, const String& str2 )
{
	if ( str1.m_nLenght != str2.m_nLenght )
		return true;

	return strcmp( str1.m_pString, str2.m_pString ) ? true : false;
}

bool operator != ( const String &str1, const char * str2 )
{
	return strcmp( str1.m_pString, str2 ) ? true : false;
}

bool operator != ( const char * str1, const String &str2 )
{
	return strcmp( str1, str2.m_pString ) ? true : false;
}

void String::Clear()
{
	m_nLenght = 0;
	memset( m_pString, 0, sizeof( char ) * m_nAllocator );
}

const char * String::CStr() const
{
	return m_pString;
}

size_t String::Size() const
{
	return m_nLenght;
}

bool String::IsEmpty() const
{
	return !m_nLenght ? true : false;
}

size_t String::Find( const char * str ) const
{
	return strspn( m_pString, str );
	return -1;
}

String::~String()
{
	if( m_pString )
		delete[] m_pString;
}