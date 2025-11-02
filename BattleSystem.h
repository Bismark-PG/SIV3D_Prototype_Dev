/*==============================================================================

	In Game Battle System [BattleSystem.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H
#include <Siv3D.hpp>

#include "Player.h"
#include "Enemy.h" 

enum class Phase
{
	CommandSelect,
	ActionResolve,
	EndCheck,
	BattleOver,
};

// 道具
struct Item
{
	String name;
	std::function<void(BattlePlayer&)> use;
};

class BattleSystem
{
private:

	Array<PlannedAction> m_queue;

public:

	BattlePlayer player;
	BattleEnemy enemy;

	Array<String> log; // バトルログ
	Phase phase = Phase::CommandSelect;
	int turn = 1;

	Array<Item> items;
	bool m_showResultPopup = false;
	String m_resultMessage = U"";

	BattleSystem(BattlePlayer p, BattleEnemy e);

	void pushLog(const String& s);
	void nextTurn();
	void updateOnce();
	void resolveAction(const PlannedAction& pa);

	bool isBattleEnded() const;
	bool isPlayerWinner() const;
};


#endif //!BATTLESYSTEM_H
