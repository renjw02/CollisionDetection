#pragma once
#include "Point.h"

class Camera
{
	Point pos;	// 相机在世界坐标系中位置
	Point lookAt;	// 视点
	float speed; 
	float radius;	// 水平移动
	float height;
	float arc;		// 欧拉角

public:
	Camera();
	Camera(float r, float h);
	void move();
	void horizentalMove(int direction);
	void mouseMove(int x, int y, int& fx, int& fy);

	Point getPos() {
		return pos;
	}

	Point getLookAt() {
		return lookAt;
	}
};

