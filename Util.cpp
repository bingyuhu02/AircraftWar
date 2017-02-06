#pragma once
#include "stdafx.h"
#include "Util.h"


bool CollisionDetector::check(Bullet obj1, Enemy obj2)
{
	Position pos1 = obj1.pos;
	Position pos2 = obj2.pos;

	Point obj1_tl = { obj1.pos.x, obj1.pos.y };
	Point obj1_tr = { obj1.pos.x + obj1.width, obj1.pos.y };
	Point obj1_bl = { obj1.pos.x, obj1.pos.y + obj1.height };
	Point obj1_br = { obj1.pos.x + obj1.width, obj1.pos.y + obj1.height };

	Point obj2_tl = { obj2.pos.x, obj2.pos.y };
	Point obj2_tr = { obj2.pos.x + obj2.width, obj2.pos.y };
	Point obj2_bl = { obj2.pos.x, obj2.pos.y + obj2.height };
	Point obj2_br = { obj2.pos.x + obj2.width, obj2.pos.y + obj2.height };

	if ((obj1_br.x > obj2_tl.x && obj1_br.y > obj2_tl.y && obj1_br.x < obj2_br.x && obj1_br.y < obj2_br.y) ||
		(obj1_bl.x < obj2_tr.x && obj1_bl.y > obj2_tr.y && obj1_bl.x > obj2_bl.x && obj1_bl.y < obj2_bl.y) ||
		(obj1_tr.x > obj2_bl.x && obj1_tr.y < obj2_bl.y && obj1_tr.x < obj2_tr.x && obj1_tr.y > obj2_tr.y) ||
		(obj1_tl.x < obj2_br.x && obj1_tl.y < obj2_br.y && obj1_tl.x > obj2_tl.x && obj1_tl.y > obj2_tl.y))
	{
		return true;
	}

	return false;
}

void Plane::move(float degree)
{
	if (degree < -20)
	{
		// Move left

		pos.x -= 10;
		if (pos.x < 0)
		{
			pos.x = 0;
		}
	}

	if (degree > 20)
	{
		// Move right
	
		pos.x += 10;
		if (pos.x > windowWidth - width - 2)
		{
			pos.x = windowWidth - width - 2;
		}
	}

}

void Plane::render(Draw* pPen) const
{
	pPen->drawMyPlane(pos.x, pos.y, width, height);
}


void Bullet::render(Draw* pPen) const
{
	pPen->drawMyBullet(pos.x, pos.y, width, height);
}

bool Bullet::move()
{
	pos.y -= 10;

	if (pos.y <= 0)
	{
		return false;
	}

	return true;
}

bool Enemy::move(int y_boundary)
{
	pos.y += m_speed;

	if (pos.y >= y_boundary - height/2)
	{
		return false;
	}

	return true;
}

void Enemy::render(Draw* pPen)
{
	pPen->drawMyEnemy(pos.x, pos.y, width, height);
}