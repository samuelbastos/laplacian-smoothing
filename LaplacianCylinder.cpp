#include <iostream>
#include <vector>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cfloat> 
#include "Point.h"

int numPointsPerCircle = 0;
int numCircles = 0;

class Circle
{
public:

	inline Circle(unsigned int ii, std::vector<Point> ps) 
	{ index = ii; points = ps; }
	~Circle() = default;

	inline void addPoint(Point p) { points.push_back(p); }

	inline unsigned int size() { return points.size(); }

	inline Point get(unsigned int i) { return points[i]; }

private:

	unsigned int index;
	std::vector<Point> points;
};

class Cylinder
{
public:
	
	inline Cylinder() {}
	~Cylinder() = default;
	
	inline void addCircle(Circle c) { cy.push_back(c); }

	inline unsigned int size() { return cy.size(); }

	inline Circle get(unsigned int i) { return cy[i]; }

private:

	std::vector<Circle> cy;
};

class Triangle
{
public:

	inline Triangle(unsigned int v11, unsigned int v22,
					unsigned int v33)
	{
		v1 = v11;
		v2 = v22;
		v3 = v33;
	}

	~Triangle() = default;

	inline unsigned int getV1() { return v1; }
	inline unsigned int getV2() { return v2; }
	inline unsigned int getV3() { return v3; }

private:

	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
};

Circle genCircle(double r, double dt, double z)
{
	std::vector<Point> v;
	double step = 2.f * M_PI / dt;

	for (double theta = 0.f; theta < 2 * M_PI; theta += step)
	{
		double x = r * std::cos(theta);
		double y = r * std::sin(theta);
		Point p = Point(v.size() + numPointsPerCircle*numCircles, x, y, z);
		v.push_back(p);
	}
	v.erase(v.end()-1);
	return Circle(numCircles, v);
}

Cylinder genCylinder(double r, double dt, double zb, double zt, double dz)
{
	Cylinder cy;
	for (double step = zb; step <= zt; step += dz)
	{
		auto circle = genCircle(r, dt, step);
		cy.addCircle(circle);
		numCircles++;
	}

	return cy;
}

// Rotaciona em torno de Y (0,1,0)
Point rotatePoint(Point u, double theta)
{
	const double rx = u.getX();
	const double ry = cos(theta)*u.getY() - sin(theta)*u.getZ();
	const double rz = sin(theta)*u.getY() + cos(theta)*u.getZ();
	return Point(u.getIndex(), rx, ry, rz);
}

void rotatePoints(std::vector<Point>& v, double theta)
{
	for (auto& point : v)
	{
		Point uplick = rotatePoint(point, theta);
		point = uplick;
	}
}

std::vector<Triangle> triangulate(Cylinder cylinder)
{
	std::vector<Triangle> mesh;
	for (unsigned int i = 0; i < cylinder.size() - 1; i++)
	{
		auto ci = cylinder.get(i);
		auto cipo = cylinder.get(i + 1);

		// Ida
		for (unsigned int j = 0; j < ci.size(); j++)
		{
			// level i ; level i plus one
			unsigned int li = j;
			unsigned int lipo = j + 1;
			if (j == ci.size() - 1)
				lipo = 0;
			auto v1 = ci.get(li).getIndex();
			auto v2 = ci.get(lipo).getIndex();
			auto v3 = cipo.get(li).getIndex();
			mesh.push_back(Triangle(v1, v2, v3));
		}

		// Volta
		for (unsigned int j = 0; j < ci.size(); j++)
		{
			// level i ; level i plus one

			unsigned int li = j;
			unsigned int lipo = j + 1;
			if (j == ci.size() - 1)
				lipo = 0;
			auto v1 = cipo.get(li).getIndex();
			auto v2 = ci.get(lipo).getIndex();
			auto v3 = cipo.get(lipo).getIndex();
			mesh.push_back(Triangle(v1, v2, v3));
		}
	}

	return mesh;
}

std::vector<Point> rotateCylinder(std::vector<Point> cylinderPoints)
{
	// Começo da escrita do cilindro rotacionado
	std::vector<Point> halfCylinderTop;
	auto begin = cylinderPoints.begin() + cylinderPoints.size() / 2;
	std::copy(begin, cylinderPoints.end(), std::back_inserter(halfCylinderTop));

	Point origin = Point(-1, 0.f, 0.f, 0.f);

	rotatePoints(halfCylinderTop, M_PI_2);

	for (auto& point : halfCylinderTop)
		point.set(point.getX(), point.getY(), point.getZ() + 10.f);

	std::vector<Point> halfCylinderBot;
	auto end = cylinderPoints.begin() + numCircles / 2 * numPointsPerCircle;
	std::copy(cylinderPoints.begin(), end, std::back_inserter(halfCylinderBot));

	std::vector<Point> rotatedCylinder;
	std::copy(halfCylinderBot.begin(), halfCylinderBot.end(), std::back_inserter(rotatedCylinder));
	std::copy(halfCylinderTop.begin(), halfCylinderTop.end(), std::back_inserter(rotatedCylinder));

	return rotatedCylinder;
}

void write(std::string name, std::vector<Point> cylinderPoints, std::vector<Triangle> mesh)
{
	std::ofstream file;
	file.open(name);

	// Escreve o cabeçalho do OFF
	file << "OFF" << std::endl;
	file << numPointsPerCircle * numCircles << " " << mesh.size()
		<< " " << 0 << std::endl;

	// Escreve os pontos no OFF
	for (auto point : cylinderPoints)
	{
		file << point.getX() << " " << point.getY() <<
			" " << point.getZ() << std::endl;
	}

	// Escreve os triangulos no OFF 
	for (auto m : mesh)
	{
		file << 3 << " " << m.getV1() << " " << m.getV2() << " " << m.getV3() << std::endl;
	}
}

void writeLaplacian(Cylinder cylinder, std::vector<Triangle> mesh, std::vector<Point> points)
{
	// escreve flei laplaciado
}

Point cross(Point a, Point b)
{
	double x = a.getY() * b.getZ() - a.getZ() * b.getY();
	double y = a.getZ() * b.getX() - a.getX() * b.getZ();
	double z = a.getX() * b.getY() - a.getY() * b.getX();
	return Point(-1, x, y, z);
}

double len2(Point p)
{
	double x = p.getX()*p.getX();
	double y = p.getY()*p.getY();
	double z = p.getZ()*p.getZ();
	return sqrt(x + y + z)*sqrt(x + y + z);
}

double lens(Point p)
{
	double x = p.getX()*p.getX();
	double y = p.getY()*p.getY();
	double z = p.getZ()*p.getZ();
	return sqrt(x + y + z);
}

// https://gamedev.stackexchange.com/questions/60630/how-do-i-find-the-circumcenter-of-a-triangle-in-3d
Point circumcenter(Point a, Point b, Point c)
{
	Point ac = c - a;
	Point ab = b - a;
	Point abXac = cross(ab, ac);

	Point num = cross(abXac, ab)*len2(ac) + cross(ac, abXac)*len2(ab);
	double dem = 2.0 * len2(abXac);
	Point cc = num / dem;
	Point res = a + cc;
	return res;
}

void unitaryLaplacianTest(Cylinder cy, std::vector<Point> points)
{
	auto current = cy.get(1);
	auto cpo = cy.get(2);
	auto cmo = cy.get(0);

	Point p0 = points[current.get(1).getIndex()];
	Point pI = points[current.get(2).getIndex()];
	Point pII = points[current.get(0).getIndex()];
	Point pIII = points[cpo.get(1).getIndex()];
	Point pIV = points[cpo.get(0).getIndex()];
	Point pV = points[cmo.get(1).getIndex()];
	Point pVI = points[cmo.get(2).getIndex()];

	// Colaboração de pIII
	double a = Point::distance(p0, pI);
	double b = Point::distance(pI, pIII);
	double c = Point::distance(pIII, p0);
	double alpha = acos((a*a + b * b - c * c) / (2 * a*b));
	double cotAlpha = 1.0 / tan(alpha);

	a = Point::distance(pIV, p0);
	b = Point::distance(p0, pIII);
	c = Point::distance(pIII, pIV);
	double beta = acos((a*a + c * c - b * b) / (2 * a*c));
	double cotBeta = 1.0 / tan(beta);

	Point dif = pIII - p0;
	double sumX = dif.getX() * (cotAlpha + cotBeta);
	double sumY = dif.getY() * (cotAlpha + cotBeta);
	double sumZ = dif.getZ() * (cotAlpha + cotBeta);

	// Colaboração de IV
	a = Point::distance(p0, pIII);
	b = Point::distance(pIII, pIV);
	c = Point::distance(pIV, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pII, p0);
	b = Point::distance(p0, pIV);
	c = Point::distance(pIV, pII);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pIV - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de II
	a = Point::distance(p0, pIV);
	b = Point::distance(pIV, pII);
	c = Point::distance(pII, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pV, p0);
	b = Point::distance(p0, pII);
	c = Point::distance(pII, pV);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pII - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de V
	a = Point::distance(p0, pII);
	b = Point::distance(pII, pV);
	c = Point::distance(pV, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pVI, p0);
	b = Point::distance(p0, pV);
	c = Point::distance(pV, pVI);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pV - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de VI
	a = Point::distance(p0, pV);
	b = Point::distance(pV, pVI);
	c = Point::distance(pVI, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pI, p0);
	b = Point::distance(p0, pVI);
	c = Point::distance(pVI, pI);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pVI - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de I
	a = Point::distance(p0, pVI);
	b = Point::distance(pVI, pI);
	c = Point::distance(pI, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pIII, p0);
	b = Point::distance(p0, pI);
	c = Point::distance(pI, pIII);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pI - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Calculo da primeira área 1
	Point c1 = circumcenter(p0, pI, pIII);
	Point m1 = (p0 + pI) / 2;
	Point m2 = (p0 + pIII) / 2;
	auto aa = Point::distance(m1, m2);
	auto bb = Point::distance(m1, p0);
	auto cc = Point::distance(m2, p0);
	auto dd = Point::distance(m1, c1);
	auto ee = Point::distance(m2, c1);

	// Heron's formula
	// https://www.geeksforgeeks.org/c-program-find-area-triangle/
	auto s1 = (aa + bb + cc) / 2.0;
	auto a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	auto s2 = (aa + dd + ee) / 2.0;
	auto a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a1 = a11 + a12;

	// Cross-product formula
	//Point av = Point(-1, m1.getX() - c1.getX(), m1.getY() - c1.getY(), m1.getZ() - c1.getZ());
	//Point bv = Point(-1, m2.getX() - c1.getX(), m2.getY() - c1.getY(), m2.getZ() - c1.getZ());
	//Point crossab = cross(av, bv);
	//double qwer = lens(crossab) / 2.0;
	//av = Point(-1, m1.getX() - p0.getX(), m1.getY() - p0.getY(), m1.getZ() - p0.getZ());
	//bv = Point(-1, m2.getX() - p0.getX(), m2.getY() - p0.getY(), m2.getZ() - p0.getZ());
	//crossab = cross(av, bv);
	//double qwer2 = lens(crossab) / 2.0;
	//double total = qwer + qwer2;

	// Cálculo da segunda área 2
	Point c2 = circumcenter(p0, pIII, pIV);
	m1 = (p0 + pIII) / 2;
	m2 = (p0 + pIV) / 2;
	aa = Point::distance(m1, m2);
	bb = Point::distance(m1, p0);
	cc = Point::distance(m2, p0);
	dd = Point::distance(m1, c2);
	ee = Point::distance(m2, c2);
	
	s1 = (aa + bb + cc) / 2.0;
	a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	s2 = (aa + dd + ee) / 2.0;
	a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a2 = a11 + a12;


	// Cálculo da terceira área 3
	Point c3 = circumcenter(p0, pIV, pII);
	m1 = (p0 + pIV) / 2;
	m2 = (p0 + pII) / 2;
	aa = Point::distance(m1, m2);
	bb = Point::distance(m1, p0);
	cc = Point::distance(m2, p0);
	dd = Point::distance(m1, c3);
	ee = Point::distance(m2, c3);

	s1 = (aa + bb + cc) / 2.0;
	a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	s2 = (aa + dd + ee) / 2.0;
	a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a3 = a11 + a12;


	// Cálculo da quarta área 4
	Point c4 = circumcenter(p0, pII, pV);
	m1 = (p0 + pII) / 2;
	m2 = (p0 + pV) / 2;
	aa = Point::distance(m1, m2);
	bb = Point::distance(m1, p0);
	cc = Point::distance(m2, p0);
	dd = Point::distance(m1, c4);
	ee = Point::distance(m2, c4);

	s1 = (aa + bb + cc) / 2.0;
	a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	s2 = (aa + dd + ee) / 2.0;
	a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a4 = a11 + a12;


	// Cálculo da quinta área 5
	Point c5 = circumcenter(p0, pV, pVI);
	m1 = (p0 + pV) / 2;
	m2 = (p0 + pVI) / 2;
	aa = Point::distance(m1, m2);
	bb = Point::distance(m1, p0);
	cc = Point::distance(m2, p0);
	dd = Point::distance(m1, c5);
	ee = Point::distance(m2, c5);

	s1 = (aa + bb + cc) / 2.0;
	a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	s2 = (aa + dd + ee) / 2.0;
	a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a5 = a11 + a12;


	// Cálculo da sexta área 6
	Point c6 = circumcenter(p0, pVI, pI);
	m1 = (p0 + pVI) / 2;
	m2 = (p0 + pI) / 2;
	aa = Point::distance(m1, m2);
	bb = Point::distance(m1, p0);
	cc = Point::distance(m2, p0);
	dd = Point::distance(m1, c6);
	ee = Point::distance(m2, c6);

	s1 = (aa + bb + cc) / 2.0;
	a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
	s2 = (aa + dd + ee) / 2.0;
	a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
	auto a6 = a11 + a12;


	auto fullArea = a1 + a2 + a3 + a4 + a5 + a6;
	double newX = sumX / (2.0 * fullArea);
	double newY = sumY / (2.0 * fullArea);
	double newZ = sumZ / (2.0 * fullArea);
	Point p0new = Point(p0.getIndex(), newX, newY, newZ);

	std::ofstream file;
	file.open("test.xyz");
	
	file << p0new.getX() << " " << p0new.getY() << " " << p0new.getZ() << std::endl;
	file << pI.getX() << " " << pI.getY() << " " << pI.getZ() << std::endl;
	file << pII.getX() << " " << pII.getY() << " " << pII.getZ() << std::endl;
	file << pIII.getX() << " " << pIII.getY() << " " << pIII.getZ() << std::endl;
	file << pIV.getX() << " " << pIV.getY() << " " << pIV.getZ() << std::endl;
	file << pV.getX() << " " << pV.getY() << " " << pV.getZ() << std::endl;
	file << pVI.getX() << " " << pVI.getY() << " " << pVI.getZ() << std::endl;
	file << p0.getX() << " " << p0.getY() << " " << p0.getZ() << std::endl;
	file.close();

	file.open("circuncenters.xyz");
	file << c1.getX() << " " << c1.getY() << " " << c1.getZ() << std::endl;
	file << c2.getX() << " " << c2.getY() << " " << c2.getZ() << std::endl;
	file << c3.getX() << " " << c3.getY() << " " << c3.getZ() << std::endl;
	file << c4.getX() << " " << c4.getY() << " " << c4.getZ() << std::endl;
	file << c5.getX() << " " << c5.getY() << " " << c5.getZ() << std::endl;
	file << c6.getX() << " " << c6.getY() << " " << c6.getZ() << std::endl;
	file.close();
}

void unitaryLaplacianTestUsingThirdOfArea(Cylinder cy, std::vector<Point> points)
{
	auto current = cy.get(1);
	auto cpo = cy.get(2);
	auto cmo = cy.get(0);

	Point p0 = points[current.get(1).getIndex()];
	Point pI = points[current.get(2).getIndex()];
	Point pII = points[current.get(0).getIndex()];
	Point pIII = points[cpo.get(1).getIndex()];
	Point pIV = points[cpo.get(0).getIndex()];
	Point pV = points[cmo.get(1).getIndex()];
	Point pVI = points[cmo.get(2).getIndex()];

	// Colaboração de pIII
	double a = Point::distance(p0, pI);
	double b = Point::distance(pI, pIII);
	double c = Point::distance(pIII, p0);
	double alpha = acos((a*a + b * b - c * c) / (2 * a*b));
	double cotAlpha = 1.0 / tan(alpha);

	a = Point::distance(pIV, p0);
	b = Point::distance(p0, pIII);
	c = Point::distance(pIII, pIV);
	double beta = acos((a*a + c * c - b * b) / (2 * a*c));
	double cotBeta = 1.0 / tan(beta);

	Point dif = pIII - p0;
	double sumX = dif.getX() * (cotAlpha + cotBeta);
	double sumY = dif.getY() * (cotAlpha + cotBeta);
	double sumZ = dif.getZ() * (cotAlpha + cotBeta);

	// Colaboração de IV
	a = Point::distance(p0, pIII);
	b = Point::distance(pIII, pIV);
	c = Point::distance(pIV, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pII, p0);
	b = Point::distance(p0, pIV);
	c = Point::distance(pIV, pII);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pIV - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de II
	a = Point::distance(p0, pIV);
	b = Point::distance(pIV, pII);
	c = Point::distance(pII, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pV, p0);
	b = Point::distance(p0, pII);
	c = Point::distance(pII, pV);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pII - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de V
	a = Point::distance(p0, pII);
	b = Point::distance(pII, pV);
	c = Point::distance(pV, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pVI, p0);
	b = Point::distance(p0, pV);
	c = Point::distance(pV, pVI);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pV - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de VI
	a = Point::distance(p0, pV);
	b = Point::distance(pV, pVI);
	c = Point::distance(pVI, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pI, p0);
	b = Point::distance(p0, pVI);
	c = Point::distance(pVI, pI);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pVI - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Colaboração de I
	a = Point::distance(p0, pVI);
	b = Point::distance(pVI, pI);
	c = Point::distance(pI, p0);
	alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

	a = Point::distance(pIII, p0);
	b = Point::distance(p0, pI);
	c = Point::distance(pI, pIII);
	beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

	dif = pI - p0;
	sumX += dif.getX() * (alpha + beta);
	sumY += dif.getY() * (alpha + beta);
	sumZ += dif.getZ() * (alpha + beta);

	// Calculo da primeira área 1
	auto aa = Point::distance(p0, pI);
	auto bb = Point::distance(pI, pIII);
	auto cc = Point::distance(pIII, p0);

	auto s1 = (aa + bb + cc) / 2.0;
	auto a1 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;

	// Cálculo da segunda área 2
	aa = Point::distance(p0, pIII);
	bb = Point::distance(pIII, pIV);
	cc = Point::distance(pIV, p0);
	s1 = (aa + bb + cc) / 2.0;
	auto a2 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;


	// Cálculo da terceira área 3
	Point c3 = circumcenter(p0, pIV, pII);
	aa = Point::distance(p0, pIV);
	bb = Point::distance(pIV, pII);
	cc = Point::distance(pII, p0);

	s1 = (aa + bb + cc) / 2.0;
	auto a3 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;

	// Cálculo da quarta área 4
	Point c4 = circumcenter(p0, pII, pV);
	aa = Point::distance(p0, pII);
	bb = Point::distance(pII, pV);
	cc = Point::distance(pV, p0);

	s1 = (aa + bb + cc) / 2.0;
	auto a4 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;

	// Cálculo da quinta área 5
	Point c5 = circumcenter(p0, pV, pVI);
	aa = Point::distance(p0, pV);
	bb = Point::distance(pV, pVI);
	cc = Point::distance(pVI, p0);

	s1 = (aa + bb + cc) / 2.0;
	auto a5 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;

	// Cálculo da sexta área 6
	Point c6 = circumcenter(p0, pVI, pI);
	aa = Point::distance(p0, pVI);
	bb = Point::distance(pVI, pI);
	cc = Point::distance(pI, p0);

	s1 = (aa + bb + cc) / 2.0;
	auto a6 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc)) / 3.0;

	auto fullArea = a1 + a2 + a3 + a4 + a5 + a6;
	double newX = sumX / (2.0 * fullArea);
	double newY = sumY / (2.0 * fullArea);
	double newZ = sumZ / (2.0 * fullArea);
	Point p0new = Point(p0.getIndex(), newX, newY, newZ);

	std::ofstream file;
	file.open("testTHIRD.xyz");

	file << p0new.getX() << " " << p0new.getY() << " " << p0new.getZ() << std::endl;
	file.close();
}

int main()
{
	double r = 2.5f;   // Raio do cilindro
	double dt = 64.f;  // Divisão do Theta
	double zb = 0.f;   // Coordenada Z da base do cilindro
	double zt = 11.f;  // Coordenada Z do topo do cilindro
	double dz = 1.f;   // Step de Z do cilindro

	// Apenas para calcular quantidade de pontos por circulo
	double step = 2.f * M_PI / dt;
	for (double theta = 0.f; theta < 2 * M_PI; theta += step)
		numPointsPerCircle++;
	numPointsPerCircle--;

	// Gera o cilindro e seus pontos
	auto cylinder = genCylinder(r, dt, zb, zt, dz);
	
	// Gera as faces (triangulos) do cilindro
	std::vector<Triangle> mesh = triangulate(cylinder);

	// Só carrega todos os pontos juntos em um vetor
	// Isso está horrível mas depois se arruma :P
	std::vector<Point> cylinderPoints;
	for (unsigned int i = 0; i < cylinder.size(); i++)
	{
		auto c = cylinder.get(i);
		for (unsigned int j = 0; j < c.size(); j++)
		{
			cylinderPoints.push_back(c.get(j));
		}
	}

	std::vector<Point> originalPoints = cylinderPoints;
	std::vector<Point> rotatedPoints = rotateCylinder(cylinderPoints);

	// Faz cópia dos pontos, apenas o vetor cópia é modificado durante a suavização
	std::vector<Point> cylinderModified;
	std::copy(rotatedPoints.begin(), rotatedPoints.end(), std::back_inserter(cylinderModified));
	std::vector<Point> aux = cylinderModified;

	unitaryLaplacianTest(cylinder, originalPoints);
	unitaryLaplacianTestUsingThirdOfArea(cylinder, originalPoints);

	for (unsigned int i = 0; i < 1; i++)
	{
		//// Está hardcoded, mas pode virar parâmetro
		//// Prende os pontos 0-2 e 9-11
		//// Prende os pontos 0-2 e 9-11
		std::cout << "i: " << i << std::endl;

		for (unsigned int ci = 3; ci < 9; ci++)
		{
			auto c = cylinder.get(ci);
			auto cpo = cylinder.get(ci + 1);
			auto cmo = cylinder.get(ci - 1);

			for (unsigned int j = 0; j < c.size(); j++)
			{
				int po = (j == c.size() - 1) ? 0 : j + 1;
				int mo = (j == 0) ? (c.size() - 1) : j - 1;

				Point p0 = cylinderModified[c.get(j).getIndex()];
				Point pI = cylinderModified[c.get(po).getIndex()];
				Point pII = cylinderModified[c.get(mo).getIndex()];
				Point pIII = cylinderModified[cpo.get(j).getIndex()];
				Point pIV = cylinderModified[cpo.get(mo).getIndex()];
				Point pV = cylinderModified[cmo.get(j).getIndex()];
				Point pVI = cylinderModified[cmo.get(po).getIndex()];

				// Colaboração de pIII
				double a = Point::distance(p0, pI);
				double b = Point::distance(pI, pIII);
				double c = Point::distance(pIII, p0);
				double alpha = acos((a*a + b * b - c * c) / (2 * a*b));
				double cotAlpha = 1.0 / tan(alpha);

				a = Point::distance(pIV, p0);
				b = Point::distance(p0, pIII);
				c = Point::distance(pIII, pIV);
				double beta = acos((a*a + c * c - b * b) / (2 * a*c));
				double cotBeta = 1.0 / tan(beta);

				Point dif = pIII - p0;
				double sumX = dif.getX() * (cotAlpha + cotBeta);
				double sumY = dif.getY() * (cotAlpha + cotBeta);
				double sumZ = dif.getZ() * (cotAlpha + cotBeta);

				// Colaboração de IV
				a = Point::distance(p0, pIII);
				b = Point::distance(pIII, pIV);
				c = Point::distance(pIV, p0);
				alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

				a = Point::distance(pII, p0);
				b = Point::distance(p0, pIV);
				c = Point::distance(pIV, pII);
				beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

				dif = pIV - p0;
				sumX += dif.getX() * (alpha + beta);
				sumY += dif.getY() * (alpha + beta);
				sumZ += dif.getZ() * (alpha + beta);

				// Colaboração de II
				a = Point::distance(p0, pIV);
				b = Point::distance(pIV, pII);
				c = Point::distance(pII, p0);
				alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

				a = Point::distance(pV, p0);
				b = Point::distance(p0, pII);
				c = Point::distance(pII, pV);
				beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

				dif = pII - p0;
				sumX += dif.getX() * (alpha + beta);
				sumY += dif.getY() * (alpha + beta);
				sumZ += dif.getZ() * (alpha + beta);

				// Colaboração de V
				a = Point::distance(p0, pII);
				b = Point::distance(pII, pV);
				c = Point::distance(pV, p0);
				alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

				a = Point::distance(pVI, p0);
				b = Point::distance(p0, pV);
				c = Point::distance(pV, pVI);
				beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

				dif = pV - p0;
				sumX += dif.getX() * (alpha + beta);
				sumY += dif.getY() * (alpha + beta);
				sumZ += dif.getZ() * (alpha + beta);

				// Colaboração de VI
				a = Point::distance(p0, pV);
				b = Point::distance(pV, pVI);
				c = Point::distance(pVI, p0);
				alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

				a = Point::distance(pI, p0);
				b = Point::distance(p0, pVI);
				c = Point::distance(pVI, pI);
				beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

				dif = pVI - p0;
				sumX += dif.getX() * (alpha + beta);
				sumY += dif.getY() * (alpha + beta);
				sumZ += dif.getZ() * (alpha + beta);

				// Colaboração de I
				a = Point::distance(p0, pVI);
				b = Point::distance(pVI, pI);
				c = Point::distance(pI, p0);
				alpha = 1.0 / tan(acos((a*a + b * b - c * c) / (2 * a*b)));

				a = Point::distance(pIII, p0);
				b = Point::distance(p0, pI);
				c = Point::distance(pI, pIII);
				beta = 1.0 / tan(acos((a*a + c * c - b * b) / (2 * a*c)));

				dif = pI - p0;
				sumX += dif.getX() * (alpha + beta);
				sumY += dif.getY() * (alpha + beta);
				sumZ += dif.getZ() * (alpha + beta);

				// Calculo da primeira área 1
				Point c1 = circumcenter(p0, pI, pIII);
				Point m1 = (p0 + pI) / 2;
				Point m2 = (p0 + pIII) / 2;
				auto aa = Point::distance(m1, m2);
				auto bb = Point::distance(m1, p0);
				auto cc = Point::distance(m2, p0);
				auto dd = Point::distance(m1, c1);
				auto ee = Point::distance(m2, c1);

				auto s1 = (aa + bb + cc) / 2.0;
				auto a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				auto s2 = (aa + dd + ee) / 2.0;
				auto a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a1 = a11 + a12;

				// Cálculo da segunda área 2
				Point c2 = circumcenter(p0, pIII, pIV);
				m1 = (p0 + pIII) / 2;
				m2 = (p0 + pIV) / 2;
				aa = Point::distance(m1, m2);
				bb = Point::distance(m1, p0);
				cc = Point::distance(m2, p0);
				dd = Point::distance(m1, c2);
				ee = Point::distance(m2, c2);
				// https://www.geeksforgeeks.org/c-program-find-area-triangle/
				s1 = (aa + bb + cc) / 2.0;
				a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				s2 = (aa + dd + ee) / 2.0;
				a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a2 = a11 + a12;
				

				// Cálculo da terceira área 3
				Point c3 = circumcenter(p0, pIV, pII);
				m1 = (p0 + pIV) / 2;
				m2 = (p0 + pII) / 2;
				aa = Point::distance(m1, m2);
				bb = Point::distance(m1, p0);
				cc = Point::distance(m2, p0);
				dd = Point::distance(m1, c3);
				ee = Point::distance(m2, c3);

				s1 = (aa + bb + cc) / 2.0;
				a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				s2 = (aa + dd + ee) / 2.0;
				a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a3 = a11 + a12;
				

				// Cálculo da quarta área 4
				Point c4 = circumcenter(p0, pII, pV);
				m1 = (p0 + pII) / 2;
				m2 = (p0 + pV) / 2;
				aa = Point::distance(m1, m2);
				bb = Point::distance(m1, p0);
				cc = Point::distance(m2, p0);
				dd = Point::distance(m1, c4);
				ee = Point::distance(m2, c4);

				s1 = (aa + bb + cc) / 2.0;
				a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				s2 = (aa + dd + ee) / 2.0;
				a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a4 = a11 + a12;
				

				// Cálculo da quinta área 5
				Point c5 = circumcenter(p0, pV, pVI);
				m1 = (p0 + pV) / 2;
				m2 = (p0 + pVI) / 2;
				aa = Point::distance(m1, m2);
				bb = Point::distance(m1, p0);
				cc = Point::distance(m2, p0);
				dd = Point::distance(m1, c5);
				ee = Point::distance(m2, c5);

				s1 = (aa + bb + cc) / 2.0;
				a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				s2 = (aa + dd + ee) / 2.0;
				a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a5 = a11 + a12;
				

				// Cálculo da sexta área 6
				Point c6 = circumcenter(p0, pVI, pI);
				m1 = (p0 + pVI) / 2;
				m2 = (p0 + pI) / 2;
				aa = Point::distance(m1, m2);
				bb = Point::distance(m1, p0);
				cc = Point::distance(m2, p0);
				dd = Point::distance(m1, c6);
				ee = Point::distance(m2, c6);

				s1 = (aa + bb + cc) / 2.0;
				a11 = sqrt(s1*(s1 - aa)*(s1 - bb)*(s1 - cc));
				s2 = (aa + dd + ee) / 2.0;
				a12 = sqrt(s2*(s2 - aa)*(s2 - dd)*(s2 - ee));
				auto a6 = a11 + a12;
				
				
				auto fullArea = a1 + a2 + a3 + a4 + a5 + a6;
				double newX = sumX / (2.0 * fullArea);
				double newY = sumY / (2.0 * fullArea);
				double newZ = sumZ / (2.0 * fullArea);
				aux[p0.getIndex()] = Point(p0.getIndex(), newX, newY,
					newZ);
			}
		}

		cylinderModified = aux;
	}
	
	write("cylinder.off", cylinderPoints, mesh);
	write("rotate.off", rotatedPoints, mesh);
	write("laplace.off", cylinderModified, mesh);

	return 0;
}