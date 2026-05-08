/*==============================================================================

	Manage Player [Player.cpp]

	Author : Team Re:ing >> Ko

	Note : 

==============================================================================*/

#include "stdafx.h"
#include "Player.h"

PlannedAction BattlePlayer::chooseAction(Character& enemy)
{
	PlannedAction pa;
	pa.actor = this;
	pa.target = &enemy;
	pa.type = selectedType.value_or(ActionType::Physical);
	return pa;
}

bool BattlePlayer::hasTypeSelected() const noexcept
{
	return selectedType.has_value();
}

bool BattlePlayer::hasItemSelected() const noexcept
{
	return selectedItemIndex.has_value();
}

ActionType BattlePlayer::getSelectedTypeOr(ActionType defaultType) const noexcept
{
	return selectedType.value_or(defaultType);
}

int BattlePlayer::getItemIndexOr(int defaultIndex) const noexcept
{
	return selectedItemIndex.value_or(defaultIndex);
}

void BattlePlayer::setSelectedType(ActionType st)
{
	selectedType = st;
}

void BattlePlayer::setSelectedItem(int itemIndex)
{
	selectedItemIndex = itemIndex;
}

void BattlePlayer::clearSelectedType() noexcept
{
	selectedType.reset();
}

void BattlePlayer::clearSelectedItem() noexcept
{
	selectedItemIndex.reset();
}

Stats BattlePlayer::ConvertPlayerStats(const PlayerStats& ps)
{
	Stats s;
	s.maxHP = ps.maxHP;
	s.hp = s.maxHP;
	s.maxMP = ps.maxMP;
	s.mp = s.maxMP;
	s.atk = ps.attack;
	s.def = ps.defense;
	s.mag = ps.magicattack;
	s.mdef = ps.magicdefense;
	s.agi = ps.speed;
	s.acc = 95; // Default accuracy
	s.eva = 5;  // Default evasion
	s.crit = 10; // Default critical chance
	return s;
}

