#pragma once

#include "D3DDevice.h"
#include <vector>

struct CField
{
	std::vector<CCell>   m_Cell;
	std::vector<int>     m_Field;
	CField( int Size );
   ~CField();
};

