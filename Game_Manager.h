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
	// Sequences (existing)
	GameplaySequence Sequence;
	Day Current_Day;
	Stopwatch Exploration_Timer;

	// Player stats system
	PlayerStats m_player;
	Grow_Manager m_growManager;
	Story_Manager m_storyManager;

	// Exploration scene (single instance, rebuilt on switch)
	MapScene m_explorationScene;

	// Battle & resources
	BattleManager m_battleManager;
	GameResourceManager m_resourceManager;

	// Fonts
	Font m_fontBattle;
	Font m_fontDays;
	Font m_fontDebug;
	Font m_fontTimer;
	Font m_fontButton;

	Texture m_texBattleBack;
	Texture m_texFinalBattleBack;

	bool Is_Game_Finished = false;
	bool m_bInTutorial;
	GameplaySequence m_nextSequenceAfterStory;

	// ===== Multi-map registry & fade (new) =====
	// EN: map key -> json path / JP: マップキー -> JSON パス
	s3d::HashTable<s3d::String, s3d::FilePath> m_mapPaths;
	enum class FadePhase { None, FadingOut, FadingIn };
	mutable FadePhase m_fade{ FadePhase::None };
	mutable double    m_fadeT{ 0.0 };
	double            m_fadeDuration{ 0.35 };
	s3d::Optional<MapScene::SceneChangeRequest> m_nextReq;

	// Main Logic
	void Go_To_Next_Day();
	void StartStorySequence(StoryID id, GameplaySequence nextSequence);
	void HandleStoryEnd(StoryID finishedStoryID);

	void StopAllBGM(const SecondsF& fadeout = SecondsF(0.5));

	void Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats);
	void Set_Sequence(GameplaySequence nextSequence);
	GameplaySequence Get_Sequence();

	// Map Logic
	void registerMaps_(); // EN/JP: 登録
	bool loadMapAtSpawn_(const s3d::String& key, const s3d::String& spawn);
	void beginFadeOut_(const MapScene::SceneChangeRequest& req);
	void applyFadeAndMaybeSwitch_(double dt);
	void drawFadeOverlay_() const;

};

#endif // GAME_MANAGER_H
