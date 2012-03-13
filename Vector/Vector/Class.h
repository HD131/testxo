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
};

