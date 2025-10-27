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

void Main()
{
	Window::SetTitle(U"Game");
	Scene::SetBackground(ColorF(0.2, 0.3, 0.4));

	// リソース初期化
	Init_Texture();
	Init_Audio();

	GameScene Current_Scene = GameScene::Title;
	Optional<Game> GM;

	const Font Font_Title(40, Typeface::Bold);
	const Font Font_Ending(30, Typeface::Bold);

	while (System::Update())
	{
		switch (Current_Scene)
		{
			// ----------------
			//	 タイトル
			// ----------------
		case GameScene::Title:
			Font_Title(U"Re:ing").drawAt(Scene::Center().movedBy(0, -50), Palette::Skyblue);

			if (SimpleGUI::Button(U"ゲーム開始 (Start)", Scene::Center().movedBy(0, 50), 200))
			{
				GM.emplace(); // 新しいゲーム開始時に Game オブジェクトを生成
				Current_Scene = GameScene::Gameplay;
				Console << U"Debug: Starting Gameplay.";
			}
			break;

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
