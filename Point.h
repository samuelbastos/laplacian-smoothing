#pragma once
#include <math.h>

class Point
{
public:
	Point(int iindex, double xx, double yy, double zz);
	~Point();

	inline void set(double xx, double yy, double zz) { x = xx; y = yy;  z = zz; }
	inline double getX() const { return x; }
	inline double getY() const { return y; }
	inline double getZ() const { return z; }
	inline int getIndex() const { return index; }

	inline static double distance(Point p1, Point p2)
	{
		double xx = (p1.x - p2.x) * (p1.x - p2.x);
		double yy = (p1.y - p2.y) * (p1.y - p2.y);
		double zz = (p1.z - p2.z) * (p1.z - p2.z);
		return sqrt(xx + yy + zz);
	}

private:

	int index;
	double x;
	double y;
	double z;
};

inline Point operator-(const Point& p1, const Point& p2)
{
	double x = p1.getX() - p2.getX();
	double y = p1.getY() - p2.getY();
	double z = p1.getZ() - p2.getZ();

	return Point(-1, x, y, z);
}

inline Point operator+(const Point& p1, const Point& p2)
{
	double x = p2.getX() + p1.getX();
	double y = p2.getY() + p1.getY();
	double z = p2.getZ() + p1.getZ();

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, const Point& p2)
{
	double x = p2.getX() * p1.getX();
	double y = p2.getY() * p1.getY();
	double z = p2.getZ() * p1.getZ();

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, double n)
{
	double x = p1.getX() * n;
	double y = p1.getY() * n;
	double z = p1.getZ() * n;

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, int n)
{
	double x = p1.getX() * n;
	double y = p1.getY() * n;
	double z = p1.getZ() * n;

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, const Point& p2)
{
	double x = p1.getX() / p2.getX();
	double y = p1.getY() / p2.getY();
	double z = p1.getZ() / p2.getZ();

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, int n)
{
	double x = p1.getX() / n;
	double y = p1.getY() / n;
	double z = p1.getZ() / n;

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, double n)
{
	double x = p1.getX() / n;
	double y = p1.getY() / n;
	double z = p1.getZ() / n;

	return Point(-1, x, y, z);
}