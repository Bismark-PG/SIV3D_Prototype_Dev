#pragma once
#include <Siv3D.hpp>


enum class MapEnemyKind : uint16 {
	Slime = 0,
	Bat,
	// TODO
};


struct MapEnemyType {
	s3d::String name; // name(for UI)
	s3d::Texture texture; // temp texture, after change to Sprite Anim
	double speed = 80.0; // chase speed（px/s）
	double alertRadius = 160.0; //alert radius to chase player
	double battleRadius = 30.0; // battle check radius（enter battle scene）

	// [Plus] Enemy Stats For Battle Scene
	int maxHP = 50;
	int attack = 5;
	int defense = 2;
	double expYield = 10.0;
};


class MapEnemyTypeDB {
public:
	void clear() { m_table.clear(); }
	void set(MapEnemyKind k, MapEnemyType t) { m_table[k] = std::move(t); }
	const MapEnemyType* get(MapEnemyKind k) const {
		if (auto it = m_table.find(k); it != m_table.end()) return &it->second; return nullptr;
	}
private:
	s3d::HashTable<MapEnemyKind, MapEnemyType> m_table;
};
