#include "Wall.h"

void Wall::init(Point a, Point b, Point c, Point d)
{
	vertexes[0] = a;
	vertexes[1] = b;
	vertexes[2] = c;
	vertexes[3] = d;
}

void Wall::initColor(GLfloat c[3])
{
    for (int i = 0; i < 3; i++)
        color[i] = c[i];
}

void Wall::renderWall()
{
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_POLYGON);
    for (auto vertex : vertexes) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();
}
