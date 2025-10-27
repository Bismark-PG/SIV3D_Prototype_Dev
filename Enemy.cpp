/*==============================================================================

	Manage Enemy [Enemy.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#include "stdafx.h"
#include "Enemy.h"

PlannedAction BattleEnemy::chooseAction(Character& opponent)
{
	PlannedAction pa;
	pa.actor = this;
	pa.target = &opponent;

	// 20% chance to defend when low hp
	// 20% chance to go with magical attack when mp is not empty
	// else physical attack
	const int chance = Random(1, 100);
	if (st.hp < st.maxHP / 3 && chance <= 20)
	{
		pa.type = ActionType::Defend;
	}
	else if (st.mp >= 5 && chance <= 40) // Debug : assume magical attack costs 5 MP
	{
		pa.type = ActionType::Magical;
	}
	else
	{
		pa.type = ActionType::Physical;
	}

	return pa;
}

Stats BattleEnemy::ConvertEnemyStats(const EnemyStats& es)
{
	Stats s;
	s.level = 1; // Debug : no Info, Use 1
	s.maxHP = es.maxHP;
	s.hp = es.currentHP; // [Debug] Get Current HP 
	s.maxMP = 10; // Debug : no Info, Use 10
	s.mp = s.maxMP;
	s.atk = es.attack;
	s.def = es.defense;
	s.mag = 5; // Debug : no Info, Use 5
	s.mdef = 3; // [Debug] no Info, Use 3
	s.agi = es.speed;
	// Debug : no Info, Use default values
	s.acc = 95;
	s.eva = 5;
	s.crit = 5;
	return s;
}

// [Edit] Get EXP Yield
double BattleEnemy::getExpYield() const
{
	return m_expYield;
}
