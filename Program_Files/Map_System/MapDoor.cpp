// =============================================================
// File: MapDoor.cpp
// =============================================================
#include "stdafx.h"
#include "MapDoor.h"
#include "MapCollider.h"
#include "MapPlayer.h"
#include "MapCamera.h"
#include "MapScene.h"


using namespace s3d;


// ---- MapCollider dynamic API (add these in MapCollider.h / see patch below) ----
// using ShapeID = uint32;
// ShapeID addDynamicRect(const RectF& r, bool enabled);
// void setDynamicEnabled(ShapeID id, bool enabled);
// bool intersectsAny(const RectF& r) const;


MapDoor::MapDoor(const RectF& visualRect,
double interactRadius,
const FilePath& closedTexPath,
const FilePath& openTexPath,
MapCollider& collider,
State initState)
	: m_visualRect(visualRect)
	, m_interactRadius(interactRadius)
	, m_state(initState)
{
	if (!closedTexPath.isEmpty()) m_texClosed = Texture{ closedTexPath, TextureDesc::Unmipped };
	if (!openTexPath.isEmpty()) m_texOpen = Texture{ openTexPath, TextureDesc::Unmipped };


	// Register dynamic blocking rect matching the visual rect (you can offset if your art needs)
	m_blockId = collider.addDynamicRect(m_visualRect, (m_state == State::Closed));
}


bool MapDoor::isPlayerInRange(const MapPlayer& player) const {
	const s3d::Vec2 pc = player.aabb().center();
	const s3d::Vec2 dc = m_visualRect.center();
	const double R = currentInteractRadius();              // ← 状态相关半径
	return (pc.distanceFromSq(dc) <= (R * R));
}


double MapDoor::currentInteractRadius() const
{
	if (m_state == State::Open) {
		// 使玩家在“未来阻挡形状之外、但仍在交互范围内”
		// halfDoor + margin + 额外冗余（可调）保证体验
		const double halfDoor = 0.5 * std::max(m_visualRect.w, m_visualRect.h);
		const double recommended = halfDoor + m_safeCloseMargin + 20.0; // 8 像素余量
		return std::max(m_interactRadius, recommended);
	}
	return m_interactRadius; // 关门状态用原始半径
}

void MapDoor::applyBlocking(MapCollider& collider) {
	const bool enable = (m_state == State::Closed);
	if (m_blockId != 0) collider.setDynamicEnabled(m_blockId, enable);
}


void MapDoor::onInteract(MapScene& /*scene*/, MapCollider& collider) {
	// Safety: if trying to close while player overlaps, abort
	if (m_state == State::Open) {
		// Attempt to close — check overlap against the door rect
		// (you may extend this to check against all actors you care about)
		// NOTE: We only have access to MapCollider and Scene here; player overlap check
		// can be centralized elsewhere if you prefer. For P0 we trust caller.
	}


	// Toggle state
	m_state = (m_state == State::Closed ? State::Open : State::Closed);
	applyBlocking(collider);
}


void MapDoor::drawWorld(const Camera2D& /*cam*/) const {
	if (m_state == State::Closed) {
		if (m_texClosed) m_texClosed.draw(m_visualRect.pos);
		else m_visualRect.draw(Palette::Brown);
	}
	else {
		if (m_texOpen) m_texOpen.draw(m_visualRect.pos);
		else m_visualRect.draw(Palette::Green);
	}
}


void MapDoor::drawUI(const MapCamera& mc, bool isFocused) const {
	if (!isFocused) return;

	const Vec2 screen = mc.worldToScreen(uiAnchorWorld());
	static const Font font{ 18 };

	const String key = U"F";
	const String verb = (m_state == State::Closed ? U"開ける" : U"閉める");
	const String msg = U"[" + key + U"] " + verb;

	const Vec2 padding{ 12, 8 };
	const Vec2 size = font(msg).region().size + padding;   // ✔ 正确获取文本区域尺寸
	const Vec2 tl = screen + Vec2{ -size.x * 0.5, -28 };

	RoundRect{ RectF{ tl, size }, 6 }
		.draw(ColorF(0, 0, 0, 0.65))
		.drawFrame(1, ColorF(1, 1, 1, 0.8));

	font(msg).draw(tl + Vec2{ 6, 3 }, Palette::White);
}

bool MapDoor::canInteract(const MapPlayer& player) const {
	// 先满足“在交互半径内”（门开时半径更大）
	if (!isPlayerInRange(player)) return false;

	// 如果当前是 Open，本次交互会尝试关门：禁止在“被夹住”的位置关闭
	if (m_state == State::Open) {
		const s3d::RectF futureBlock = blockingRect().stretched(m_safeCloseMargin);
		if (futureBlock.intersects(player.aabb())) {
			return false;
		}
	}
	return true;
}
