#include "Class.h"

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