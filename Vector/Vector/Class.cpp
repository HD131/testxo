#include "Class.h"

//---------------------float-----------------------------
Vec3f Vec3f::operator + ( const Vec3f &v) const
{	
	return Vec3f( x + v.x, y + v.y, z + v.z );
}

Vec3f Vec3f::operator - ( const Vec3f &v) const
{	
	return Vec3f( x + v.x, y + v.y, z + v.z );
}

Vec3f Vec3f::operator * ( float a ) const
{	
	return Vec3f( x * a, y * a, z * a );
}

Vec3f Vec3f::operator / ( float a ) const
{	
	return Vec3f( x / a, y / a, z / a );
}

Vec3f Vec3f::operator + ( float a ) const
{	
	return Vec3f( x + a, y + a, z + a );
}

Vec3f Vec3f::operator - ( float a ) const
{	
	return Vec3f( x - a, y - a, z - a );
}
//----------------------int----------------------------
Vec3i Vec3i::operator + ( const Vec3i &v) const
{	
	return Vec3i( x + v.x, y + v.y, z + v.z );
}

Vec3i Vec3i::operator - ( const Vec3i &v) const
{	
	return Vec3i( x + v.x, y + v.y, z + v.z );
}

Vec3i Vec3i::operator * ( int a ) const
{	
	return Vec3i( x * a, y * a, z * a );
}

Vec3i Vec3i::operator / ( int a ) const
{	
	return Vec3i( x / a, y / a, z / a );
}

Vec3i Vec3i::operator + ( int a ) const
{	
	return Vec3i( x + a, y + a, z + a );
}

Vec3i Vec3i::operator - ( int a ) const
{	
	return Vec3i( x - a, y - a, z - a );
}
//---------------------double-----------------------------
Vec3d Vec3d::operator + ( const Vec3d &v) const
{	
	return Vec3d( x + v.x, y + v.y, z + v.z );
}

Vec3d Vec3d::operator - ( const Vec3d &v) const
{	
	return Vec3d( x + v.x, y + v.y, z + v.z );
}

Vec3d Vec3d::operator * ( double a ) const
{	
	return Vec3d( x * a, y * a, z * a );
}

Vec3d Vec3d::operator / ( double a ) const
{	
	return Vec3d( x / a, y / a, z / a );
}

Vec3d Vec3d::operator + ( double a ) const
{	
	return Vec3d( x + a, y + a, z + a );
}

Vec3d Vec3d::operator - ( double a ) const
{	
	return Vec3d( x - a, y - a, z - a );
}
//--------------------------------------------------