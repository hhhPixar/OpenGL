#pragma once

/*
 * Application.h —— 引擎主类（单例）
 *
 * 职责：Application 是整个引擎的"大脑"，单例存在，负责：
 *   1) 持有一个 Window（窗口，含 OpenGL 上下文）
 *   2) 持有一个 LayerStack（层栈，装所有功能层）
 *   3) 持有一个 ImGuiLayer（作为 overlay 的调试 UI 层，单独留指针方便 Begin/End）
 *   4) 提供 Run() 主循环：每帧算 Timestep、更新各层、渲染 ImGui、交换缓冲
 *   5) 提供 OnEvent()：把窗口事件派发给各层处理
 *
 * 在引擎里的角色：客户端代码通常写一个继承自 Application 的类（或在 main 里直接用），
 *       创建它、PushLayer 各种游戏层，然后调用 Run() 启动主循环直到关窗口。
 *
 * 关键依赖：
 *   - Core.h          —— 通用宏（GLCORE_ASSERT、GLCORE_BIND_EVENT_FN）
 *   - Window.h        —— 窗口抽象类（用 std::unique_ptr 持有）
 *   - LayerStack.h    —— 层栈容器
 *   - Events/Event.h、ApplicationEvent.h —— 事件类型（OnEvent 接收、OnWindowClose 处理）
 *   - Timestep.h      —— 帧时间类型
 *   - ImGui/ImGuiLayer.h —— ImGui 调试层
 *
 * 设计要点：
 *   - 单例：static s_Instance + static Get()，全局唯一，方便各处拿到 Application。
 *   - Window 用 std::unique_ptr 管理，独占所有权，Application 析构时自动释放窗口。
 *   - m_ImGuiLayer 是裸指针：对象 new 出来后 PushOverlay 进 LayerStack（层栈拥有它），
 *     Application 自己只留个观察指针，方便在 Run 里调 Begin/End。
 *   - virtual ~Application() = default：为客户端继承 Application 留虚析构。
 */

#include "Core.h"

#include "Window.h"
#include "LayerStack.h"
#include "../Events/Event.h"
#include "../Events/ApplicationEvent.h"

#include "Timestep.h"

#include "../ImGui/ImGuiLayer.h"

namespace GLCore {

	/*
	 * Application —— 引擎主类，单例。
	 *
	 * 使用流程：
	 *   Application app("名字", 1280, 720);   // 构造时初始化日志/窗口/事件回调/ImGui
	 *   app.PushLayer(new MyGameLayer());    // 加入业务层
	 *   app.Run();                          // 进入主循环
	 */
	class Application
	{
	public:
		/*
		 * 构造函数，带默认参数（名字/宽/高）。
		 *   name   窗口标题
		 *   width/height 窗口客户区像素
		 * 构造内部会：Log::Init() -> 创建 Window -> SetEventCallback 绑定 OnEvent
		 *   -> 创建 ImGuiLayer 并 PushOverlay。
		 */
		Application(const std::string& name = "OpenGL Sandbox", uint32_t width = 1280, uint32_t height = 720);

		// 虚析构 = default，方便子类继承后正确释放
		virtual ~Application() = default;

		// 主循环：每帧推进直到窗口关闭，详见 Application.cpp
		void Run();

		/*
		 * OnEvent —— 事件入口。
		 *   窗口产生事件后，会通过构造时绑定的回调调用到这里。
		 *   这里用 EventDispatcher 派发 WindowClose 等，再从层栈顶到底逆序传给各层 OnEvent。
		 */
		void OnEvent(Event& e);

		// 加入普通层（委托给 LayerStack::PushLayer）
		void PushLayer(Layer* layer);
		// 加入覆盖层（委托给 LayerStack::PushOverlay，通常用于 UI/调试层）
		void PushOverlay(Layer* layer);

		/*
		 * GetWindow —— 返回窗口引用，inline 头文件内定义。
		 *   返回 Window& 引用而不是指针，避免空指针语义；*m_Window 解引用 unique_ptr 拿到对象。
		 */
		inline Window& GetWindow() { return *m_Window; }

		/*
		 * Get —— 单例访问入口。
		 *   static：不需要对象就能调用，Application::Get()。
		 *   返回 Application& 引用，s_Instance 在构造函数里被设为 this。
		 *   这是"饿汉式单例"的简化版——构造时登记自己。
		 */
		inline static Application& Get() { return *s_Instance; }
	private:
		// 处理窗口关闭事件：把 m_Running 置 false，主循环退出
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		/*
		 * m_Window：窗口对象，std::unique_ptr 独占所有权。
		 *   unique_ptr 是"独占式智能指针"：只能有一个 unique_ptr 拥有该对象，
		 *   析构时自动 delete，无需手写清理（RAII：资源获取即初始化，生命周期绑定到变量）。
		 *   相比裸指针，避免忘记 delete 造成内存泄漏。
		 */
		std::unique_ptr<Window> m_Window;
		/*
		 * m_ImGuiLayer：裸指针，但所有权实际在 LayerStack（PushOverlay 后由层栈 delete）。
		 *   这里留指针只是方便在 Run() 里调 ImGuiLayer::Begin/End。
		 */
		ImGuiLayer* m_ImGuiLayer;
		// 主循环是否继续运行；窗口关闭事件会把它置 false
		bool m_Running = true;
		// 层栈，装所有层（含 overlay 段的 ImGuiLayer）
		LayerStack m_LayerStack;
		// 上一帧的时间(秒)，用于算 Timestep = 当前时间 - 上一帧时间
		float m_LastFrameTime = 0.0f;
	private:
		/*
		 * s_Instance：静态单例指针，全引擎唯一。
		 *   静态成员属于类本身，需要在 .cpp 里定义一次（见 Application.cpp）。
		 *   构造函数里 if(!s_Instance) 检查 + 赋值 this，保证只登记一个。
		 */
		static Application* s_Instance;
	};

}
