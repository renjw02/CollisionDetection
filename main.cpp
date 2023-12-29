#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <fstream>
#include <random>
#include <stdlib.h>
#include "Camera.h"
#include "Wall.h"
#include "Ball.h"
#include "Collision.cuh"

//#define TEST
#ifdef TEST
    LARGE_INTEGER t1, t2, tc;
    int update_times = 0;
#endif // TEST


Camera camera(30, 30);
int fx, fy;
const float interval = 0.02;
const float max_radius = 1;
float length, width, height;
int column;
float grid_size;
int grid_x, grid_y, grid_z;
int ball_nums;
Wall walls[6];
Ball *balls;

void readConfig() {
    std::ifstream file("./config.txt");
    if (!file.is_open()) { // 检查文件是否成功打开
        std::cerr << "无法打开文件\n";
        exit(-1);
    }

    float temp[10] = { 0 };
    std::string input;
    int k = 0;
    while (file >> input) {
        temp[k++] = std::stof(input);
        //std::cout << tmp[k - 1] << ' ';
    }

    file.close(); // 关闭文件
    length = temp[0];
    width = temp[1];
    height = temp[2];
    column = temp[3];
    ball_nums = column * column * column;
}

// 渲染墙壁
void drawWalls() {
    // 3面不画便于观察
    for (int i = 0; i < 3; i++)
        walls[i].renderWall();
}

// 渲染小球
void drawBalls() {
    for (int i = 0; i < ball_nums; i++) {
        balls[i].render();
    }
}

// 初始化小球
void initBalls() {
    balls = new Ball[ball_nums];
    grid_size = max_radius * 1.5;
    grid_x = ceil(length * 2 / grid_size);
    grid_y = ceil(height / grid_size);
    grid_z = ceil(width * 2 / grid_size);
    GLfloat color[4] = { 0.2, 0.3, 0.4, 1 };

    // 小球平均初始化于空间中
    float diff_x = (2 * length - 2 * max_radius) / (column - 1);
    float diff_z = (2 * width - 2 * max_radius) / (column - 1);
    float diff_y = (height - 2 * max_radius) / (column - 1);

    for (int i = 0; i < column; i++)
    {
        for (int j = 0; j < column; j++)
        {
            for (int k = 0; k < column; k++)
            {
                int index = i * column * column + j * column + k;

                float place_x = diff_x * i + max_radius - length;
                float place_z = diff_z * j + max_radius - width;
                float place_y = diff_y * k + max_radius;
                Point pos(place_x, place_y, place_z);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis1(-15, 15);
                // 速度在-15到15之间
                float speed_x = dis1(gen);
                float speed_y = dis1(gen);
                float speed_z = dis1(gen);
                Point speed(speed_x, speed_y, speed_z);

                // 半径在0.5到1.5之间
                std::uniform_real_distribution<> dis2(0.5, 1.5);
                float radius = dis2(gen);
                // 质量在0.5到1.5之间
                float weight = dis2(gen);
                // 系数在0.5到1之间
                std::uniform_real_distribution<> dis3(0, 0.5);
                float coefficient = dis3(gen);

                balls[index].init(pos, radius, speed, weight, coefficient, color);
            }
        }

    }
}

// 初始化墙壁
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
#ifdef TEST
    update_times++;
    if (update_times == 1000) {
        std::cout << "render 1000 times\n";
        QueryPerformanceCounter(&t2);
        double time = (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart;
        std::cout << "time = " << time << std::endl;
        exit(0);
    }
#endif // TEST

    glutPostRedisplay();
    glutTimerFunc(20, update, 1);
}

void onKeyClick(unsigned char key, int x, int y)
{
    int type = -1;
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



void init() {
#ifdef DEBUG
    const GLubyte* OpenGLVersion = glGetString(GL_VERSION); //返回当前OpenGL实现的版本号  
    const GLubyte* gluVersion = gluGetString(GLU_VERSION); //返回当前GLU工具库版本
    printf("OpenGL实现的版本号：%s\n", OpenGLVersion);
    printf("OGLU工具库版本：%s\n", gluVersion);
#endif // DEBUG
    
    readConfig();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Collision Detection");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE);

    initWalls();
    initBalls();
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

void drawScene() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    setCamera();
   
    drawWalls();
    collisionDetection(balls, interval, length, width, height, grid_size, grid_x, grid_y, grid_z, ball_nums);
    drawBalls();


    glutSwapBuffers();
}

int main(int argc, char** argv) {
#ifdef TEST
    QueryPerformanceFrequency(&tc);
    QueryPerformanceCounter(&t1);
#endif // TEST

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

    glutMainLoop();

    return 0;
}