/*==============================================================================

	Manage Baattle System [Battle_Manager.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#pragma once
#include <Siv3D.hpp>
#include "BattleSystem.h" 
#include "Player.h"       
#include "Enemy.h"        

enum class BattleResult
{
	InProgress,
	PlayerWin,
	PlayerLose
};

class BattleManager
{
public:
	BattleManager();
	void StartBattle(const PlayerStats& playerStats, const EnemyStats& enemyStats);
	BattleResult Update();
	void Draw() const;
	double GetExperienceYield() const;

private:
	static Array<String> wrapLine(const Font& f, const String& text, double maxWidth);

	Optional<BattleSystem> m_battleSystem;
	BattleResult m_result = BattleResult::InProgress;
	double m_expYield = 0;

	Font m_font;
	Font m_bigFont;
	Font m_logFont;
	RectF m_cmdPanel;
	RectF m_logPanel;
	Vec2 m_cmdBase;
	double m_cmdButtonWidth;
};
