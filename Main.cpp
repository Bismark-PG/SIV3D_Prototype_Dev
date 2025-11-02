/*==============================================================================

	Project Main CPP [Main.cpp]

	Project : Grow Up Adventure Battle Game
	Project Name : nullptr

	Author : Team Re:ing
	Date : 2025. 10. 23
	Version : 1.0.0

	Note List :

==============================================================================*/
#include "stdafx.h"

enum class TitleState
{
	Main,
	Warning,
	TutorialCheck
};

void Main()
{
	Window::SetTitle(U"Game");
	Scene::SetBackground(ColorF(0.2, 0.3, 0.4));

	// リソース初期化
	Init_Texture();
	Init_Audio();

	GameScene Current_Scene = GameScene::Title;
	//GameScene Current_Scene = GameScene::Gameplay;
	Optional<Game> GM;


	TitleState currentTitleState = TitleState::Main;
	Stopwatch titleFadeTimer;
	titleFadeTimer.start();

	const Font Font_Ending(30, Typeface::Bold);
	const Font Font_Warning(30, Typeface::Bold);
	const Font Font_Text(22);


#ifdef _DEBUG
	// ---------- DEBUG MODE ----------

	bool debugBattle = false;
	Optional<BattleManager> dbg;

	if (debugBattle) {
		dbg.emplace();

		PlayerStats p{ .attack = 12, .magicattack = 10, .defense = 6, .magicdefense = 5, .speed = 8 };
		EnemyStats  e{ .id = 1, .name = U"Slime", .maxHP = 60, .currentHP = 60, .attack = 7, .defense = 2, .speed = 5, .expYield = 12.0 };

		dbg->StartBattle(p, e);
	}

	// -------------------------------
#endif


	while (System::Update())
	{

#ifdef _DEBUG
		if (debugBattle && dbg) {
			if (KeyF5.down()) {
				PlayerStats p{ .attack = 14, .magicattack = 12, .defense = 6, .magicdefense = 6, .speed = 9 };
				EnemyStats  e{ .id = 2, .name = U"Wolf", .maxHP = 200, .currentHP = 200, .attack = 20, .defense = 3, .speed = 7, .expYield = 20.0 };
				dbg->StartBattle(p, e);
			}

			const auto r = dbg->Update();
			dbg->Draw();

			if (r != BattleResult::InProgress && KeySpace.down()) {
				PlayerStats p{};
				EnemyStats  e{};
				dbg->StartBattle(p, e);
			}
			continue;
		}
#endif

		switch (Current_Scene)
		{
			// ----------------
			//	 タイトル
			// ----------------
		case GameScene::Title:
		{
			switch (currentTitleState)
			{
			case TitleState::Main:
			{
				const Texture& texBG = TextureAsset(U"Title_BG");
				texBG.resized(Scene::Size()).draw(0, 0);

				const Texture& texLogo = TextureAsset(U"Title");
				const double alpha = Min(titleFadeTimer.sF() / 2.0, 1.0);
				const RectF topRegion(0, 0, Scene::Width(), Scene::Height() * 2.0 / 3.0);

				texLogo.scaled(2.0).drawAt(topRegion.center(), ColorF(1.0, alpha));

				if (titleFadeTimer.sF() >= 3.0)
				{
					const double buttonWidth = 200.0;
					const Vec2 buttonPos(Scene::Center().x - (buttonWidth / 2.0), Scene::Height() * 5.0 / 6.0);

					if (SimpleGUI::Button(U"ゲーム開始", buttonPos, buttonWidth))
					{
						currentTitleState = TitleState::Warning;
					}
				}

				break;
			}
			case TitleState::Warning:
			{
				RectF(Scene::Size()).draw(ColorF(0.1, 0.1, 0.1, 0.9));

				Font_Warning(U"警告").drawAt(Scene::Center().movedBy(0, -100), ColorF(1.0));

				Font_Text(U"本作にはホラー要素や残酷な表現が含まれています。")
					.drawAt(Scene::Center().movedBy(0, 0), ColorF(1.0));
				Font_Text(U"心臓の弱い方、妊娠中の方などのご利用はご注意ください。")
					.drawAt(Scene::Center().movedBy(0, 40), ColorF(1.0));

				const double buttonWidth = 120;
				const double buttonHeight = 40;
				const Vec2 buttonBottomRight = Scene::Rect().br().movedBy(-40, -40);
				const Vec2 buttonTopLeft = buttonBottomRight.movedBy(-buttonWidth, -buttonHeight);

				if (SimpleGUI::Button(U"次へ", buttonTopLeft, buttonWidth))
				{
					currentTitleState = TitleState::TutorialCheck;
				}
				break;
			}

			case TitleState::TutorialCheck:
			{
				RectF(Scene::Size()).draw(ColorF(0.1, 0.1, 0.1, 0.9));

				Font_Warning(U"チュートリアルをスキップしますか？")
					.drawAt(Scene::Center().movedBy(0, -50), ColorF(1.0));

				if (SimpleGUI::Button(U"はい", Scene::Center().movedBy(-300, 50), 200))
				{
					GM.emplace(true);
					Current_Scene = GameScene::Gameplay;
					currentTitleState = TitleState::Main;
					Console << U"Debug: Starting Gameplay (Skipped Tutorial).";
				}

				if (SimpleGUI::Button(U"いいえ", Scene::Center().movedBy(100, 50), 200))
				{
					GM.emplace(false);
					Current_Scene = GameScene::Gameplay;
					currentTitleState = TitleState::Main;
					Console << U"Debug: Starting Gameplay with Tutorial.";
				}
				break;
			}
			}
			break; 
		}

			// ----------------
			//	ゲームプレイ
			// ----------------
		case GameScene::Gameplay:
			if (GM) // Game オブジェクトが有効な場合のみ更新・描画
			{
				GM->Update();
				GM->Draw();

				// ゲーム終了条件を確認し処理
				if (GM->Is_Game_Done())
				{
					Console << U"Debug: Game is Done. Transitioning to Ending.";
					Current_Scene = GameScene::Ending; // エンディングシーンへ遷移
				}
			}
			else
			{
				Console << U"Error: GM is not valid in Gameplay scene!";
				Current_Scene = GameScene::Title; // 安全のためタイトルへ戻る
			}
			break;

			// ----------------
			//	 エンディング
			// ----------------
		case GameScene::Ending:
			RectF(Scene::Size()).draw(ColorF(0.1, 0.1, 0.1, 0.9));

			Font_Warning(U"タイトルに戻りますか？")
				.drawAt(Scene::Center().movedBy(0, -50), ColorF(1.0));

			if (SimpleGUI::Button(U"はい", Scene::Center().movedBy(-300, 50), 200))
			{
				GM.reset(); // Game オブジェクトを破棄
				Current_Scene = GameScene::Title; // タイトルシーンへ遷移
				titleFadeTimer.restart();
				Console << U"Debug: Returning to Title Screen.";
			}

			if (SimpleGUI::Button(U"いいえ", Scene::Center().movedBy(100, 50), 200))
			{
				System::Exit();
			}
			break;
		}
	}
}
