#include "stdafx.h"
#include "MapEnemyType.h"

void RegisterAllEnemyTypes(MapEnemyTypeDB& db) {
	db.clear();


	// Slime —— 示例1
	{
		MapEnemyType t;
		t.name = U"Slime";
		t.speed = 70.0;
		t.alertRadius = 180.0;
		t.battleRadius = 40.0;

		const s3d::String sheet = U"../Assets/MapPlayer/char_move/kunekune.png"; // TextureAsset 名（启动时注册）
		const s3d::Size   frame{ 32, 32 };

		auto setRow = [&](EnemyState st, Facing4 f, int row, int count, double fps, bool loop) {
			auto& c = t.anim.clips[(int)st][(int)f];
			c.assetName = sheet;
			c.frame = frame;
			c.count = count;
			c.fps = fps;
			c.loop = loop;
			c.row = row;   // >=0：sheet 第几行
			c.colStart = 0;
			};

		// Idle（0..3 行）
		setRow(EnemyState::Idle, Facing4::Down,  0, 9, 8.0, true);
		setRow(EnemyState::Idle, Facing4::Right, 1, 9, 8.0, true);
		setRow(EnemyState::Idle, Facing4::Left,  2, 9, 8.0, true);
		setRow(EnemyState::Idle, Facing4::Up,    3, 9, 8.0, true);

		// Chase/Move（4..7 行）
		setRow(EnemyState::Chase, Facing4::Down,  4, 9, 8.0, true);
		setRow(EnemyState::Chase, Facing4::Right, 5, 9, 8.0, true);
		setRow(EnemyState::Chase, Facing4::Left,  6, 9, 8.0, true);
		setRow(EnemyState::Chase, Facing4::Up,    7, 9, 8.0, true);

		db.set(MapEnemyKind::Slime, std::move(t));
	}

	// TODO: need more enemy types...??
}
