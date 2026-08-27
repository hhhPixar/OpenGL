#pragma once
/*
 * MouseEvent.h —— 鼠标事件
 * ============================================================
 * 定义鼠标相关事件,继承自 Event 基类。
 *
 * 继承层次:
 *   Event
 *     ├─ MouseMovedEvent          (鼠标移动,带 x/y 坐标)
 *     ├─ MouseScrolledEvent       (滚轮滚动,带 x/y 偏移)
 *     └─ MouseButtonEvent         (中间基类,存 m_Button + 归类)
 *         ├─ MouseButtonPressedEvent   (鼠标键按下)
 *         └─ MouseButtonReleasedEvent  (鼠标键松开)
 *
 * 鼠标坐标是浮点数(支持高 DPI / 亚像素位置);鼠标键是整数(左/右/中键编号)。
 *
 * 依赖:Event.h(基类与 EVENT_CLASS_TYPE / EVENT_CLASS_CATEGORY 宏)。
 */

#include "Event.h"

namespace GLCore {

	/*
	 * 鼠标移动事件。携带鼠标当前位置 x/y(浮点,单位通常是像素,相对窗口左上角)。
	 * 鼠标移动时 GLFW 回调频率较高,会频繁产生此事件。
	 */
	class MouseMovedEvent : public Event
	{
	public:
		// 构造:传入鼠标 x、y 坐标。
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		// 获取鼠标当前位置的 x / y 坐标。
		inline float GetX() const { return m_MouseX; }
		inline float GetY() const { return m_MouseY; }

		// 拼出 "MouseMovedEvent: x, y" 用于日志。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		// 绑定类型 MouseMoved;下面一行把它归类为"鼠标 + 输入"(位或 |)。
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		// 鼠标 x、y 坐标成员。
		float m_MouseX, m_MouseY;
	};

	/*
	 * 鼠标滚轮事件。xOffset 通常是水平滚轮偏移,yOffset 是垂直滚轮偏移
	 * (普通鼠标一般只有垂直滚轮,yOffset 为正向上滚、为负向下滚)。
	 */
	class MouseScrolledEvent : public Event
	{
	public:
		// 构造:传入滚轮 x、y 偏移。
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		// 获取滚轮偏移量。
		inline float GetXOffset() const { return m_XOffset; }
		inline float GetYOffset() const { return m_YOffset; }

		// 拼出 "MouseScrolledEvent: xOffset, yOffset"。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		// 绑定类型 MouseScrolled;归类为"鼠标 + 输入"。
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		// 滚轮 x、y 偏移成员。
		float m_XOffset, m_YOffset;
	};

	/*
	 * 鼠标按键事件的共同基类(中间层)。只存按了哪个键(m_Button,左/右/中键编号),
	 * 构造函数 protected 仅供子类调用,大类统一设为"鼠标 + 输入"。
	 * 与 KeyEvent 类似,它本身不绑定具体 EventType——具体类型由子类用 EVENT_CLASS_TYPE 设。
	 */
	class MouseButtonEvent : public Event
	{
	public:
		// 获取鼠标键编号(GLFW_MOUSE_BUTTON_LEFT / RIGHT / MIDDLE 等)。
		inline int GetMouseButton() const { return m_Button; }

		// 鼠标按键事件统一归类为"鼠标 + 输入"。
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:
		// protected 构造:仅供子类(按下/松开)调用,传入按键编号。
		MouseButtonEvent(int button)
			: m_Button(button) {}

		// 鼠标按键编号成员。protected 表示子类可直接访问。
		int m_Button;
	};

	// 鼠标键按下事件。
	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		// 构造:把按键编号交给基类 MouseButtonEvent。
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		// 拼出 "MouseButtonPressedEvent: <button>"。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		// 绑定类型 MouseButtonPressed(宏原理见 Event.h,用到了 # 与 ##)。
		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	// 鼠标键松开事件。
	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		// 构造:把按键编号交给基类 MouseButtonEvent。
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		// 拼出 "MouseButtonReleasedEvent: <button>"。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		// 绑定类型 MouseButtonReleased。
		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}