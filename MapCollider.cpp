#include "stdafx.h"
#include "MapCollider.h"
using namespace s3d;

void MapCollider::clear() { m_shapes.clear(); }
void MapCollider::addRect(const RectF& r) { MapCollisionShape s; s.type = MapCollisionShape::Type::Rect; s.rect = r; m_shapes << s; }
void MapCollider::addPoly(const Polygon& p) { MapCollisionShape s; s.type = MapCollisionShape::Type::Poly; s.poly = p; m_shapes << s; }

bool MapCollider::loadCollisionFromTiledJSON(const FilePath& path) {
	clear();
	const JSON json = JSON::Load(path);
	if (!json) {
		Console << U"[MapCollider] Failed to load JSON: " << path;
		return false;
	}
	for (const auto& layer : json[U"layers"].arrayView()) {
		const String type = layer[U"type"].getString();
		if (type == U"objectgroup" && layer[U"name"].getString() == U"collision") {
			for (const auto& obj : layer[U"objects"].arrayView()) {
				const double x = obj[U"x"].get<double>();
				const double y = obj[U"y"].get<double>();
				const bool   isPoly = obj.hasElement(U"polygon");

				if (isPoly) {
					Array<Vec2> pts;
					for (const auto& p : obj[U"polygon"].arrayView()) {
						pts << Vec2{ x + p[U"x"].get<double>(), y + p[U"y"].get<double>() };
					}
					if (pts.size() >= 3) addPoly(Polygon{ pts });
				}
				else {
					const double w = obj[U"width"].get<double>();
					const double h = obj[U"height"].get<double>();
					addRect(RectF{ x, y, w, h });
				}
			}
		}
	}
	return true;
}

Vec2 MapCollider::solveAABB(const RectF& aabb, const Vec2& desiredDelta) const {
	Vec2 allowed{ 0, 0 };
	if (desiredDelta.x != 0.0) {
		const Vec2 dx{ desiredDelta.x, 0.0 };
		const double t = maxSafeT(aabb.movedBy(allowed), dx);
		allowed.x += dx.x * t;
	}
	if (desiredDelta.y != 0.0) {
		const Vec2 dy{ 0.0, desiredDelta.y };
		const double t = maxSafeT(aabb.movedBy(allowed), dy);
		allowed.y += dy.y * t;
	}
	return allowed;
}

void MapCollider::drawDebug() const {
	for (const auto& s : m_shapes) {
		if (s.type == MapCollisionShape::Type::Rect) {
			s.rect.draw(ColorF{ 1,0,0,0.2 }); s.rect.drawFrame(1, 0, Palette::Red);
		}
		else {
			s.poly.draw(ColorF{ 1,0,0,0.18 }); s.poly.drawFrame(1, Palette::Red);
		}
	}
}

bool MapCollider::intersectsAny(const s3d::RectF& r) const
{
	for (const auto& s : m_shapes)
	{
		if (s.type == MapCollisionShape::Type::Rect)
		{
			if (r.intersects(s.rect))            // ✅ RectF vs RectF
				return true;
		}
		else
		{
			if (s.poly.intersects(r))           // ✅ Polygon vs RectF
				return true;
		}
	}
	return false;
}

double MapCollider::maxSafeT(const RectF& r, const Vec2& delta, int iters) const {
	double low = 0.0, high = 1.0;
	if (!intersectsAny(r.movedBy(delta))) return 1.0;
	if (intersectsAny(r)) return 0.0;
	for (int i = 0; i < iters; ++i) {
		const double mid = (low + high) * 0.5;
		const RectF test = r.movedBy(delta * mid);
		if (!intersectsAny(test)) low = mid; else high = mid;
	}
	return low;
}
