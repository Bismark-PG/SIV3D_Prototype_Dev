#include "MapPlayer.h"
#include <cmath> // std::round

using namespace s3d;

// ============ Basic ============
void MapPlayer::setCenter(const Vec2& c) { m_center = c; }
void MapPlayer::setSize(const SizeF& s) { m_size = s; }
void MapPlayer::setSpeed(double s) { m_speed = s; }

const Vec2& MapPlayer::center() const { return m_center; }

RectF MapPlayer::aabb() const
{
	// EN: center-based AABB / JP: 中心基準AABB
	const Vec2 half = m_size * 0.5;
	return RectF{ m_center - half, m_size };
}

double MapPlayer::speed() const { return m_speed; }

// ============ Movement intent ============
Vec2 MapPlayer::calcDesiredDelta(double dt) const
{
	Vec2 dir{ 0, 0 };
	// EN: Arrow + WASD / JP: 矢印＋WASD
	if (KeyLeft.pressed() || KeyA.pressed()) dir.x -= 1.0;
	if (KeyRight.pressed() || KeyD.pressed()) dir.x += 1.0;
	if (KeyUp.pressed() || KeyW.pressed()) dir.y -= 1.0;
	if (KeyDown.pressed() || KeyS.pressed()) dir.y += 1.0;

	if (dir.isZero()) return Vec2{ 0, 0 };

	// EN: Normalize to keep diagonal speed same / JP: 斜め速度を正規化
	dir = dir.setLength(m_speed * dt);
	return dir;
}

void MapPlayer::applyMove(const Vec2& allowedDelta)
{
	m_center += allowedDelta;
	// EN: when moving, update last facing / JP: 実移動があれば最後の向きを更新
	if (!allowedDelta.isZero())
	{
		m_lastMoveFacing = determineFacingFromVector(allowedDelta, m_lastMoveFacing);
	}
}

// ============ Clips setup ============
void MapPlayer::setClip(PlayerState st, Facing4 f, const AnimClip& clip)
{
	m_clips[(int)st][(int)f] = clip;
	if (clip.valid()) m_hasAnyClip = true;
}

void MapPlayer::setClipFromStrip(PlayerState st, Facing4 f,
								 const FilePath& png,
								 Size frame, int count,
								 double fps, bool loop)
{
	AnimClip c;
	c.atlas = Texture{ png, TextureDesc::Unmipped };
	c.frame = frame;
	c.count = count > 0 ? count : (c.atlas ? (c.atlas.width() / Max(1, frame.x)) : 0);
	c.fps = fps;
	c.start = 0;
	c.loop = loop;
	setClip(st, f, c);
}

void MapPlayer::setClipFromSheetRow(PlayerState st, Facing4 f,
									const FilePath& sheet,
									Size frame,
									int rowIndex, int count,
									double fps, bool loop,
									int colStart)
{
	AnimClip c;
	c.atlas = Texture{ sheet, TextureDesc::Unmipped };
	c.frame = frame;
	c.fps = fps;
	c.loop = loop;

	if (c.atlas && frame.x > 0)
	{
		const int cols = c.atlas.width() / frame.x;
		c.start = rowIndex * cols + colStart;
		c.count = (count > 0 ? count : (cols - colStart));
	}
	setClip(st, f, c);
}

void MapPlayer::setClipsFromSheet8(const FilePath& sheet, Size frame,
								   const std::array<RowBinding, 8>& rows,
								   int colStart)
{
	Texture shared{ sheet, TextureDesc::Unmipped };

	for (const auto& r : rows)
	{
		AnimClip c;
		c.atlas = shared;
		c.frame = frame;
		c.fps = r.fps;
		c.loop = r.loop;

		if (shared && frame.x > 0)
		{
			const int cols = shared.width() / frame.x;
			c.start = r.row * cols + colStart;
			c.count = (r.count > 0 ? r.count : (cols - colStart));
		}
		setClip(r.st, r.face, c);
	}
	Console << U"[Player] sheet size: " << shared.size()
		<< U", frame: " << frame
		<< U", example count: " << m_clips[(int)PlayerState::Idle][(int)Facing4::Down].count;
}

// ===== Convenience initializers =====
void MapPlayer::initAnimationsFromSheet8(const FilePath& sheet,
										 Size frame,
										 const std::array<RowBinding, 8>& rows,
										 int colStart)
{
	setClipsFromSheet8(sheet, frame, rows, colStart);
	setSize(frame); // EN: AABB = frame / JP: 当たり判定 = フレームサイズ
}

void MapPlayer::initAnimationStrip(PlayerState st, Facing4 f,
								   const FilePath& png,
								   Size frame, int count,
								   double fps, bool loop)
{
	setClipFromStrip(st, f, png, frame, count, fps, loop);
	// Note: not changing AABB here to avoid conflicts if different clips have different frames.
}

// ============ Update & Draw ============
MapPlayer::Facing4 MapPlayer::determineFacingFromVector(const Vec2& v, Facing4 fallback)
{
	// EN: If zero, keep last / JP: 0ベクトルなら前回を維持
	if (v.isZero()) return fallback;

	// EN: Axis-dominant: |x|>=|y| → Left/Right; else Up/Down
	// JP: 主軸優先: |x|>=|y| → 左右、それ以外は上下
	if (Abs(v.x) >= Abs(v.y))
		return (v.x >= 0.0 ? Facing4::Right : Facing4::Left);
	else
		return (v.y >= 0.0 ? Facing4::Down : Facing4::Up);
}

void MapPlayer::switchIfNeeded(PlayerState newState, Facing4 newFacing)
{
	if (newState != m_state || newFacing != m_facing)
	{
		m_state = newState;
		m_facing = newFacing;
		resetAnimator();
	}
}

void MapPlayer::resetAnimator()
{
	m_frameIndex = 0;
	m_timeAcc = 0.0;
}

Rect MapPlayer::calcSrcRect(const AnimClip& c, int frameIndex)
{
	if (!c.valid()) return Rect{ 0, 0, 0, 0 };

	const int cols = c.atlas.width() / c.frame.x;
	const int idx = c.start + frameIndex;
	const int sx = (idx % cols) * c.frame.x;
	const int sy = (idx / cols) * c.frame.y;
	return Rect{ sx, sy, c.frame.x, c.frame.y };
}

void MapPlayer::updateAnimation(double dt, const Vec2& inputDirRaw)
{
	// 1) Decide state
	const bool moving = !inputDirRaw.isZero();
	const PlayerState nextState = moving ? PlayerState::Move : PlayerState::Idle;

	// 2) Decide facing (moving dir or last)
	const Facing4 faceByInput = determineFacingFromVector(inputDirRaw, m_lastMoveFacing);
	const Facing4 nextFacing = moving ? faceByInput : m_lastMoveFacing;

	if (moving) m_lastMoveFacing = faceByInput;

	// 3) Switch if changed
	switchIfNeeded(nextState, nextFacing);

	// 4) Advance animator for current clip
	const AnimClip& clip = m_clips[(int)m_state][(int)m_facing];
	if (!clip.valid()) return;

	m_timeAcc += dt;
	const double step = (clip.fps > 0.0 ? (1.0 / clip.fps) : 1.0);

	while (m_timeAcc >= step)
	{
		m_timeAcc -= step;
		if (m_frameIndex + 1 < clip.count)
		{
			++m_frameIndex;
		}
		else
		{
			if (clip.loop) m_frameIndex = 0;
			else m_frameIndex = clip.count - 1;
		}
	}
}

void MapPlayer::draw() const
{
	const AnimClip& clip = m_clips[(int)m_state][(int)m_facing];
	if (clip.valid())
	{
		static bool once = false;
		if (!once) { Console << U"[Player] No valid clip for state/facing"; once = true; }

		const Rect src = calcSrcRect(clip, m_frameIndex);

		// EN: Draw centered at m_center (pixel-perfect friendly)
		// JP: 中心座標 m_center に描画（ピクセル合わせ）
		const Vec2 half = Vec2{ clip.frame } *0.5;
		clip.atlas(src).draw(m_center - half);
	}
	else
	{
		// EN: Fallback: draw a placeholder box if no clip configured
		// JP: 代替：未設定時は箱を描く
		aabb().draw(ColorF{ 0.2, 0.8, 1.0, 0.6 }).drawFrame(2, Palette::Blue);
	}
}
