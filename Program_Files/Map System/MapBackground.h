#pragma once
#include <Siv3D.hpp>

// EN: Draw-only background of a Tiled map (tile layers).
//     Supports multiple tilesets: atlas & image-collection.
// JP: Tiled のタイルレイヤ描画専用モジュール。
//     複数タイルセット（アトラス／画像コレクション）に対応。

class MapBackground {
public:
	// EN: Load map (tile layers + tilesets). Supports inline and external .tsx.
	// JP: マップ（タイルレイヤ＋タイルセット）を読み込み。.tsx 外部参照も対応。
	bool loadFromTiledJSON(const s3d::FilePath& path);

	// EN: Draw with given camera (RAII transformer inside).
	// JP: 指定カメラで描画（内部でトランスフォーマを適用）。
	void draw(const s3d::Camera2D& cam) const;

	// EN: World bounds in pixels (map size).
	// JP: ワールド境界（ピクセル単位）。
	s3d::RectF worldBounds() const;

	s3d::Size  mapTileCount() const;  // map width/height in tiles
	s3d::Point tileSize()     const;  // per-tile size (px)

private:
	struct Tileset {
		enum class Kind { Atlas, ImageCollection };
		int           firstGID{};
		Kind          kind{ Kind::Atlas };

		// Atlas tileset
		s3d::Texture  atlas;
		int           tileW{ 0 }, tileH{ 0 };
		int           cols{ 0 }, rows{ 0 };

		// Image collection tileset
		s3d::Array<s3d::Texture> images; // index == local ID
	};

	struct TileLayer {
		s3d::String name;
		bool        visible{ true };
		int         width{ 0 }, height{ 0 };
		s3d::Array<s3d::uint32> gids; // 0 = empty (row-major)
	};

	int                findTileset(s3d::uint32 gid) const;
	static s3d::uint32 decodeGID(s3d::uint32 gid);

private:
	s3d::Array<Tileset>   m_tilesets;
	s3d::Array<TileLayer> m_layers;

	s3d::Size  m_mapTiles{ 0,0 };
	int        m_tileW{ 32 }, m_tileH{ 32 };
};
