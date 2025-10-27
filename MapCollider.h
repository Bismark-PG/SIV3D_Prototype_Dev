#pragma once
#include <Siv3D.hpp>

// EN: Map collision container + solver. Declarations only.
// JP: マップ衝突の保管と解決。宣言のみ。

struct MapCollisionShape {
	enum class Type { Rect, Poly };
	Type          type{ Type::Rect };
	s3d::RectF    rect;
	s3d::Polygon  poly;
};

class MapCollider {
public:
	void clear();
	void addRect(const s3d::RectF& r);
	void addPoly(const s3d::Polygon& p);

	// EN: Load only "collision" object layer from Tiled JSON.
	// JP: Tiled JSONの「collision」オブジェクトレイヤのみ読み込む。
	bool loadCollisionFromTiledJSON(const s3d::FilePath& path);

	s3d::Vec2 solveAABB(const s3d::RectF& aabb, const s3d::Vec2& desiredDelta) const;

	bool   intersectsAny(const s3d::RectF& r) const;

	void drawDebug() const;

private:

	double maxSafeT(const s3d::RectF& r, const s3d::Vec2& delta, int iters = 10) const;

private:
	s3d::Array<MapCollisionShape> m_shapes;
};
