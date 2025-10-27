/*==============================================================================

	Manager For In Game Character [Character.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/

#include "stdafx.h"
#include "character.h"

int Character::getLevel() const
{
	return st.level;
}

void Character::healHP(int v)
{
	st.hp = Min(st.maxHP, st.hp + v);
}

void Character::healMP(int v)
{
	st.mp = Min(st.maxMP, st.mp + v);
}

bool Character::spendMP(int v)
{
	if (st.mp < v)
	{
		return false;
	}

	st.mp -= v;
	return true;
}

int Character::getHP() const
{
	return st.hp;
}

int Character::getHPMax() const
{
	return st.maxHP;
}

int Character::getMP() const
{
	return st.mp;
}

int Character::getMPMax() const
{
	return st.maxMP;
}

String Character::getName() const
{
	return name;
}

int Character::getAttack() const
{
	return st.atk;
}

int Character::getDefense() const
{
	return st.def;
}

int Character::getMagicalAttack() const
{
	return st.mag;
}

int Character::getMagicalDefense() const
{
	return st.mdef;
}

int Character::getAgility() const
{
	return st.agi;
}

void Character::takeDamage(int v)
{
	if (isDefending)
	{
		v = (v + 1) / 2; //防御する時ダメージが元の半分を受ける
	}
	st.hp = Max(0, st.hp - v);
}

void Character::setIsDefending(bool isDef)
{
	isDefending = isDef;
}

bool Character::getIsPlayer() const
{
	return isPlayer;
}
