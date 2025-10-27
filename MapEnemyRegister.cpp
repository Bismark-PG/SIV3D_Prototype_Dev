#include "stdafx.h"
#include "MapEnemyType.h"

void RegisterAllEnemyTypes(MapEnemyTypeDB& db) {
	db.clear();


	// Slime —— 示例1
	{
		MapEnemyType t;
		t.name = U"Slime";
		// t.texture = LoadOrDummy(U"Assets/textures/slime.png");
		t.texture = TextureAsset(U"Map_Enemy_Slime"); // Use TextureAsset
		t.speed = 70.0;
		t.alertRadius = 999.0;
		t.battleRadius = 10.0;

		// make battle stats
		t.maxHP = 50;
		t.attack = 5;
		t.defense = 2;
		t.expYield = 15.0; // Manage stats in Game_Manager.cpp

		db.set(MapEnemyKind::Slime, std::move(t));
	}

	// TODO: need more enemy types...??
}
