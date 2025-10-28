#include "stdafx.h"
#include "MapCamera.h"
#include <cmath> 
using namespace s3d;

void MapCamera::init(const Size& screenSize, const RectF& worldBounds, double scale) {
	setViewport(screenSize, scale);
	m_centerCont = m_center;
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
	// 1) 死区大小（世界单位）
	const double s = (m_scale > 0.0 ? m_scale : 1.0);
	const s3d::Vec2 softHalfWorld = m_softDeadHalfScreenPx * (1.0 / s);

	const double eps = 0.5 / s;
	const double softX = s3d::Max(0.0, softHalfWorld.x - eps);
	const double softY = s3d::Max(0.0, softHalfWorld.y - eps);

	// 2) 只推动越界的部分 —— 用连续中心 m_centerCont
	s3d::Vec2 desired = m_centerCont;
	{
		const s3d::Vec2 delta = targetCenter - m_centerCont;
		const double ax = s3d::Abs(delta.x);
		const double ay = s3d::Abs(delta.y);
		if (ax > softX) desired.x = m_centerCont.x + s3d::Sign(delta.x) * (ax - softX);
		if (ay > softY) desired.y = m_centerCont.y + s3d::Sign(delta.y) * (ay - softY);
	}

	// 3) 对连续中心做临界阻尼
	const double smoothTime = 0.22;
	m_centerCont.x = SmoothDamp1(m_centerCont.x, desired.x, m_smoothVX, smoothTime, dt);
	m_centerCont.y = SmoothDamp1(m_centerCont.y, desired.y, m_smoothVY, smoothTime, dt);

	// 4) 对连续中心做“边界夹紧（不量化）”
	//    注意：这里不要做像素对齐！
	{
		m_viewHalf = m_screenSize * (0.5 / m_scale);
		const double viewW = m_viewHalf.x * 2.0;
		const double viewH = m_viewHalf.y * 2.0;

		if (m_world.w <= viewW) m_centerCont.x = m_world.center().x;
		if (m_world.h <= viewH) m_centerCont.y = m_world.center().y;

		const double minX = m_world.leftX() + m_viewHalf.x;
		const double maxX = m_world.rightX() - m_viewHalf.x;
		const double minY = m_world.topY() + m_viewHalf.y;
		const double maxY = m_world.bottomY() - m_viewHalf.y;

		if (m_world.w > viewW) m_centerCont.x = s3d::Clamp(m_centerCont.x, minX, maxX);
		if (m_world.h > viewH) m_centerCont.y = s3d::Clamp(m_centerCont.y, minY, maxY);
	}

	// 5) 从连续中心生成“对齐中心”供绘制（量化只发生在渲染输出）
	const double snapS = (m_scale > 0.0 ? m_scale : 1.0);
	m_center.x = std::round(m_centerCont.x * snapS) / snapS;
	m_center.y = std::round(m_centerCont.y * snapS) / snapS;

	// 6) 写回相机
	m_cam.setScale(m_scale);
	m_cam.setCenter(m_center);
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
