/*==============================================================================

	ゲームのリソース管理 [Game_Resource_Manager.cpp]

	Author : Team Re:ing >> Michito Kunii

	Date : 2025/10/23

	Note : ゲームのリソース管理

==============================================================================*/
#include "stdafx.h" 

void GameResourceManager::load()
{
	//m_itemDatabase.emplace(U"star",
	//	ItemData{ U"スター", U"10秒間、無敵になる", TextureAsset(U"Item_Star") });

	//m_itemDatabase.emplace(U"box",
	//	ItemData{ U"ハテナボックス", U"攻撃力が 5 上昇する", TextureAsset(U"Item_Box") });

	//m_itemDatabase.emplace(U"ramen",
	//	ItemData{ U"ラーメン", U"HPが 30 回復する", TextureAsset(U"Item_Ramen") });
}

void GameResourceManager::spawnItem(const String& key, const Vec2& pos)
{
	if (not m_itemDatabase.contains(key)) return;
	ActiveItem newItem;
	newItem.key = key; newItem.pos = pos;
	newItem.collision = Circle{ pos, 45 };
	m_activeItems.push_back(newItem);
}

Optional<String> GameResourceManager::update(const Circle& playerHitbox)
{
	Optional<String> nearestItemKey = none;

	// アイテムの更新処理
	m_activeItems.remove_if([&](const ActiveItem& item) {

		const bool isNear = item.collision.intersects(playerHitbox);

		// もし近くにいたら、そのキーを保存
		if (isNear)
		{
			nearestItemKey = item.key;
		}

		if (isNear)
		{
			// インベントリに追加
			if (KeyE.down())
			{
				if (m_playerInventory.contains(item.key))
				{
					m_playerInventory[item.key] += 1;
				}
				else
				{
					m_playerInventory.emplace(item.key, 1);
				}
				return true;
			}
		}
		return false;
	});

	return nearestItemKey;
}

int32 GameResourceManager::getInventoryItemCount(const String& key) const
{
	if (m_playerInventory.contains(key)) return m_playerInventory.at(key);
	else return 0;
}

const GameResourceManager::ItemData& GameResourceManager::getItemData(const String& key) const
{
	return m_itemDatabase.at(key);
}

bool GameResourceManager::decreaseItemCount(const String& key)
{
	if (m_playerInventory.contains(key))
	{
		if (m_playerInventory.at(key) > 0)
		{
			m_playerInventory[key] -= 1;
			return true;
		}
	}
	return false;
}

void GameResourceManager::draw() const
{
	for (const auto& item : m_activeItems)
	{
		const Texture& texture = m_itemDatabase.at(item.key).texture;
		texture.resized(80).drawAt(item.pos);
	}
}
