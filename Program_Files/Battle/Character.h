/*==============================================================================

	Manager For In Game Character [Character.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#ifndef CHARACTER_H
#define CHARACTER_H
#include <Siv3D.hpp>
enum class ActionType
{
	Physical,
	Magical,
	Defend,
	Item,
	ActionTypeMax,
};

class Character;

struct Stats
{
	int level = 1;
	int maxHP = 100, hp = 100;
	int maxMP = 30, mp = 30;
	int atk = 20, def = 18;  // 物理
	int mag = 18, mdef = 10; // 魔法
	int agi = 15; // 素早さ
	int acc = 95, eva = 5; // 命中/回避
	int crit = 10; // 会心
};

// Actions
struct PlannedAction
{
	Character* actor = nullptr;
	Character* target = nullptr;
	ActionType type = ActionType::Physical;
	int initiative = 0;
};

// Base class
class Character
{
public:
	Stats st;
	String name;
	bool isDefending = false;
	bool isPlayer = false;

	Character(String n, Stats s, bool isP) : name(n), st(s), isPlayer(isP) {}
	virtual ~Character() = default;

	int getLevel() const;
	void healHP(int v);
	void healMP(int v);
	bool spendMP(int v);
	int getHP() const;
	int getHPMax() const;
	int getMP() const;
	int getMPMax() const;
	String getName() const;
	int getAttack() const;
	int getDefense() const;
	int getMagicalAttack() const;
	int getMagicalDefense() const;
	int getAgility() const;
	void takeDamage(int v);
	void setIsDefending(bool isDef);
	bool getIsPlayer() const;

	bool isAlive() const { return st.hp > 0; }

	virtual PlannedAction chooseAction(Character& opponent) = 0;
};

#endif // CHARACTER_H
