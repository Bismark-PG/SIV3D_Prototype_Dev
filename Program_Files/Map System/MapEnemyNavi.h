#pragma once
#include <Siv3D.hpp>
class MapCollider;

struct NaviGridConfig {
	s3d::RectF worldBounds{ 0,0, 1024,768 };
	double cell = 32.0;
	double agentRadius = 10.0;
};

class MapEnemyNavi {
public:
	void clear();

	void buildFromCollider(const MapCollider& col, const NaviGridConfig& cfg);
	void setDynamicBlocked(const s3d::RectF& worldRect, bool blocked);

	bool findPath(const s3d::Vec2& worldStart,
				  const s3d::Vec2& worldGoal,
				  s3d::Array<s3d::Vec2>& outPath) const;

	bool isWalkableWorld(const s3d::Vec2& p) const;
	s3d::Vec2 cellCenterWorld(int x, int y) const;
	s3d::Point worldToCell(const s3d::Vec2& p) const;

	uint32 dynamicLayerVersion() const { return m_dynamicVersion; }
	void drawDebug() const;

private:
	struct Node { int x{}, y{}; };
	int  width()  const { return m_w; }
	int  height() const { return m_h; }
	bool inBounds(int x, int y) const { return (0 <= x && x < m_w && 0 <= y && y < m_h); }
	int  index(int x, int y) const { return y * m_w + x; }
	bool isBlocked(int x, int y) const {
		const int i = index(x, y); return (m_static[i] || m_dynamic[i]);
	}

	struct PQItem { int idx; double f; };

	s3d::RectF m_worldBounds{ 0,0,0,0 };
	double m_cell = 32.0;
	int m_w = 0, m_h = 0;
	double m_agentRadius = 10.0;

	s3d::Array<uint8> m_static;
	s3d::Array<uint8> m_dynamic;
	uint32 m_dynamicVersion = 0;
};
