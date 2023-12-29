#pragma once
#include <GL/glut.h>
#include "Point.h"

class Ball
{
public:
	Point pos;
	float radius;
	Point speed;
	float weight;
	float coefficient;
	
	GLfloat color[4];

	void init(Point p, float r, Point s, float w, float k, GLfloat c[]);
	void render();
};

