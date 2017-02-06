#pragma once
#include "stdafx.h"
#include "Draw.h"

struct Position
{
	int x;
	int y;

	Position(int x, int y)
		: x(x)
		, y(y)
	{}
};

struct Point
{
	int x;
	int y;
};

inline bool operator < (const Point &c1, const Point &c2)
{
	if (c1.x < c2.x && c1.y < c2.y)
	{
		return true;
	}

	return false;
}

inline bool operator > (const Point &c1, const Point &c2)
{
	if (c1.x > c2.x && c1.y > c2.y)
	{
		return true;
	}

	return false;
}



class FlyingObject
{
public:

	Position pos;
	unsigned int width;
	unsigned int height;

	FlyingObject(Position pos, unsigned int width, unsigned int height)
		: pos(pos)
		, width(width)
		, height(height)
	{}
	
	~FlyingObject() = default;
};

class Bullet : public FlyingObject
{
	bool visible;
public:
	Bullet(int x, int y, int width, int height)
		: FlyingObject(Position(x, y), width, height)
	{
		visible = false;
	}

	void setPosition(int x, int y)
	{
		pos.x = x;
		pos.y = y;
	}
	int  getX() { return pos.x; }
	void setX(int x) { pos.x = x; }
	void setY(int y) { pos.y = y; }
	void setVisible(bool isVisible) { visible = isVisible; }
	bool getVisible() const { return visible; }
	bool move();
	void render(Draw* pPen) const;
};

class Plane : public FlyingObject
{
	int windowWidth;

public:
	Plane(int x, int y, int width, int height, int winWidth)
		:FlyingObject(Position(x, y), width, height)
		, windowWidth(winWidth)
	{	}

	int  getX() { return pos.x; }
	int getWidth() { return width; }
	void move(float degree);
	void render(Draw* pPen) const;
};

class Enemy : public FlyingObject
{
	int m_speed;

public:
	Enemy(int x, int y, int width, int height)
		:FlyingObject(Position(x, y), width, height)
	{
		m_speed = rand() % 2  * 3 + 6;
	}

	bool move(int y_boundary);
	void render(Draw* pPen);
};

class CollisionDetector
{
public:
	static bool check(Bullet obj1, Enemy obj2);
};