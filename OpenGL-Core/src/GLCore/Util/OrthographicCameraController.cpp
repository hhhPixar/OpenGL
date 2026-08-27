/*
 * OrthographicCameraController.cpp —— 正交相机控制器的实现
 *
 * 在引擎中的角色：把输入(键盘/鼠标/事件)转成对 OrthographicCamera 的控制。
 *   - 构造：按宽高比和初始 zoom 算出投影范围交给相机；
 *   - OnUpdate：每帧查 WASD/QE 按键算出相机位移和旋转，再调相机的 SetPosition/SetRotation；
 *   - OnEvent：用 EventDispatcher 把滚轮/窗口 resize 事件分发给对应处理函数；
 *   - OnMouseScrolled：根据滚轮偏移调 zoom 并重设投影；
 *   - OnWindowResized：按新宽高比重算投影，保持画面比例正确。
 *
 * 关键依赖：glm（三角函数和角度转换）、Input.h（按键查询）、KeyCodes.h（HZ_KEY_* 宏）。
 */
#include "glpch.h"
#include "OrthographicCameraController.h"

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"

namespace GLCore::Utils {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		// 构造函数：成员初始化列表里直接构造 OrthographicCamera，
		// 投影范围由 aspectRatio * zoomLevel 决定——横向半宽 = aspect*zoom，纵向半高 = zoom。
		// 这样不同宽高比都能正确显示；zoom 越小越"拉近"。
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{
	}

	// 每帧轮询输入更新相机。ts 是上一帧耗时（秒），让位移随帧率变化保持速度一致。
	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		// 左移(A)和右移(D)：用当前旋转角 m_CameraRotation 算出世界 x/y 方向的分量，
		// 这样相机即使旋转了，A/D 也是沿"相机当前朝向"移动，而不是世界 x 轴。
		// cos/sin 用弧度，所以先 glm::radians 转换；速度乘 ts 实现"每秒多少单位"。
		if (Input::IsKeyPressed(HZ_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		// D 键右移，方向同上。
		else if (Input::IsKeyPressed(HZ_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		// 前进(W)和后退(S)：用旋转角的 -sin/cos 给出垂直方向，构成与 A/D 正交的方向。
		if (Input::IsKeyPressed(HZ_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		// S 键后退，方向同上。
		else if (Input::IsKeyPressed(HZ_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		// 仅当启用旋转时才处理 Q/E 转向。
		if (m_Rotation)
		{
			// Q 键逆时针旋转。
			if (Input::IsKeyPressed(HZ_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			// E 键顺时针旋转。
			if (Input::IsKeyPressed(HZ_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			// 把旋转角限制在 (-180, 180] 内，避免数值无限增长导致精度问题。
			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			// 把新角度提交给相机，相机内部会重算 View。
			m_Camera.SetRotation(m_CameraRotation);
		}

		// 把新位置提交给相机。
		m_Camera.SetPosition(m_CameraPosition);

		// 平移速度跟 zoom 联动：拉近时(zoom 小)移动慢，拉远时(zoom 大)移动快，
		// 让不同缩放下手感一致。
		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	// 事件入口：构造一个 EventDispatcher，把 MouseScrolled 和 WindowResize 事件
	// 分别绑定到本类的私有处理函数。GLCORE_BIND_EVENT_FN 是把成员函数包成可调用对象的宏。
	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GLCORE_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	// 滚轮事件处理：yOffset 正负表示上下滚。调 zoom 后重设投影范围。
	// 返回 false 表示"不阻断事件"——其它监听者仍可继续处理。
	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		// 越向上滚 yOffset 越大，zoom 减小->拉近；乘 0.25 调节灵敏度。
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		// 设个下限，避免 zoom 过小出现 0 或负值导致投影矩阵退化。
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		// 用新 zoom 重设投影范围（左右按 aspect 缩放，上下按 zoom）。
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		// 返回 false：不阻断事件传播，其它监听者仍可处理。
		return false;
	}

	// 窗口大小变化处理：重算宽高比并据此更新投影，保持画面不变形。
	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		// 新宽高比 = 新宽 / 新高。
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		// 按新宽高比重设投影，保持画面不变形。
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		// （同上）不阻断事件传播。
		return false;
	}

}