/*==============================================================================

	Manager For In Game Logic [Game_Manager.cpp]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#include "stdafx.h"
#include "Game_Manager.h"

Game::Game(bool bSkipTutorial)
	: Sequence(GameplaySequence::Exploration)
	, Current_Day(Day::Day1)
	, m_growManager(m_player)
	, m_storyManager(m_player)
	, m_fontBattle(40)
	, m_fontDays(25)
	, m_fontDebug(16)
	, m_fontTimer(20)
	, m_fontButton(20)
	, m_bInTutorial(not bSkipTutorial)
	, m_nextSequenceAfterStory(GameplaySequence::Exploration)
{
	m_resourceManager.load();

	registerMaps_();

	if (bSkipTutorial)
	{
		Current_Day = Day::Day2;
		if (!loadMapAtSpawn_(U"town", U"main")) {
			Console << U"!!!!!!!! MAP LOAD FAILED !!!!!!!!";
		}
		StartStorySequence(StoryID::Day2_Start, GameplaySequence::Exploration);
	}
	else
	{
		Current_Day = Day::Day1;
		if (!loadMapAtSpawn_(U"town", U"main")) {
			Console << U"!!!!!!!! MAP LOAD FAILED !!!!!!!!";
		}
		StartStorySequence(StoryID::Opening1, GameplaySequence::Story);
	}
}


void Game::Update()
{
	const double dt = Scene::DeltaTime();

	switch (Sequence)
	{
	case GameplaySequence::Exploration:
	{
		const bool freeze = (m_fade == FadePhase::FadingOut);

		if (!freeze)
		{
			if (KeyTab.down())
			{
				static bool debugCollision = false;
				static bool debugNavi = false;
				debugCollision = !debugCollision;
				m_explorationScene.setDebugCollision(debugCollision);
				debugNavi = !debugNavi;
				m_explorationScene.setDebugNavi(debugNavi);
			}

			m_explorationScene.update(dt);
			m_resourceManager.update(Circle{ m_explorationScene.player().center(), 10 });

			if (auto enemyOpt = m_explorationScene.checkBattleTrigger())
			{
				Set_Sequence(GameplaySequence::Battle, enemyOpt);
			}

			if (auto req = m_explorationScene.pollSceneChangeRequest())
			{
				beginFadeOut_(*req);
			}

			if (Exploration_Timer.sF() > 180) // [Debug] 180 sec
			{
				Set_Sequence(GameplaySequence::Grow);
			}
		}

		applyFadeAndMaybeSwitch_(dt);
		break;
	}

	case GameplaySequence::Battle:
	case GameplaySequence::FinalBattle:
	{
		BattleResult result = m_battleManager.Update();
		if (result != BattleResult::InProgress)
		{
			if (result == BattleResult::PlayerWin)
			{
				Player_Gains_Exp(m_battleManager.GetExperienceYield());
				if (m_bInTutorial)
				{
					StartStorySequence(StoryID::Tutorial_Introduce, GameplaySequence::Story);
				}
				else if (Sequence == GameplaySequence::FinalBattle)
				{
					StartStorySequence(StoryID::Ending_Clear, GameplaySequence::Story);
				}
				else
				{
					Set_Sequence(GameplaySequence::Exploration);
				}
			}
			else {
				Console << U"lose...";
				Is_Game_Finished = true;
			}
			if (Sequence != GameplaySequence::FinalBattle)
			{
				m_explorationScene.removeTriggeredEnemy();
			}
		}
	}
	break;

	case GameplaySequence::Grow:
		if (m_growManager.Update())
		{
			Go_To_Next_Day();
		}
		break;

	case GameplaySequence::Story:
		if (m_storyManager.Update())
		{
			HandleStoryEnd(m_storyManager.GetLastStoryID());
		}
		break;

	case GameplaySequence::Dialogue:
		if (SimpleGUI::Button(U"Next day", Vec2{ 300, 400 }, 200))
		{
			Go_To_Next_Day();
		}
		break;
	}
}

void Game::Draw() const
{
	Scene::Rect().draw(ColorF(0.1, 0.1, 0.1));

	switch (Sequence)
	{
	case GameplaySequence::Exploration:
		m_explorationScene.draw();
		m_fontDebug(U"[W/A/S/D] Move   [Tab] Toggle Collider Debug").draw(20, 20, ColorF{ 0.9 });
		{
			const int totalSeconds = static_cast<int>(Exploration_Timer.sF());
			const int minutes = totalSeconds / 60;
			const int seconds = totalSeconds % 60;
			const String timeStr = U"{:0>2}:{:0>2}"_fmt(minutes, seconds);
			m_fontTimer(timeStr).draw(Arg::topRight(Scene::Rect().tr().movedBy(-100, 20)), Palette::White);
		}

		drawFadeOverlay_();
		break;

	case GameplaySequence::Battle:
		m_battleManager.Draw();
		break;

	case GameplaySequence::FinalBattle:
		m_battleManager.Draw();
		m_fontBattle(U"[Final Battle]").drawAt(Scene::Center().movedBy(0, -200), Palette::Orange);
		break;

	case GameplaySequence::Grow:
		m_growManager.Draw();
		break;

	case GameplaySequence::Story:
		if (m_nextSequenceAfterStory == GameplaySequence::Exploration || Current_Day >= Day::Day2)
		{
			m_explorationScene.draw();
		}
		else if (m_nextSequenceAfterStory == GameplaySequence::Battle)
		{
			Scene::Rect().draw(ColorF(0.0, 0.0, 0.0));
		}
		else if (m_nextSequenceAfterStory == GameplaySequence::FinalBattle)
		{
			Scene::Rect().draw(ColorF(0.0, 0.0, 0.0));
		}
		m_storyManager.Draw();
		break;

	case GameplaySequence::Dialogue:
		break;
	}

	m_fontDays(U"{} Days"_fmt(static_cast<int>(Current_Day))).draw(Arg::topRight(Scene::Rect().tr().movedBy(-20, 20)));
}

bool Game::Is_Game_Done() const
{
	return Is_Game_Finished;
}

void Game::Player_Gains_Exp(double exp)
{
	m_growManager.Add_Exp(exp);
}

void Game::Go_To_Next_Day()
{
	int Next_Day = static_cast<int>(Current_Day) + 1;
	if (Next_Day > static_cast<int>(Day::Final))
	{
		Is_Game_Finished = true;
	}
	else
	{
		Current_Day = static_cast<Day>(Next_Day);
		if (Current_Day == Day::Final)
		{
			StartStorySequence(StoryID::FinalBattle_Start, GameplaySequence::FinalBattle);
		}
		else if (Current_Day == Day::Day2)
		{
			StartStorySequence(StoryID::Day2_Start, GameplaySequence::Exploration);
		}
		else
		{
			Set_Sequence(GameplaySequence::Exploration);
		}
	}
}

void Game::Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats)
{
	GameplaySequence Pre_Sequence = Sequence;

	if (Sequence == GameplaySequence::Exploration && nextSequence != GameplaySequence::Exploration)
	{
		Exploration_Timer.pause();
	}
	else if (Pre_Sequence != GameplaySequence::Exploration && nextSequence == GameplaySequence::Exploration)
	{
		if (Pre_Sequence == GameplaySequence::Story && Current_Day == Day::Day1)
		{
			Exploration_Timer.restart();
		}
		else if (Exploration_Timer.isPaused())
		{
			Exploration_Timer.resume();
		}
		else if (not Exploration_Timer.isRunning())
		{
			Exploration_Timer.restart();
		}
	}

	AudioAsset(U"Sample_Main").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Battle").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Talk").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Ending").stop(SecondsF(0.5));

	Sequence = nextSequence;
	Console << U"Debug: Sequence changed from " << FromEnum(Pre_Sequence) << U" to " << FromEnum(Sequence);

	switch (Sequence)
	{
	case GameplaySequence::Exploration:
		AudioAsset(U"Sample_Main").setLoop(true);
		AudioAsset(U"Sample_Main").play(SecondsF(0.5), MixBus0);
		break;
	case GameplaySequence::Battle:
		if (enemyStats)
		{
			m_battleManager.StartBattle(m_player, *enemyStats);
			AudioAsset(U"Sample_Battle").setLoop(true);
			AudioAsset(U"Sample_Battle").play(SecondsF(0.5), MixBus0);
		}
		else {
			if (m_bInTutorial)
			{
				EnemyStats tutorialEnemy;
				tutorialEnemy.name = U"口裂け女 (Tutorial)";
				tutorialEnemy.maxHP = 50;
				tutorialEnemy.currentHP = 50;
				tutorialEnemy.attack = 5;
				tutorialEnemy.defense = 1;
				tutorialEnemy.speed = 3;
				tutorialEnemy.expYield = 10;
				m_battleManager.StartBattle(m_player, tutorialEnemy);
				AudioAsset(U"Sample_Battle").setLoop(true);
				AudioAsset(U"Sample_Battle").play(SecondsF(0.5), MixBus0);
			}
			else
			{
				Console << U"Error: Battle sequence started without enemy stats!";
				Set_Sequence(GameplaySequence::Exploration);
			}
		}
		break;
	case GameplaySequence::FinalBattle:
	{
		EnemyStats bossStats;
		bossStats.name = U"Final Boss";
		bossStats.maxHP = 500;
		bossStats.currentHP = 500;
		bossStats.attack = 20;
		bossStats.defense = 10;
		bossStats.speed = 10;
		bossStats.expYield = 1000;
		m_battleManager.StartBattle(m_player, bossStats);
		AudioAsset(U"Sample_Battle").setLoop(true);
		AudioAsset(U"Sample_Battle").play(SecondsF(0.5), MixBus0);
	}
	break;
	case GameplaySequence::Grow:
		m_growManager.OnSceneStart();
		AudioAsset(U"Sample_Talk").setLoop(true);
		AudioAsset(U"Sample_Talk").play(SecondsF(0.5), MixBus0);
		break;
	case GameplaySequence::Story:
	case GameplaySequence::Dialogue:
		AudioAsset(U"Sample_Talk").setLoop(true);
		AudioAsset(U"Sample_Talk").play(SecondsF(0.5), MixBus0);
		break;
	}
}

void Game::Set_Sequence(GameplaySequence nextSequence)
{
	Set_Sequence(nextSequence, none);
}

GameplaySequence Game::Get_Sequence()
{
	return Sequence;
}

void Game::StartStorySequence(StoryID id, GameplaySequence nextSequence)
{
	m_storyManager.StartStory(id);
	m_nextSequenceAfterStory = nextSequence;
	Set_Sequence(GameplaySequence::Story);
}

void Game::HandleStoryEnd(StoryID finishedStoryID)
{
	switch (finishedStoryID)
	{
	case StoryID::Opening1:
		StartStorySequence(StoryID::Opening2, GameplaySequence::Story);
		break;
	case StoryID::Opening2:
		StartStorySequence(StoryID::Opening3, GameplaySequence::Story);
		break;
	case StoryID::Opening3:
		StartStorySequence(StoryID::Tutorial_Name, GameplaySequence::Story);
		break;
	case StoryID::Tutorial_Battle:
		Set_Sequence(GameplaySequence::Battle);
		break;
	case StoryID::Tutorial_Introduce:
		StartStorySequence(StoryID::Tutorial_End, GameplaySequence::Story);
		break;
	case StoryID::Tutorial_End:
		m_bInTutorial = false;
		Current_Day = Day::Day2;
		StartStorySequence(StoryID::Day2_Start, GameplaySequence::Exploration);
		break;
	case StoryID::Day2_Start:
		Set_Sequence(GameplaySequence::Exploration);
		break;
	case StoryID::FinalBattle_Start:
		Set_Sequence(GameplaySequence::FinalBattle);
		break;
	case StoryID::Ending_Clear:
		Is_Game_Finished = true;
		break;
	default:
		Set_Sequence(GameplaySequence::Exploration);
		break;
	}
}

void Game::registerMaps_()
{
	m_mapPaths[U"town"] = U"../Assets/map/town.json";
	m_mapPaths[U"school"] = U"../Assets/map/school.json";
}

bool Game::loadMapAtSpawn_(const String& key, const String& spawn)
{
	const auto it = m_mapPaths.find(key);
	if (it == m_mapPaths.end()) {
		Console << U"[Game] Unknown map key: " << key;
		return false;
	}

	MapScene newScene;
	if (!newScene.loadFromTiledJSON(it->second)) {
		Console << U"[Game] loadFromTiledJSON failed: " << it->second;
		return false;
	}

	if (!newScene.placePlayerAtSpawn(spawn)) {
		Console << U"[Game] Spawn not found: " << spawn << U" in " << key
			<< U" (fallback to default 256,256)";
	}

	m_explorationScene = std::move(newScene);
	return true;
}

void Game::beginFadeOut_(const MapScene::SceneChangeRequest& req)
{
	m_nextReq = req;
	m_fade = FadePhase::FadingOut;
	m_fadeT = 0.0;
}

void Game::applyFadeAndMaybeSwitch_(double dt)
{
	if (m_fade == FadePhase::None) return;

	const double delta = dt / m_fadeDuration;

	if (m_fade == FadePhase::FadingOut)
	{
		m_fadeT += delta;
		if (m_fadeT >= 1.0)
		{
			if (m_nextReq)
			{
				const auto req = *m_nextReq;
				if (!loadMapAtSpawn_(req.targetMapKey, req.targetSpawn)) {
					Console << U"[Game] Scene switch failed";
				}
			}
			m_nextReq.reset();
			m_fade = FadePhase::FadingIn;
			m_fadeT = 1.0;
		}
	}
	else if (m_fade == FadePhase::FadingIn)
	{
		m_fadeT -= delta;
		if (m_fadeT <= 0.0)
		{
			m_fade = FadePhase::None;
			m_fadeT = 0.0;
		}
	}
}

void Game::drawFadeOverlay_() const
{
	if (m_fade == FadePhase::None) return;
	const double alpha = Clamp(m_fadeT, 0.0, 1.0);
	Rect{ 0, 0, Scene::Width(), Scene::Height() }.draw(ColorF{ 0, 0, 0, alpha });
}



/*==============================================================================
Temporary annotation processing
==============================================================================*/
//#include "stdafx.h"
//#include "Game_Manager.h"
//
//using namespace s3d;
//
//Game::Game()
//	: Sequence(GameplaySequence::Exploration)
//	, Current_Day(Day::Day1)
//	, m_growManager(m_player)
//	, m_fontBattle(40)
//	, m_fontDays(25)
//	, m_fontDebug(16)
//	, m_fontTimer(20)
//	, m_fontButton(20)
//{
//	m_resourceManager.load();
//
//	// --- register multiple maps (key -> json path) ---
//	registerMaps_();
//
//	// --- boot map & spawn (change as you like) ---
//	if (!loadMapAtSpawn_(U"town", U"main")) {
//		Console << U"!!!!!!!! MAP LOAD FAILED !!!!!!!!";
//	}
//
//	Exploration_Timer.start();
//
//	AudioAsset(U"Sample_Main").setLoop(true);
//	AudioAsset(U"Sample_Main").play(SecondsF(0.5), MixBus0);
//}
//
//void Game::registerMaps_()
//{
//	// EN: Put your real paths here (consider ResolveAsset if needed).
//	// JP: 実際のパスに置き換えてください（必要なら ResolveAsset のような補助を）
//	m_mapPaths[U"town"] = U"../Assets/map/town/town.json";
//	m_mapPaths[U"school"] = U"../Assets/map/school/school.json";
//}
//
//bool Game::loadMapAtSpawn_(const String& key, const String& spawn)
//{
//	const auto it = m_mapPaths.find(key);
//	if (it == m_mapPaths.end()) {
//		Console << U"[Game] Unknown map key: " << key;
//		return false;
//	}
//
//	MapScene newScene;
//	if (!newScene.loadFromTiledJSON(it->second)) {
//		Console << U"[Game] loadFromTiledJSON failed: " << it->second;
//		return false;
//	}
//
//	if (!newScene.placePlayerAtSpawn(spawn)) {
//		Console << U"[Game] Spawn not found: " << spawn << U" in " << key
//			<< U" (fallback to default 256,256)";
//		// keep default position
//	}
//
//	m_explorationScene = std::move(newScene);
//	return true;
//}
//
//void Game::beginFadeOut_(const MapScene::SceneChangeRequest& req)
//{
//	m_nextReq = req;
//	m_fade = FadePhase::FadingOut;
//	m_fadeT = 0.0;
//}
//
//void Game::applyFadeAndMaybeSwitch_(double dt)
//{
//	if (m_fade == FadePhase::None) return;
//
//	const double delta = dt / m_fadeDuration;
//
//	if (m_fade == FadePhase::FadingOut)
//	{
//		m_fadeT += delta;
//		if (m_fadeT >= 1.0)
//		{
//			// switch at peak
//			if (m_nextReq)
//			{
//				const auto req = *m_nextReq;
//				if (!loadMapAtSpawn_(req.targetMapKey, req.targetSpawn)) {
//					Console << U"[Game] Scene switch failed";
//				}
//			}
//			m_nextReq.reset();
//			m_fade = FadePhase::FadingIn;
//			m_fadeT = 1.0;
//		}
//	}
//	else if (m_fade == FadePhase::FadingIn)
//	{
//		m_fadeT -= delta;
//		if (m_fadeT <= 0.0)
//		{
//			m_fade = FadePhase::None;
//			m_fadeT = 0.0;
//		}
//	}
//}
//
//void Game::drawFadeOverlay_() const
//{
//	if (m_fade == FadePhase::None) return;
//	const double alpha = Clamp(m_fadeT, 0.0, 1.0);
//	Rect{ 0, 0, Scene::Width(), Scene::Height() }.draw(ColorF{ 0, 0, 0, alpha });
//}
//
//void Game::Update()
//{
//	const double dt = Scene::DeltaTime();
//
//	switch (Sequence)
//	{
//		// ----------------
//		// Exploration
//		// ----------------
//	case GameplaySequence::Exploration:
//	{
//		if (KeyTab.down())
//		{
//			static bool debugCollision = false;
//			static bool debugNavi = false;
//
//			debugCollision = !debugCollision;
//			m_explorationScene.setDebugCollision(debugCollision);
//
//			debugNavi = !debugNavi;
//			m_explorationScene.setDebugNavi(debugNavi);
//		}
//
//		const bool freeze = (m_fade == FadePhase::FadingOut);
//
//		if (!freeze)
//		{
//			m_explorationScene.update(dt);
//			m_resourceManager.update(Circle{ m_explorationScene.player().center(), 10 });
//
//			if (auto enemyOpt = m_explorationScene.checkBattleTrigger())
//			{
//				Set_Sequence(GameplaySequence::Battle, enemyOpt);
//			}
//
//			// ---- poll portal request (NEW) ----
//			if (auto req = m_explorationScene.pollSceneChangeRequest())
//			{
//				beginFadeOut_(*req); // start fade-out; switch at the peak
//			}
//
//			if (Exploration_Timer.sF() > 10) // [Debug] was 180
//			{
//				Set_Sequence(GameplaySequence::Grow);
//			}
//		}
//
//		// progress fade, maybe switch scene in the middle
//		applyFadeAndMaybeSwitch_(dt);
//		break;
//	}
//
//	// ----------------
//	// Battle
//	// ----------------
//	case GameplaySequence::Battle:
//	case GameplaySequence::FinalBattle:
//	{
//		BattleResult result = m_battleManager.Update();
//		if (result != BattleResult::InProgress)
//		{
//			if (result == BattleResult::PlayerWin)
//			{
//				Player_Gains_Exp(m_battleManager.GetExperienceYield());
//
//				if (Sequence == GameplaySequence::FinalBattle)
//				{
//					Console << U"Win...";
//					Is_Game_Finished = true;
//				}
//				else
//				{
//					Set_Sequence(GameplaySequence::Exploration);
//				}
//			}
//			else { // PlayerLose
//				Console << U"lose...";
//				Is_Game_Finished = true;
//			}
//
//			if (Sequence != GameplaySequence::FinalBattle)
//			{
//				m_explorationScene.removeTriggeredEnemy();
//			}
//		}
//		break;
//	}
//
//	// ----------------
//	// Grow
//	// ----------------
//	case GameplaySequence::Grow:
//		m_growManager.Update();
//
//		if (not m_growManager.IsShowingUI())
//		{
//			Rect nextDayButtonRect(Scene::Center().x - 100, Scene::Height() - 100, 200, 50);
//			if (nextDayButtonRect.leftClicked())
//			{
//				Go_To_Next_Day();
//			}
//		}
//		break;
//
//		// ----------------
//		// Dialogue (unused)
//		// ----------------
//	case GameplaySequence::Dialogue:
//		if (SimpleGUI::Button(U"Next day", Vec2{ 300, 400 }, 200))
//		{
//			Go_To_Next_Day();
//		}
//		break;
//	}
//}
//
//void Game::Draw() const
//{
//	Scene::Rect().draw(ColorF(0.1, 0.1, 0.1));
//
//	switch (Sequence)
//	{
//	case GameplaySequence::Exploration:
//		m_explorationScene.draw();
//		m_fontDebug(U"[W/A/S/D] Move   [Tab] Toggle Collider/Navi Debug").draw(20, 20, ColorF{ 0.9 });
//
//		{
//			const int totalSeconds = static_cast<int>(Exploration_Timer.sF());
//			const int minutes = totalSeconds / 60;
//			const int seconds = totalSeconds % 60;
//			const String timeStr = U"{:0>2}:{:0>2}"_fmt(minutes, seconds);
//			m_fontTimer(timeStr).draw(Arg::topRight(Scene::Rect().tr().movedBy(-100, 20)), Palette::White);
//		}
//
//		drawFadeOverlay_(); // NEW: overlay
//		break;
//
//	case GameplaySequence::Battle:
//		m_battleManager.Draw();
//		break;
//
//	case GameplaySequence::FinalBattle:
//		m_battleManager.Draw();
//		m_fontBattle(U"[Final Battle]").drawAt(Scene::Center().x - 260, 20, Palette::Orange);
//		break;
//
//	case GameplaySequence::Grow:
//		m_growManager.Draw();
//
//		if (not m_growManager.IsShowingUI())
//		{
//			Rect nextDayButtonRect(Scene::Center().x - 100, Scene::Height() - 100, 200, 50);
//			nextDayButtonRect.draw(Palette::Forestgreen);
//			m_fontButton(U"次の日へ (Next Day)").drawAt(nextDayButtonRect.center());
//		}
//		break;
//
//	case GameplaySequence::Dialogue:
//		// Draw Dialogue UI Here
//		break;
//	}
//
//	m_fontDays(U"{} Days"_fmt(static_cast<int>(Current_Day))).draw(Arg::topRight(Scene::Rect().tr().movedBy(-20, 20)));
//}
//
//bool Game::Is_Game_Done() const
//{
//	return Is_Game_Finished;
//}
//
//void Game::Player_Gains_Exp(double exp)
//{
//	m_growManager.Add_Exp(exp);
//}
//
//void Game::Go_To_Next_Day()
//{
//	int Next_Day = static_cast<int>(Current_Day) + 1;
//
//	if (Next_Day > static_cast<int>(Day::Final))
//	{
//		Is_Game_Finished = true;
//	}
//	else
//	{
//		Current_Day = static_cast<Day>(Next_Day);
//
//		if (Current_Day == Day::Final)
//		{
//			Set_Sequence(GameplaySequence::FinalBattle);
//		}
//		else
//		{
//			Set_Sequence(GameplaySequence::Exploration);
//		}
//	}
//}
//
//void Game::Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats)
//{
//	GameplaySequence Pre_Sequence = Sequence;
//
//	if (Sequence == GameplaySequence::Exploration && nextSequence != GameplaySequence::Exploration)
//	{
//		Exploration_Timer.pause();
//	}
//	else if ((Sequence == GameplaySequence::Battle || Sequence == GameplaySequence::FinalBattle)
//		&& nextSequence == GameplaySequence::Exploration)
//	{
//		Exploration_Timer.resume();
//	}
//	else if ((Sequence == GameplaySequence::Grow || Sequence == GameplaySequence::Dialogue)
//		&& nextSequence == GameplaySequence::Exploration)
//	{
//		Exploration_Timer.restart();
//	}
//
//	AudioAsset(U"Sample_Main").stop(SecondsF(0.5));
//	AudioAsset(U"Sample_Battle").stop(SecondsF(0.5));
//	AudioAsset(U"Sample_Talk").stop(SecondsF(0.5));
//	AudioAsset(U"Sample_Ending").stop(SecondsF(0.5));
//
//	Sequence = nextSequence;
//	Console << U"Debug: Sequence changed from " << FromEnum(Pre_Sequence) << U" to " << FromEnum(Sequence);
//
//	switch (Sequence)
//	{
//	case GameplaySequence::Exploration:
//		AudioAsset(U"Sample_Main").setLoop(true);
//		AudioAsset(U"Sample_Main").play(SecondsF(0.5), MixBus0);
//		break;
//
//	case GameplaySequence::Battle:
//		if (enemyStats)
//		{
//			m_battleManager.StartBattle(m_player, *enemyStats);
//			AudioAsset(U"Sample_Battle").setLoop(true);
//			AudioAsset(U"Sample_Battle").play(SecondsF(0.5), MixBus0);
//		}
//		else {
//			Console << U"Error: Battle sequence started without enemy stats!";
//			Set_Sequence(GameplaySequence::Exploration);
//		}
//		break;
//
//	case GameplaySequence::FinalBattle:
//	{
//		EnemyStats bossStats;
//		bossStats.name = U"Final Boss";
//		bossStats.maxHP = 500;
//		bossStats.currentHP = 500;
//		bossStats.attack = 20;
//		bossStats.defense = 10;
//		bossStats.speed = 10;
//		bossStats.expYield = 1000;
//
//		m_battleManager.StartBattle(m_player, bossStats);
//		AudioAsset(U"Sample_Battle").setLoop(true);
//		AudioAsset(U"Sample_Battle").play(SecondsF(0.5), MixBus0);
//	}
//	break;
//
//	case GameplaySequence::Grow:
//	case GameplaySequence::Dialogue:
//		AudioAsset(U"Sample_Talk").setLoop(true);
//		AudioAsset(U"Sample_Talk").play(SecondsF(0.5), MixBus0);
//		break;
//	}
//}
//
//void Game::Set_Sequence(GameplaySequence nextSequence)
//{
//	Set_Sequence(nextSequence, none);
//}
//
//GameplaySequence Game::Get_Sequence()
//{
//	return Sequence;
//}
