/*
 * OrthographicCameraController.h —— 正交相机控制器的声明
 *
 * 在引擎中的角色：在 OrthographicCamera 基础上再加一层"输入/事件驱动"逻辑，
 * 把键盘(WASD 平移、QE 旋转)、鼠标(滚轮缩放)、窗口(resize 适配宽高比)等输入
 * 翻译成对底层 OrthographicCamera 的 SetPosition/SetProjection 调用。
 * 应用层每帧调 OnUpdate 轮询输入、有事件时调 OnEvent 分发，就能得到一个可玩的 2D 相机。
 *
 * 关键依赖：
 *   - OrthographicCamera.h：被控的相机本体；
 *   - Timestep.h：OnUpdate 收到的每帧时间步长，用于"按时间缩放"移动速度，保证不同帧率下移动一致；
 *   - ApplicationEvent.h / MouseEvent.h：窗口大小变化、鼠标滚轮事件类型；
 *   -（实现里还用到 Input.h、KeyCodes.h 来查询按键）。
 */
#pragma once

#include "OrthographicCamera.h"
#include "GLCore/Core/Timestep.h"

#include "GLCore/Events/ApplicationEvent.h"
#include "GLCore/Events/MouseEvent.h"

namespace GLCore::Utils {

	// 相机控制器：把输入翻译成对 OrthographicCamera 的操作，组合(owns)一个相机对象。
	class OrthographicCameraController
	{
	public:
		// 构造：aspectRatio 是宽/高（例如 16/9）。用它确定投影范围的横向尺度。
		// rotation=true 时启用 QE 旋转，否则相机始终朝向固定。给默认参数后可只传一个宽高比。
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		// 每帧调用：轮询按键状态、计算相机位移并提交给 OrthographicCamera。
		// ts 是上一帧到当前帧的实际耗时，乘到速度上让运动与帧率解耦。
		void OnUpdate(Timestep ts);
		// 收到事件时调用：派发到 OnMouseScrolled / OnWindowResized 处理。
		// Event& 是基类引用，能装下任何具体事件子类（多态）。
		void OnEvent(Event& e);

		// 提供非 const 和 const 两个重载：const 对象拿 const 引用、普通对象拿可修改引用。
		// 这两个 GetCamera 仅返回类型不同——这是 C++ 允许的"基于 const 的重载"。
		OrthographicCamera& GetCamera() { return m_Camera; }
		// const 版本：const 控制器对象只能拿到只读相机引用。
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		// 返回当前缩放等级。
		float GetZoomLevel() const { return m_ZoomLevel; }
		// 外部直接设置缩放等级（注意：这里不自动重设投影，需配合 OnMouseScrolled 逻辑）。
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
	private:
		// 事件处理函数，返回 bool 表示"是否已消费该事件"（事件系统约定）。
		bool OnMouseScrolled(MouseScrolledEvent& e);
		// 窗口大小变化处理函数。
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		// 宽高比，resize 时更新。
		float m_AspectRatio;
		// 缩放等级：越大看到的范围越大（相机离得越远）。
		float m_ZoomLevel = 1.0f;
		// 被控制的实际相机对象（值语义，组合在控制器里，控制器析构时自动销毁）。
		OrthographicCamera m_Camera;

		// 是否启用旋转控制。
		bool m_Rotation;

		// 默认成员初始化：这里用花括号初始化和直接 = 数值都行，效果一样。
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		// 相机旋转角（度数，逆时针方向为正）。
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		// 平移速度和旋转速度（每秒单位数 / 每秒度数），OnUpdate 里乘 ts 得到每帧增量。
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};

}