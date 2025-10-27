/*==============================================================================

	Manage Grow System [Grow_Manager.cpp] [MODIFIED]

	Author : Team Re:ing >> Yamato

	Note : 

==============================================================================*/
#include "stdafx.h"
#include "Grow_Manager.h"

Grow_Manager::Grow_Manager(PlayerStats& playerStats)
	: m_playerStats(playerStats), m_font(20)
{
}

void Grow_Manager::Add_Exp(double expAmount)
{
	m_playerStats.statPoints += static_cast<int>(expAmount);
}

// [MODIFIED] Update function handles UI clicks and debug key
void Grow_Manager::Update()
{
	// --- [DEBUG] Add stat points ---
	if (KeyP.down()) // Pキーでデバッグ用ポイント追加
	{
		m_playerStats.statPoints += 10;
		Print << U"Debug: Added 10 stat points. Total: " << m_playerStats.statPoints;
	}
	// --- Debug End ---

	if (m_showUI) // ステータス割り振りUIが開いている場合
	{
		// UI内の「+」ボタンクリック処理
		Array<int*> stats = { &m_playerStats.attack, &m_playerStats.magicattack, &m_playerStats.defense, &m_playerStats.magicdefense,&m_playerStats.speed };
		for (int i = 0; i < stats.size(); ++i)
		{
			int y = 150 + i * 60;
			Rect plusButtonRect(450, y, 40, 40);

			// ポイントがあり、ボタンがクリックされたらステータス上昇
			if (m_playerStats.statPoints > 0 && plusButtonRect.leftClicked())
			{
				(*stats[i])++;
				m_playerStats.statPoints--;
			}
		}

		// 「閉じる」ボタンクリック処理
		Rect closeButtonRect(325, 510, 100, 40);
		if (closeButtonRect.leftClicked()) {
			m_showUI = false; // UIを閉じる
		}
	}
	else // ステータス割り振りUIが閉じている場合 (通常のHUD状態)
	{
		// UIを開く
		if (KeyS.down())
		{
			m_showUI = true;
		}

	}
}

// Ensure Draw remains const	
void Grow_Manager::Draw() const
{
	if (m_showUI) // ステータス割り振りUIを描画
	{
		Draw_Status_UI();
	}
	else // 通常のHUDを描画
	{
		// Main HUD display
		m_font(U"Available EXP (Stat Points): {}"_fmt(m_playerStats.statPoints)).draw(20, 20);
		m_font(U"ATK:{} MATK:{} DEF:{} MDEF:{} SPD:{}"_fmt(m_playerStats.attack, m_playerStats.magicattack, m_playerStats.defense, m_playerStats.magicdefense, m_playerStats.speed)).draw(20, 50);
		m_font(U"[P] Add 10 EXP (Debug)").draw(20, 100);

		if (m_playerStats.statPoints > 0)
		{
			m_font(U"You have EXP to spend! [S] Open Status"_fmt()).draw(20, 130, Palette::Orange);
		}
		else
		{
			m_font(U"[S] Open Status"_fmt()).draw(20, 130);
		}
	}
}

bool Grow_Manager::IsShowingUI() const
{
	return m_showUI;
}

// Ensure Draw_Status_UI remains const
void Grow_Manager::Draw_Status_UI() const
{
	Rect back(200, 100, 350, 400);
	back.draw(ColorF(0.1, 0.1, 0.2, 0.9));
	m_font(U"ステータス割り振り (EXP消費)"_fmt()).drawAt(back.center().x, back.y + 30);

	Array<String> names = { U"攻撃力 (ATK)", U"魔法攻撃力 (MATK)", U"防御力 (DEF)", U"魔法防御力 (MDEF)",U"素早さ (SPD)" };
	// Use const pointer for safety as Draw should not modify stats directly
	const Array<const int*> stats = { &m_playerStats.attack, &m_playerStats.magicattack, &m_playerStats.defense, &m_playerStats.magicdefense, &m_playerStats.speed };

	for (int i = 0; i < names.size(); ++i)
	{
		int y = 150 + i * 60;
		m_font(U"{}: {}"_fmt(names[i], *stats[i])).draw(250, y);

		Rect plusButtonRect(450, y, 40, 40);
		bool canIncrease = (m_playerStats.statPoints > 0);
		plusButtonRect.draw(canIncrease ? Palette::Lightgreen : Palette::Gray);
		m_font(U"+").drawAt(plusButtonRect.center(), canIncrease ? Palette::Black : Palette::Darkgray);
		// Click handling is done in Update()
	}

	m_font(U"残りEXP (ポイント): {}"_fmt(m_playerStats.statPoints)).draw(250, 450);

	Rect closeButtonRect(325, 510, 100, 40);
	closeButtonRect.draw(Palette::Skyblue);
	m_font(U"閉じる"_fmt()).drawAt(closeButtonRect.center());
	// Click handling is done in Update()
}
