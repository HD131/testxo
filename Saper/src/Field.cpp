#include "Field.h"

CField::CField( int Size )
{
	m_Cell.resize(Size*Size);
	m_Field.resize(Size*Size);
	int x,y;
	for ( y = 0; y < Size; ++y )
		for ( x = 0; x < Size; ++x )
		{
			m_Cell[x*Size+y].SetCenter(  y - int((Size-1)/2) , 0 ,x - int((Size-1)/2));	
			m_Cell[x*Size+y].m_Value = Empty;
			m_Field[x*Size+y]        = Empty;
		}
		int mine = 0;
		srand( time(0) );
		while ( mine < MaxMine )
		{
			x = rand() % Size;
			y = rand() % Size;
			if ( m_Cell[x*Size+y].m_Value == Empty )
			{
				m_Cell[x*Size+y].m_Value = Mine;
				++mine;
			}
		}			
		for ( y = 0; y < Size; ++y )
			for ( x = 0; x < Size; ++x )
			{
				mine = 0;
				if (  m_Cell[x*Size+y].m_Value == Empty )
				{					
					for ( int a = x - 1; a < x + 2; ++a )
						for ( int b = y - 1; b < y + 2; ++b)						
							if (  (a >=0) && (b >= 0) && (a < Size) && (b < Size) && (m_Cell[a*Size+b].m_Value == Mine) )							
								++mine;	

				}
				if ( mine > 0)
					m_Cell[x*Size+y].m_Value = mine;
			}
}

CField::~CField()
{
	m_Cell.~vector();
	m_Field.~vector();
}
