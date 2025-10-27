/*==============================================================================

	Manage Map Scene [MapScene.h]

	Author : Team Re:ing >> Chen

	Note :

==============================================================================*/
#include "stdafx.h"
#include "MapScene.h"
#include "MapEnemyType.h"
using namespace s3d;

// include in MapEnemyRegister.cpp
void RegisterAllEnemyTypes(MapEnemyTypeDB& db);

bool MapScene::loadFromTiledJSON(const FilePath& path) {
	m_debugCollision = false;
	m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;

	// 1) Load background (tile layers)
	if (!m_bg.loadFromTiledJSON(path)) {
		return false;
	}

	// 2) Load collisions from object layer "collision"
	if (!m_collider.loadCollisionFromTiledJSON(path)) {
		// Allow empty collision layer; not an error
		Console << U"[MapScene] No collision or failed to parse collision layer.";
	}

	// 3) Init player & camera via their scripts
	m_player.init(Vec2{ 64, 64 }, SizeF{ 20, 28 }, 180.0);

	// [Edit] Edit name >> sample_player
	m_player.setTexture(TextureAsset(U"Sample_Player"));

	m_camera.init(Scene::Size(), m_bg.worldBounds(), 1.0);
	m_camera.setSoftDeadZone({ 100, 70 });

	//bind Navigation grid
	{
		NaviGridConfig cfg;
		cfg.worldBounds = m_bg.worldBounds(); // from world bounds
		cfg.cell = 32.0;                      // can be adjusted
		cfg.agentRadius = 10.0;               // >= half radius of enemy
		m_navi.buildFromCollider(m_collider, cfg);
	}

	// [ENEMY] register enemytype & binding to db
	RegisterAllEnemyTypes(m_enemyTypes);
	m_enemies.bindTypeDB(&m_enemyTypes);
	m_enemies.bindNavi(&m_navi); //bind navi

	// [ENEMY] spawn some test enemies(for test)
	{
		m_enemies.spawn(MapEnemyKind::Slime, Vec2{ 420, 140 });
		//m_enemies.spawn(MapEnemyKind::Bat, Vec2{ 420, 260 });
	}

	return true;
}

void MapScene::update(double dt)
{
	const Vec2 desired = m_player.updateInput(dt);
	const Vec2 allowed = m_collider.solveAABB(m_player.aabb(), desired);
	m_player.postCollisionApply(allowed);

	m_enemies.update(m_collider, m_player.center(), dt);

	const s3d::Vec2 now = m_player.center();
	if (!m_prevPlayerCenterInit) {
		m_prevPlayerCenter = now;
		m_prevPlayerCenterInit = true;
	}

	const s3d::Vec2 vel = (now - m_prevPlayerCenter) / s3d::Max(1e-6, dt);
	m_camera.updateFollowSmooth(now, vel, dt);
	m_prevPlayerCenter = now;
}

// [Edit] Data Movement : MapEnemyKind -> MapEnemyTypeDB -> EnemyStats
Optional<EnemyStats> MapScene::checkBattleTrigger()
{
	// m_enemies.pollBattleTrigger()is returned MapEnemyKind & EnemyID
	Optional<std::pair<MapEnemyKind, MapEnemySystem::EnemyID>> triggerInfo = m_enemies.pollBattleTrigger();

	if (triggerInfo)
	{
		// Get Enemy Type from DB
		MapEnemyKind triggeredKind = triggerInfo->first;
		m_triggeredEnemyID = triggerInfo->second; // [Plus] Save triggered enemy ID

		// Get MapEnemyType from DB
		const MapEnemyType* type = m_enemyTypes.get(triggeredKind);
		if (not type) {
			Console << U"Error: Battle triggered but MapEnemyKind {} not in DB!"_fmt((int)triggeredKind);
			m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID; // Reset ID on error
			return none;
		}

		Console << U"[Battle] trigger with " << type->name << U" (ID: " << m_triggeredEnemyID << U")";

		// Create EnemyStats from MapEnemyType
		EnemyStats stats;
		stats.name = type->name;
		stats.maxHP = type->maxHP;
		stats.currentHP = type->maxHP; // Start Battle Whit MaxHP
		stats.attack = type->attack;
		stats.defense = type->defense;
		stats.speed = static_cast<int>(type->speed / 15.0); // [Debug] Need Speed Balance / In now, Use speed/15
		stats.expYield = type->expYield;

		// [Edit] Return EnemyStats
		return stats;
	}

	m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID; // NO trigger, reset ID
	return none;
}

void MapScene::removeTriggeredEnemy() {
	if (m_triggeredEnemyID != MapEnemySystem::InvalidEnemyID) {
		Console << U"[MapScene] Requesting removal of enemy ID: " << m_triggeredEnemyID;
		m_enemies.removeEnemyByID(m_triggeredEnemyID);
		m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID; // Deset ID after removal
	}
	else {
		Console << U"Warning: removeTriggeredEnemy called with invalid ID.";
	}
}
void MapScene::draw() const {
	// A) 背景：它自己内部接受相机对象
	m_bg.draw(m_camera.camera());

	// B) 场景里统一套一次相机，然后画“用世界坐标绘制”的对象
	{
		const auto t = m_camera.camera().createTransformer();
		if (m_debugNavi) m_navi.drawDebug(); // 可视化网格与阻挡
		m_enemies.draw(m_camera.camera()); // 这里的 cam 仅用于将来做裁剪，可不使用
		m_player.draw();
		if (m_debugCollision) m_collider.drawDebug();
	}

	drawVisibilityMask(); // 这个若是屏幕坐标 UI，放在 transformer 外
}

MapPlayer& MapScene::player() { return m_player; }

const MapPlayer& MapScene::player() const { return m_player; }

void MapScene::setDebugCollision(bool enabled)
{
	m_debugCollision = enabled;
}

void MapScene::setDebugNavi(bool enabled)
{
	m_debugNavi = enabled;
}

void MapScene::drawVisibilityMask() const
{
	if (!m_maskEnabled) return;

	using namespace s3d;

	const Rect  screen = Scene::Rect();
	const Vec2  c = screen.center();

	// 计算从屏幕中心到四个角的最远距离，作为外半径，确保边角完全被覆盖
	const double d0 = c.distanceFrom(screen.tl());
	const double d1 = c.distanceFrom(screen.tr());
	const double d2 = c.distanceFrom(screen.bl());
	const double d3 = c.distanceFrom(screen.br());
	const double outerR = Max({ d0, d1, d2, d3 });

	const double innerR = Clamp(m_maskInnerR, 0.0, outerR - 1.0);
	const double width = Max(outerR - innerR, 1.0);
	const double step = Clamp(m_maskStep, 1.0, 32.0);

	// 先整体铺一层“非常淡”的黑，避免 rings 之间的缝隙（可选）
	// Rect(screen).draw(ColorF(0, 0, 0, 0.02));

	// 从内半径到外半径，逐步加深不透明度（近似径向渐变）
	for (double r = innerR; r < outerR; r += step)
	{
		const double t = (r - innerR) / width;         // 0..1
		const double a = Pow(t, m_maskGamma) * m_maskMaxAlpha;
		Circle(c, r + step * 0.5).drawFrame(step + 0.5, ColorF(0, 0, 0, a));
	}
}
