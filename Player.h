/*==============================================================================

	Manage Player [Player.h]

	Author : Team Re:ing >> Yamato / Ko

	Note : 

==============================================================================*/
#pragma once
#include "Character.h"

// Stats for outside battle (Grow_Manager)
struct PlayerStats
{
	int attack = 10;
	int magicattack = 10;
	int defense = 8;
	int magicdefense = 8;
	int speed = 6;
	int maxHP = 100;
	int maxMP = 30; 
	int statPoints = 0; // Represents spendable EXP points

	String partnerName = U"？？？";  // Aibo Name
};

// Battle-specific player class
class BattlePlayer : public Character
{
public:
	// Constructor to match usage in Battle_Manager.cpp
	BattlePlayer(String n, Stats s) : Character(n, s, true) {}

	PlannedAction chooseAction(Character& enemy) override;

	bool hasTypeSelected() const noexcept;
	bool hasItemSelected() const noexcept;
	ActionType getSelectedTypeOr(ActionType defaultType = ActionType::Physical) const noexcept;
	int getItemIndexOr(int defaultIndex = -1) const noexcept;

	void setSelectedType(ActionType st);
	void setSelectedItem(int itemIndex);
	void clearSelectedType() noexcept;
	void clearSelectedItem() noexcept;

	static Stats ConvertPlayerStats(const PlayerStats& ps);

private:
	Optional<ActionType> selectedType;
	Optional<int> selectedItemIndex;
};

