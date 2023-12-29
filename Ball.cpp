#include "Ball.h"
#include <GL/glut.h>

void Ball::init(Point p, float r, Point s, float w, float k, GLfloat c[])
{
	pos = p;
	radius = r;
	speed = s;
	weight = w;
	coefficient = k;

	for (int i = 0; i < 4; i++) {
		color[i] = c[i];
	}
}

void Ball::render()
{
	glColor4f(color[0], color[1], color[2], color[3]);
	//翡翠绿

	GLfloat mat_ambient[] = { 0.247250, 0.199500, 0.074500, 1.000000 };
	GLfloat mat_diffuse[] = { 0.751640, 0.606480, 0.226480, 1.000000 };
	GLfloat mat_specular[] = { 0.628281, 0.555802, 0.366065, 1.000000 };
	GLfloat mat_shininess[] = { 51.200001 }; //材质RGBA镜面指数，数值在0～128范围内
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glutSolidSphere(radius, 50, 50);
	glPopMatrix();
}
