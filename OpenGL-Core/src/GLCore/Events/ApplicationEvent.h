#pragma once
/*
 * ApplicationEvent.h —— "应用/窗口"类事件
 * ============================================================
 * 定义和"窗口、应用运行阶段"相关的事件类,都继承自 Event 基类。
 *
 * 这些事件多数由 GLFW 的窗口回调产生:窗口大小改变、关闭等。
 * GLFW 检测到这些情况会调用我们注册的回调函数,回调里构造出对应的事件对象,
 * 再塞进引擎的事件系统分发出去。
 *
 * 而 AppTick / AppUpdate / AppRender 是引擎自己每帧产生的"阶段性"事件,
 * 用来标记一帧里的不同阶段,并非来自 GLFW。
 *
 * 本文件所有事件都归属 EventCategoryApplication 大类。
 *
 * 依赖:Event.h(基类与 EVENT_CLASS_TYPE / EVENT_CLASS_CATEGORY 宏)。
 */

#include "Event.h"

namespace GLCore {

	// 窗口大小改变事件。GLFW 窗口大小回调里带上新的宽高,构造此事件。
	// 持有新尺寸 m_Width/m_Height,并提供 getter。
	class WindowResizeEvent : public Event
	{
	public:
		// 构造函数:传入新窗口尺寸。初始化列表 : m_Width(width), m_Height(height) 给两个成员赋初值。
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) {}

		// 获取新窗口宽 / 高。inline 建议编译器内联,适合这种短小访问器。
		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		// 重写 ToString:拼出 "WindowResizeEvent: 宽, 高" 这样的可读字符串,供日志打印。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		// 一行宏绑定事件类型=WindowResize、名字="WindowResize"(宏原理见 Event.h,用到了 # 与 ##)。
		EVENT_CLASS_TYPE(WindowResize)
		// 归类为"应用类"事件。
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		// 新窗口的宽、高(像素)。
		uint32_t m_Width, m_Height;
	};

	// 窗口关闭事件。用户点了窗口关闭按钮(或按 Alt+F4 等)时由 GLFW 触发。
	// 没有额外数据,构造函数空实现即可。
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		// 绑定类型=WindowClose;下面一行归类为应用类。两宏的原理见 Event.h。
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	// 应用 Tick 事件:引擎每个"帧"开始时产生一次,可用于驱动每帧都要跑的逻辑。
	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	// 应用 Update 事件:一帧中的"更新"阶段产生,常用于更新游戏状态/对象位置等。
	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	// 应用 Render 事件:一帧中的"渲染"阶段产生,触发实际绘制 OpenGL 画面。
	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
}