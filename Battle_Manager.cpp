/*==============================================================================

	Manage Baattle System [Battle_Manager.h]

	Author : Team Re:ing >> Ko

	Note : Battle System UI

==============================================================================*/
#include "stdafx.h"
#include "Battle_Manager.h"

constexpr int kLogLineHeight = 14; // ログの行の高さ

BattleManager::BattleManager()
	: m_font(20)
	, m_bigFont(26, Typeface::Bold)
	, m_logFont(kLogLineHeight)
	, m_cmdPanel(20, 400, 480, 180)   // コマンドパネルの位置/サイズ
	, m_logPanel(520, 400, 260, 180)  // ログパネルの位置/サイズ
	, m_cmdBase(m_cmdPanel.pos.movedBy(20, 60)) // コマンドボタンの開始位置
	, m_cmdButtonWidth(150.0)         // コマンドボタンの幅
	, m_itemMenuOpen(false)
{
	// player animation initialization
	m_anim.emplace(U"../Assets/BattleCharacter/BattlePlayer.png", 96, 128, 7, 1, 0.0, 252.0, 0.0, 0.0);
	m_anim->BattleAnim_SetFPS(8);
	m_anim->BattleAnim_SetLoop(true);
	m_anim->BattleAnim_SetScale(1.5);
	m_anim->BattleAnim_SetPos(Vec2(100, 180));
	m_anim->BattleAnim_Start(true);
}

// [修正] 戦闘開始: PlayerStats と EnemyStats を受け取り BattleSystem を生成
void BattleManager::StartBattle(const PlayerStats& playerStats, const EnemyStats& enemyStats)
{
	// PlayerStats -> Stats 変換
	Stats playerBattleStats = BattlePlayer::ConvertPlayerStats(playerStats);
	// EnemyStats -> Stats 変換
	Stats enemyBattleStats = BattleEnemy::ConvertEnemyStats(enemyStats);

	// BattleSystem 生成 (Optional に emplace)
	m_battleSystem.emplace(
		BattlePlayer{ U"Player", playerBattleStats }, // 名前は PlayerStats にないため仮指定
		// [修正] BattleEnemy のコンストラクタに名前、ステータス、「経験値」を渡す
		BattleEnemy{ enemyStats.name, enemyBattleStats, enemyStats.expYield }
	);

	m_result = BattleResult::InProgress; // 結果を初期化
	m_expYield = 0; // 経験値を初期化
}

BattleResult BattleManager::Update()
{
	if (m_result != BattleResult::InProgress)
	{
		return m_result;
	}

	if (m_waitingForPopup)
	{
		if (KeyZ.down())
		{
			if (m_battleSystem && m_battleSystem->isPlayerWinner())
			{
				m_result = BattleResult::PlayerWin;
				m_expYield = m_battleSystem->enemy.getExpYield();
			}
			else
			{
				m_result = BattleResult::PlayerLose;
			}
			m_battleSystem.reset();
			m_waitingForPopup = false;

			return m_result;
		}

		return BattleResult::InProgress;
	}

	if (!m_battleSystem)
	{
		return BattleResult::InProgress;
	}

	if (m_battleSystem->phase != Phase::CommandSelect) {
		m_itemMenuOpen = false;
	}

	if (m_anim) m_anim->BattleAnim_Update();

	m_battleSystem->updateOnce();

	if (m_battleSystem->isBattleEnded())
	{
		m_waitingForPopup = true;
	}

	return BattleResult::InProgress;
}

void BattleManager::Draw() const
{
	// BattleSystem がなければ何も描画しない
	if (!m_battleSystem) return;

	// --- キャラクターステータス表示 ---
	//m_bigFont(U"Turn {}"_fmt(m_battleSystem->turn)).draw(20, 20);
	//Line(Vec2(20, 60), Vec2(Scene::Width() - 20, 60)).draw(2);

	// Player Animation
	if (m_anim) m_anim->BattleAnim_Draw();

	// Player Status
	const RectF playerStatBox(310, 275, 200, 110);
	playerStatBox.draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(1, Palette::Gray);
	m_font(U"[{}]   Lv{}"_fmt(m_battleSystem->player.getName(), m_battleSystem->player.getLevel())).draw(320, 280);
	m_font(U"HP {}/{}"_fmt(
		m_battleSystem->player.getHP(), m_battleSystem->player.getHPMax()
	)).draw(370, 320);
	m_font(U"MP {}/{}"_fmt(
		m_battleSystem->player.getMP(), m_battleSystem->player.getMPMax()
	)).draw(370, 350);

	// Enemy Status
	//const double enemyX = Scene::Width() - 220;
	const RectF enemyStatBox(270, 25, 200, 80);
	enemyStatBox.draw(ColorF(0.1, 0.1, 0.1, 0.7)).drawFrame(1, Palette::Gray);
	m_font(U"[{}] Lv{}"_fmt(m_battleSystem->enemy.getName(), m_battleSystem->enemy.getLevel())).draw(280, 30);
	m_font(U"HP {}/{}"_fmt(
		m_battleSystem->enemy.getHP(), m_battleSystem->enemy.getHPMax()
	)).draw(330, 70);

	// コマンドパネル
	if (not m_waitingForPopup) // 팝업이 아닐 때만
	{
		drawCommandUI();

		// Resolving Overlay drawing
		if (m_battleSystem->phase != Phase::CommandSelect
			&& m_battleSystem->phase != Phase::BattleOver)
		{
			// 半透明オーバレイ
			const RectF overlay = m_cmdPanel.stretched(-2);
			overlay.draw(ColorF(0.0, 0.65));

			// 省略号
			const int dots = static_cast<int>(Scene::Time() * 3.0) % 4;
			String dotsStr(dots, U'.');

			const Vec2 center = overlay.center();
			m_font(U"Resolving" + dotsStr).drawAt(center.movedBy(0, -6), Palette::White);
			m_font(U"処理中…操作できません" + dotsStr).drawAt(center.movedBy(0, 16), Palette::Gray);
		}

		// --- Button Draw Logic Done ---

		// --- ログパネル ---
		m_logPanel.draw(ColorF(0, 0.1)).drawFrame(2, 0, Palette::White);
		m_bigFont(U"Battle Log").draw(m_logPanel.x + 10, m_logPanel.y + 5);

		const int    innerLeft = 10;
		const int    innerTop = 50;
		const int    innerRight = 10;
		const double usableWidth = m_logPanel.w - innerLeft - innerRight;

		// ログの改行処理
		Array<String> folded;
		folded.reserve(m_battleSystem->log.size() * 2);
		for (const auto& raw : m_battleSystem->log)
		{
			const auto parts = wrapLine(m_logFont, raw, usableWidth);
			folded.insert(folded.end(), parts.begin(), parts.end());
		}

		const int maxLines = static_cast<int>((m_logPanel.h - innerTop - 10) / kLogLineHeight); // 若干の余白を追加
		const int beginIdx = static_cast<int>(Max<int>(0, folded.size() - maxLines));

		// ログ描画 (パネル内部座標基準)
		{
			const ScopedViewport2D viewport(m_logPanel.asRect()); // パネル領域にビューポートを制限
			int y = innerTop;
			for (int i = beginIdx; i < folded.size(); ++i)
			{
				m_logFont(folded[i]).draw(innerLeft, y);
				y += kLogLineHeight;
				// [修正] パネル下部を超えないように y 座標をチェック
				if (y >= (m_logPanel.h - 10)) break;
			}
		}
	}

	// --- 戦闘終了メッセージ ---
	if (m_waitingForPopup && m_battleSystem)
	{
		drawBattleOverPopup(m_battleSystem->m_resultMessage);
	}
}

// [修正] 経験値返還関数
double BattleManager::GetExperienceYield() const
{
	return m_expYield;
}

// [修正] ログ改行関数 (static メンバー関数)
Array<String> BattleManager::wrapLine(const Font& f, const String& text, double maxWidth)
{
	Array<String> lines;
	if (text.isEmpty())
	{
		lines << U"";
		return lines;
	}

	String cur;
	double w = 0.0;

	for (const auto& g : f.getGlyphs(text))
	{
		// [修正] 改行文字 ('\n') 処理を追加
		if (g.codePoint == U'\n') {
			lines << cur;
			cur.clear();
			w = 0.0;
			continue;
		}

		const double adv = g.xAdvance;
		// [修正] 幅超過で強制改行する前に、現在の単語(cur)が空でないか確認
		if ((w + adv) > maxWidth && !cur.isEmpty())
		{
			lines << cur;
			cur.clear();
			w = 0.0;
		}
		cur.push_back(g.codePoint);
		w += adv;
	}

	if (!cur.isEmpty()) lines << cur;
	return lines;
}

void BattleManager::drawCommandUI() const
{
	// --- コマンドパネル ---
	m_cmdPanel.draw(ColorF(0, 0.1)).drawFrame(2, 0, Palette::White);
	m_bigFont(U"Commands").draw(m_cmdPanel.x + 10, m_cmdPanel.y + 5);

	// 操作可能な状態か確認 (コマンド選択フェーズ + 両者生存)
	const bool canOperate =
		m_battleSystem
		&& (m_battleSystem->phase == Phase::CommandSelect)
		&& m_battleSystem->player.isAlive()
		&& m_battleSystem->enemy.isAlive();

	// --- コマンドボタン処理 ---
	if (!m_itemMenuOpen)
	{
		if (SimpleGUI::Button(U"物理攻撃", m_cmdBase, m_cmdButtonWidth, canOperate))
		{
			const_cast<BattleManager*>(this)->m_battleSystem->player.setSelectedType(ActionType::Physical);
		}
		if (SimpleGUI::Button(U"魔法攻撃", m_cmdBase.movedBy(170, 0), m_cmdButtonWidth, canOperate))
		{
			const_cast<BattleManager*>(this)->m_battleSystem->player.setSelectedType(ActionType::Magical);
		}
		if (SimpleGUI::Button(U"防御", m_cmdBase.movedBy(0, 60), m_cmdButtonWidth, canOperate))
		{
			const_cast<BattleManager*>(this)->m_battleSystem->player.setSelectedType(ActionType::Defend);
		}
		if (SimpleGUI::Button(U"アイテム", m_cmdBase.movedBy(170, 60), m_cmdButtonWidth, canOperate))
		{
			// [修正] TODO: 実際のアイテムインベントリとの連携が必要
			// 現在は仮に2つのアイテム(Potion, Ether)をトグル
			const_cast<BattleManager*>(this)->m_itemMenuOpen = true;
		}
	}
	else
	{
		drawItemMenu(canOperate);
	}

	// 選択中のアイテム表示
	if (m_battleSystem->player.getSelectedTypeOr() == ActionType::Item)
	{
		const int itemIndex = m_battleSystem->player.getItemIndexOr(0);
		if (InRange(itemIndex, 0, (int)m_battleSystem->items.size() - 1))
		{
			m_font(U"選択中：{}"_fmt(m_battleSystem->items[itemIndex].name))
				.draw(m_cmdPanel.x + 10, m_cmdPanel.y + 150, Palette::Aqua);
		}
	}
}

void BattleManager::drawItemMenu(bool canOperate) const
{
	// item selecting menu
	const Vec2 menuPos = m_cmdPanel.pos.movedBy(10, 70);
	const double w = m_cmdPanel.w - 16;
	const double h = 100;

	RoundRect(menuPos.x, menuPos.y, w, h, 8).draw(ColorF(0, 0.6));
	m_font(U"アイテムを選択してください").draw(menuPos.movedBy(0, -28), Palette::Aqua);

	const Vec2 hpBtnPos = menuPos.movedBy(12, 12);
	const Vec2 mpBtnPos = menuPos.movedBy(12 + (w - 24) * 0.5, 12);
	const Vec2 cancelPos = menuPos.movedBy(w - 150 - 20, h - 44);

	bool clickedHP = SimpleGUI::Button(U"HP回復薬（HP+30）", hpBtnPos, (w - 36) * 0.5, canOperate);
	bool clickedMP = SimpleGUI::Button(U"MP回復薬（MP+15）", mpBtnPos, (w - 36) * 0.5, canOperate);
	bool clickedCancel = SimpleGUI::Button(U"キャンセル", cancelPos, 150, true);

	if (clickedHP || clickedMP)
	{
		// for testing
		const int targetIndex = clickedHP ? 0 : 1;

		if ((int)m_battleSystem->items.size() <= targetIndex) {
			const_cast<BattleManager*>(this)->m_battleSystem->log << U"[System] テスト用アイテムが未登録です (index {})"_fmt(targetIndex);
			const_cast<BattleManager*>(this)->m_itemMenuOpen = false;
		}
		else {
			const_cast<BattleManager*>(this)->m_battleSystem->player.setSelectedType(ActionType::Item);
			const_cast<BattleManager*>(this)->m_battleSystem->player.setSelectedItem(targetIndex);
			const_cast<BattleManager*>(this)->m_itemMenuOpen = false;
		}
	}
	else if (clickedCancel)
	{
		const_cast<BattleManager*>(this)->m_itemMenuOpen = false;
	}
}

void BattleManager::drawBattleOverPopup(const String& message) const
{
	const RectF rect(Arg::center = Scene::Center(), 300, 100);
	rect.draw(ColorF(0.1, 0.1, 0.1, 0.9)).drawFrame(2, Palette::White);

	m_bigFont(message).drawAt(rect.center().movedBy(0, -10), Palette::White);
	m_font(U"Z : 確認").draw(Arg::bottomRight(rect.br().movedBy(-15, -10)), Palette::White);
}
