#pragma once

class Point {
public:
	float x;
	float y;
	float z;

	Point() {
		x = 0;
		y = 0;
		z = 0;
	}

	Point(float nx, float ny, float nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	void setPoint(float nx, float ny, float nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	void setPoint(Point p) {
		x = p.posX();
		y = p.posY();
		z = p.posZ();
	}

	float posX() {
		return x;
	}

	float posY() {
		return y;
	}

	float posZ() {
		return z;
	}

	Point operator+(const Point& b)
	{
		Point c;
		c.x = x + b.x;
		c.y = y + b.y;
		c.z = z + b.z;
		return c;
	}

	Point operator-(const Point& b)
	{
		Point c;
		c.x = x - b.x;
		c.y = y - b.y;
		c.z = z - b.z;
		return c;
	}

	Point operator*(const float& b)
	{
		Point c;
		c.x = x * b;
		c.y = y * b;
		c.z = z * b;
		return c;
	}

	float operator*(const Point& b)
	{
		float sum = 0;
		sum += x * b.x;
		sum += y * b.y;
		sum += z * b.z;
		return sum;
	}
};