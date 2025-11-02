/*==============================================================================

	Manage Map Scene [MapScene.cpp]

	Author : Team Re:ing >> Chen

==============================================================================*/
#include "stdafx.h"
#include "MapScene.h"
#include "MapEnemyType.h"
#include "MapInteractiveSystem.h"
#include "MapDoor.h"

using namespace s3d;

// include in MapEnemyRegister.cpp
void RegisterAllEnemyTypes(MapEnemyTypeDB& db);

bool MapScene::loadFromTiledJSON(const FilePath& path)
{
	m_debugCollision = false;
	m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;
	m_spawns.clear();
	m_portals.clear();
	m_activePrompt.reset();
	m_pendingChange.reset();
	m_prevPlayerCenterInit = false;

	// 1) Load background (tile layers)
	if (!m_bg.loadFromTiledJSON(path)) {
		return false;
	}

	// 2) Load collisions (object layer "collision")
	if (!m_collider.loadCollisionFromTiledJSON(path)) {
		Console << U"[MapScene] No collision or failed to parse collision layer.";
	}

	// 3) Player basic init (fallback if no spawn)
	m_player.setCenter(Vec2{ 256, 256 });
	m_player.setSpeed(180.0);

	// 4) Player animations (existing demo)
	{
		using St = MapPlayer::PlayerState;
		using Fa = MapPlayer::Facing4;

		const s3d::FilePath sheet = U"../Assets/MapPlayer/form_sed.png";
		const s3d::Size frame{ 32, 32 };

		std::array<MapPlayer::RowBinding, 8> rows{ {
			{ St::Idle, Fa::Up,    0, 10, 24.0, true },
			{ St::Idle, Fa::Right, 1, 10, 24.0, true },
			{ St::Idle, Fa::Left,  2, 10, 24.0, true },
			{ St::Idle, Fa::Down,  3, 10, 24.0, true },
			{ St::Move, Fa::Up,    4, 10, 24.0, true },
			{ St::Move, Fa::Right, 5, 10, 24.0, true },
			{ St::Move, Fa::Left,  6, 10, 24.0, true },
			{ St::Move, Fa::Down,  7, 10, 24.0, true },
		} };
		m_player.initAnimationsFromSheet8(sheet, frame, rows);
	}

	// 5) Camera init
	m_camera.init(Scene::Size(), m_bg.worldBounds(), 1.0);
	m_camera.setSoftDeadZone({ 100, 70 });
	m_camera.updateFollow(m_player.center()); // snap first frame

	// 6) Navigation (existing)
	{
		NaviGridConfig cfg;
		cfg.worldBounds = m_bg.worldBounds();
		cfg.cell = 32.0;
		cfg.agentRadius = 10.0;
		m_navi.buildFromCollider(m_collider, cfg);
	}

	// 7) EnemyType / EnemySystem (existing)
	RegisterAllEnemyTypes(m_enemyTypes);
	m_enemies.bindTypeDB(&m_enemyTypes);
	m_enemies.bindNavi(&m_navi);

	// 8) Test enemy (existing)
	m_enemies.clear();
	//spawnEnemiesOnLoad_(5, MapEnemyKind::Slime, 150.0);
	//{
	//	m_enemies.spawn(MapEnemyKind::Slime, Vec2{ 420, 140 });
	//	// m_enemies.spawn(MapEnemyKind::Bat, Vec2{ 420, 260 });
	//}

	// 9) Example door (existing)
	{
		const s3d::RectF doorRect{ s3d::Vec2{ 420, 260 }, s3d::SizeF{ 32, 48 } };
		auto door = std::make_unique<MapDoor>(
			doorRect, 28.0, U"", U"", m_collider, MapDoor::State::Closed
		);
		m_interactives.add(std::move(door));
	}

	// 10) Parse spawn & portal from JSON (new)
	{
		const JSON json = JSON::Load(path);
		if (json) {
			parseSpawnsAndPortalsFromJSON_(json);
		}
		else {
			Console << U"[MapScene] JSON reload failed for spawn/portal: " << path;
		}
	}

	return true;
}

void MapScene::parseSpawnsAndPortalsFromJSON_(const JSON& json)
{
	if (!json.hasElement(U"layers")) return;

	for (const auto& layer : json[U"layers"].arrayView())
	{
		const String type = layer[U"type"].getString();
		if (type != U"objectgroup") continue;

		const String lname = layer[U"name"].getString();

		// ---- spawn layer: point objects ----
		if (lname == U"spawn")
		{
			if (!layer.hasElement(U"objects")) continue;

			for (const auto& obj : layer[U"objects"].arrayView())
			{
				const String name = obj[U"name"].getOr<String>(U"");
				if (name.isEmpty()) continue;

				const double x = obj[U"x"].get<double>();
				const double y = obj[U"y"].get<double>();

				// EN: Tiled point's (x,y) is baseline of the point.
				// JP: Tiled の Point は (x,y) が基準点
				m_spawns.emplace(name, Vec2{ x, y });
			}
		}
		// ---- portal layer: rect or polygon + properties ----
		else if (lname == U"portal")
		{
			if (!layer.hasElement(U"objects")) continue;

			for (const auto& obj : layer[U"objects"].arrayView())
			{
				Portal p;

				// Properties
				if (obj.hasElement(U"properties"))
				{
					for (const auto& prop : obj[U"properties"].arrayView())
					{
						const String pn = prop[U"name"].getString();
						if (pn == U"target_map")
							p.targetMapKey = prop[U"value"].getString();
						else if (pn == U"target_spawn")
							p.targetSpawn = prop[U"value"].getString();
						else if (pn == U"prompt")
							p.prompt = prop[U"value"].getString();
					}
				}

				// Shape
				const double x = obj[U"x"].get<double>();
				const double y = obj[U"y"].get<double>();
				const double w = obj[U"width"].getOr<double>(0.0);
				const double h = obj[U"height"].getOr<double>(0.0);

				if (obj.hasElement(U"polygon"))
				{
					Array<Vec2> pts;
					for (const auto& pt : obj[U"polygon"].arrayView())
					{
						const double px = pt[U"x"].get<double>();
						const double py = pt[U"y"].get<double>();
						pts << Vec2{ x + px, y + py };
					}
					p.type = Portal::Type::Poly;
					p.poly = Polygon{ pts };
				}
				else
				{
					p.type = Portal::Type::Rect;
					p.rect = RectF{ x, y, w, h };
				}

				if (!p.targetMapKey.isEmpty() && !p.targetSpawn.isEmpty())
				{
					m_portals << std::move(p);
				}
				else
				{
					Console << U"[MapScene] Portal missing target_map/target_spawn - skipped";
				}
			}
		}
	}

	Console << U"[MapScene] spawns=" << m_spawns.size()
		<< U", portals=" << m_portals.size();
}

void MapScene::spawnEnemiesOnLoad_(int count, MapEnemyKind kind, double minSpawnDist)
{
	const RectF bounds = m_bg.worldBounds();
	const Vec2 playerSpawn = m_player.center();
	const SizeF enemySize(24, 24);
	const double minSpawnDistSq = minSpawnDist * minSpawnDist;

	int spawned = 0;
	int attempts = 0; // 무한 루프 방지

	while (spawned < count && attempts < 200)
	{
		attempts++;
		const Vec2 pos = RandomVec2(bounds);

		if (pos.distanceFromSq(playerSpawn) < minSpawnDistSq)
		{
			continue;
		}

		const RectF enemyAABB = RectF{ Arg::center = pos, enemySize };
		if (m_collider.intersectsAny(enemyAABB))
		{
			continue;
		}

		m_enemies.spawn(kind, pos);
		spawned++;
	}

	if (spawned < count)
	{
		Console << U"Warning: Could only spawn {}/{} enemies after {} attempts."_fmt(spawned, count, attempts);
	}
}

bool MapScene::placePlayerAtSpawn(const String& spawnName)
{
	const auto it = m_spawns.find(spawnName);
	if (it == m_spawns.end()) return false;

	m_player.setCenter(it->second);
	m_camera.updateFollow(m_player.center()); // snap camera
	m_prevPlayerCenterInit = false;           // reset velocity filter
	return true;
}

void MapScene::updateMovementEnemiesAndCamera_(double dt)
{
	// 1) input → desired
	const Vec2 wanted = m_player.calcDesiredDelta(dt);

	// 2) collide resolve
	const Vec2 allowed = m_collider.solveAABB(m_player.aabb(), wanted);

	// 3) apply move
	m_player.applyMove(allowed);

	// 4) animation by INPUT (not by allowed)
	m_player.updateAnimation(dt, wanted);

	// 5) enemies & interactives
	m_enemies.update(m_collider, m_player.center(), dt);
	m_interactives.update(dt, m_player, m_camera, m_collider, *this);

	// 6) camera smooth follow (your existing style)
	const Vec2 now = m_player.center();
	if (!m_prevPlayerCenterInit) {
		m_prevPlayerCenter = now; m_prevPlayerCenterInit = true;
	}
	const Vec2 vel = (now - m_prevPlayerCenter) / Max(1e-6, dt);
	m_camera.updateFollowSmooth(now, vel, dt);
	m_prevPlayerCenter = now;
}

bool MapScene::portalIntersectsPlayer_(const Portal& p) const
{
	const RectF r = m_player.aabb();
	if (p.type == Portal::Type::Rect) {
		return r.intersects(p.rect);
	}
	else {
		return p.poly.intersects(r);
	}
}

void MapScene::detectPortalAndPrompt_()
{
	m_activePrompt.reset();

	for (const auto& p : m_portals)
	{
		if (portalIntersectsPlayer_(p))
		{
			if (!p.prompt.isEmpty()) {
				m_activePrompt = p.prompt;
			}
			if (KeyF.down()) {
				m_pendingChange = SceneChangeRequest{ p.targetMapKey, p.targetSpawn };
			}
			return; // show only one prompt
		}
	}
}

void MapScene::update(double dt)
{
	updateMovementEnemiesAndCamera_(dt);
	detectPortalAndPrompt_();
}

void MapScene::draw() const
{
	// A) 背景（内部已经用 Camera2D）
	m_bg.draw(m_camera.camera());

	// B) 世界物体（统一套一次相机）
	{
		const auto t = m_camera.camera().createTransformer();
		if (m_debugNavi) m_navi.drawDebug();
		m_enemies.draw(m_camera.camera());
		m_player.draw();
		m_interactives.drawWorld(m_camera.camera());
		if (m_debugCollision) m_collider.drawDebug();
	}

	// C) 交互 UI（屏幕坐标）
	m_interactives.drawUI(m_camera);

	// D) 可见性遮罩（屏幕坐标）
	drawVisibilityMask();

	// E) 传送提示（屏幕坐标）
	if (m_activePrompt)
	{
		static const Font font{ 20, Typeface::Bold };
		const String text = U"[F] " + *m_activePrompt;
		const Vec2 pos = Vec2{ 20, Scene::Height() - 40 };
		RoundRect{ pos, font(text).region().w + 16, 28, 6 }.draw(ColorF{ 0, 0, 0, 0.55 });
		font(text).draw(pos + Vec2{ 8, 4 }, Palette::White);
	}
}

MapPlayer& MapScene::player() { return m_player; }
const MapPlayer& MapScene::player() const { return m_player; }

void MapScene::setDebugCollision(bool enabled) { m_debugCollision = enabled; }
void MapScene::setDebugNavi(bool enabled) { m_debugNavi = enabled; }

// ----- battle trigger (existing) -----
s3d::Optional<EnemyStats> MapScene::checkBattleTrigger()
{
	if (auto info = m_enemies.pollBattleTrigger())
	{
		const MapEnemyKind kind = info->first;
		m_triggeredEnemyID = info->second;

		const MapEnemyType* type = m_enemyTypes.get(kind);
		if (!type) {
			Console << U"Error: Battle triggered but kind {} not in DB!"_fmt((int)kind);
			m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;
			return none;
		}
		Console << U"[Battle] trigger with " << type->name << U" (ID: " << m_triggeredEnemyID << U")";

		EnemyStats stats;
		stats.name = type->name;
		stats.maxHP = type->maxHP;
		stats.currentHP = type->maxHP;
		stats.attack = type->attack;
		stats.defense = type->defense;
		stats.speed = static_cast<int>(type->speed / 15.0);
		stats.expYield = type->expYield;
		return stats;
	}
	m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;
	return none;
}

void MapScene::removeTriggeredEnemy()
{
	if (m_triggeredEnemyID != MapEnemySystem::InvalidEnemyID) {
		Console << U"[MapScene] Request removal of enemy ID: " << m_triggeredEnemyID;
		m_enemies.removeEnemyByID(m_triggeredEnemyID);
		m_triggeredEnemyID = MapEnemySystem::InvalidEnemyID;
	}
	else {
		Console << U"Warning: removeTriggeredEnemy called with invalid ID.";
	}
}

s3d::Optional<MapScene::SceneChangeRequest> MapScene::pollSceneChangeRequest()
{
	if (!m_pendingChange) return none;
	auto out = m_pendingChange;
	m_pendingChange.reset();
	return out;
}

// ----- existing visibility mask -----
void MapScene::drawVisibilityMask() const
{
	if (!m_maskEnabled) return;

	const Rect  screen = Scene::Rect();
	const Vec2  c = screen.center();

	const double d0 = c.distanceFrom(screen.tl());
	const double d1 = c.distanceFrom(screen.tr());
	const double d2 = c.distanceFrom(screen.bl());
	const double d3 = c.distanceFrom(screen.br());
	const double outerR = Max({ d0, d1, d2, d3 });

	const double innerR = Clamp(m_maskInnerR, 0.0, outerR - 1.0);
	const double width = Max(outerR - innerR, 1.0);
	const double step = Clamp(m_maskStep, 1.0, 32.0);

	for (double r = innerR; r < outerR; r += step)
	{
		const double t = (r - innerR) / width;
		const double a = Pow(t, m_maskGamma) * m_maskMaxAlpha;
		Circle(c, r + step * 0.5).drawFrame(step + 0.5, ColorF(0, 0, 0, a));
	}
}
