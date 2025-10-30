/*==============================================================================

	Manager For In Game Logic [Game_Manager.cpp]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#include "stdafx.h"
#include "Game_Manager.h"

Game::Game()
	: Sequence(GameplaySequence::Exploration)
	, Current_Day(Day::Day1)
	, m_growManager(m_player)
	, m_fontBattle(40)
	, m_fontDays(25)
	, m_fontDebug(16)
	, m_fontTimer(20)
	, m_fontButton(20)
{
	m_resourceManager.load();

	const FilePath mapPath = U"../Assets/map/town.json";
	if (not m_explorationScene.loadFromTiledJSON(mapPath))
	{
		Console << U"!!!!!!!! MAP LOAD FAILED !!!!!!!!";
		Console << U"Failed to load map: " << mapPath;
	}

	Exploration_Timer.start();

	AudioAsset(U"Sample_Main").setLoop(true);
	AudioAsset(U"Sample_Main").play(SecondsF(0.5), MixBus0);
}

void Game::Update()
{
	const double dt = Scene::DeltaTime();

	switch (Sequence)
	{
		// ---------------
		//	探索フェイズ
		// ----------------
	case GameplaySequence::Exploration:

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

		if (Exploration_Timer.sF() > 10) // [Debug] 180 sec, Now Use 10 sec
		{
			Set_Sequence(GameplaySequence::Grow);
		}
		break;

		// --------------------------------
		//	戦闘フェーズ / 最終戦闘フェーズ
		// --------------------------------
	case GameplaySequence::Battle:
	case GameplaySequence::FinalBattle: 
	{
		BattleResult result = m_battleManager.Update();
		if (result != BattleResult::InProgress)
		{
			if (result == BattleResult::PlayerWin)
			{
				Player_Gains_Exp(m_battleManager.GetExperienceYield());

				if (Sequence == GameplaySequence::FinalBattle)
				{
					Console << U"Win...";
					Is_Game_Finished = true;
				}
				else
				{
					Set_Sequence(GameplaySequence::Exploration);
				}
			}
			else { // PlayerLose
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

		// ----------------
		//	育成フェーズ
		// ----------------
	case GameplaySequence::Grow:
		m_growManager.Update(); // Grow_ManagerはUIの開閉とステータス割り振り処理のみ担当

		// [追加] Game_Managerが「次の日へ」ボタンのクリックを直接処理する
		// Grow_ManagerのUIが表示されていない時だけボタンを表示・処理
		if (not m_growManager.IsShowingUI()) // IsShowingUI()はGrow_Managerに必要
		{
			Rect nextDayButtonRect(Scene::Center().x - 100, Scene::Height() - 100, 200, 50);
			if (nextDayButtonRect.leftClicked())
			{
				Go_To_Next_Day(); // 次の日へ進む
			}
		}
		break;

		// ----------------
		//	対話フェーズ
		// ----------------
	case GameplaySequence::Dialogue: // 現在未使用
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
		break;

	case GameplaySequence::Battle:
		m_battleManager.Draw();
		break;

	case GameplaySequence::FinalBattle:
		m_battleManager.Draw();
		m_fontBattle(U"[Final Battle]").drawAt(Scene::Center().x - 260, 20, Palette::Orange);
		break;

	case GameplaySequence::Grow:
		m_growManager.Draw();

		if (not m_growManager.IsShowingUI())
		{
			Rect nextDayButtonRect(Scene::Center().x - 100, Scene::Height() - 100, 200, 50);
			nextDayButtonRect.draw(Palette::Forestgreen);
			m_fontButton(U"次の日へ (Next Day)").drawAt(nextDayButtonRect.center());
		}
		break;

	case GameplaySequence::Dialogue:
		// Draw Dialogue UI Here
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
			Set_Sequence(GameplaySequence::FinalBattle);
		}
		else
		{
			Set_Sequence(GameplaySequence::Exploration);
		}
	}
}

void Game::Set_Sequence(GameplaySequence nextSequence, Optional<EnemyStats> enemyStats)
{
	GameplaySequence Pre_Sequence = Sequence; // 前のシーケンスを保存 (デバッグ用)

	if (Sequence == GameplaySequence::Exploration && nextSequence != GameplaySequence::Exploration)
	{
		Exploration_Timer.pause();
	}
	else if ((Sequence == GameplaySequence::Battle || Sequence == GameplaySequence::FinalBattle)
		&& nextSequence == GameplaySequence::Exploration)
	{
		Exploration_Timer.resume();
	}
	else if ((Sequence == GameplaySequence::Grow || Sequence == GameplaySequence::Dialogue)
		&& nextSequence == GameplaySequence::Exploration)
	{
		Exploration_Timer.restart();
	}

	AudioAsset(U"Sample_Main").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Battle").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Talk").stop(SecondsF(0.5));
	AudioAsset(U"Sample_Ending").stop(SecondsF(0.5));

	Sequence = nextSequence; // シーケンス更新
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
			Console << U"Error: Battle sequence started without enemy stats!";
			Set_Sequence(GameplaySequence::Exploration);
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
