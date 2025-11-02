/*==============================================================================

	Manage Grow System [Grow_Manager.cpp] [MODIFIED]

	Author : Team Re:ing >> Yamato

	Note : 

==============================================================================*/
#include "stdafx.h"
#include "Grow_Manager.h"

namespace
{
	struct GrowItem
	{
		String name;
		const int* base;
		const int* temp;
		int increment;
	};
}

Grow_Manager::Grow_Manager(PlayerStats& playerStats)
	: m_playerStats(playerStats),
	m_font(22),
	m_partnerPos(500, 240)
{
	m_partnerAnim.emplace(U"../Assets/BattleCharacter/BattlePlayer.png", 96, 128, 7, 1, 0.0, 128.0, 0.0, 0.0);
	if (m_partnerAnim)
	{
		m_partnerAnim->BattleAnim_SetFPS(8);
		m_partnerAnim->BattleAnim_SetLoop(true);
		m_partnerAnim->BattleAnim_SetScale(1.5);
		m_partnerAnim->BattleAnim_SetPos(m_partnerPos);
		m_partnerAnim->BattleAnim_Start(true);
	}
}

void Grow_Manager::OnSceneStart(Day currentDay)
{
	m_editing = false;
	m_changed = false;
	m_showPopup = false;
	m_popupTimer = 0.0;

	switch (currentDay)
	{
	case Day::Day2:
	case Day::Day3:
		m_currentBackground = TextureAsset(U"Grow_Back1");
		break;
	case Day::Day4:
	case Day::Day5:
		m_currentBackground = TextureAsset(U"Grow_Back2");
		break;
	case Day::Day6:
	case Day::Day7:
		m_currentBackground = TextureAsset(U"Grow_Back3");
		break;
	}
}

void Grow_Manager::Add_Exp(double expAmount)
{
	m_playerStats.statPoints += static_cast<int>(expAmount);
}

bool Grow_Manager::Update()
{
	if (m_partnerAnim)
	{
		m_partnerAnim->BattleAnim_Update();
	}

	const double delta = Scene::DeltaTime();
	bool isDone = false;

	if (m_showPopup)
	{
		m_popupTimer -= delta;
		if (m_popupTimer <= 0)
		{
			m_showPopup = false;
		}
	}
#if defined(DEBUG) || defined(_DEBUG)
	if (KeyP.down())
	{
		m_playerStats.statPoints += 10;
		Print << U"Debug: Added 10 stat points. Total: " << m_playerStats.statPoints;
	}
#endif

	if (!m_editing)
	{
		m_tempAttack = m_playerStats.attack;
		m_tempDefense = m_playerStats.defense;
		m_tempMagicAttack = m_playerStats.magicattack;
		m_tempMagicDefense = m_playerStats.magicdefense;
		m_tempSpeed = m_playerStats.speed;
		m_tempMaxHP = m_playerStats.maxHP;
		m_tempMaxMP = m_playerStats.maxMP;
		m_usedPoints = 0;
		m_editing = true;
	}

	Array<GrowItem> items =
	{
		{ U"物理攻撃", &m_playerStats.attack, &m_tempAttack, 5 },
		{ U"物理防御", &m_playerStats.defense, &m_tempDefense, 3 },
		{ U"魔法攻撃", &m_playerStats.magicattack, &m_tempMagicAttack, 5 },
		{ U"魔法防御", &m_playerStats.magicdefense, &m_tempMagicDefense, 3 },
		{ U"素早さ", &m_playerStats.speed, &m_tempSpeed, 1 },
		{ U"HP", &m_playerStats.maxHP, &m_tempMaxHP, 10 },
		{ U"MP", &m_playerStats.maxMP, &m_tempMaxMP, 5 },
	};

	int y = 140;
	for (auto& item : items)
	{
		Rect plusButton(250, y - 5, 30, 30);
		Rect minusButton(285, y - 5, 30, 30);

		if (plusButton.leftClicked() && m_usedPoints < m_playerStats.statPoints)
		{
			if (item.name == U"物理攻撃") m_tempAttack += item.increment;
			else if (item.name == U"物理防御") m_tempDefense += item.increment;
			else if (item.name == U"魔法攻撃") m_tempMagicAttack += item.increment;
			else if (item.name == U"魔法防御") m_tempMagicDefense += item.increment;
			else if (item.name == U"素早さ") m_tempSpeed += item.increment;
			else if (item.name == U"HP") m_tempMaxHP += item.increment;
			else if (item.name == U"MP") m_tempMaxMP += item.increment;

			m_usedPoints++;
			m_changed = true;
		}

		if (minusButton.leftClicked() && (*item.temp > *item.base))
		{
			if (item.name == U"物理攻撃") m_tempAttack -= item.increment;
			else if (item.name == U"物理防御") m_tempDefense -= item.increment;
			else if (item.name == U"魔法攻撃") m_tempMagicAttack -= item.increment;
			else if (item.name == U"魔法防御") m_tempMagicDefense -= item.increment;
			else if (item.name == U"素早さ") m_tempSpeed -= item.increment;
			else if (item.name == U"HP") m_tempMaxHP -= item.increment;
			else if (item.name == U"MP") m_tempMaxMP -= item.increment;

			m_usedPoints--;
			m_changed = true;
		}
		y += 50;
	}

	if (m_changed)
	{
		Rect applyBtn(200, y + 60, 80, 35);
		if (applyBtn.leftClicked())
		{
			m_playerStats.attack = m_tempAttack;
			m_playerStats.defense = m_tempDefense;
			m_playerStats.magicattack = m_tempMagicAttack;
			m_playerStats.magicdefense = m_tempMagicDefense;
			m_playerStats.speed = m_tempSpeed;
			m_playerStats.maxHP = m_tempMaxHP;
			m_playerStats.maxMP = m_tempMaxMP;
			m_playerStats.statPoints -= m_usedPoints;

			m_showPopup = true;
			m_popupTimer = 3.5;
			m_editing = false;
			m_changed = false;
		}
	}

	Rect closeBtn(300, y + 60, 80, 35);
	if (closeBtn.leftClicked())
	{
		m_editing = false;
		m_changed = false;

		isDone = true;
	}

	return isDone;
}

void Grow_Manager::Draw() const
{
	if (m_showPopup)
	{
		double alpha = m_popupTimer / 3.5;
		if (alpha < 0) alpha = 0;

		RectF popupRect(Scene::Center().x - 180, 100, 360, 60);
		popupRect.draw(ColorF(0.1, 0.1, 0.1, alpha * 0.8)).drawFrame(2, ColorF(1, alpha));
		m_font(U"ステータスを確定しました").drawAt(popupRect.center(), ColorF(1, 1, 1, alpha));
	}

	if (m_currentBackground)
	{
		m_currentBackground.scaled(Scene::Width() / (double)m_currentBackground.width()).draw();
	}
	else
	{
		Scene::Rect().draw(Palette::Yellow);
	}


	if (m_partnerAnim)
	{
		m_partnerAnim->BattleAnim_Draw();
	}

	RectF panel(60, 80, 320, 420);
	panel.draw(ColorF(0.8, 0.9, 1.0, 0.7)).drawFrame(2, Palette::Gray);
	m_font(U"ステータス").draw(80, 90);

	const Array<GrowItem> items =
	{
		{ U"物理攻撃", &m_playerStats.attack, &m_tempAttack, 0 },
		{ U"物理防御", &m_playerStats.defense, &m_tempDefense, 0 },
		{ U"魔法攻撃", &m_playerStats.magicattack, &m_tempMagicAttack, 0 },
		{ U"魔法防御", &m_playerStats.magicdefense, &m_tempMagicDefense, 0 },
		{ U"素早さ", &m_playerStats.speed, &m_tempSpeed, 0 },
		{ U"HP", &m_playerStats.maxHP, &m_tempMaxHP, 0 },
		{ U"MP", &m_playerStats.maxMP, &m_tempMaxMP, 0 },
	};

	int y = 140;
	for (auto& item : items)
	{
		m_font(U"{}: {}"_fmt(item.name, *item.temp)).draw(80, y);

		Rect plusButton(250, y - 5, 30, 30);
		Rect minusButton(285, y - 5, 30, 30);

		plusButton.draw((m_usedPoints < m_playerStats.statPoints) ? Palette::Skyblue : Palette::Gray);
		minusButton.draw((*item.temp > *item.base) ? Palette::Lightcoral : Palette::Gray);

		m_font(U"+").drawAt(plusButton.center());
		m_font(U"−").drawAt(minusButton.center());

		y += 50;
	}

	const String resourceText = U"残りリソース: {}"_fmt(m_playerStats.statPoints - m_usedPoints);
	const RectF resourceRect = m_font(resourceText).region(Arg::center(panel.center().x, y + 20));
	resourceRect.stretched(10, 5).draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(1, Palette::Gray);
	m_font(resourceText).drawAt(resourceRect.center(), Palette::White);

	if (m_changed)
	{
		Rect applyBtn(200, y + 60, 80, 35);
		applyBtn.draw(Palette::Green).drawFrame(1, Palette::Black);
		m_font(U"決定").drawAt(applyBtn.center(), Palette::Black);
	}

	Rect closeBtn(300, y + 60, 80, 35);
	closeBtn.draw(Palette::Lightgray).drawFrame(1, Palette::Black);
	m_font(U"翌日に").drawAt(closeBtn.center(), Palette::Black);


	if (m_playerStats.statPoints > 0)
	{
		RectF popupRect(Scene::Width() - 400, 20, 385, 50);
		popupRect.draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(2, Palette::Gray);
		m_font(U"ステータスポイントが使用可能です").drawAt(popupRect.center(), Palette::Ghostwhite);
	}

#if defined(DEBUG) || defined(_DEBUG)
	m_font(U"[P] Add 10 EXP (Debug)").draw(20, 20);
#endif
}
