/*==============================================================================

	Manage Grow System [Grow_Manager.h] [MODIFIED]

	Author : Team Re:ing >> Yamato

	Note : Removed level system. Uses statPoints as spendable EXP.

==============================================================================*/
#pragma once
#include <Siv3D.hpp>
#include "Player.h"

class Grow_Manager
{
public:
	Grow_Manager(PlayerStats& playerStats);

	void Add_Exp(double expAmount);
	void Update();
	void Draw() const;

	bool IsShowingUI() const;

private:
	void Draw_Status_UI() const;

	PlayerStats& m_playerStats;
	bool m_showUI = false;
	Font m_font;
};
