#include "stdafx.h"
#include "MapEnemySystem.h"
#include "MapCollider.h"
using namespace s3d;


MapEnemy& MapEnemySystem::spawn(MapEnemyKind kind, const Vec2& spawnWorld) {
	assert(m_db && "MapEnemySystem: TypeDB not bound");
	const MapEnemyType* t = m_db ? m_db->get(kind) : nullptr;
	m_list << MapEnemy{};
	auto& e = m_list.back();
	// [Edit] Memory safety check
	if (t) e.init(kind, spawnWorld, *t);
	else {
		MapEnemyType defaultType;
		e.init(kind, spawnWorld, defaultType);
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"Warning: Spawning enemy kind {} with default type!"_fmt((int)kind);
#endif
	}

	//when spawn, bind navi
	if (m_navi) e.bindNavi(m_navi);

	return e;
}


void MapEnemySystem::update(const MapCollider& collider, const Vec2& playerPos, double dt) {
	for (auto& e : m_list) {
		// [plus] Skip update if in battle
		// if (e.isInBattle()) continue; 
		e.updateThink(playerPos, dt);
		const Vec2 desired = e.desiredDelta();
		const Vec2 allowed = collider.solveAABB(e.aabb(), desired);
		e.postCollisionApply(allowed);
		e.updateAnimation(dt, allowed);
	}
}


void MapEnemySystem::draw(const Camera2D& cam) const {
	//const auto _ = cam.createTransformer();
	for (const auto& e : m_list) e.draw();
}

// [edit] Return enemy kind along with ID
Optional<std::pair<MapEnemyKind, MapEnemySystem::EnemyID>> MapEnemySystem::pollBattleTrigger() {
	for (size_t i = 0; i < m_list.size(); ++i) {
		if (m_list[i].isBattleTriggered()) {
			m_list[i].clearBattleTrigger();
			// [edit] return kind and index as ID
			return std::make_pair(m_list[i].kind(), i);
		}
	}
	return none;
}

// [Plus] Remove enemy by ID
void MapEnemySystem::removeEnemyByID(EnemyID id) {
	if (id < m_list.size()) {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"[MapEnemySystem] Removing enemy at index: " << id;
#endif
		m_list.remove_at(id);
	}
	else {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"Warning: Tried to remove enemy with invalid ID: " << id;
#endif
	}
}
