#pragma once
#include <Siv3D.hpp>
#include "MapEnemyType.h"
#include "MapEnemy.h"


class MapCollider; // fwd
class MapEnemyNavi; // fwd

class MapEnemySystem {
public:
	// [Plus] Enemy ID For battle identification
	using EnemyID = size_t;
	static constexpr EnemyID InvalidEnemyID = static_cast<EnemyID>(-1);

	void bindTypeDB(const MapEnemyTypeDB* db) { m_db = db; }


	MapEnemy& spawn(MapEnemyKind kind, const s3d::Vec2& spawnWorld);


	void clear() { m_list.clear(); }


	void update(const MapCollider& collider, const s3d::Vec2& playerPos, double dt);


	void draw(const s3d::Camera2D& cam) const;

	//navi
	void bindNavi(const MapEnemyNavi* navi) { m_navi = navi; for (auto& e : m_list) e.bindNavi(navi); }

	// [Edit] edit return type to include EnemyID
	Optional<std::pair<MapEnemyKind, EnemyID>> pollBattleTrigger();

	// [Plus] Destroy enemy by ID
	void removeEnemyByID(EnemyID id);


private:
	const MapEnemyTypeDB* m_db{ nullptr };
	s3d::Array<MapEnemy> m_list;

	//navi
	const MapEnemyNavi* m_navi = nullptr;
};
