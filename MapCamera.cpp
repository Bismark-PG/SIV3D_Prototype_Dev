#include "stdafx.h"
#include "MapCamera.h"
#include <cmath> 
using namespace s3d;

void MapCamera::init(const Size& screenSize, const RectF& worldBounds, double scale) {
	setViewport(screenSize, scale);
	setWorldBounds(worldBounds);
}

void MapCamera::setViewport(const Size& screenSize, double scale) {
	m_scale = scale;
	m_cam = Camera2D{ Vec2{ 0,0 }, m_scale, Camera2DParameters::NoControl() };
	m_viewHalf = screenSize * (0.5 / m_scale);
	m_screenSize = screenSize;
}

void MapCamera::setWorldBounds(const RectF& bounds) {
	m_world = bounds;
}

static double SmoothDamp1(double current, double target,
						  double& currentVelocity, double smoothTime, double dt)
{
	// 经典 SmoothDamp（近似临界阻尼，无振荡）
	const double st = (smoothTime > 1e-4 ? smoothTime : 1e-4);
	const double omega = 2.0 / st;
	const double x = omega * dt;
	const double exp = 1.0 / (1.0 + x + 0.48 * x * x + 0.235 * x * x * x);

	const double change = current - target;
	const double temp = (currentVelocity + omega * change) * dt;
	currentVelocity = (currentVelocity - omega * temp) * exp;

	return target + (change + temp) * exp;
}

void MapCamera::setSoftDeadZone(const s3d::Vec2& halfSizeScreenPx)
{
	using s3d::Max;

	m_softDeadHalfScreenPx.x = std::floor(s3d::Max(0.0, halfSizeScreenPx.x));
	m_softDeadHalfScreenPx.y = std::floor(s3d::Max(0.0, halfSizeScreenPx.y));
}

void MapCamera::updateFollowSmooth(const s3d::Vec2& targetCenter,
								   const s3d::Vec2& targetVelocity,
								   double dt)
{
	// 1) 屏幕像素死区 -> 世界单位
	const double s = (m_scale > 0.0 ? m_scale : 1.0);
	const s3d::Vec2 softHalfWorld = m_softDeadHalfScreenPx * (1.0 / s);

	// ⭐ 让“死区边界”具有一点点滞后，避免恰好踩在边上来回跳
	const double eps = 0.5 / s; // 半个世界像素
	const double softX = s3d::Max(0.0, softHalfWorld.x - eps);
	const double softY = s3d::Max(0.0, softHalfWorld.y - eps);

	// 2) 只推动“越界的那一段”（excess），连续无跳变
	s3d::Vec2 desired = m_center; // 先假设不动
	{
		const s3d::Vec2 delta = targetCenter - m_center;

		// X 轴
		const double ax = s3d::Abs(delta.x);
		if (ax > softX) {
			const double excess = ax - softX;
			desired.x = m_center.x + s3d::Sign(delta.x) * excess;
		}

		// Y 轴
		const double ay = s3d::Abs(delta.y);
		if (ay > softY) {
			const double excess = ay - softY;
			desired.y = m_center.y + s3d::Sign(delta.y) * excess;
		}
	}

	// 3) 临界阻尼平滑
	const double smoothTime = 0.20; // 如果还嫌跟得太紧可加到 0.22
	m_center.x = SmoothDamp1(m_center.x, desired.x, m_smoothVX, smoothTime, dt);
	m_center.y = SmoothDamp1(m_center.y, desired.y, m_smoothVY, smoothTime, dt);

	// 4) 边界 & 像素对齐
	applyClampAndSnap();

	m_cam.setCenter(m_center);
	m_cam.setScale(m_scale);
}

void MapCamera::updateFollow(const Vec2& targetCenter) {
	m_center = targetCenter;
	applyClampAndSnap();
}

void MapCamera::follow(const Vec2& targetCenter) {
	updateFollow(targetCenter); // alias
}

const Camera2D& MapCamera::camera() const { return m_cam; }

Vec2 MapCamera::worldToScreen(const Vec2& world) const {
	return (world - m_center) * m_scale + screenCenter();
}
Vec2 MapCamera::screenToWorld(const Vec2& screen) const {
	return (screen - screenCenter()) * (1.0 / m_scale) + m_center;
}
Vec2 MapCamera::worldToCamera(const Vec2& world) const { return (world - m_center); }
Vec2 MapCamera::screenCenter() const { return Vec2{ m_screenSize } *0.5; }

void MapCamera::applyClampAndSnap()
{
	using namespace s3d;


	m_viewHalf = m_screenSize * (0.5 / m_scale);

	const double viewW = m_viewHalf.x * 2.0;
	const double viewH = m_viewHalf.y * 2.0;

	// 1) 先处理「极小地图」：哪一轴比视口小，就把相机锁到那一轴的世界中心
	if (m_world.w <= viewW) {
		m_center.x = m_world.center().x;
	}
	if (m_world.h <= viewH) {
		m_center.y = m_world.center().y;
	}

	// 2) 其余轴正常 clamp 到世界边界
	const double minX = m_world.leftX() + m_viewHalf.x;
	const double maxX = m_world.rightX() - m_viewHalf.x;
	const double minY = m_world.topY() + m_viewHalf.y;
	const double maxY = m_world.bottomY() - m_viewHalf.y;

	// 仅在该轴不是“极小世界”时才 clamp
	if (m_world.w > viewW) m_center.x = Clamp(m_center.x, minX, maxX);
	if (m_world.h > viewH) m_center.y = Clamp(m_center.y, minY, maxY);

	// 3) 像素对齐（按 1/scale 栅格），最后一次性写回
	const double s = (m_scale > 0.0 ? m_scale : 1.0);
	m_center.x = std::round(m_center.x * s) / s;
	m_center.y = std::round(m_center.y * s) / s;

	m_cam.setScale(m_scale);
	m_cam.setCenter(m_center);
}
