#pragma once
#include <Siv3D.hpp>
#include "MapEnemyType.h"


class MapCollider; // fwd
class MapEnemyNavi; //fwd

class MapEnemy {
public:
	enum class State { Idle, Chase };


	MapEnemy() = default;


	// 初始化：绑定类型与出生点
	void init(MapEnemyKind kind, const s3d::Vec2& spawnWorld, const MapEnemyType& typeRef);


	// 基础信息
	const s3d::Vec2& center() const { return m_center; }
	void setCenter(const s3d::Vec2& c) { m_center = c; }
	s3d::RectF aabb() const { return s3d::RectF{ m_center - m_size * 0.5, m_size }; }


	// 逻辑
	void updateThink(const s3d::Vec2& playerPos, double dt); // 仅产生“期望位移”
	s3d::Vec2 desiredDelta() const { return m_desired; }
	void postCollisionApply(const s3d::Vec2& allowedDelta) { m_center += allowedDelta; }


	// 绘制
	void draw() const;


	// 访问
	State state() const { return m_state; }
	const MapEnemyType* type() const { return m_type; }
	MapEnemyKind kind() const { return m_kind; }


	// 触发战斗的占位：由系统统一收集并回调到场景（本阶段仅接口）
	bool isBattleTriggered() const { return m_battleTriggered; }
	void clearBattleTrigger() { m_battleTriggered = false; }

	//Navigate
	void bindNavi(const MapEnemyNavi* navi) { m_navi = navi; }
	void onDynamicLayerChanged(uint32 ver) { m_seenDynamicVersion = ver; }


private:
	MapEnemyKind m_kind{ MapEnemyKind::Slime };
	const MapEnemyType* m_type{ nullptr }; // 指向 DB 内部数据


	s3d::Vec2 m_center{ 0,0 };
	s3d::SizeF m_size{ 24,24 }; // 简易 AABB（可做成随贴图尺寸调整）


	State m_state{ State::Idle };
	s3d::Vec2 m_desired{ 0,0 }; // 本帧期望位移（交由碰撞修正）

	//navigate
	const MapEnemyNavi* m_navi = nullptr;
	s3d::Array<s3d::Vec2> m_path;
	size_t  m_waypoint = 0;
	double  m_repathCD = 0.0;
	uint32  m_seenDynamicVersion = 0;

	bool m_battleTriggered{ false };
};
