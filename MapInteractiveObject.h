// =============================================================
// File: MapInteractiveObject.h
// Base class for any interactive object on the map (world space)
// =============================================================
#pragma once
#include "stdafx.h"
#include <Siv3D.hpp> // if your stdafx already includes Siv3D headers, you can drop this


class MapScene; // fwd
class MapPlayer; // fwd
class MapCamera; // fwd
class MapCollider; // fwd


/// Common base for all interactives
class MapInteractiveObject {
public:
	MapInteractiveObject() = default;
	virtual ~MapInteractiveObject() = default;


	// Must return the world-space anchor used for UI prompt placement (e.g., door top-center)
	virtual s3d::Vec2 uiAnchorWorld() const = 0;


	// World-space test for whether the player is in range to interact
	virtual bool isPlayerInRange(const MapPlayer& player) const = 0;


	// Some interactives (e.g., "Locked" door) may apply extra gating
	virtual bool canInteract(const MapPlayer& player) const { return isPlayerInRange(player); }


	// Perform the interaction (toggle, pickup, etc.)
	virtual void onInteract(MapScene& scene, MapCollider& collider) = 0;


	// World-space draw (called inside camera transformer)
	virtual void drawWorld(const s3d::Camera2D& cam) const = 0;


	// Screen-space UI (called outside camera transformer)
	virtual void drawUI(const MapCamera& mc, bool isFocused) const = 0;
};
