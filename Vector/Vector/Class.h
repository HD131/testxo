#pragma once

typedef class Vec3f
{
public:
	float x;
	float y;
	float z;
	Vec3f()
	{	};
	Vec3f( float X, float Y, float Z ):x(X), y(Y), z(Z)
	{	};

	// бинарные операторы
	Vec3f operator + ( const Vec3f& ) const;
	Vec3f operator - ( const Vec3f& ) const;
	Vec3f operator * ( float ) const;
	Vec3f operator / ( float ) const;
	Vec3f operator + ( float ) const;
	Vec3f operator - ( float ) const;
};

typedef class Vec3i
{
public:
	int x;
	int y;
	int z;
	Vec3i()
	{	};
	Vec3i( int X, int Y, int Z ):x(X), y(Y), z(Z)
	{	};

	// бинарные операторы
	Vec3i operator + ( const Vec3i& ) const;
	Vec3i operator - ( const Vec3i& ) const;
	Vec3i operator * ( int ) const;
	Vec3i operator / ( int ) const;
	Vec3i operator + ( int ) const;
	Vec3i operator - ( int ) const;
};

typedef class Vec3d
{
public:
	double x;
	double y;
	double z;
	Vec3d()
	{	};
	Vec3d( double X, double Y, double Z ):x(X), y(Y), z(Z)
	{	};

	// бинарные операторы
	Vec3d operator + ( const Vec3d& ) const;
	Vec3d operator - ( const Vec3d& ) const;
	Vec3d operator * ( double ) const;
	Vec3d operator / ( double ) const;
	Vec3d operator + ( double ) const;
	Vec3d operator - ( double ) const;
};

