#pragma once
#include <Siv3D.hpp>

// EN: Player (center-based AABB). Declarations only.
// JP: プレイヤー（中心基準AABB）。宣言のみ。

class MapPlayer {
public:
	MapPlayer();

	// EN: Initialize player core properties at once.
	// JP: コアプロパティを一括初期化。
	void init(const s3d::Vec2& center, const s3d::SizeF& size, double speed);

	// void setTextureFromFile(const s3d::FilePath& path); 
	void setTexture(const s3d::Texture& tex);
	void setSize(const s3d::SizeF& size);
	void setCenter(const s3d::Vec2& center);
	void setSpeed(double pxPerSec);

	s3d::Vec2  center() const;
	s3d::SizeF size() const;

	s3d::RectF aabb() const;

	// EN: Read input and produce desired move (no collision here).
	// JP: 入力から理想移動量を生成（衝突処理はしない）。
	s3d::Vec2 updateInput(double dt) const;

	// EN: Back-compat alias (same as updateInput).
	// JP: 互換のため（updateInput と同義）。
	s3d::Vec2 wantMove(double dt) const;

	// EN: Apply allowed movement after collision resolution.
	// JP: 衝突解決後の許可移動量を適用。
	void postCollisionApply(const s3d::Vec2& allowedDelta);

	// EN: Back-compat alias (same as postCollisionApply).
	// JP: 互換のため（postCollisionApply と同義）。
	void applyMove(const s3d::Vec2& allowedDelta);

	void draw() const;

private:
	s3d::Texture m_tex;
	s3d::Vec2    m_center;
	s3d::SizeF   m_size;
	double       m_speed;
};
