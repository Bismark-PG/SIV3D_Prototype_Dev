#pragma once
#include <Siv3D.hpp>
#include <array>

// ==========================
// MapPlayer (4-dir Anim)
// ==========================
// EN: Player with 4-direction Idle/Move sprite animations.
// JP: 4方向の待機/移動アニメを持つプレイヤークラス。

class MapPlayer {
public:
	// ----- Public enums -----
	enum class PlayerState { Idle = 0, Move = 1 };     // EN: 2 states / JP: 2状態
	enum class Facing4 { Up = 0, Down, Left, Right }; // EN: 4 directions / JP: 4方向

	// ----- Animation meta (one clip) -----
	struct AnimClip {
		s3d::Texture atlas;     // EN: sprite sheet / JP: スプライトシート
		s3d::Size    frame{ 0, 0 };
		int          count{ 0 };   // EN: total frames / JP: 総フレーム数
		double       fps{ 6.0 };   // EN: frames per second / JP: フレームレート
		int          start{ 0 };   // EN: start index in sheet / JP: シート内の開始インデックス
		bool         loop{ true }; // EN: loop playback / JP: ループ再生

		bool valid() const { return atlas && frame.x > 0 && frame.y > 0 && count > 0; }
	};

	// ===== Basic control =====
	void setCenter(const s3d::Vec2& c);
	void setSize(const s3d::SizeF& s);
	void setSpeed(double s);

	const s3d::Vec2& center() const;
	s3d::RectF       aabb()   const; // EN: center-based AABB / JP: 中心基準の当たり判定
	double           speed()  const;

	// ===== Movement intent (for collision pipeline) =====
	// EN: Compute desired delta from keyboard input (WASD / Arrow) with speed * dt.
	// JP: 入力（矢印/WASD）から移動量を計算（speed * dt）。
	s3d::Vec2 calcDesiredDelta(double dt) const;

	// EN: Apply actual allowed movement after collision resolved.
	// JP: 衝突解決後の許可移動量を適用。
	void applyMove(const s3d::Vec2& allowedDelta);

	// ===== Animation setup helpers =====
	// EN: Direct set a clip (state x facing) / JP: 直接設定（状態×方向）
	void setClip(PlayerState st, Facing4 f, const AnimClip& clip);

	// EN: 1 PNG = 1 animation (horizontal strip)
	// JP: 1枚で1アニメ（横一列）
	void setClipFromStrip(PlayerState st, Facing4 f,
						  const s3d::FilePath& png,
						  s3d::Size frame, int count,
						  double fps, bool loop);

	// EN: Big sheet where one row is one animation
	// JP: 大シートの1行を1アニメとして切り出し
	void setClipFromSheetRow(PlayerState st, Facing4 f,
							 const s3d::FilePath& sheet,
							 s3d::Size frame,
							 int rowIndex, int count,
							 double fps, bool loop,
							 int colStart = 0);

	struct RowBinding {
		PlayerState st;
		Facing4     face;
		int         row;
		int         count;
		double      fps;
		bool        loop;
	};

	// EN: Bulk bind 8 clips from a single sheet via row bindings.
	// JP: 1枚シートから8本をまとめて登録
	void setClipsFromSheet8(const s3d::FilePath& sheet, s3d::Size frame,
							const std::array<RowBinding, 8>& rows,
							int colStart = 0);

	// ===== Convenience initializers (semantic sugar) =====
	// EN: Init all 8 clips from one sheet and set AABB from frame
	// JP: 1枚のシートから8本を一括設定し、当たり判定サイズをframeに
	void initAnimationsFromSheet8(const s3d::FilePath& sheet,
								  s3d::Size frame,
								  const std::array<RowBinding, 8>& rows,
								  int colStart = 0);

	// EN: Init one clip from a strip PNG (does not change AABB)
	// JP: ストリップPNGから1本のアニメを設定（AABBは変更しない）
	void initAnimationStrip(PlayerState st, Facing4 f,
							const s3d::FilePath& png,
							s3d::Size frame, int count,
							double fps, bool loop);

	// ===== Update & Draw =====
	// EN: Update animation state from input direction and dt (call once per frame).
	// JP: 入力方向とdtからアニメ更新（毎フレーム呼ぶ）
	void updateAnimation(double dt, const s3d::Vec2& inputDir);

	// EN: Draw current frame (center-based). Camera transform should be active in caller.
	// JP: 現在フレームを描画（中心基準）。カメラ適用は呼び出し側で行う。
	void draw() const;

private:
	// ----- Internals -----
	static Facing4  determineFacingFromVector(const s3d::Vec2& v, Facing4 fallback);
	static s3d::Rect calcSrcRect(const AnimClip& c, int frameIndex);

	void switchIfNeeded(PlayerState newState, Facing4 newFacing);
	void resetAnimator();

private:
	// Movement
	s3d::Vec2  m_center{ 64, 64 };
	s3d::SizeF m_size{ 20, 28 };
	double     m_speed{ 180.0 };

	// Animation bank [state][facing]
	AnimClip m_clips[2][4];

	// Animator runtime
	PlayerState m_state{ PlayerState::Idle };
	Facing4     m_facing{ Facing4::Down };
	Facing4     m_lastMoveFacing{ Facing4::Down };

	int     m_frameIndex{ 0 };
	double  m_timeAcc{ 0.0 }; // EN: accumulated time / JP: 経過時間

	bool    m_hasAnyClip{ false }; // EN: used to know if any clip is configured / JP: 設定済みか
};
