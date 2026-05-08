#pragma once
#include "stdafx.h"
#include <Siv3D.hpp>
#include <memory>
#include <vector>
#include "MapInteractiveObject.h"

//class MapInteractiveObject; // fwd
class MapPlayer; // fwd
class MapCamera; // fwd
class MapCollider; // fwd
class MapScene; // fwd


class MapInteractiveSystem {
public:
	void add(std::unique_ptr<MapInteractiveObject> obj) {
		m_objects.push_back(std::move(obj));
	}


	void clear() { m_objects.clear(); m_focusIndex = -1; }


	// Choose nearest interactive in range; dispatch F key
	void update(double /*dt*/, MapPlayer& player, MapCamera& camera, MapCollider& collider, MapScene& scene) {
		// Pick focus (nearest in range)
		int nearest = -1;
		double nearestDist2 = s3d::Math::Inf;


		for (int i = 0; i < (int)m_objects.size(); ++i) {
			if (m_objects[i]->isPlayerInRange(player)) {
				const s3d::Vec2 a = m_objects[i]->uiAnchorWorld();
				const s3d::Vec2 b = playerCenter(player);
				const double d2 = a.distanceFromSq(b);
				if (d2 < nearestDist2) { nearestDist2 = d2; nearest = i; }
			}
		}
		m_focusIndex = nearest;


		// Trigger
		if (m_focusIndex >= 0 && s3d::KeyF.down()) {
			// Additional gating (e.g., locked)
			// Note: re-check in case gating depends on player state
			if (m_objects[m_focusIndex]->canInteract(player)) {
				m_objects[m_focusIndex]->onInteract(scene, collider);
			}
		}
	}


	void drawWorld(const s3d::Camera2D& cam) const {
		for (const auto& obj : m_objects) obj->drawWorld(cam);
	}


	void drawUI(const MapCamera& mc) const {
		for (int i = 0; i < (int)m_objects.size(); ++i) {
			const bool focused = (i == m_focusIndex);
			m_objects[i]->drawUI(mc, focused);
		}
	}


private:
	std::vector<std::unique_ptr<MapInteractiveObject>> m_objects;
	int m_focusIndex = -1;


	// Helper to fetch player's center in world. Replace with your actual API if needed.
	static s3d::Vec2 playerCenter(const MapPlayer& player);
};
