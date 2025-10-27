#include "stdafx.h"
#include "MapEnemyNavi.h"
#include "MapCollider.h"
using namespace s3d;

void MapEnemyNavi::clear() {
	m_static.clear(); m_dynamic.clear();
	m_w = m_h = 0; m_dynamicVersion = 0; m_worldBounds = RectF{ 0,0,0,0 };
}

void MapEnemyNavi::buildFromCollider(const MapCollider& col, const NaviGridConfig& cfg) {
	clear();
	m_worldBounds = cfg.worldBounds; m_cell = cfg.cell; m_agentRadius = cfg.agentRadius;
	m_w = static_cast<int>(Math::Ceil(m_worldBounds.w / m_cell));
	m_h = static_cast<int>(Math::Ceil(m_worldBounds.h / m_cell));
	m_static.assign(m_w * m_h, 0); m_dynamic.assign(m_w * m_h, 0);

	// 逐格检测（用代理AABB），需要 MapCollider 提供：bool intersectsAny(const RectF&) const;
	for (int y = 0; y < m_h; ++y) for (int x = 0; x < m_w; ++x) {
		const s3d::RectF cell{
			cellCenterWorld(x, y) - s3d::Vec2{ m_cell * 0.5, m_cell * 0.5 },
			s3d::SizeF{ m_cell, m_cell }
		};
		// 把格子“外扩”代理半径，再和碰撞测试（Minkowski +e）
		const s3d::RectF inflated = cell.stretched(m_agentRadius);
		const bool blocked = col.intersectsAny(inflated);
		m_static[index(x, y)] = blocked ? 1 : 0;
	}
}

void MapEnemyNavi::setDynamicBlocked(const RectF& worldRect, bool blocked) {
	if (m_w == 0 || m_h == 0) return;

	const double x0w = s3d::Min(worldRect.x, worldRect.x + worldRect.w);
	const double y0w = s3d::Min(worldRect.y, worldRect.y + worldRect.h);
	const double x1w = s3d::Max(worldRect.x, worldRect.x + worldRect.w);
	const double y1w = s3d::Max(worldRect.y, worldRect.y + worldRect.h);

	// 与 cell 成比例的 epsilon（例如 1e-6 个 cell）
	const double eps = m_cell * 1e-6;
	const double x1adj = x1w - eps;
	const double y1adj = y1w - eps;

	auto toCellFloor = [&](double x, double y) -> s3d::Point {
		const double rx = (x - m_worldBounds.x) / m_cell;
		const double ry = (y - m_worldBounds.y) / m_cell;
		return { (int)s3d::Floor(rx), (int)s3d::Floor(ry) };
		};

	const s3d::Point cMin = toCellFloor(x0w, y0w);
	const s3d::Point cMax = toCellFloor(x1adj, y1adj);

	const int cx0 = s3d::Clamp(cMin.x, 0, m_w - 1);
	const int cy0 = s3d::Clamp(cMin.y, 0, m_h - 1);
	const int cx1 = s3d::Clamp(cMax.x, 0, m_w - 1);
	const int cy1 = s3d::Clamp(cMax.y, 0, m_h - 1);
	if (cx0 > cx1 || cy0 > cy1) return;

	for (int y = cy0; y <= cy1; ++y)
		for (int x = cx0; x <= cx1; ++x)
			m_dynamic[index(x, y)] = blocked ? 1 : 0;

	++m_dynamicVersion;
}

bool MapEnemyNavi::isWalkableWorld(const Vec2& p) const {
	if (m_w == 0 || m_h == 0) return false;
	const Point c = worldToCell(p);
	if (!inBounds(c.x, c.y)) return false;
	return !isBlocked(c.x, c.y);
}

Point MapEnemyNavi::worldToCell(const Vec2& p) const {
	const double rx = (p.x - m_worldBounds.x) / m_cell;
	const double ry = (p.y - m_worldBounds.y) / m_cell;
	return Point{ static_cast<int>(Floor(rx)), static_cast<int>(Floor(ry)) };
}

Vec2 MapEnemyNavi::cellCenterWorld(int x, int y) const {
	return Vec2{ m_worldBounds.x + (x + 0.5) * m_cell,
				 m_worldBounds.y + (y + 0.5) * m_cell };
}

bool MapEnemyNavi::findPath(const Vec2& sWorld, const Vec2& gWorld, Array<Vec2>& outPath) const {
	outPath.clear(); if (m_w == 0 || m_h == 0) return false;
	const Point s = worldToCell(sWorld), g = worldToCell(gWorld);
	if (!inBounds(s.x, s.y) || !inBounds(g.x, g.y)) return false;

	const auto passable = [&](int x, int y) { return inBounds(x, y) && !isBlocked(x, y); };
	if (!passable(s.x, s.y) || !passable(g.x, g.y)) return false;

	const int N = m_w * m_h;
	Array<double> gCost(N, Math::Inf);
	Array<int>    came(N, -1);
	auto h = [&](int x, int y) { return (cellCenterWorld(x, y) - cellCenterWorld(g.x, g.y)).length(); };

	struct Cmp { bool operator()(const PQItem& a, const PQItem& b) const { return a.f > b.f; } };
	std::priority_queue<PQItem, Array<PQItem>, Cmp> pq;

	const int sIdx = index(s.x, s.y);
	gCost[sIdx] = 0; pq.push({ sIdx, h(s.x,s.y) });

	const Point dirs[8] = { {1,0},{-1,0},{0,1},{0,-1}, {1,1},{1,-1},{-1,1},{-1,-1} };

	auto foreachNeighbor = [&](int x, int y, auto&& fn) {
		for (int k = 0; k < 8; ++k) {
			int nx = x + dirs[k].x, ny = y + dirs[k].y;
			if (!passable(nx, ny)) continue;
			// 防穿角：对角要求至少一条正交边可走
			if (k >= 4) { if (isBlocked(x, ny) && isBlocked(nx, y)) continue; }
			const double step = (k < 4 ? m_cell : m_cell * Math::Sqrt2);
			fn(nx, ny, step);
		}
		};

	bool found = false; int goalIdx = -1;
	while (!pq.empty()) {
		const auto cur = pq.top(); pq.pop();
		const int cx = cur.idx % m_w; const int cy = cur.idx / m_w;
		if (cx == g.x && cy == g.y) { found = true; goalIdx = cur.idx; break; }
		const double base = gCost[cur.idx];
		foreachNeighbor(cx, cy, [&](int nx, int ny, double step) {
			const int ni = index(nx, ny);
			const double ng = base + step;
			if (ng < gCost[ni]) { gCost[ni] = ng; came[ni] = cur.idx; pq.push({ ni, ng + h(nx,ny) }); }
		});
	}

	if (!found) return false;

	// 回溯
	Array<Point> rev;
	for (int i = goalIdx; i != -1; i = came[i]) rev << Point{ i % m_w, i / m_w };
	std::reverse(rev.begin(), rev.end());

	outPath.reserve(rev.size());
	for (const auto& c : rev) outPath << cellCenterWorld(c.x, c.y);
	return true;
}

void MapEnemyNavi::drawDebug() const {
	if (m_w == 0 || m_h == 0) return;
	const Color blockedC{ 255,64,64, 80 };
	const Color gridC{ 64,64,64, 80 };
	for (int y = 0; y < m_h; ++y) for (int x = 0; x < m_w; ++x) {
		const RectF r{ cellCenterWorld(x,y) - Vec2{m_cell * 0.5, m_cell * 0.5}, SizeF{m_cell, m_cell} };
		r.drawFrame(1, 0, gridC);
		if (isBlocked(x, y)) r.draw(blockedC);
	}
}
