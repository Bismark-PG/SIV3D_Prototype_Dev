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
	Optional<Game> GM;


	TitleState currentTitleState = TitleState::Main;

	const Font Font_Title(40, Typeface::Bold);
	const Font Font_Ending(30, Typeface::Bold);
	const Font Font_Warning(30, Typeface::Bold);
	const Font Font_Text(22);

	while (System::Update())
	{
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
				Font_Title(U"Re:ing").drawAt(Scene::Center().movedBy(0, -100), ColorF(0.8, 0.9, 1.0));

				if (SimpleGUI::Button(U"ゲーム開始", Scene::Center().movedBy(0, 60), 200))
				{
					currentTitleState = TitleState::Warning;
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
				RectF(Scene::Size()).draw(ColorF(0.2, 0.3, 0.4));

				Font_Warning(U"チュートリアルをスキップしますか？")
					.drawAt(Scene::Center().movedBy(0, -50), ColorF(1.0));

				if (SimpleGUI::Button(U"はい (スキップ)", Scene::Center().movedBy(-300, 50), 200))
				{
					GM.emplace(true);
					Current_Scene = GameScene::Gameplay;
					currentTitleState = TitleState::Main;
					Console << U"Debug: Starting Gameplay (Skipped Tutorial).";
				}

				if (SimpleGUI::Button(U"いいえ (開始)", Scene::Center().movedBy(100, 50), 200))
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
			// 簡単なエンディングメッセージ表示
			Font_Ending(U"ゲーム終了 (Game Over or Clear)").drawAt(Scene::Center().movedBy(0, -30));

			if (SimpleGUI::Button(U"タイトルへ (To Title)", Scene::Center().movedBy(0, 30), 200))
			{
				GM.reset(); // Game オブジェクトを破棄
				Current_Scene = GameScene::Title; // タイトルシーンへ遷移
				Console << U"Debug: Returning to Title Screen.";

				AudioAsset(U"Sample_Main").stop(SecondsF(0.5));
				AudioAsset(U"Sample_Battle").stop(SecondsF(0.5));
				AudioAsset(U"Sample_Talk").stop(SecondsF(0.5));
				AudioAsset(U"Sample_Ending").stop(SecondsF(0.5)); // BGM停止など追加処理
			}
			break;
		}
	}
}
