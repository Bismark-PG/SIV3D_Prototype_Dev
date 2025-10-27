/*==============================================================================

	In Game Battle System [BattleSystem.cpp]

	Author : Team Re:ing >> Ko

	Note :

==============================================================================*/
#include "stdafx.h"
#include "BattleSystem.h"

// ダメージ計算ツール
namespace Formula
{
	// 当たり判定
	bool hitCheck(const Character& a, const Character& b)
	{
		// TODO : Need Stats
		return Random(1, 100) <= 95; // 命中ディフォルト95%
	}

	// 物理攻撃公式
	int physicalDamage(const Character& a, const Character& b, int power = 10)
	{
		double base = a.getAttack() - b.getDefense() * 0.5;
		base = Max(1.0, base) * (power / 10.0);
		// TODO : クリティカル判定
		double rand = Random(0.85, 1.0);
		return Max(1, static_cast<int>(base * rand));
	}

	// 魔法攻撃公式
	int magicDamage(const Character& a, const Character& b, int power = 12)
	{
		double base = a.getMagicalAttack() - b.getMagicalDefense() * 0.5;
		base = Max(1.0, base) * (power / 10.0);
		double rand = Random(0.90, 1.05);
		return Max(1, static_cast<int>(base * rand));
	}
}


// [修正] コンストラクタ: BattlePlayer と BattleEnemy を受け取るように変更
BattleSystem::BattleSystem(BattlePlayer p, BattleEnemy e)
	: player(std::move(p)), enemy(std::move(e))
{
	// [修正] Item の使用対象を Player -> BattlePlayer に変更
	items.push_back({ U"Potion(HP+30)", [](BattlePlayer& pl) {pl.healHP(30); } });
	items.push_back({ U"Ether(MP+15)",  [](BattlePlayer& pl) {pl.healMP(15); } });

	pushLog(U"{} があらわれた！"_fmt(enemy.getName())); // [修正] 戦闘開始ログを追加
}

void BattleSystem::pushLog(const String& s)
{
	log.push_back(s);

	// [修正] ログの最大行数を増加 (12 -> 15)
	if (log.size() > 15)
	{
		log.pop_front();
	}
}

void BattleSystem::nextTurn()
{
	++turn;
	phase = Phase::CommandSelect;

	player.setIsDefending(false);
	enemy.setIsDefending(false);

	// [修正] プレイヤーの選択リセットは EndCheck で行う方が安全
	// player.clearSelectedType();
	// player.clearSelectedItem();
}

void BattleSystem::updateOnce()
{
	// [修正] 戦闘終了時は何もしない
	if (phase == Phase::BattleOver) return;

	switch (phase)
	{
	case Phase::CommandSelect:
		// [修正] プレイヤーが倒れている場合はコマンド選択不可
		if (!player.isAlive()) {
			phase = Phase::EndCheck; // 바로 종료 체크로
			break;
		}

		if (!player.hasTypeSelected()) return; // まだコマンドを選択していない

		{
			auto paEnemy = enemy.chooseAction(player);
			auto paPlayer = player.chooseAction(enemy);

			// お互いの素早さ
			paEnemy.initiative = enemy.getAgility() + Random(0, 5);
			paPlayer.initiative = player.getAgility() + Random(0, 5);

			m_queue.clear();
			// [修正] 倒れているキャラクターは行動キューに追加しない
			if (enemy.isAlive()) m_queue << paEnemy;
			if (player.isAlive()) m_queue << paPlayer;

			std::sort(m_queue.begin(), m_queue.end(),
				[](const PlannedAction& a, const PlannedAction& b)
				{
					return a.initiative > b.initiative;
				});
		}

		phase = Phase::ActionResolve;
		break;

	case Phase::ActionResolve:
		if (m_queue.isEmpty())
		{
			phase = Phase::EndCheck;
		}
		else
		{
			auto act = m_queue.front();
			m_queue.pop_front();

			// [修正] 行動直前にアクターが生存しているか再確認
			if (act.actor && act.actor->isAlive())
			{
				resolveAction(act);
			}
			// [修正] アクション処理後にターゲットが倒れたか確認し、キューをクリア (不要なアクションを防止)
			if (act.target && !act.target->isAlive()) {
				m_queue.clear();
				phase = Phase::EndCheck; // すぐに終了チェックへ
			}
		}
		break;

	case Phase::EndCheck:
		if (!player.isAlive() || !enemy.isAlive())
		{
			// [修正] 戦闘終了状態に移行
			phase = Phase::BattleOver;
			if (player.isAlive()) {
				pushLog(U"{} をやっつけた！"_fmt(enemy.getName()));
			}
			else {
				pushLog(U"{} はまけてしまった..."_fmt(player.getName()));
			}
		}
		else // 次のターンの準備
		{
			player.clearSelectedType();
			player.clearSelectedItem();
			nextTurn();
		}
		break;

		// [修正] BattleOver 状態を追加 (何もしない)
	case Phase::BattleOver:
		break;
	}
}

void BattleSystem::resolveAction(const PlannedAction& pa)
{
	Character* actor = pa.actor;
	Character* target = pa.target;

	// [修正] この関数開始時のアクター生存確認は updateOnce で行われるため削除可能
	// if (!actor->isAlive()) return; 

	switch (pa.type)
	{
	case ActionType::Physical:
	{
		if (!Formula::hitCheck(*actor, *target))
		{
			pushLog(U"{} の攻撃ははずれた！"_fmt(actor->getName()));
			break;
		}

		int dmg = Formula::physicalDamage(*actor, *target, 12);
		target->takeDamage(dmg);
		pushLog(U"{} が物理攻撃！ {} に {} のダメージ！（{}/{}）"_fmt(
			actor->getName(), target->getName(), dmg, target->getHP(), target->getHPMax()
		));
		break;
	}

	case ActionType::Magical:
	{
		if (!actor->spendMP(5))
		{
			pushLog(U"{} のMPが足りない！"_fmt(actor->getName()));
			// [修正] MP不足時は物理攻撃に切り替えず、行動失敗として処理
			// PlannedAction fallback = pa;
			// fallback.type = ActionType::Physical;
			// resolveAction(fallback);
			break;
		}

		if (!Formula::hitCheck(*actor, *target))
		{
			pushLog(U"{} の魔法攻撃ははずれた！"_fmt(actor->getName()));
			break;
		}

		int dmg = Formula::magicDamage(*actor, *target, 14);
		target->takeDamage(dmg);
		pushLog(U"{} が魔法攻撃！ {} に {} のダメージ！（{}/{}）"_fmt(
			actor->getName(), target->getName(), dmg, target->getHP(), target->getHPMax()
		));
		break;
	}

	case ActionType::Defend:
	{
		actor->setIsDefending(true);
		pushLog(U"{} は防御している！"_fmt(actor->getName()));
		break;
	}

	case ActionType::Item:
	{
		// [修正] 敵はアイテム使用不可 (BattleEnemy::chooseActionで既に防止済み)
		// if (!actor->getIsPlayer()) { ... } 제거

		// [修正] actor を BattlePlayer へ安全にキャスト
		BattlePlayer* pl = dynamic_cast<BattlePlayer*>(actor);
		if (!pl || !pl->hasItemSelected() || !InRange(pl->getItemIndexOr(-1), 0, static_cast<int>(items.size() - 1)))
		{
			pushLog(U"アイテム選択が無効！");
			break;
		}

		const auto itemIndex = pl->getItemIndexOr();
		items[itemIndex].use(*pl);
		pushLog(U"{} は {} を使った！ (HP: {}/{}, MP: {}/{})"_fmt(
			actor->getName(), items[itemIndex].name, pl->getHP(), pl->getHPMax(), pl->getMP(), pl->getMPMax()
		));
		// [修正] アイテム使用後に選択を解除
		pl->clearSelectedItem();
		pl->clearSelectedType(); // アイテム使用は即時発動のため、タイプ選択も解除
		break;
	}
	default: // [修正] 万が一のための例外処理
		pushLog(U"未定義のアクションタイプ！");
		break;
	}
}

// [修正] 戦闘終了確認用の関数
bool BattleSystem::isBattleEnded() const
{
	return phase == Phase::BattleOver;
}

// [修正] プレイヤー勝利確認用の関数
bool BattleSystem::isPlayerWinner() const
{
	// 戦闘が終了しており、プレイヤーが生存していれば勝利
	return isBattleEnded() && player.isAlive();
}

