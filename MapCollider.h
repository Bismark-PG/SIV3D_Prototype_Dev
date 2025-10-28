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

	// === Dynamic colliders (for doors, switches, etc.) ===
	using ShapeID = uint32;

	ShapeID addDynamicRect(const s3d::RectF& r, bool enabled = true);
	ShapeID addDynamicPoly(const s3d::Polygon& p, bool enabled = true);
	void    setDynamicEnabled(ShapeID id, bool enabled);
	void    removeDynamic(ShapeID id);

private:

	double maxSafeT(const s3d::RectF& r, const s3d::Vec2& delta, int iters = 10) const;

	s3d::Array<MapCollisionShape> m_shapes;

	// ---- dynamic shapes storage ----
	enum class DynType { Rect, Poly };
	struct DynamicShape {
		ShapeID       id = 0;
		bool          enabled = true;
		DynType       type = DynType::Rect;
		s3d::RectF    rect;
		s3d::Polygon  poly;
	};
	s3d::Array<DynamicShape> m_dynamic;
	ShapeID m_nextDynId = 1;
};
