#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "Camera.h"
#include "Wall.h"

Camera camera(30, 30);
int fx, fy;
const float interval = 0.02;
const float length = 10, width = 10, height = 20;
Wall walls[6];

// 需读入数据


void init();
void setCamera();
void drawScene();
void drawPolygon(Point a, Point b, Point c, Point d);
void readConfig();

void drawWalls() {
   /* for (auto wall : walls) {
        wall.renderWall();
    }*/
    for (int i = 0; i < 5; i++)
        walls[i].renderWall();
}

void initWalls() {
    Point bottomA(-length, 0, -width);
    Point bottomB(-length, 0, width);
    Point bottomC(length, 0, -width);
    Point bottomD(length, 0, width);
    Point topA(-length, height, -width);
    Point topB(-length, height, width);
    Point topC(length, height, -width);
    Point topD(length, height, width);
    GLfloat border[3] = { 0.5,0.5,0.5 };
    GLfloat bottom[3] = { 0.3,0.2,0.1 };

    walls[0].init(bottomA, bottomB, bottomD, bottomC); // bottom
    walls[1].init(bottomA, bottomB, topB, topA);
    walls[3].init(bottomC, bottomD, topD, topC);
    walls[2].init(bottomA, bottomC, topC, topA);
    walls[4].init(bottomB, bottomD, topD, topB);
    walls[5].init(topA, topB, topD, topC); // top

    walls[0].initColor(bottom);
    for (int i = 1; i <= 4; i++)
        walls[i].initColor(border);
}

void onMouseClick(int button, int state, int x, int y)
{
    fx = x;
    fy = y;
}

void onMouseMove(int x, int y) {
    camera.mouseMove(x, y, fx, fy);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80.0f, (float)w / h, 1.0f, 1000.0f); // 视角：眼睛睁开的角度
    glMatrixMode(GL_MODELVIEW);
}

void update(int value)
{
    glutPostRedisplay();
    glutTimerFunc(20, update, 1);
}

void onKeyClick(unsigned char key, int x, int y)
{
    int type = -1;
    //std::cout << x << ',' << y << std::endl;
    if (key == 'w')
    {
        type = 0;
    }
    else if (key == 'a')
    {
        type = 1;
    }
    else if (key == 's')
    {
        type = 2;
    }
    else if (key == 'd')
    {
        type = 3;
    }
    camera.horizentalMove(type);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    init();
    // 绑定显示函数
    glutDisplayFunc(drawScene);
    //绑定计时器
    glutTimerFunc(0, update, 0);
    //绑定鼠标函数
    glutMouseFunc(onMouseClick);
    glutMotionFunc(onMouseMove);

    //绑定键盘
    glutKeyboardFunc(onKeyClick);

    //绑定更新函数
    glutReshapeFunc(reshape);

    
    //initShader();
    //frame();

    glutMainLoop();
    system("pause");
    return 0;
}

void init() {
    //readConfig();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Scene");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE);

    initWalls();
}



void setCamera()
{
    glLoadIdentity();
    gluLookAt(camera.getPos().posX(), camera.getPos().posY(), camera.getPos().posZ(), camera.getLookAt().posX(),
        camera.getLookAt().posY(), camera.getLookAt().posZ(), 0, 1, 0);
}


void drawPolygon(Point a, Point b, Point c, Point d)
{
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_POLYGON);
    glVertex3f(a.posX(), a.posY(), a.posZ());
    glVertex3f(b.posX(), b.posY(), b.posZ());
    glVertex3f(c.posX(), c.posY(), c.posZ());
    glVertex3f(d.posX(), d.posY(), d.posZ());
    glEnd();
}



//void readConfig()
//{
//    std::ifstream fin;
//    fin.open("./config.txt");
//    if (!fin.is_open()) {
//        exit(-1);
//    }
//
//    float tmp[18];
//    std::string input;
//    int k = 0;
//    while (fin >> input) {
//        tmp[k++] = std::stof(input);
//        //std::cout << tmp[k - 1] << ' ';
//    }
//    fin.close();
//
//    pos1.setPoint(tmp[0], tmp[1], tmp[2]);
//    pos2.setPoint(tmp[3], tmp[4], tmp[5]);
//    pos3.setPoint(tmp[6], tmp[7], tmp[8]);
//    speed.setPoint(tmp[9], tmp[10], tmp[11]);
//    color1[0] = tmp[12];
//    color1[1] = tmp[13];
//    color1[2] = tmp[14];
//    color2[0] = tmp[15];
//    color2[1] = tmp[16];
//    color2[2] = tmp[17];
//}

void drawScene() {
    // 渲染原始场景到帧缓冲对象
    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    setCamera();
   
    //drawPolygon(Point(0, 0, 0), Point(0, 5, 0), Point(5, 5, 2), Point(5, 0, 2));
    drawWalls();

    glutSwapBuffers();
    //glFlush();
}