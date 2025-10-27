#include "stdafx.h"
#include "MapEnemy.h"
#include "MapEnemyNavi.h"

using namespace s3d;


void MapEnemy::init(MapEnemyKind kind, const Vec2& spawnWorld, const MapEnemyType& typeRef) {
	m_kind = kind;
	m_type = &typeRef;
	m_center = spawnWorld;
	// use texture size
	if (m_type && m_type->texture) {
		const auto sz = m_type->texture.size();
		m_size = SizeF{ static_cast<double>(sz.x), static_cast<double>(sz.y) } *0.5; // 稍小点
	}
}


void MapEnemy::updateThink(const Vec2& playerPos, double dt) {
	m_desired.set(0, 0);
	if (!m_type) return;


	const double d = m_center.distanceFrom(playerPos);


	// map status
	if (m_state == State::Idle) {
		if (d <= m_type->alertRadius) m_state = State::Chase;
	}
	else if (m_state == State::Chase) {
		if (d > m_type->alertRadius * 1.4) m_state = State::Idle; // 简单的滞后防抖
	}


	// TODO chase behaviour
	if (m_state == State::Chase) {
		if (d <= m_type->battleRadius) m_battleTriggered = true;

		// 触发重寻路：定时 / 需要 / 动态层版本变化
		m_repathCD -= dt;
		const bool needPath = (m_path.isEmpty() || m_waypoint >= m_path.size());
		if (m_navi && (m_repathCD <= 0.0 || needPath ||
			m_seenDynamicVersion != m_navi->dynamicLayerVersion())) {
			s3d::Array<s3d::Vec2> newPath;
			if (m_navi->findPath(m_center, playerPos, newPath)) {
				m_path = std::move(newPath);
				m_waypoint = 0;
			}
			m_repathCD = 0.5; // 节流
			m_seenDynamicVersion = m_navi->dynamicLayerVersion();
		}

		// 沿路径推进；无路径时退化为原地（或直线追）
		if (!m_path.isEmpty() && m_waypoint < m_path.size()) {
			const auto target = m_path[m_waypoint];
			auto toT = (target - m_center);
			const double dist = toT.length();
			const double step = m_type->speed * dt;
			if (dist <= Max(6.0, step)) { ++m_waypoint; }
			else { m_desired = toT.setLength(step); }
		}
		else {
			// 退化策略：直线追（你也可以改成站定）
			if (d > 1e-3) m_desired = (playerPos - m_center).setLength(m_type->speed * dt);
		}
	}
}


void MapEnemy::draw() const {
	const RectF r = aabb();
	if (m_type && m_type->texture) m_type->texture.resized(r.size).draw(r.pos);
	else { r.draw(Palette::Tomato); r.drawFrame(1, 0, Palette::Black); }
}
