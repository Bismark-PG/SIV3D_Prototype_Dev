#pragma once
#include <Siv3D.hpp>

// EN: Minimal camera. Declarations only.
// JP: 最小限カメラ。宣言のみ。

class MapCamera {
public:
	// EN: Initialize viewport + world bounds + scale at once.
	// JP: ビューポート・ワールド境界・スケールを一括初期化。
	void init(const s3d::Size& screenSize, const s3d::RectF& worldBounds, double scale = 1.0);

	void setViewport(const s3d::Size& screenSize, double scale = 1.0);
	void setWorldBounds(const s3d::RectF& bounds);

	// EN: Update follow target and clamp.
	// JP: 追従＆クランプ更新。
	void updateFollow(const s3d::Vec2& targetCenter);

	// soft dead zone
	void setSoftDeadZone(const s3d::Vec2& halfSizeScreenPx);

	// smooth track
	void updateFollowSmooth(const s3d::Vec2& targetCenter,
							const s3d::Vec2& targetVelocity,
							double dt);

	// EN: Back-compat alias.
	// JP: 後方互換のため。
	void follow(const s3d::Vec2& targetCenter);



	const s3d::Camera2D& camera() const;

	s3d::Vec2 worldToScreen(const s3d::Vec2& world) const;
	s3d::Vec2 screenToWorld(const s3d::Vec2& screen) const;
	s3d::Vec2 worldToCamera(const s3d::Vec2& world) const;
	s3d::Vec2 screenCenter() const;

private:
	s3d::Camera2D m_cam;
	s3d::RectF    m_world{ 0,0,1,1 };
	s3d::Vec2     m_viewHalf{ 320, 180 };
	s3d::Size     m_screenSize{ 640, 360 };
	double        m_scale{ 1.0 };
	s3d::Vec2     m_center{ 0,0 };

	// soft dead zone
	s3d::Vec2  m_softDeadHalfScreenPx{ 0, 0 };

	//smooth movement
	double     m_smoothVX{ 0.0 };
	double     m_smoothVY{ 0.0 };

	void       applyClampAndSnap();


};
