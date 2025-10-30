#include "stdafx.h"
#include "BattleAnim.h"

BattleAnim::BattleAnim(const FilePath& path, int frameW, int frameH, int cols, int rows,
	float originX, float originY, float spacingX, float spacingY)
	: m_tex(path)
{
	// if texture is empty
	if (m_tex.isEmpty()) {
		Print << U"[ERR] BattleAnim: failed to load texture: " << path;
		return;
	}

	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			const int x = originX + c * (frameW + spacingX);
			const int y = originY + r * (frameH + spacingY);
			m_frames << m_tex(Rect{ x, y, frameW, frameH });
		}
	}

	if (m_frames.isEmpty()) {
		Print << U"[ERR] BattleAnim: no valid frames extracted from " << path;
	}
}

void BattleAnim::BattleAnim_Start(bool restart)
{
	if (restart) { m_frame = 0; m_accum = 0.0; }
	m_playing = true;
}

void BattleAnim::BattleAnim_Stop()
{
	m_playing = false;
}

void BattleAnim::BattleAnim_SetLoop(bool value)
{
	m_loop = value;
}

void BattleAnim::BattleAnim_SetFPS(double fps)
{
	m_frameTime = (fps > 0.0 ? 1.0 / fps : 0.1);
}

void BattleAnim::BattleAnim_SetPos(const Vec2& p)
{
	m_pos = p;
}

void BattleAnim::BattleAnim_SetScale(double s)
{
	m_scale = s;
}

void BattleAnim::BattleAnim_Update()
{
	if (!m_playing || m_frames.isEmpty()) return;
	m_accum += Scene::DeltaTime();

	while (m_accum >= m_frameTime) {
		m_accum -= m_frameTime;
		if (++m_frame >= m_frames.size()) {
			if (m_loop) m_frame = 0;
			else { m_frame = m_frames.size() - 1; m_playing = false; }
		}
	}
}

void BattleAnim::BattleAnim_Draw() const
{
	if (m_frames.isEmpty()) return;
	m_frames[m_frame].scaled(m_scale).draw(m_pos);
}

bool BattleAnim::BattleAnim_isPlaying() const
{
	return m_playing;
}

