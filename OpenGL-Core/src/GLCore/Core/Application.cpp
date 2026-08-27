/*
 * Application.cpp —— 引擎主类实现
 *
 * 职责：实现 Application 的构造、主循环 Run、事件派发 OnEvent 等关键方法。
 *       这是引擎最核心的文件，理解了它就理解了引擎的运行模型。
 *
 * 在引擎里的角色：
 *   - 构造时完成"初始化四件套"：日志 -> 单例登记 -> 创建窗口并绑定事件回调 -> 加入 ImGui 层
 *   - Run() 是主循环，每帧推进游戏逻辑 + 渲染 ImGui + 交换缓冲
 *   - OnEvent 是事件枢纽，从窗口进来的事件在这里派发与向下传播
 *
 * 关键依赖：
 *   - glpch.h          —— 预编译头
 *   - Application.h    —— 本类声明
 *   - Log.h            —— 日志（构造时 Init）
 *   - Input.h          —— 输入系统（此处仅 include，未直接调用）
 *   - glfw/glfw3.h     —— GLFW 库，Run 里用 glfwGetTime() 取秒数算帧时间
 *
 * 核心概念：
 *   - BIND_EVENT_FN 宏 = std::bind(&Application::OnEvent, this, std::placeholders::_1)
 *     把"成员函数 + this + 占位符"绑成一个可调用对象，作为 GLFW 窗口的事件回调。
 *   - 单例：s_Instance 静态指针在构造里登记 this，Get() 返回它。
 */
#include "glpch.h"
#include "Application.h"

#include "Log.h"

#include "Input.h"

#include <glfw/glfw3.h>

namespace GLCore {

/*
 * BIND_EVENT_FN —— 本文件内部用的"事件回调绑定宏"
 *
 *   用法：BIND_EVENT_FN(OnEvent)  展开为 std::bind(&Application::OnEvent, this, std::placeholders::_1)
 *
 *   为什么需要它？这是初学者最容易卡的地方，逐步解释：
 *     1) GLFW 窗口产生事件后，需要一个"可调用对象"作为回调。回调签名是 void(Event&)。
 *     2) 我们希望回调去调用 Application 的成员函数 OnEvent(Event&)。
 *        但成员函数不能单独调用——必须有一个对象 this 才能调用成员函数
 *        （因为成员函数要访问对象的成员变量，比如 m_LayerStack）。
 *     3) std::bind 把"成员函数指针 &Application::OnEvent" 和 "对象指针 this"
 *        绑在一起，生成一个"已经记住 this"的可调用对象(std::function)。
 *     4) std::placeholders::_1 是"占位符"，表示"第一个参数暂时不填，将来调用时再传进去"。
 *        也就是说回调被调用时传入的那个 Event&，会作为 OnEvent 的实参。
 *
 *   最终效果：窗口事件发生 -> 回调被调用(传入 Event&) -> 自动转发到 this->OnEvent(Event&)。
 *   注意：这个宏在文件作用域#define，但它只在本 .cpp 的命名空间内有效，
 *   与 Core.h 里的 GLCORE_BIND_EVENT_FN 不同（后者是全局的、用 this，但 fn 形式不同）。
 */
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	/*
	 * 静态成员的定义：s_Instance 在头文件里声明，这里真正分配并初始化为 nullptr。
	 *   整个程序里只能定义一次，否则链接会报"重复符号"。
	 */
	Application* Application::s_Instance = nullptr;

	/*
	 * 构造函数 —— 完成引擎初始化。
	 *
	 * 初始化顺序（很重要，有依赖关系）：
	 *   1) 如果是第一个 Application 实例，先 Log::Init() 初始化日志（之后才能用 LOG_XXX）
	 *   2) 断言确保没有第二个实例（单例），随后 s_Instance = this 登记自己
	 *   3) 创建 Window（Window::Create 工厂方法，返回 Window*），
	 *      用 std::unique_ptr 包起来接管所有权；并 SetEventCallback 把 BIND_EVENT_FN(OnEvent) 设给窗口
	 *   4) new 一个 ImGuiLayer 并 PushOverlay 进层栈（层栈拥有它，m_ImGuiLayer 只是观察指针）
	 *
	 *   注意{name, width, height} 是用花括号构造 WindowProps 结构体的简写。
	 */
	Application::Application(const std::string& name, uint32_t width, uint32_t height)
	{
		if (!s_Instance)
		{
			// Initialize core
			// 只在第一个实例构造时初始化日志系统（避免重复初始化）
			Log::Init();
		}

		// 断言：s_Instance 应当为空，否则说明已经存在 Application（破坏单例）
		GLCORE_ASSERT(!s_Instance, "Application already exists!");
		// 登记单例：让 Get() 能返回本对象
		s_Instance = this;

		// Window::Create 返回裸指针 Window*，用 unique_ptr 接管 -> 自动管理窗口生命周期
		m_Window = std::unique_ptr<Window>(Window::Create({ name, width, height }));
		// 把 OnEvent 绑成回调注册给窗口：以后窗口的按键/鼠标/关闭事件都会进 OnEvent
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		// Renderer::Init();

		// 创建 ImGui 调试层并作为 overlay 加入层栈（层栈将拥有并最终 delete 它）
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	// 加入普通层：直接委托给 LayerStack
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	// 加入覆盖层：直接委托给 LayerStack
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	/*
	 * OnEvent —— 事件中枢。
	 *
	 *   当窗口事件发生时，构造里注册的回调会调用到这里，参数 e 是事件对象。
	 *
	 *   做两件事：
	 *   1) 用 EventDispatcher 派发"自己关心的"事件类型：这里只处理 WindowClose，
	 *      匹配则调用 OnWindowClose。Dispatch 的实参又是 BIND_EVENT_FN(OnWindowClose)
	 *      形式的可调用对象。
	 *   2) 把事件从层栈顶到底逆序传给每一层的 OnEvent，某层把 e.Handled 标记为 true 就停止传播
	 *      （表示"这事件我处理了，下面层不需要再收到"）。
	 *
	 *   为什么从后往前？因为 overlay（如 ImGui）在末尾（栈顶），通常希望它优先拦下鼠标事件；
	 *      先给上层，被处理过就不再往下传，避免下层误响应。
	 */
	void Application::OnEvent(Event& e)
	{
		// 构造一个 dispatcher，内部持有事件 e 的引用
		EventDispatcher dispatcher(e);
		// Dispatch<WindowCloseEvent>：若 e 实际是 WindowCloseEvent，就调用 OnWindowClose(e)
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		/*
		 * 从 end() 往 begin() 逆序遍历（事件传播：栈顶 -> 栈底）
		 *
		 *   it 从 m_LayerStack.end()（尾后位置）开始，
		 *   循环里先 --it 前进一步（指向最后一个元素），再解引用 *it 拿到 Layer*。
		 *   一旦 e.Handled 为 true（某层标记已处理），立即 break 不再往下传。
		 *   这种写法是为了"从后往前"迭代 std::vector（vector 没有逆向范围 for 的简写）。
		 */
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	/*
	 * Run —— 主循环。
	 *
	 *   while (m_Running) 一直转，直到窗口关闭事件把 m_Running 置 false。
	 *   每帧做四件事：
	 *     1) 算 Timestep：用 glfwGetTime() 取当前秒数，减去上一帧时间得到本帧耗时，
	 *        传给每层 OnUpdate 让游戏逻辑按时间推进（而非按帧数）。
	 *     2) 范围 for 遍历层栈，调用每层 OnUpdate(timestep) 更新逻辑。
	 *     3) ImGui 渲染：Begin() 准备 ImGui 上下文 -> 各层 OnImGuiRender() 提交面板 -> End() 真正绘制。
	 *     4) m_Window->OnUpdate() 交换前后缓冲(显示新画面)并轮询输入事件。
	 *
	 *   关于 glfwGetTime()：返回自 GLFW 初始化以来的秒数(double)，转 float 用。
	 */
	void Application::Run()
	{
		while (m_Running)
		{
			// 取当前时间(秒)，glfwGetTime 是 GLFW 提供的高精度计时器
			float time = (float)glfwGetTime();
			// 本帧耗时 = 当前时间 - 上一帧时间；包成 Timestep 类型传给各层
			Timestep timestep = time - m_LastFrameTime;
			// 记下当前时间，供下一帧用
			m_LastFrameTime = time;

			// 范围 for：依次取每个 Layer* 调用 OnUpdate，更新游戏逻辑
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timestep);

			/*
			 * ImGui 三段式渲染：
			 *   Begin() —— 启动新一帧 ImGui，准备上下文（必须在所有 OnImGuiRender 之前）
			 *   OnImGuiRender() —— 各层在这里 ImGui::Begin/End 提交要画的面板（只是登记，还没画）
			 *   End() —— 真正生成 draw data 并渲染到屏幕（必须最后调用）
			 * 这三步顺序不能乱，否则 ImGui 面板不会显示或崩溃。
			 */
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			// 窗口更新：交换双缓冲（把后台渲染好的画面显示出来）+ poll 处理输入事件
			// 这些事件会触发回调，最终回到上面的 OnEvent
			m_Window->OnUpdate();
		}
	}

	/*
	 * OnWindowClose —— 处理"窗口关闭"事件。
	 *
	 *   把 m_Running 置 false，让 Run 的 while 循环退出，引擎结束。
	 *   返回 true 表示"这个事件我处理了"，EventDispatcher 会据此把 e.Handled 置 true，
	 *   这样下面层就不会再收到这条关闭事件。
	 */
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}
