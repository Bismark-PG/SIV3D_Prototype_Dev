/*==============================================================================

	Manage Baattle System [Battle_Manager.h]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#include "stdafx.h"
#include "Battle_Manager.h"

constexpr int kLogLineHeight = 16; // ログの行の高さ

BattleManager::BattleManager()
	: m_font(20)
	, m_bigFont(28, Typeface::Bold)
	, m_logFont(kLogLineHeight)
	, m_cmdPanel(20, 340, 420, 220)   // コマンドパネルの位置/サイズ
	, m_logPanel(480, 340, 460, 220)  // ログパネルの位置/サイズ
	, m_cmdBase(m_cmdPanel.pos.movedBy(20, 60)) // コマンドボタンの開始位置
	, m_cmdButtonWidth(180.0)         // コマンドボタンの幅
{
	// コンストラクタで特にすることはない
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
	// BattleSystem がなければ (戦闘開始前) 何もしない
	if (!m_battleSystem) return BattleResult::InProgress;

	// すでに戦闘が終了していれば結果を返す
	if (m_result != BattleResult::InProgress) return m_result;

	// 操作可能な状態か確認 (コマンド選択フェーズ + 両者生存)
	const bool canOperate = (m_battleSystem->phase == Phase::CommandSelect)
		&& m_battleSystem->player.isAlive()
		&& m_battleSystem->enemy.isAlive();

	// --- コマンドボタン処理 ---
	if (SimpleGUI::Button(U"物理攻撃", m_cmdBase, m_cmdButtonWidth, canOperate))
	{
		m_battleSystem->player.setSelectedType(ActionType::Physical);
	}
	if (SimpleGUI::Button(U"魔法攻撃（MP-5）", m_cmdBase.movedBy(200, 0), m_cmdButtonWidth, canOperate))
	{
		m_battleSystem->player.setSelectedType(ActionType::Magical);
	}
	if (SimpleGUI::Button(U"防御", m_cmdBase.movedBy(0, 60), m_cmdButtonWidth, canOperate))
	{
		m_battleSystem->player.setSelectedType(ActionType::Defend);
	}
	if (SimpleGUI::Button(U"アイテム", m_cmdBase.movedBy(200, 60), m_cmdButtonWidth, canOperate))
	{
		// [修正] TODO: 実際のアイテムインベントリとの連携が必要
		// 現在は仮に2つのアイテム(Potion, Ether)をトグル
		m_battleSystem->player.setSelectedType(ActionType::Item);
		int currentItem = m_battleSystem->player.getItemIndexOr(-1);
		int nextItem = (currentItem + 1) % m_battleSystem->items.size(); // アイテム数で循環
		m_battleSystem->player.setSelectedItem(nextItem);
	}

	// --- 実行 / 次へ ボタン処理 ---
	const bool needCommand = (m_battleSystem->phase == Phase::CommandSelect)
		&& m_battleSystem->player.isAlive()
		&& m_battleSystem->enemy.isAlive();
	const String actionLabel = needCommand ? U"実行" : U"次へ";

	// [수정] Draw 함수에서 버튼을 그리므로 여기서는 Update 로직만 남김
	if (SimpleGUI::Button(actionLabel, Vec2(m_cmdPanel.x + 120, m_cmdPanel.y + 170), 120,
		(m_battleSystem->player.hasTypeSelected() || m_battleSystem->phase != Phase::CommandSelect)
		&& (m_battleSystem->player.isAlive() || m_battleSystem->enemy.isAlive()) // どちらかが生きていれば進行可能
		&& m_battleSystem->phase != Phase::BattleOver // まだ終了していなければ
	))
	{
		m_battleSystem->updateOnce();

		// -> BattleSystem::EndCheckで次のターン開始時に解除するように変更
	}

	// --- 戦闘終了チェック ---
	if (m_battleSystem->isBattleEnded())
	{
		if (m_battleSystem->isPlayerWinner())
		{
			m_result = BattleResult::PlayerWin;
			m_expYield = m_battleSystem->enemy.getExpYield(); // 経験値を保存
			// [修正] TODO: アイテムドロップ処理
		}
		else
		{
			m_result = BattleResult::PlayerLose;
		}
		m_battleSystem.reset(); // 戦闘終了時に BattleSystem オブジェクトを破棄
	}

	return m_result;
}

void BattleManager::Draw() const
{
	// BattleSystem がなければ何も描画しない
	if (!m_battleSystem) return;

	// --- キャラクターステータス表示 ---
	m_bigFont(U"Turn {}"_fmt(m_battleSystem->turn)).draw(20, 20);
	Line(Vec2(20, 60), Vec2(Scene::Width() - 20, 60)).draw(2);

	// Player Status
	m_font(U"[{}] Lv{}"_fmt(m_battleSystem->player.getName(), m_battleSystem->player.getLevel())).draw(20, 80);
	m_font(U"HP {}/{} MP {}/{}"_fmt(
		m_battleSystem->player.getHP(), m_battleSystem->player.getHPMax(),
		m_battleSystem->player.getMP(), m_battleSystem->player.getMPMax()
	)).draw(20, 110);

	// Enemy Status
	const double enemyX = Scene::Width() - 220;
	m_font(U"[{}] Lv{}"_fmt(m_battleSystem->enemy.getName(), m_battleSystem->enemy.getLevel())).draw(enemyX, 80);
	m_font(U"HP {}/{} MP {}/{}"_fmt(
		m_battleSystem->enemy.getHP(), m_battleSystem->enemy.getHPMax(),
		m_battleSystem->enemy.getMP(), m_battleSystem->enemy.getMPMax()
	)).draw(enemyX, 110);

	// --- コマンドパネル ---
	m_cmdPanel.draw(ColorF(0, 0.1)).drawFrame(2, 0, Palette::White);
	m_bigFont(U"Commands").draw(m_cmdPanel.x + 10, m_cmdPanel.y + 10);

	// [Pluse] Button Draw Logic
	const bool canOperate = (m_battleSystem->phase == Phase::CommandSelect)
		&& m_battleSystem->player.isAlive()
		&& m_battleSystem->enemy.isAlive();

	SimpleGUI::Button(U"物理攻撃", m_cmdBase, m_cmdButtonWidth, canOperate);
	SimpleGUI::Button(U"魔法攻撃（MP-5）", m_cmdBase.movedBy(200, 0), m_cmdButtonWidth, canOperate);
	SimpleGUI::Button(U"防御", m_cmdBase.movedBy(0, 60), m_cmdButtonWidth, canOperate);
	SimpleGUI::Button(U"アイテム", m_cmdBase.movedBy(200, 60), m_cmdButtonWidth, canOperate);

	const bool needCommand = (m_battleSystem->phase == Phase::CommandSelect)
		&& m_battleSystem->player.isAlive()
		&& m_battleSystem->enemy.isAlive();
	const String actionLabel = needCommand ? U"実行" : U"次へ";

	SimpleGUI::Button(actionLabel, Vec2(m_cmdPanel.x + 120, m_cmdPanel.y + 170), 120,
		(m_battleSystem->player.hasTypeSelected() || m_battleSystem->phase != Phase::CommandSelect)
		&& (m_battleSystem->player.isAlive() || m_battleSystem->enemy.isAlive())
		&& m_battleSystem->phase != Phase::BattleOver);
	// --- Button Draw Logic Done ---


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

	// --- ログパネル ---
	m_logPanel.draw(ColorF(0, 0.1)).drawFrame(2, 0, Palette::White);
	m_bigFont(U"Battle Log").draw(m_logPanel.x + 10, m_logPanel.y + 10);

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

	// --- 戦闘終了メッセージ ---
	// [修正] BattleSystem の内部状態の代わりに BattleManager の m_result を使用
	if (m_result == BattleResult::PlayerWin)
	{
		m_bigFont(U"勝利！").drawAt(Scene::Center().movedBy(0, -40), Palette::Yellow);
		// [修正] TODO: 経験値/アイテム獲得結果の表示を追加
		m_font(U"経験値 {} を獲得"_fmt(static_cast<int>(m_expYield))).drawAt(Scene::Center(), Palette::Yellow);
	}
	else if (m_result == BattleResult::PlayerLose)
	{
		m_bigFont(U"失敗...").drawAt(Scene::Center().movedBy(0, -40), Palette::Red);
		// [修正] TODO: ゲームオーバー処理の UI/ロジックが必要
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

