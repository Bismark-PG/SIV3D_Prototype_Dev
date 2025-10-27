#include "stdafx.h"
#include "MapPlayer.h"
using namespace s3d;

MapPlayer::MapPlayer()
	: m_center{ 64, 64 }
	, m_size{ 20, 28 }
	, m_speed{ 180.0 } {
}

void MapPlayer::init(const Vec2& center, const SizeF& size, double speed) {
	m_center = center;
	m_size = size;
	m_speed = speed;
}

// [수정됨] setTextureFromFile 함수 제거 (W100 경고 유발)
//void MapPlayer::setTextureFromFile(const s3d::FilePath& path)
//{
//	if (!s3d::FileSystem::Exists(path)) {
//		s3d::Console << U"[MapPlayer] Texture not found: " << path;
//		m_tex.release();
//		return;
//	}
//	s3d::Texture tex{ path, s3d::TextureDesc::Mipped };
//	if (!tex) {
//		s3d::Console << U"[MapPlayer] Failed to load texture: " << path;
//		m_tex.release();
//		return;
//	}
//	m_tex = std::move(tex);
//}

void MapPlayer::setTexture(const Texture& tex) { m_tex = tex; }
void MapPlayer::setSize(const SizeF& size) { m_size = size; }
void MapPlayer::setCenter(const Vec2& center) { m_center = center; }
void MapPlayer::setSpeed(double pxPerSec) { m_speed = pxPerSec; }

Vec2  MapPlayer::center() const { return m_center; }
SizeF MapPlayer::size()   const { return m_size; }

RectF MapPlayer::aabb() const {
	return RectF{ m_center - m_size * 0.5, m_size };
}

Vec2 MapPlayer::updateInput(double dt) const {
	Vec2 dir{ 0, 0 };
	if (KeyA.pressed()) dir.x -= 1.0;
	if (KeyD.pressed()) dir.x += 1.0;
	if (KeyW.pressed()) dir.y -= 1.0;
	if (KeyS.pressed()) dir.y += 1.0;

	if (dir.lengthSq() > 0.0) {
		dir = dir.setLength(m_speed * dt);
	}
	return dir;
}

Vec2 MapPlayer::wantMove(double dt) const {
	return updateInput(dt); // alias
}

void MapPlayer::postCollisionApply(const Vec2& allowedDelta) {
	m_center += allowedDelta;
}

void MapPlayer::applyMove(const Vec2& allowedDelta) {
	postCollisionApply(allowedDelta); // alias
}

void MapPlayer::draw() const {
	const RectF r = aabb();
	if (m_tex) {
		m_tex.resized(r.size).draw(r.pos);
	}
	else {
		r.draw(Palette::Deepskyblue);
		r.drawFrame(1, 0, Palette::Black);
	}
}
