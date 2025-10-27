/*==============================================================================

	Manager For In Game Logic [Game_Manager.h]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H
#include <Siv3D.hpp>
#include "Resource.h"
#include "In_Game_Enum.h"
#include "Player.h"
#include "Grow_Manager.h"
#include "MapScene.h"
#include "Battle_Manager.h"
#include "Game_Resource_Manager.h"
#include "Enemy.h"

class Game
{
public:
	Game();
	void Update();
	void Draw() const;

	bool Is_Game_Done() const;

	void Player_Gains_Exp(double exp);

private:
	GameplaySequence Sequence;
	Day Current_Day;
	Stopwatch Exploration_Timer;

	PlayerStats m_player;
	Grow_Manager m_growManager;
	MapScene m_explorationScene;

	BattleManager m_battleManager;
	GameResourceManager m_resourceManager;

	Font m_fontBattle;
	Font m_fontDays;
	Font m_fontDebug;
	Font m_fontTimer;
	Font m_fontButton;

	void Go_To_Next_Day();


	// Main Logic
	void Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats);
	// Sub Logic for Useful
	void Set_Sequence(GameplaySequence nextSequence);

	GameplaySequence Get_Sequence();

	bool Is_Game_Finished = false;
};

#endif // GAME_MANAGER_H
