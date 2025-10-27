/*==============================================================================

	Manage Enemy [Enemy.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#ifndef BATTLE_ENEMY_HPP
#define BATTLE_ENEMY_HPP
#include "Character.h"

// Stats for outside battle (MapScene)
struct EnemyStats
{
	int id = 0;
	String name = U"Enemy";
	int maxHP = 50;
	int currentHP = 50;
	int attack = 5;
	int defense = 2;
	int speed = 5;
	double expYield = 10.0;
};

class BattleEnemy : public Character
{
public:
	// Get Stats from EnemyStats
	BattleEnemy(String n, Stats s, double exp)
		: Character(n, s, false), m_expYield(exp) {
	}

	PlannedAction chooseAction(Character& opponent) override;
	static Stats ConvertEnemyStats(const EnemyStats& es);
	double getExpYield() const;

private:
	double m_expYield; // Save EXP
};

#endif // !BATTLE_ENEMY_HPP
