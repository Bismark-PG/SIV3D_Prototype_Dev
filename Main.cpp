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

#include "Battle_Manager.h"
#include "Character.h"
#include "Player.h"
#include "Enemy.h"

void Main()
{
	Window::SetTitle(U"Game");
	Scene::SetBackground(ColorF(0.2, 0.3, 0.4));

	// リソース初期化
	Init_Texture();
	Init_Audio();

	//GameScene Current_Scene = GameScene::Title;
	GameScene Current_Scene = GameScene::Gameplay;
	Optional<Game> GM;

	const Font Font_Title(40, Typeface::Bold);
	const Font Font_Ending(30, Typeface::Bold);


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
