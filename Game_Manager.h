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
#include "Story.h"

class Game
{
public:
	Game(bool bSkipTutorial);
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
	Story_Manager m_storyManager;
	MapScene m_explorationScene;

	BattleManager m_battleManager;
	GameResourceManager m_resourceManager;

	Font m_fontBattle;
	Font m_fontDays;
	Font m_fontDebug;
	Font m_fontTimer;
	Font m_fontButton;

	bool Is_Game_Finished = false;
	bool m_bInTutorial;
	GameplaySequence m_nextSequenceAfterStory;

	void Go_To_Next_Day();
	void StartStorySequence(StoryID id, GameplaySequence nextSequence);
	void HandleStoryEnd(StoryID finishedStoryID);

	// Main Logic
	void Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats);
	// Sub Logic for Useful
	void Set_Sequence(GameplaySequence nextSequence);

	GameplaySequence Get_Sequence();
};

#endif // GAME_MANAGER_H
