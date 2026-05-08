#include "stdafx.h"
#include "MapBackground.h"
#include <algorithm> // std::max

using namespace s3d;

namespace
{
	// EN: Minimal TSX parser by text scanning.
	//     If <tile> blocks exist with nested <image>, treat as Image Collection.
	//     Otherwise, expect single <image source="..."> as Atlas.
	// JP: TSX をテキスト走査で簡易解析。
	//     <tile>…<image> があれば画像コレクション、なければ単一 <image> のアトラスとみなす。
	struct TSXParseResult {
		bool isImageCollection{ false };
		int  tileW{ 0 }, tileH{ 0 };
		int  cols{ 0 }, rows{ 0 };           // rows will be derived later for atlas
		FilePath atlasImageAbs;              // atlas absolute image path
		Array<FilePath> imageListAbs;        // image-collection absolute image paths
	};

	bool ParseTSXByText(const FilePath& tsxPath, TSXParseResult& out)
	{
		TextReader reader{ tsxPath };
		if (!reader) {
#if defined(DEBUG) || defined(_DEBUG)
			Console << U"[MapBackground] Failed to open TSX: " << tsxPath;
#endif
			return false;
		}

		String xml, line;
		while (reader.readLine(line)) { xml += line; xml += U'\n'; }

		auto extractAttr = [&](StringView key, const String& src, String& outStr) -> bool {
			const String pat = Format(key, U"=\"");
			const size_t pos = src.indexOf(pat);
			if (pos == String::npos) return false;
			const size_t start = pos + pat.size();
			const size_t end = src.indexOf(U"\"", start);
			if (end == String::npos) return false;
			outStr = src.substr(start, end - start);
			return true;
			};

		// <tileset ... tilewidth="" tileheight="" [columns=""] ...>
		{
			String v;
			if (!extractAttr(U"tilewidth", xml, v)) {
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] TSX missing tilewidth: " << tsxPath;
#endif
				return false;
			}
			out.tileW = Parse<int>(v);

			if (!extractAttr(U"tileheight", xml, v)) {
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] TSX missing tileheight: " << tsxPath;
#endif
				return false;
			}
			out.tileH = Parse<int>(v);

			if (extractAttr(U"columns", xml, v)) {
				out.cols = Parse<int>(v);
			}
		}

		// Image-collection detection
		if (xml.indexOf(U"<tile") != String::npos) {
			out.isImageCollection = true;

			size_t pos = 0;
			while (true) {
				const size_t tilePos = xml.indexOf(U"<tile", pos);
				if (tilePos == String::npos) break;
				const size_t tileEnd = xml.indexOf(U"</tile>", tilePos);
				const size_t searchEnd = (tileEnd == String::npos ? xml.size() : tileEnd);

				const String block = xml.substr(tilePos, searchEnd - tilePos);

				// 1) <tile image="..."> (rare)
				String src;
				if (extractAttr(U"image", block, src)) {
					const FilePath abs = FileSystem::PathAppend(FileSystem::ParentPath(tsxPath), src);
					out.imageListAbs << abs;
				}
				else {
					// 2) <tile><image source="..."/></tile> (usual)
					const size_t imgPos = block.indexOf(U"<image");
					if (imgPos != String::npos) {
						const String imgTag = block.substr(imgPos);
						String src2;
						if (extractAttr(U"source", imgTag, src2)) {
							const FilePath abs2 = FileSystem::PathAppend(FileSystem::ParentPath(tsxPath), src2);
							out.imageListAbs << abs2;
						}
					}
				}
				pos = (tileEnd == String::npos ? searchEnd : tileEnd + 7);
			}

			if (out.imageListAbs.empty()) {
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] TSX image-collection: no images found: " << tsxPath;
#endif
				return false;
			}
			return true;
		}

		// Atlas tileset: single <image source="...">
		String imageRel;
		if (!extractAttr(U"source", xml, imageRel)) {
#if defined(DEBUG) || defined(_DEBUG)
			Console << U"[MapBackground] TSX atlas has no <image source>: " << tsxPath;
#endif
			return false;
		}
		out.isImageCollection = false;
		out.atlasImageAbs = FileSystem::PathAppend(FileSystem::ParentPath(tsxPath), imageRel);
		return true; // rows derived after loading texture
	}
}

// ----------------- helpers -----------------

int MapBackground::findTileset(uint32 gid) const
{
	if (gid == 0) return -1;
	int idx = -1;
	for (int i = 0; i < (int)m_tilesets.size(); ++i) {
		if ((int)gid >= m_tilesets[i].firstGID) idx = i;
		else break;
	}
	return idx;
}

uint32 MapBackground::decodeGID(uint32 gid)
{
	// EN: Strip Tiled flip flags (H/V/D). Flipping not handled in this minimal version.
	// JP: 反転フラグを除去（最小版では反転描画は未対応）。
	return (gid & 0x1FFFFFFF);
}

// ----------------- load -----------------

bool MapBackground::loadFromTiledJSON(const FilePath& path)
{
	m_tilesets.clear();
	m_layers.clear();

	const JSON json = JSON::Load(path);
	if (!json) {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"[MapBackground] Failed to load JSON: " << path;
#endif
		return false;
	}

	const String jtype = json[U"type"].getOr<String>(U"(no type)");
	if (jtype != U"map") {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"[MapBackground] Invalid JSON type (expected 'map'): " << jtype;
#endif
		return false;
	}

	if (!(json.hasElement(U"tilewidth") && json.hasElement(U"tileheight")
		&& json.hasElement(U"width") && json.hasElement(U"height"))) {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"[MapBackground] Missing required fields (tilewidth/tileheight/width/height).";
#endif
		return false;
	}

	m_mapTiles = Size{ json[U"width"].get<int>(), json[U"height"].get<int>() };
	m_tileW = json[U"tilewidth"].get<int>();
	m_tileH = json[U"tileheight"].get<int>();

	// ---- tilesets ----
	const auto tilesets = json[U"tilesets"];
	if (!tilesets || !tilesets.isArray()) {
#if defined(DEBUG) || defined(_DEBUG)
		Console << U"[MapBackground] 'tilesets' missing or not an array.";
#endif
		return false;
	}

	for (size_t i = 0; i < tilesets.size(); ++i)
	{
		const auto& ts = tilesets[i];

		Tileset t;
		t.firstGID = ts[U"firstgid"].getOr<int>(0);
		if (t.firstGID <= 0) {
#if defined(DEBUG) || defined(_DEBUG)
			Console << U"[MapBackground] tileset[" << i << U"] invalid firstgid.";
#endif
			return false;
		}

		// Case A: Inline atlas tileset
		if (ts.hasElement(U"tilewidth") && ts.hasElement(U"tileheight") && ts.hasElement(U"image"))
		{
			t.kind = Tileset::Kind::Atlas;
			t.tileW = ts[U"tilewidth"].get<int>();
			t.tileH = ts[U"tileheight"].get<int>();
			const String imageRel = ts[U"image"].getString();
			const FilePath atlasAbs = FileSystem::PathAppend(FileSystem::ParentPath(path), imageRel);
#if defined(DEBUG) || defined(_DEBUG)
			Console << U"[MapBackground] tileset[" << i << U"] image: " << atlasAbs;
#endif
			t.atlas = s3d::Texture{ atlasAbs, s3d::TextureDesc::Unmipped };
			if (!t.atlas)
			{
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] Failed tileset image: " << atlasAbs;
#endif
				return false;
			}
			t.cols = (t.atlas.width() / Max(1, t.tileW));
			t.rows = (t.atlas.height() / Max(1, t.tileH));
			if (t.cols <= 0 || t.rows <= 0)
			{
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] Invalid atlas grid.";
#endif
				return false;
			}
		}
		// Case B: Inline image-collection tileset (tiles[] with image)
		else if (ts.hasElement(U"tiles") && !ts.hasElement(U"image"))
		{
			t.kind = Tileset::Kind::ImageCollection;
			t.tileW = ts[U"tilewidth"].get<int>();
			t.tileH = ts[U"tileheight"].get<int>();

			const auto tilesArr = ts[U"tiles"];
			if (!tilesArr || !tilesArr.isArray() || (tilesArr.size() == 0)) {
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] tileset[" << i << U"] has empty 'tiles'.";
#endif
				return false;
			}

			int maxID = -1;
			for (const auto& it : tilesArr.arrayView()) {
				maxID = std::max(maxID, it[U"id"].getOr<int>(-1));
			}
			if (maxID < 0) {
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] tileset[" << i << U"] no valid tile ids.";
#endif
				return false;
			}
			t.images.assign(maxID + 1, Texture{});

			for (const auto& it : tilesArr.arrayView()) {
				const int id = it[U"id"].get<int>();
				const String imgRel = it[U"image"].getString();
				const FilePath imgAbs = FileSystem::PathAppend(FileSystem::ParentPath(path), imgRel);
				s3d::Texture tex{ imgAbs, s3d::TextureDesc::Unmipped };
				if (!tex)
				{
#if defined(DEBUG) || defined(_DEBUG)
					Console << U"[MapBackground] Failed image-collection tile: " << imgAbs;
#endif
				return false;
				}
				t.images[id] = std::move(tex);
			}
		}
		// Case C: External .tsx (atlas or image-collection)
		else if (ts.hasElement(U"source"))
		{
			const FilePath tsxRel = ts[U"source"].getString();
			const FilePath tsxPath = FileSystem::PathAppend(FileSystem::ParentPath(path), tsxRel);

			TSXParseResult pr;
			if (!ParseTSXByText(tsxPath, pr)) return false;

			if (!pr.isImageCollection) {
				t.kind = Tileset::Kind::Atlas;
				t.tileW = pr.tileW; t.tileH = pr.tileH;
#if defined(DEBUG) || defined(_DEBUG)
				Console << U"[MapBackground] tileset[" << i << U"] image: " << pr.atlasImageAbs;
#endif
				t.atlas = Texture{ pr.atlasImageAbs };

				if (!t.atlas)
				{
#if defined(DEBUG) || defined(_DEBUG)
					Console << U"[MapBackground] Failed tileset image: " << pr.atlasImageAbs;
#endif
					return false;
				}
				t.cols = (pr.cols > 0 ? pr.cols : (t.atlas.width() / Max(1, t.tileW)));
				t.rows = (t.atlas.height() / Max(1, t.tileH));
				if (t.cols <= 0 || t.rows <= 0)
				{
#if defined(DEBUG) || defined(_DEBUG)
					Console << U"[MapBackground] Invalid atlas grid.";
#endif
				return false;
				}
			}
			else {
				t.kind = Tileset::Kind::ImageCollection;
				t.tileW = pr.tileW; t.tileH = pr.tileH;

				const int maxID = static_cast<int>(pr.imageListAbs.size()) - 1;
				t.images.assign(maxID + 1, Texture{});
				for (int id = 0; id <= maxID; ++id) {
					const FilePath imgAbs = pr.imageListAbs[id];
					Texture tex{ imgAbs };
					if (!tex)
					{
#if defined(DEBUG) || defined(_DEBUG)
						Console << U"[MapBackground] Failed image-collection tile: " << imgAbs;
#endif
						return false;
					}
					t.images[id] = std::move(tex);
				}
			}
		}
		else
		{
#if defined(DEBUG) || defined(_DEBUG)
			Console << U"[MapBackground] tileset[" << i << U"] unsupported structure (no image/source/tiles).";
#endif
			return false;
		}

		m_tilesets << t;
	}

	m_tilesets.sort_by([](const Tileset& a, const Tileset& b) { return a.firstGID < b.firstGID; });

	// ---- tile layers ----
	for (const auto& layer : json[U"layers"].arrayView())
	{
		const String type = layer[U"type"].getString();
		if (type != U"tilelayer") continue;

		TileLayer tl;
		tl.name = layer[U"name"].getString();
		tl.visible = layer[U"visible"].getOr<bool>(true);
		tl.width = layer[U"width"].get<int>();
		tl.height = layer[U"height"].get<int>();

		for (const auto& v : layer[U"data"].arrayView()) {
			tl.gids << static_cast<uint32>(v.get<uint32>());
		}
		m_layers << tl;
	}

	return true;
}

// ----------------- draw -----------------

void MapBackground::draw(const Camera2D& cam) const
{

	const s3d::ScopedRenderStates2D _sampler{ s3d::SamplerState::ClampNearest };

	// EN: Apply camera with RAII transformer
	// JP: RAII トランスフォーマでカメラ適用
	const auto t = cam.createTransformer();

	for (const auto& tl : m_layers)
	{
		if (!tl.visible) continue;

		for (int y = 0; y < tl.height; ++y)
			for (int x = 0; x < tl.width; ++x)
			{
				const uint32 raw = tl.gids[y * tl.width + x];
				if (raw == 0) continue;

				const uint32 gid = decodeGID(raw);
				const int tsIdx = findTileset(gid);
				if (tsIdx < 0) continue;
				const auto& ts = m_tilesets[tsIdx];

				const int local = static_cast<int>(gid) - ts.firstGID;
				if (local < 0) continue;

				const Vec2 dstPos{ x * m_tileW, y * m_tileH };

				if (ts.kind == Tileset::Kind::Atlas) {
					const int sx = (local % ts.cols) * ts.tileW;
					const int sy = (local / ts.cols) * ts.tileH;
					const Rect src{ sx, sy, ts.tileW, ts.tileH };
					ts.atlas(src).draw(dstPos); // NOTE: flip not handled in this minimal version
				}
				else { // Image Collection
					if (local >= (int)ts.images.size()) continue;
					const Texture& tex = ts.images[local];
					if (!tex) continue;
					// EN: Resize to map tile size for grid alignment.
					// JP: マップのタイルサイズに合わせてリサイズ描画。
					tex.resized(m_tileW, m_tileH).draw(dstPos);
				}
			}
	}
}

// ----------------- accessors -----------------

RectF MapBackground::worldBounds() const
{
	return RectF{ 0, 0,
		(double)(m_mapTiles.x * m_tileW),
		(double)(m_mapTiles.y * m_tileH) };
}

Size MapBackground::mapTileCount() const { return m_mapTiles; }
Point MapBackground::tileSize()     const { return { m_tileW, m_tileH }; }
