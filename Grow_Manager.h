/*==============================================================================

	Manage Grow System [Grow_Manager.h] [MODIFIED]

	Author : Team Re:ing >> Yamato

	Note : Removed level system. Uses statPoints as spendable EXP.

==============================================================================*/
#pragma once
#include <Siv3D.hpp>
#include "Player.h"
#include "BattleAnim.h"  
#include "In_Game_Enum.h"

class Grow_Manager
{
public:
	Grow_Manager(PlayerStats& playerStats);

	void Add_Exp(double expAmount);

	bool Update();

	void Draw() const;

	void OnSceneStart(Day currentDay);

private:
	PlayerStats& m_playerStats;
	Font m_font;

	bool m_editing = false;
	bool m_changed = false;
	int m_usedPoints = 0;  

	int m_tempAttack, m_tempDefense, m_tempMagicAttack, m_tempMagicDefense, m_tempSpeed, m_tempMaxHP, m_tempMaxMP;

	Texture m_currentBackground;

	bool m_showPopup = false; 
	double m_popupTimer = 0.0;

	Optional<BattleAnim> m_partnerAnim;
	Vec2 m_partnerPos;
};

