#pragma once
/* ============================================================================
 * WindowsWindow.h —— Window 抽象接口的 GLFW 实现(平台层 / Windows)
 *
 * 这是平台无关接口 Window 的具体实现:用 GLFW 库来真正创建窗口、管理 OpenGL
 * 上下文、轮询输入、交换缓冲。上层只看到 Window*,通过 Window::Create 工厂
 * 拿到的就是这个 WindowsWindow 实例(new 出来的,返回基类指针)。
 *
 * 关键设计:WindowData 内嵌结构体。
 *   GLFW 是 C 库,它的回调(按键、鼠标等)都是 C 风格函数指针,签名固定,
 *   只能附带一个 void* user pointer。我们没法直接在 C 回调里访问 C++ 对象的
 *   成员。解决办法:把"标题/宽高/VSync 开关/事件回调"打包成一个 WindowData
 *   结构,用 glfwSetWindowUserPointer 把它的地址塞给 GLFW 窗口;回调触发时,
 *   用 glfwGetWindowUserPointer 取回这个指针,cast 成 WindowData*,在里面
 *   构造对应的 Event,再交给 EventCallback 上报。这就是"把 C 回调桥接到
 *   C++ 事件"的关键中介。
 *
 * 关键依赖:
 *   - Window.h : 被实现的抽象接口(继承 Window)
 *   - GLFW/glfw3.h : 具体平台库,GLFWwindow* 就来自这里
 * ========================================================================== */

#include "GLCore/Core/Window.h"

#include <GLFW/glfw3.h>

namespace GLCore {

	/* ---------------------------------------------------------------------------
	 * WindowsWindow:Window 的 GLFW 实现。
	 * public 继承 Window,把基类的纯虚函数都 override 一遍。上层拿 Window*
	 * 调用 OnUpdate/GetNativeWindow 等,实际跑的是这里的实现(多态)。
	 * ------------------------------------------------------------------------- */
	class WindowsWindow : public Window
	{
	public:
		/* 构造时直接 Init(创建 GLFW 窗口);析构时 Shutdown(销毁窗口)。
		 * 不做默认构造——开窗口必须带属性。 */
		WindowsWindow(const WindowProps& props);
		/* virtual 析构:基类 Window 的析构是 virtual,这里也要 virtual 才能
		 * 保证通过 Window* 删除时调到这个子类析构(进而 Shutdown 销毁 GLFW 窗口)。 */
		virtual ~WindowsWindow();

		/* 每帧调用:glfwPollEvents() 处理系统消息 + glfwSwapBuffers() 交换缓冲。
		 * override 表示"我重写了基类的纯虚函数"。 */
		void OnUpdate() override;

		/* inline + override:直接返回成员,内联到调用处以避免函数调用开销。
		 * const 表示不修改对象。返回 m_Data 里的宽/高。 */
		inline uint32_t GetWidth() const override { return m_Data.Width; }
		inline uint32_t GetHeight() const override { return m_Data.Height; }

		// Window attributes
		/* 把外部回调存进 m_Data.EventCallback;之后 GLFW 回调触发时就调它发事件。
		 * inline 直接赋值即可。 */
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		/* VSync 的开关/查询,实现在 .cpp 里(要调 glfwSwapInterval)。 */
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		/* 返回原生 GLFW 窗口指针。返回 void* 是为了满足基类签名(基类不能出现
		 * GLFW 类型);调用方(WindowsInput)再 static_cast 回 GLFWwindow*。
		 * virtual 这里可写可不写(基类已是 virtual),写了更明确。 */
		inline virtual void* GetNativeWindow() const { return m_Window; }
	private:
		/* 初始化:初始化 GLFW、创建窗口、加载 glad、注册所有 GLFW 回调。
		 * virtual 是为了允许子类进一步定制(虽然现在没子类),也是 Hazel 的习惯写法。 */
		virtual void Init(const WindowProps& props);
		/* 销毁 GLFW 窗口。 */
		virtual void Shutdown();
	private:
		/* 底层 GLFW 窗口句柄。所有 GLFW 操作都要靠它。 */
		GLFWwindow* m_Window;

		/* WindowData:把要在 C 回调里用到的上下文打包。回调签名只能带 void*,
		 * 所以传 &m_Data 进去,回调里取出来 cast 成 WindowData* 用。 */
		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}