#pragma once
#include "Point.h"
#include <GL/glut.h>

class Wall
{
public:
	Point vertexes[4];
	GLfloat color[3];

	void init(Point a, Point b, Point c, Point d);
	void initColor(GLfloat c[3]);
	void renderWall();
};

