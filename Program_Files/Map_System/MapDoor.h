// =============================================================
// File: MapDoor.h
// Concrete interactive: Door (Open/Closed), with dynamic collider toggling
// =============================================================
#pragma once
#include "stdafx.h"
#include <Siv3D.hpp>
#include "MapInteractiveObject.h"


class MapDoor final : public MapInteractiveObject {
public:
	enum class State { Closed, Open };

	MapDoor(const s3d::RectF& visualRect,
			double interactRadius,
			const s3d::FilePath& closedTexPath,
			const s3d::FilePath& openTexPath,
			class MapCollider& collider,
			State initState = State::Closed);

	// 基类实现
	s3d::Vec2 uiAnchorWorld() const override { return m_visualRect.center().movedBy(0, -m_visualRect.h * 0.5); }
	bool isPlayerInRange(const MapPlayer& player) const override;  // ← 会使用“按状态切换”的半径
	bool canInteract(const MapPlayer& player) const override;      // ← 安全关闭判定
	void onInteract(MapScene& scene, MapCollider& collider) override;
	void drawWorld(const s3d::Camera2D& cam) const override;
	void drawUI(const MapCamera& mc, bool isFocused) const override;

private:
	// —— 帮助函数 ——
	s3d::RectF blockingRect() const { return m_visualRect; }
	double currentInteractRadius() const; // ← 关键：门开时半径更大
	void applyBlocking(MapCollider& collider);

	// —— 数据 —— 
	s3d::RectF  m_visualRect{};
	double      m_interactRadius = 28.0;    // 作为“门关时”的半径基准
	State       m_state = State::Closed;

	// 关门安全边距：略微扩大禁止关闭区，避免擦边误判
	double      m_safeCloseMargin = 1.0;

	s3d::Texture m_texClosed;
	s3d::Texture m_texOpen;

	uint32 m_blockId = 0; // 动态碰撞体 id
};

