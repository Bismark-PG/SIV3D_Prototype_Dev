#include "stdafx.h"
#include "MapEnemy.h"
#include "MapEnemyNavi.h"

using namespace s3d;
// 只在 Debug 打
#ifndef NDEBUG
#define DBG_LOG(...) (Console.writeln(Format(__VA_ARGS__)))
#else
#define DBG_LOG(...) ((void)0)
#endif

static bool ValidateClipAndLog(const EnemyAnimClipSpec& in, const Texture& atlas) {
	if (!atlas) { DBG_LOG(U"[EnemyAnim] atlas not loaded: {}", in.assetName); return false; }
	if (in.frame.x <= 0 || in.frame.y <= 0 || in.count <= 0) {
		DBG_LOG(U"[EnemyAnim] bad frame/count for {}", in.assetName); return false;
	}
	const int atlasCols = atlas.width() / in.frame.x;
	const int atlasRows = atlas.height() / in.frame.y;

	bool ok = true;
	if (in.row >= 0) {
		if (in.row >= atlasRows) {
			DBG_LOG(U"[EnemyAnim] row {} out of rows {} ({}).",
				   in.row, atlasRows, in.assetName); ok = false;
		}
	}
	else {
		if (atlasRows < 1) { DBG_LOG(U"[EnemyAnim] strip rows<1 for {}", in.assetName); ok = false; }
	}
	if (in.colStart < 0 || (in.colStart + in.count) > atlasCols) {
		DBG_LOG(U"[EnemyAnim] col range [{}..{}) exceeds cols {} (asset={}).",
				in.colStart, in.colStart + in.count, atlasCols, in.assetName);
		ok = false;
	}
	return ok;
}

static Texture loadFromAssetOrPath(const String& key) {
	if (key.isEmpty()) return Texture();
	if (TextureAsset::IsRegistered(key)) return TextureAsset(key);
	if (FileSystem::Exists(key))         return Texture{ key, TextureDesc::Mipped };
	return Texture();
}

void MapEnemy::initAnimationsFromType(const EnemyAnimSpec& spec) {
	m_anyAnim = false;
	for (int s = 0; s < 2; ++s) {
		for (int f = 0; f < 4; ++f) {
			const auto& in = spec.clips[s][f];
			if (in.frame.isZero() || in.count <= 0 || in.assetName.isEmpty()) continue;

			AnimClipRT out;
			out.atlas = loadFromAssetOrPath(in.assetName);
			out.frame = in.frame;
			out.count = in.count;
			out.fps = in.fps;
			out.loop = in.loop;
			out.start = Max(0, in.colStart);
			out.row = in.row;

			if (!ValidateClipAndLog(in, out.atlas)) {
				continue; // 不合法就跳过，这样 draw 会走兜底
			}

			DBG_LOG(U"[EnemyAnim] ok: asset={} size={}x{} frame={}x{} row={} start={} count={}",
					in.assetName, out.atlas.width(), out.atlas.height(),
					out.frame.x, out.frame.y, out.row, out.start, out.count);

			m_clips[s][f] = out;
			m_anyAnim = true;
		}
	}
	resetAnimator();
}



void MapEnemy::init(MapEnemyKind kind, const Vec2& spawnWorld, const MapEnemyType& typeRef) {
	m_kind = kind;
	m_type = &typeRef;
	m_center = spawnWorld;
	
	initAnimationsFromType(m_type->anim);
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


Facing4 MapEnemy::determineFacingFromVector(const Vec2& v, Facing4 fallback) {
	if (v.lengthSq() < 1e-4) return fallback;
	if (Abs(v.x) >= Abs(v.y)) {
		return (v.x >= 0) ? Facing4::Right : Facing4::Left;
	}
	else {
		return (v.y >= 0) ? Facing4::Down : Facing4::Up;
	}
}

void MapEnemy::switchIfNeeded(EnemyState newState, Facing4 newFacing) {
	if (newState != m_animState || newFacing != m_facing) {
		m_animState = newState;
		m_facing = newFacing;
		resetAnimator();
	}
}

void MapEnemy::resetAnimator() {
	m_timeAcc = 0.0;
	m_frameIndex = 0;
}

void MapEnemy::postCollisionApply(const Vec2& allowedDelta) {
	m_center += allowedDelta;
	// 动画推进放在 updateAnimation()，由 System 调用
}

void MapEnemy::updateAnimation(double dt, const Vec2& allowedDelta) {
	if (!m_anyAnim) return;

	const bool moving = (allowedDelta.lengthSq() > 1e-6);

	// 用 AI 状态驱动动画大类（也可单独判断 moving→Idle/Chase）
	EnemyState desiredState = (m_state == State::Chase) ? EnemyState::Chase : EnemyState::Idle;

	// 用“实际位移”决定朝向，静止则沿用上次方向
	Facing4 face = m_lastMoveFacing;
	if (moving) {
		face = determineFacingFromVector(allowedDelta, m_lastMoveFacing);
		m_lastMoveFacing = face;
	}
	switchIfNeeded(desiredState, face);

	const AnimClipRT& c = m_clips[(int)m_animState][(int)m_facing];
	if (!c.valid()) return;

	m_timeAcc += dt;
	const double spf = (c.fps > 0 ? (1.0 / c.fps) : 1.0);
	while (m_timeAcc >= spf) {
		m_timeAcc -= spf;
		if (m_frameIndex + 1 < c.count) ++m_frameIndex;
		else m_frameIndex = c.loop ? 0 : (c.count - 1);
	}
}

void MapEnemy::draw() const {
	if (m_anyAnim) {
		const AnimClipRT& c = m_clips[(int)m_animState][(int)m_facing];
		if (c.valid()) {
			const int fx = c.start + m_frameIndex;
			const int fy = (c.row >= 0) ? c.row : 0;  // row>=0: sheet 行；<0: strip 单行
			const Rect src{ fx * c.frame.x, fy * c.frame.y, c.frame };
			c.atlas(src).drawAt(m_center);
			return;
		}
	}
	// 兜底：没有动画就画占位
	const RectF r = aabb();
	r.draw(Palette::Orange);
	r.drawFrame(1, 0, Palette::Black);
}
