/*==============================================================================

	ゲームのリソース管理 [Game_Resource_Manager.h]

	Author : Team Re:ing >> Michito Kunii

	Date : 2025/10/23

	Note : ゲームのリソース管理

==============================================================================*/
#pragma once

class GameResourceManager
{
public:

	struct ItemData
	{
		String name;
		String description;
		Texture texture;
	};

	void load();
	Optional<String> update(const Circle& playerHitbox);
	void draw() const;
	void spawnItem(const String& key, const Vec2& pos);
	int32 getInventoryItemCount(const String& key) const;
	const ItemData& getItemData(const String& key) const;
	bool decreaseItemCount(const String& key);

private:

	struct ActiveItem
	{
		String key;
		Vec2 pos;
		Circle collision;
	};

	HashTable<String, ItemData> m_itemDatabase;
	Array<ActiveItem> m_activeItems;
	HashTable<String, int32> m_playerInventory;
};
