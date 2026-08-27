#pragma once
/* ============================================================================
 * Window.h —— 窗口抽象接口(平台无关层)
 *
 * 这个文件定义了引擎里"窗口"的抽象基类 Window,以及创建窗口用的配置结构
 * WindowProps。它是"平台抽象层"模式的上层接口:引擎其他部分只依赖这里的
 * 抽象类,不直接接触 GLFW/Win32/SDL 这些具体平台库。具体实现(WindowsWindow)
 * 在 Platform/Windows 下,通过编译期的 #ifdef 在 Window::Create 工厂里被选中。
 *
 * 关键依赖:
 *   - Core.h      : 引擎基础宏(如 GLCORE_ASSERT)和公用类型
 *   - Event.h     : 事件系统(Event 及其子类),窗口回调通过 EventCallbackFn 上报
 *   - std::function: 用来定义事件回调的类型别名 EventCallbackFn
 * ========================================================================== */

#include "glpch.h"

#include "GLCore/Core/Core.h"
#include "GLCore/Events/Event.h"

namespace GLCore {

	/* ---------------------------------------------------------------------------
	 * WindowProps:创建窗口时用的"属性包"。
	 * 把标题、宽度、高度打包成一个结构体,调用方只要填这几个字段就能开窗口。
	 * 给了默认值,所以调用时可以不传参(用默认 1280x720 / "OpenGL Sandbox")。
	 * ------------------------------------------------------------------------- */
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		/* 构造函数:带默认参数。用"初始化列表 : Title(title), ..."给成员赋值,
		 * 比在函数体里赋值更高效(直接构造,而非先默认构造再赋值)。
		 * const std::string& :常量引用,避免拷贝整串字符串。 */
		WindowProps(const std::string& title = "OpenGL Sandbox",
			        uint32_t width = 1280,
			        uint32_t height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	/* ---------------------------------------------------------------------------
	 * Window:窗口的抽象基类(接口类)。
	 *
	 * 所谓"接口类",就是只有纯虚函数(声明后面带 = 0)的类,它本身不能实例化
	 * (不能 new Window()),只能作为基类被继承。子类(WindowsWindow)负责把
	 * 每个纯虚函数都实现一遍。上层代码拿着 Window* 指针调用,实际跑的是子类的
	 * 实现——这就是 C++ 的"多态"(运行时根据真实对象类型调对应函数)。
	 *
	 * 这样设计的好处:换平台时(比如从 GLFW 换成别的窗口库),只需要写一个新的
	 * 子类,上层所有调用 Window 的代码完全不用改。
	 * ------------------------------------------------------------------------- */
	class Window
	{
	public:
		/* using 是 C++11 的类型别名(老写法是 typedef)。这里把
		 * std::function<void(Event&)> 起个短名叫 EventCallbackFn。
		 * std::function 是个"可调用对象包装器",能装函数指针、lambda、
		 * 仿函数等任何"能像 void(Event&) 这样调用"的东西。
		 * 这里它表示"收到一个 Event 引用时,以它为参数回调一次"。
		 * 应用层(比如 Application)会把这个回调设置进窗口,窗口里发生任何
		 * 事件(按键、鼠标移动、窗口大小改变……)时,就调这个回调把事件发出去。 */
		using EventCallbackFn = std::function<void(Event&)>;

		/* 虚析构函数 = default。基类有虚函数就一定要把析构函数声明成 virtual,
		 * 否则用基类指针 delete 子类对象时,子类的析构不会被调用,会内存泄漏。
		 * = default 表示"用编译器默认生成的实现"(什么都不做再往上调用基类析构)。 */
		virtual ~Window() = default;

		/* 每帧调用:处理操作系统消息(比如 GLFW 的 glfwPollEvents)并交换前后缓冲。
		 * = 0 表示纯虚函数,必须在子类里实现,否则子类也是抽象类不能实例化。 */
		virtual void OnUpdate() = 0;

		/* const 成员函数:承诺不修改对象状态。返回窗口当前的宽/高。
		 * 这俩也是纯虚,由子类从自己保存的数据里取出来返回。 */
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		/* 设置事件回调:把外部传进来的 callback 存起来,以后窗口里出事件就调它。
		 * const EventCallbackFn& :常量引用,避免拷贝 std::function 对象。 */
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		/* 设置/查询垂直同步(VSync)。VSync 开启时,glfwSwapInterval(1) 会让
		 * 交换缓冲和显示器刷新同步,避免画面撕裂;关闭则不限速,帧率可更高但会撕裂。 */
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		/* 返回底层原生窗口指针(在 Windows 实现里就是 GLFWwindow*)。
		 * 返回 void* 是因为这里在平台无关层,不能出现 GLFW 这种具体类型;
		 * 调用方(比如 Input)自己 static_cast 回 GLFWwindow* 用。 */
		virtual void* GetNativeWindow() const = 0;

		/* 静态工厂方法:根据编译期的平台宏(#ifdef)选择具体实现来 new 出一个窗口。
		 * 返回基类指针 Window*,调用方只看到抽象接口,不关心背后是 WindowsWindow
		 * 还是别的。这种"用函数封装 new,根据条件返回不同子类"叫简单工厂模式。 */
		static Window* Create(const WindowProps& props = WindowProps());
	};

}