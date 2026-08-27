/* ============================================================================
 * WindowsWindow.cpp —— WindowsWindow 的具体实现(平台层 / Windows + GLFW)
 *
 * 这个文件是把抽象的 Window 接口"落地"的地方:用 GLFW 创建窗口、建立
 * OpenGL 上下文、用 glad 加载 GL 函数指针、注册各种 GLFW 回调并把它们转成
 * 引擎的 Event。它和 WindowsWindow.h 是一对(.h 声明,.cpp 实现)。
 *
 * 关键流程(Init 里):
 *   1) glfwInit()              —— 初始化 GLFW 库(整个进程只做一次)。
 *   2) glfwCreateWindow()      —— 创建窗口,返回 GLFWwindow*;此时也建好
 *                                  了该窗口的 OpenGL 上下文。
 *   3) glfwMakeContextCurrent()—— 把上面那个上下文设为"当前线程的当前上下文",
 *                                  后续的 gl 调用都作用于它。
 *   4) gladLoadGLLoader()      —— glad 是个 GL 函数指针加载器;OpenGL 的函数
 *                                  地址是运行时按显卡/驱动决定的,必须先加载才能
 *                                  调 glGetString 等任何 GL 函数。所以这步必须
 *                                  在创建上下文之后。
 *   5) glfwSetWindowUserPointer()—— 把 &m_Data 塞进窗口,GLFW 回调里能取回。
 *   6) glfwSetXxxCallback()     —— 注册窗口大小/关闭/按键/字符/鼠标键/滚轮/
 *                                  光标移动等回调;每个回调里构造对应 Event,
 *                                  交给 m_Data.EventCallback 上报。
 *
 * OnUpdate 每帧:glfwPollEvents() 处理输入/系统消息,glfwSwapBuffers() 把
 * 后缓冲翻到屏幕(双缓冲,避免画面闪烁)。
 *
 * 关键依赖:
 *   - WindowsWindow.h          : 本类的声明(含 WindowData)
 *   - ApplicationEvent/MouseEvent/KeyEvent : 各种 Event 类,回调里要构造它们
 *   - GLFW/glfw3.h             : 窗口/输入/上下文 API
 *   - glad/glad.h              : GL 函数指针加载
 * ========================================================================== */
#include "glpch.h"
#include "WindowsWindow.h"

#include "GLCore/Events/ApplicationEvent.h"
#include "GLCore/Events/MouseEvent.h"
#include "GLCore/Events/KeyEvent.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace GLCore {
	
	/* 标记 GLFW 库是否已初始化。glfwInit() 全进程只需调一次,所以用这个
	 * 文件级 static 变量做"只初始化一次"的哨兵。static 在这里表示"文件内
	 * 可见、内部链接",别的 .cpp 访问不到它。 */
	static bool s_GLFWInitialized = false;

	/* GLFW 的错误回调:GLFW 出错时会调这个 C 函数。我们把错误用日志打出来。
	 * 注意这是普通 C 风格函数(不是 lambda),因为 glfwSetErrorCallback 要的是
	 * 函数指针。LOG_ERROR 是引擎的日志宏(类似 printf 的 {0}{1} 占位)。 */
	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	/* Window::Create 工厂的实现。这就是抽象层 Window 里那个 static Create
	 * 的"真正实现位置"——在 .cpp 里。Hazel 风格通常会在这里用
	 * #ifdef GLCORE_PLATFORM_WINDOWS 等宏选平台:new WindowsWindow 或 new 别的。
	 * 这里简化成直接返回 WindowsWindow。返回基类指针 Window*。 */
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	/* 构造函数:把创建窗口的活全交给 Init。委托模式,构造即初始化。 */
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	/* 析构:销毁窗口(Shutdown 里 glfwDestroyWindow)。 */
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	/* Init:核心。建窗口、加载 glad、注册所有回调。 */
	void WindowsWindow::Init(const WindowProps& props)
	{
		/* 先把外部传进来的属性拷一份到 m_Data。m_Data 是 WindowData,会在
		 * GLFW 回调里通过 user pointer 被取用,所以这里要先填好。 */
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		/* GLFW 库只初始化一次。success 是 glfwInit 的返回值(非零表示成功)。
		 * GLCORE_ASSERT 是断言宏:失败时打印消息并中断(调试期早暴露问题)。 */
		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			GLCORE_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		/* glfwCreateWindow:创建窗口和它的 OpenGL 上下文。参数依次:宽、高、
		 * 标题(C 字符串,所以用 .c_str() 把 std::string 转成 const char*)、
		 * 监视器(nullptr=窗口模式)、共享上下文(nullptr=不共享)。
		 * (int) 是 C 风格转换,把 uint32_t 转回 int 给 GLFW。返回 GLFWwindow*。 */
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		/* 把刚创建的上下文设为当前线程的当前上下文。必须先做这步,glad 才能
		 * 加载到本上下文的 GL 函数指针。 */
		glfwMakeContextCurrent(m_Window);
		/* glad:GL 函数指针加载器。OpenGL 的函数(如 glGetString)地址是运行时
		 * 由显卡驱动决定的,不能直接调;glad 通过 glfwGetProcAddress 拿到每个
		 * 函数的真实地址并填进 glad 的函数指针表。这步之后才能调任何 GL 函数。
		 * GLADloadproc 是 glad 定义的函数指针类型,(GLADloadproc) 是强制转换。 */
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GLCORE_ASSERT(status, "Failed to initialize Glad!");

		/* 顺便打印一下显卡信息(厂商/渲染器/版本),方便排查环境问题。
		 * glGetString 返回 const GLubyte*,这里用 {0} 占位由 LOG 格式化。 */
		LOG_INFO("OpenGL Info:");
		LOG_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		LOG_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		LOG_INFO("  Version: {0}", glGetString(GL_VERSION));

		/* 把 m_Data 的地址塞给 GLFW 窗口。这是"把 C++ 上下文传给 C 回调"的
		 * 关键:GLFW 回调签名只能带一个 void* user pointer,我们就把 &m_Data
		 * 存进去,回调里 glfwGetWindowUserPointer 取出来 cast 成 WindowData* 用。 */
		glfwSetWindowUserPointer(m_Window, &m_Data);
		/* 默认开启垂直同步(VSync):glfwSwapInterval(1) 让交换缓冲和显示器刷新
		 * 同步,避免画面撕裂。 */
		SetVSync(true);

		// Set GLFW callbacks
		/* 下面这一串 glfwSetXxxCallback 注册各种回调。每个回调都是个 lambda:
		 * [](GLFWwindow* window, ...){...}。lambda 是个"匿名函数对象",能当
		 * 函数指针用(GLFW 要的就是这种 C 风格回调签名)。
		 * 回调体里的套路都一样:先用 glfwGetWindowUserPointer 取回 WindowData*,
		 * cast 后用它的字段(更新宽高、调 EventCallback 上报事件)。 */

		/* 窗口大小改变:产生 WindowResizeEvent(新宽高)。 */
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		/* 用户点关闭按钮:产生 WindowCloseEvent。 */
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		/* 键盘按键:GLFW 给出 key(键码)、scancode(扫描码,一般不用)、
		 * action(按下/释放/重复)、mods(修饰键 Shift/Ctrl/Alt)。按 action 分发:
		 *   GLFW_PRESS   -> KeyPressedEvent(key, 0)  (0=首次按下,非重复)
		 *   GLFW_RELEASE -> KeyReleasedEvent(key)
		 *   GLFW_REPEAT  -> KeyPressedEvent(key, 1)  (1=按住不放产生的重复)
		 * 这就是"按键事件"和轮询(IsKeyPressed)的区别:这里只在状态变化时通知一次。 */
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		/* 字符输入:用户输入可打印字符时触发(会考虑输入法/Shift)。产生
		 * KeyTypedEvent(字符的 Unicode 码点)。和 KeyPressed 不同——KeyPressed
		 * 是物理按键,KeyTyped 是"打出来的字符",做文本输入框要用它。 */
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		/* 鼠标按键:产生 MouseButtonPressedEvent / MouseButtonReleasedEvent。 */
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		/* 滚轮滚动:产生 MouseScrolledEvent(xOffset, yOffset)。通常 yOffset
		 * 是 +1/-1 表示向上/向下滚一格;(float) 把 double 转成 float。 */
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		/* 鼠标移动:产生 MouseMovedEvent(x, y)。坐标是相对窗口左上角的像素位置。 */
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	/* 销毁窗口。glfwDestroyWindow 会释放窗口和它的 OpenGL 上下文。
	 * 注意:这里没有 glfwTerminate()——那会卸载整个 GLFW 库,要等程序退出时做。 */
	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	/* 每帧调用,干两件事:
	 *   glfwPollEvents() —— 处理操作系统消息队列(按键、鼠标移动等会在这里
	 *                        触发上面注册的回调);
	 *   glfwSwapBuffers()—— 把后台缓冲翻到屏幕(双缓冲渲染:先在后台画好,
	 *                        再整帧翻过去,避免画一半就被显示出来导致闪烁)。 */
	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	/* VSync 开关。glfwSwapInterval(1)= 跟显示器刷新同步(开);(0)= 不限速(关)。
	 * 顺便把状态记进 m_Data,IsVSync() 好查。 */
	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	/* 查询当前 VSync 状态。const 表示不修改对象。 */
	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

}
