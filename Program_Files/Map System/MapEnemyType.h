#pragma once
#include <Siv3D.hpp>

// 与 Player 保持一致的朝向/状态
enum class EnemyState : int { Idle = 0, Chase = 1 };
enum class Facing4 : int { Down = 0, Right = 1, Left = 2, Up = 3 };

// 单个动画剪辑的“规格”（只描述资源与切法，不加载纹理）
struct EnemyAnimClipSpec {
	s3d::String assetName;   // 推荐用 TextureAsset 名，也可直接 png 路径
	s3d::Size   frame{ 0, 0 };
	int         count{ 0 };
	double      fps{ 8.0 };
	bool        loop{ true };
	int         row{ -1 };        // >=0: sheet 第几行；<0: strip（单行）
	int         colStart{ 0 };    // 从第几列开始切
};

// 8 个动画（状态×方向）
struct EnemyAnimSpec {
	EnemyAnimClipSpec clips[2][4]; // [EnemyState][Facing4]
	bool isValid() const {
		for (int s = 0; s < 2; ++s)
			for (int f = 0; f < 4; ++f) {
				const auto& c = clips[s][f];
				if (c.frame.x > 0 && c.frame.y > 0 && c.count > 0) return true;
			}
		return false;
	}
};

enum class MapEnemyKind : uint16 {
	Slime = 0,
	Bat,
	// TODO
};


struct MapEnemyType {
	s3d::String name; // name(for UI)
	s3d::Texture texture; // temp texture, after change to Sprite Anim
	double speed = 80.0; // chase speed（px/s）
	double alertRadius = 160.0; //alert radius to chase player
	double battleRadius = 30.0; // battle check radius（enter battle scene）

	// [Plus] Enemy Stats For Battle Scene
	int maxHP = 50;
	int attack = 5;
	int defense = 2;
	double expYield = 10.0;

	EnemyAnimSpec anim;
};


class MapEnemyTypeDB {
public:
	void clear() { m_table.clear(); }
	void set(MapEnemyKind k, MapEnemyType t) { m_table[k] = std::move(t); }
	const MapEnemyType* get(MapEnemyKind k) const {
		if (auto it = m_table.find(k); it != m_table.end()) return &it->second; return nullptr;
	}
private:
	s3d::HashTable<MapEnemyKind, MapEnemyType> m_table;
};
