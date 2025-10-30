/*==============================================================================

	Animation In Battle System [BattleAnim.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#pragma once
#include <Siv3D.hpp>

class BattleAnim
{
public:
	BattleAnim(const FilePath& path, int frameW, int frameH, int cols, int rows,
	float originX, float originY, float spacingX, float spacingY);

	void BattleAnim_Start(bool restart);
	void BattleAnim_Stop();
	void BattleAnim_SetLoop(bool value);
	void BattleAnim_SetFPS(double fps);
	void BattleAnim_SetPos(const Vec2& p);
	void BattleAnim_SetScale(double s);

	void BattleAnim_Update();
	void BattleAnim_Draw() const;

	bool BattleAnim_isPlaying() const;

private:
	Texture m_tex;
	Array<TextureRegion> m_frames;

	size_t m_frame = 0;
	double m_frameTime = 1.0 / 12.0;
	double m_accum = 0.0;
	bool   m_loop = true;
	bool   m_playing = false;

	Vec2   m_pos{ 0, 0 };
	double m_scale = 1.0;
	ColorF m_tint = Palette::White;
};
