#pragma once

class String
{
public:
					String( const char * szStr = 0 );
					String( char Char );
					String( const String& Str );
					~String();

public:
	void			Clear();
	const char *	CStr() const;
	size_t			Size() const;
	bool			IsEmpty() const;
	size_t			Find( const char * str ) const;

	String			operator + ( const String& str ) const;

	String&			operator = ( const String & str );
	String&			operator = ( const char * str );

	char&			operator [] ( size_t n );
	const char&		operator [] ( size_t n ) const;

	String&			operator += ( const String & str );
	String&			operator += ( const char * str );

	friend bool		operator == ( const String& str1, const String& str2 );
	friend bool		operator == ( const String& str1, const char *  str2 );
	friend bool		operator == ( const char *  str1, const String& str2 );

	friend bool		operator != ( const String& str1, const String& str2 );
	friend bool		operator != ( const char *  str1, const String& str2 );
	friend bool		operator != ( const String& str1, const char *  str2 );

private:
	void			Allocate( const size_t& nSize );

private:
	char *			m_pString;
	size_t			m_nLenght;
	size_t			m_nAllocator;
};