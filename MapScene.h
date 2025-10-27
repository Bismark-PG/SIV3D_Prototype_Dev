#pragma once
#include <Siv3D.hpp>
#include "MapPlayer.h"
#include "MapCamera.h"
#include "MapCollider.h"
#include "MapBackground.h"
#include "MapEnemyType.h"
#include "MapEnemySystem.h"
#include "MapEnemyNavi.h"
#include "Enemy.h" // For EnemyStats

// EN: Logic orchestrator: updates input/collision/camera and invokes background draw.
// JP: ロジックのハブ：入力/衝突/カメラを更新し、背景描画を呼ぶ。

class MapScene {
public:
	// EN: Load map (background & collisions) and init player/camera.
	// JP: マップ（背景と衝突）を読み込み、プレイヤー/カメラを初期化。
	bool loadFromTiledJSON(const s3d::FilePath& path);

	void update(double dt);
	void draw() const;

	MapPlayer& player();
	const MapPlayer& player() const;

	void setDebugCollision(bool enabled);
	void setDebugNavi(bool enabled);

	// [Plus] Enemy System Accessors
	Optional<EnemyStats> checkBattleTrigger();

	// [Plus] Destroy triggered enemy
	void removeTriggeredEnemy();

private:
	MapBackground m_bg;
	MapCollider   m_collider;
	MapPlayer     m_player;
	MapCamera     m_camera;

	bool          m_debugCollision{ false };

	MapEnemyTypeDB m_enemyTypes; // NEW: enemy type db
	MapEnemySystem m_enemies; // NEW: enemy system
	bool m_debugEnemy = true; // optional: debug trigger

	s3d::Vec2 m_prevPlayerCenter{ 0, 0 };
	bool      m_prevPlayerCenterInit = false;

	//Navigate
	MapEnemyNavi m_navi;
	bool m_debugNavi = false;

	//SceneMask
	bool   m_maskEnabled = true;   // 是否启用遮罩
	double m_maskInnerR = 120.0;  // 可见半径（屏幕像素，越大越亮）
	double m_maskMaxAlpha = 1;   // 边缘最大不透明度（0~1）
	double m_maskGamma = 2;    // 渐变曲线（>1 边缘更重，<1 更线性）
	double m_maskStep = 2.0;    // 环形“描边”的厚度/步长（像素，越小越平滑）

	// [Edit] save triggered enemy ID
	MapEnemySystem::EnemyID m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;

	void drawVisibilityMask() const; // 屏幕空间遮罩
};
