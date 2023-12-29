#include <math.h>
#include "Camera.h"

#define PI 3.14

Camera::Camera()
{
}

Camera::Camera(float r, float h)
{
	this->radius = r;
	this->height = h;
	this->arc = 0;
	this->speed = 0.15;
	this->lookAt.setPoint(0, 0, 0);
	move();
}

void Camera::move()
{
	this->pos.setPoint(this->radius * -sin(this->arc) + this->lookAt.posX(),
		this->height + this->lookAt.posY(),
		this->radius * cos(this->arc) + this->lookAt.posZ());
}

// 键盘控制前后左右移动
void Camera::horizentalMove(int direction)
{
	float theta_x = 0, theta_z = 0;
	// w a s d
	switch (direction)
	{
	case 0:
		theta_x = sin(this->arc) * this->speed;
		theta_z = -cos(this->arc) * this->speed;
		break;
	case 1:
		theta_x = -cos(this->arc) * this->speed;
		theta_z = -sin(this->arc) * this->speed;
		break;
	case 2:
		theta_x = -sin(this->arc) * this->speed;
		theta_z = cos(this->arc) * this->speed;
		break;
	case 3:
		theta_x = cos(this->arc) * this->speed;
		theta_z = sin(this->arc) * this->speed;
		break;
	default:
		break;
	}

	this->lookAt.setPoint(lookAt.posX() + theta_x, lookAt.posY(), lookAt.posZ() + theta_z);
	move();
}

// 鼠标控制上下移动
void Camera::mouseMove(int x, int y, int& fx, int& fy)
{
	int dx = x - fx;
	int dy = y - fy;
	this->arc += dx * this->speed;
	while (this->arc < 0) this->arc += 2.0 * PI;
	while (this->arc >= 2.0 * PI) this->arc -= 2.0 * PI;
	this->height += dy * this->speed;
	move();
	fx = x;
	fy = y;
}
