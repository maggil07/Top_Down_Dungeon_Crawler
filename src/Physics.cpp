#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto overlap = Vec2(0, 0);
	
	//Prevent non-valid entities
	if (a->hasComponent<CTransform>() && a->hasComponent<CBoundingBox>() &&
		b->hasComponent<CTransform>() && b->hasComponent<CBoundingBox>())
	{
		const float x1 = a->getComponent<CTransform>()->pos.x;
		const float x2 = b->getComponent<CTransform>()->pos.x;

		const float y1 = a->getComponent<CTransform>()->pos.y;
		const float y2 = b->getComponent<CTransform>()->pos.y;

		const float w1 = a->getComponent<CBoundingBox>()->size.x;
		const float h1 = a->getComponent<CBoundingBox>()->size.y;

		const float w2 = b->getComponent<CBoundingBox>()->size.x;
		const float h2 = b->getComponent<CBoundingBox>()->size.y;

		Vec2 delta = Vec2(abs(x1 - x2), abs(y1 - y2));

		const float ox = (w1 / 2) + (w2 / 2) - delta.x;
		const float oy = (h1 / 2) + (h2 / 2) - delta.y;

		overlap = Vec2(ox, oy);
	}

	return overlap;
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto overlap = Vec2(0, 0);

	//Prevent non-valid entities
	if (a->hasComponent<CTransform>() && a->hasComponent<CBoundingBox>() &&
		b->hasComponent<CTransform>() && b->hasComponent<CBoundingBox>())
	{
		const float x1 = a->getComponent<CTransform>()->prevPos.x;
		const float x2 = b->getComponent<CTransform>()->prevPos.x;

		const float y1 = a->getComponent<CTransform>()->prevPos.y;
		const float y2 = b->getComponent<CTransform>()->prevPos.y;

		const float w1 = a->getComponent<CBoundingBox>()->size.x;
		const float h1 = a->getComponent<CBoundingBox>()->size.y;

		const float w2 = b->getComponent<CBoundingBox>()->size.x;
		const float h2 = b->getComponent<CBoundingBox>()->size.y;

		Vec2 delta = Vec2(abs(x1 - x2), abs(y1 - y2));

		auto ox = a->getComponent<CBoundingBox>()->halfSize.x + b->getComponent<CBoundingBox>()->halfSize.x - delta.x;
		auto oy = a->getComponent<CBoundingBox>()->halfSize.y + b->getComponent<CBoundingBox>()->halfSize.y - delta.y;

		overlap = Vec2(ox, oy);
	}

	return overlap;
}

Intersect Physics::LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d)
{    
	Vec2 r = (b - a);
	Vec2 s = (d - c);

	float rxs = r * s;

	Vec2 cma = c - a;

	float t = (cma * s) / rxs;
	float u = (cma * r) / rxs;

	if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
	{
		return { true, Vec2(a.x + t * r.x, a.y + t * r.y) };
	}
	else 
	{
		return { false, Vec2(0,0) };
	}
}

bool Physics::EntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e)
{
	bool doesIntesect = false;

	//Prevent non-valid entities
	if (e->hasComponent<CTransform>() && e->hasComponent<CBoundingBox>())
	{
		Intersect left = LineIntersect(a, b,
			Vec2(e->getComponent<CTransform>()->pos.x - e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y - e->getComponent<CBoundingBox>()->halfSize.y),
			Vec2(e->getComponent<CTransform>()->pos.x - e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y + e->getComponent<CBoundingBox>()->halfSize.y));
		Intersect right = LineIntersect(a, b,
			Vec2(e->getComponent<CTransform>()->pos.x + e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y + e->getComponent<CBoundingBox>()->halfSize.y),
			Vec2(e->getComponent<CTransform>()->pos.x + e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y + e->getComponent<CBoundingBox>()->halfSize.y));
		Intersect top = LineIntersect(a, b,
			Vec2(e->getComponent<CTransform>()->pos.x - e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y - e->getComponent<CBoundingBox>()->halfSize.y),
			Vec2(e->getComponent<CTransform>()->pos.x + e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y - e->getComponent<CBoundingBox>()->halfSize.y));
		Intersect bottom = LineIntersect(a, b,
			Vec2(e->getComponent<CTransform>()->pos.x + e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y + e->getComponent<CBoundingBox>()->halfSize.y),
			Vec2(e->getComponent<CTransform>()->pos.x - e->getComponent<CBoundingBox>()->halfSize.x, e->getComponent<CTransform>()->pos.y + e->getComponent<CBoundingBox>()->halfSize.y));

		doesIntesect = left.result || right.result || top.result || bottom.result;
	}

	return doesIntesect;
}