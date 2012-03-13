#include "Class.h"
#include <iostream>
using namespace std;

int main()
{
	Vec3f v1(1,2,3);
	Vec3f v2(1,2,3);
	Vec3f v3 = v1 / 2;

	cout << v3.x << "\t" << v3.y << "\t" << v3.z;
	cin.get();
	return 0;
}