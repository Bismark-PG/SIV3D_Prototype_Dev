/*==============================================================================

	Manager For In Game Logic [Game_Manager.cpp] [MODIFIED_v5]

	Note :
	- v4 (Story + Map + Resources)
	- Integrated teammate's latest map path fixes.

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
	, m_texBattleBack(TextureAsset(U"Battle_Back1"))
	, m_texFinalBattleBack(TextureAsset(U"Battle_Back3"))
	, m_bInTutorial(not bSkipTutorial)
	, m_nextSequenceAfterStory(GameplaySequence::Exploration)
{
	m_resourceManager.load();

	// --- register multiple maps (key -> json path) ---
	registerMaps_();

	// --- boot map & spawn (change as you like) ---
	if (bSkipTutorial)
	{
		Current_Day = Day::Day2;
		if (!loadMapAtSpawn_(U"town", U"main")) {
			Console << U"!!!!!!!! MAP LOAD FAILED !!!!!!!!";
		}
		StartStorySequence(StoryID::Story_For_Skip, GameplaySequence::Story);
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

	const bool storyJustEnded = m_storyManager.Update();
	if (storyJustEnded)
	{
		HandleStoryEnd(m_storyManager.GetLastStoryID());
	}

	switch (Sequence)
	{
	// ----------------
	// Exploration
	// ----------------
	case GameplaySequence::Exploration:
	{
		if (not m_storyManager.IsActive())
		{
			if (Exploration_Timer.isPaused())
			{
				Exploration_Timer.resume();
			}

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

				// ---- poll portal request (NEW) ----
				if (auto req = m_explorationScene.pollSceneChangeRequest())
				{
					beginFadeOut_(*req); // start fade-out; switch at the peak
				}

				if (Exploration_Timer.sF() > 20) // [Debug] 180 sec
				{
					Set_Sequence(GameplaySequence::Grow);
				}
			}
		}
		else
		{
			if (Exploration_Timer.isRunning())
			{
				Exploration_Timer.pause();
			}
		}
		
		// progress fade, maybe switch scene in the middle
		applyFadeAndMaybeSwitch_(dt);
		break;
	}

	// ----------------
	// Battle
	// ----------------
	case GameplaySequence::Battle:
	case GameplaySequence::FinalBattle:
	{
		if (not m_storyManager.IsActive())
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
						StartStorySequence(StoryID::Ending_Good, GameplaySequence::Story);
					}
					else
					{
						Set_Sequence(GameplaySequence::Exploration);
					}
				}
				else // PlayerLose
				{
					if (Sequence == GameplaySequence::FinalBattle)
					{
						StartStorySequence(StoryID::Ending_Bad, GameplaySequence::Story);
					}
					else
					{
						Console << U"lose...";
						Is_Game_Finished = true;
					}
				}

				if (Sequence != GameplaySequence::FinalBattle)
				{
					m_explorationScene.removeTriggeredEnemy();
				}
			}
		}
	}
	break;

	// ----------------
	// Grow
	// ----------------
	case GameplaySequence::Grow:
		if (not m_storyManager.IsActive())
		{
			if (m_growManager.Update())
			{
				Go_To_Next_Day();
			}
		}
		break;

	case GameplaySequence::Story:
		break;

	// ----------------
	// Dialogue (unused)
	// ----------------
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

#if defined(DEBUG) || defined(_DEBUG)
		m_fontDebug(U"[W/A/S/D] Move   [Tab] Toggle Collider/Navi Debug").draw(20, 20, ColorF{ 0.9 });
#else
		m_fontDebug(U"[ W / A / S / D ] Move").draw(20, 20, ColorF{ 0.9 });
#endif

		{
			const int totalSeconds = static_cast<int>(Exploration_Timer.sF());
			const int minutes = totalSeconds / 60;
			const int seconds = totalSeconds % 60;
			const String timeStr = U"{:0>2}:{:0>2}"_fmt(minutes, seconds);

			const Vec2 timerAnchorPos = Scene::Rect().tr().movedBy(-150, 20);
			const RectF timerTextRect = m_fontTimer(timeStr).region(Arg::topRight = timerAnchorPos);
			timerTextRect.stretched(10, 5).draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(1, Palette::Gray);
			m_fontTimer(timeStr).draw(Arg::topRight = timerAnchorPos, ColorF(1.0));
		}

		drawFadeOverlay_(); // NEW: overlay
		break;

	case GameplaySequence::Battle:
		m_texBattleBack.scaled(Scene::Width() / (double)m_texBattleBack.width()).draw(0, 0);
		m_battleManager.Draw();
		break;

	case GameplaySequence::FinalBattle:
		m_texFinalBattleBack.scaled(Scene::Width() / (double)m_texFinalBattleBack.width()).draw(0, 0);
		m_battleManager.Draw();
		m_fontBattle(U"[Final Battle]").drawAt(Scene::Center().movedBy(0, -200), ColorF(1.0, 0.5, 0.0));
		break;

	case GameplaySequence::Grow:
		m_growManager.Draw();
		break;

	case GameplaySequence::Story:
		break;

	case GameplaySequence::Dialogue:
		// Draw Dialogue UI Here
		break;
	}

	m_storyManager.Draw();

	if (Current_Day != Day::Final && Current_Day != Day::Day1 && Sequence == GameplaySequence::Exploration)
	{
		const String dayText = U"{} Days"_fmt(static_cast<int>(Current_Day));
		const Vec2 dayAnchorPos = Scene::Rect().tr().movedBy(-20, 20);
		const RectF dayTextRect = m_fontDays(dayText).region(Arg::topRight = dayAnchorPos);
		dayTextRect.stretched(10, 5).draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(1, Palette::Gray);
		m_fontDays(dayText).draw(Arg::topRight = dayAnchorPos, ColorF(1.0));
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
			if (Pre_Sequence == GameplaySequence::Grow)
			{
				Exploration_Timer.restart();
			}
			else
			{
				Exploration_Timer.resume();
			}
		}
		else if (not Exploration_Timer.isRunning())
		{
			Exploration_Timer.restart();
		}
	}

	if (nextSequence != GameplaySequence::Story)
	{
		StopAllBGM();
	}

	Sequence = nextSequence;
	Console << U"Debug: Sequence changed from " << FromEnum(Pre_Sequence) << U" to " << FromEnum(Sequence);

	switch (Sequence)
	{
	case GameplaySequence::Exploration:
		AudioAsset(U"Explorer").setLoop(true);
		AudioAsset(U"Explorer").play(SecondsF(0.5), MixBus0);
		break;

	case GameplaySequence::Battle:
		if (enemyStats)
		{
			m_battleManager.StartBattle(m_player, *enemyStats);
			AudioAsset(U"Battle").setLoop(true);
			AudioAsset(U"Battle").play(SecondsF(0.5), MixBus0);
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

				AudioAsset(U"Battle").setLoop(true);
				AudioAsset(U"Battle").play(SecondsF(0.5), MixBus0);
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
		bossStats.maxHP = 500; bossStats.currentHP = 500;
		bossStats.attack = 20; bossStats.defense = 10; bossStats.speed = 10;
		bossStats.expYield = 1000;
		m_battleManager.StartBattle(m_player, bossStats);
		AudioAsset(U"Final_Battle").setLoop(true);
		AudioAsset(U"Final_Battle").play(SecondsF(0.5), MixBus0);
	}
	break;

	case GameplaySequence::Grow:
		AudioAsset(U"Explorer").setLoop(true);
		AudioAsset(U"Explorer").play(SecondsF(0.5), MixBus0);
		m_growManager.OnSceneStart(Current_Day);
		break;

	case GameplaySequence::Story:
		break;

	case GameplaySequence::Dialogue:
		AudioAsset(U"Explorer").setLoop(true);
		AudioAsset(U"Explorer").play(SecondsF(0.5), MixBus0);
		break;
	}
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
			if (!loadMapAtSpawn_(U"town", U"main")) {
				Console << U"!!!!!!!! MAP LOAD FAILED ON NEW DAY !!!!!!!!";
			}

			Set_Sequence(GameplaySequence::Exploration);
		}
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

	const auto renderType = m_storyManager.GetRenderType();

	if (renderType == StoryRenderType::Standalone)
	{
		StopAllBGM();
		const String bgm = m_storyManager.GetCurrentBGM();
		if (not bgm.isEmpty())
		{
			AudioAsset(bgm).setLoop(true);
			AudioAsset(bgm).play(SecondsF(0.5), MixBus0);
		}

		Set_Sequence(GameplaySequence::Story);
	}
}

void Game::HandleStoryEnd(StoryID finishedStoryID)
{
	switch (finishedStoryID)
	{
	case StoryID::Opening1:
		StartStorySequence(StoryID::Opening2, GameplaySequence::Story); // Standalone
		break;

	case StoryID::Opening2:
		StartStorySequence(StoryID::Opening3, GameplaySequence::Story); // Standalone
		break;

	case StoryID::Opening3:
		StartStorySequence(StoryID::Tutorial_Name, GameplaySequence::Story); // Standalone
		break;

	case StoryID::Tutorial_Name:
		if (m_bInTutorial)
		{
			Set_Sequence(GameplaySequence::Battle);
			StartStorySequence(StoryID::Tutorial_Battle, GameplaySequence::Battle);
		}
		else
		{
			m_bInTutorial = false;
			Current_Day = Day::Day2;
			Set_Sequence(GameplaySequence::Exploration);
			StartStorySequence(StoryID::Day2_Start, GameplaySequence::Exploration);
		}
		break;

	case StoryID::Tutorial_Battle:
		break;

	case StoryID::Tutorial_Introduce:
		if (m_bInTutorial)
		{
			StartStorySequence(StoryID::Tutorial_End, GameplaySequence::Story); // Standalone
		}
		else
		{
			StartStorySequence(StoryID::Skip_Input_Partner_Name, GameplaySequence::Story); // Standalone
		}
		break;

	case StoryID::Tutorial_End:
		StartStorySequence(StoryID::Day1_End, GameplaySequence::Story); // Standalone
		break;

	case StoryID::Story_For_Skip:
		StartStorySequence(StoryID::Skip_Input_Player_Name, GameplaySequence::Story);
		break;

	case StoryID::Skip_Input_Player_Name:
		StartStorySequence(StoryID::Skip_Input_Partner_Name, GameplaySequence::Story);
		break;

	case StoryID::Day1_End:
	case StoryID::Skip_Input_Partner_Name:
		m_bInTutorial = false;
		Current_Day = Day::Day2;
		Set_Sequence(GameplaySequence::Exploration);
		StartStorySequence(StoryID::Day2_Start, GameplaySequence::Exploration);
		break;

	case StoryID::Day2_Start:
		break;

	case StoryID::FinalBattle_Start:
		Set_Sequence(GameplaySequence::FinalBattle);
		break;

	case StoryID::Ending_Bad:
		Is_Game_Finished = true;
		break;

	case StoryID::Ending_Good:
		Is_Game_Finished = true;
		break;

	case StoryID::NONE:
		break;
	}
}

void Game::StopAllBGM(const SecondsF& fadeout)
{
	const Array<String> bgms = {
		U"Chase", U"Ending_Bad", U"Ending_Good", U"Explorer",
		U"Final_Battle", U"Battle", U"Scary", U"禁止区",
		U"君が残したメロディー", U"歪む霊域",
		U"Sample_Main", U"Sample_Battle", U"Sample_Talk", U"Sample_Ending"
	};

	for (const auto& name : bgms)
	{
		if (AudioAsset::IsRegistered(name))
		{
			AudioAsset(name).stop(fadeout);
		}
	}
}

void Game::registerMaps_()
{
	// EN: Put your real paths here (consider ResolveAsset if needed).
	// JP: 実際のパスに置き換えてください（必要なら ResolveAsset のような補助を）
	m_mapPaths[U"town"] = U"../Assets/map/town/town.json";
	m_mapPaths[U"school"] = U"../Assets/map/school/school.json";
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
		// keep default position
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
			// switch at peak
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

