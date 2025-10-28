// =============================================================
// File: MapInteractiveSystem.cpp
// =============================================================
#include "stdafx.h"
#include "MapInteractiveSystem.h"
#include "MapPlayer.h"
#include "MapCamera.h"


using namespace s3d;


// Provide a tiny shim; adapt to your MapPlayer implementation
// Expecting MapPlayer::aabb() that returns RectF (world-space)
Vec2 MapInteractiveSystem::playerCenter(const MapPlayer& player) {
	// Declare here to avoid including MapPlayer in header
	struct PlayerProbe { RectF(*getAABB)(const MapPlayer&); };
	// If your MapPlayer exposes aabb() as member, simply: return player.aabb().center();
	return player.aabb().center();
}
