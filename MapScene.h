#pragma once
#include <Siv3D.hpp>
#include "MapPlayer.h"
#include "MapCamera.h"
#include "MapCollider.h"
#include "MapBackground.h"
#include "MapEnemyType.h"
#include "MapEnemySystem.h"
#include "MapEnemyNavi.h"
#include "Enemy.h"              // For EnemyStats
#include "MapInteractiveSystem.h"

// ==========================
// MapScene (single map)
// ==========================
// EN: Orchestrates a single map: background, collider, player, camera, enemies, interactives.
//     Now supports Tiled 'spawn' (point spawns) and 'portal' (area teleporters).
// JP: 1つのマップを管理：背景・当たり判定・プレイヤ・カメラ・敵・インタラクティブ。
//     Tiled の 'spawn'（出現点）と 'portal'（転送領域）をサポート。

class MapScene {
public:
	// ----- Scene change request (portal trigger) -----
	struct SceneChangeRequest {
		s3d::String targetMapKey;   // EN: key of target map / JP: 切替先マップキー
		s3d::String targetSpawn;    // EN: target spawn name   / JP: 先のスポーン名
	};

	// ----- Portal shape -----
	struct Portal {
		enum class Type { Rect, Poly };
		Type type{ Type::Rect };
		s3d::RectF  rect;
		s3d::Polygon poly;

		s3d::String targetMapKey;
		s3d::String targetSpawn;
		s3d::String prompt; // EN: UI hint text / JP: 画面ヒント
	};

public:
	// EN: Load map (background & collisions etc.) and init systems
	// JP: マップ（背景・衝突など）を読み込み、各システムを初期化
	bool loadFromTiledJSON(const s3d::FilePath& path);

	// EN: Per-frame update (move, collide, enemies, interactives, camera, portals)
	// JP: 毎フレーム更新（移動・衝突・敵・インタラクティブ・カメラ・ポータル）
	void update(double dt);

	// EN: Draw world & UI
	// JP: ワールドとUIの描画
	void draw() const;

	// EN: Accessors
	// JP: アクセサ
	MapPlayer& player();
	const MapPlayer& player() const;

	void setDebugCollision(bool enabled);
	void setDebugNavi(bool enabled);

	// EN: Battle trigger (existing)
	// JP: 戦闘トリガー（既存）
	s3d::Optional<EnemyStats> checkBattleTrigger();
	void removeTriggeredEnemy();

	// ===== New for multi-map =====
	// EN: Place player to a named spawn; returns false if not found.
	// JP: 指定スポーン名に配置。見つからない場合は false。
	bool placePlayerAtSpawn(const s3d::String& spawnName);

	// EN: Poll & clear a pending scene-change request (from portal)
	// JP: ポータルによるシーン切替要求を取得してクリア
	s3d::Optional<SceneChangeRequest> pollSceneChangeRequest();

private:
	// ----- internal helpers -----
	void parseSpawnsAndPortalsFromJSON_(const s3d::JSON& json);
	void spawnEnemiesOnLoad_(int count, MapEnemyKind kind, double minSpawnDist);
	void updateMovementEnemiesAndCamera_(double dt);
	void detectPortalAndPrompt_();
	bool portalIntersectsPlayer_(const Portal& p) const;
	void drawVisibilityMask() const; // 你的现有屏幕遮罩

private:
	// Core modules
	MapBackground m_bg;
	MapCollider   m_collider;
	MapPlayer     m_player;
	MapCamera     m_camera;
	MapInteractiveSystem m_interactives;

	// Enemy system (existing)
	MapEnemyTypeDB m_enemyTypes;
	MapEnemySystem m_enemies;
	MapEnemySystem::EnemyID m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;
	MapEnemyNavi   m_navi;

	// Debug flags
	bool m_debugCollision{ false };
	bool m_debugNavi{ false };

	// Camera smooth helper
	s3d::Vec2 m_prevPlayerCenter{ 0, 0 };
	bool      m_prevPlayerCenterInit = false;

	// Visibility mask (existing)
	bool   m_maskEnabled = true;
	double m_maskInnerR = 120.0;
	double m_maskMaxAlpha = 1.0;
	double m_maskGamma = 2.0;
	double m_maskStep = 2.0;

	// ===== New data: spawns & portals =====
	s3d::HashTable<s3d::String, s3d::Vec2> m_spawns; // name -> position(px)
	s3d::Array<Portal> m_portals;

	// Per-frame UI prompt (screen-space)
	mutable s3d::Optional<s3d::String> m_activePrompt;

	// Pending scene change request
	s3d::Optional<SceneChangeRequest> m_pendingChange;
};
