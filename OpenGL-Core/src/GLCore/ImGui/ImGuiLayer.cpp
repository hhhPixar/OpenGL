/*
 * ImGuiLayer.cpp
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     ImGuiLayer 类的实现文件。把 .h 里声明的方法真正写出来。
 *
 * 在引擎里的角色:
 *     负责把 Dear ImGui 这个 GUI 库接入到 GLCore 引擎里。它做四件事:
 *       1. OnAttach()  —— 初始化 ImGui 上下文 + 设风格 + 接好 GLFW 平台后端
 *                          和 OpenGL3 渲染后端;
 *       2. Begin()    —— 每帧开头,让后端准备新数据并开新帧;
 *       3. End()      —— 每帧末尾,把 ImGui 这帧所有控件渲染上屏(含多视口);
 *       4. OnEvent()  —— 拦截落在 ImGui 窗口上的鼠标点击,不传给游戏逻辑。
 *
 * 关键依赖(三个外部库的头):
 *     - imgui.h                      : Dear ImGui 核心头,提供 ImGui::XXX 接口
 *     - imgui_impl_glfw.h            : ImGui 的"平台后端",把 GLFW 窗口/键盘/鼠标输入接到 ImGui
 *     - imgui_impl_opengl3.h         : ImGui 的"渲染后端",用 OpenGL3 着色器画 ImGui 的顶点
 *     - glad/glad.h                  : OpenGL 函数加载器,让代码能调用现代 OpenGL 接口
 *     - GLFW/glfw3.h                 : 窗口/输入库,这里拿原生窗口句柄喂给 ImGui 后端
 */

#include "glpch.h"
// glpch.h 是本工程的预编译头,里面集中 include 了常用的 STL 头和 Log.h,
// 编译器会预先把它编一次、之后所有 cpp 直接复用,从而大幅加速编译。
#include "ImGuiLayer.h"

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

// Application 是引擎的"大管家",单例,管着主窗口、层栈和主循环。
// 这里要用它拿到主窗口的原生句柄,交给 ImGui 的 GLFW 后端。
#include "../Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace GLCore {

	ImGuiLayer::ImGuiLayer()
		// 初始化列表:在进入构造函数体之前,先初始化父类 Layer 并传入名字 "ImGuiLayer"。
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{
		// ===== 1. 创建 ImGui 上下文 =====
		// IMGUI_CHECKVERSION 确保头文件和编译进来的 ImGui 版本一致(防止头库不匹配的诡异 bug)。
		// CreateContext 创建 ImGui 的核心上下文,后续所有 ImGui:: 调用都作用于这个上下文。
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// ImGuiIO 是 ImGui 的"输入输出总管",几乎所有全局配置(键盘、字号、视口等)都从这里改。
		ImGuiIO& io = ImGui::GetIO();
		// io.ConfigFlags 是一组配置位标记,用 |= 逐个"或"上去即可开启某项特性。
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		// Docking:允许把多个面板拖拽停靠在一起组成大窗口(类似 IDE 的标签布局)。
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		// Viewports:允许 ImGui 窗口飘出主窗口,变成操作系统级别的独立窗口(多视口)。
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// ===== 2. 设置样式 =====
		// StyleColorsDark 选一套深色主题配色。ImGui 自带几套配色函数,这是最常用的一种。
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// 开启多视口后,独立窗口边角不能像主窗口那样圆角,否则视觉不一致;同时背景要不透明。
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;                    // 窗口圆角设为 0(直角)
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;       // 窗口背景 alpha 设为 1(完全不透明)
		}

		// ===== 3. 接好平台 / 渲染后端 =====
		// Application::Get() 是单例访问点(返回 Application 引用 &),整个进程里只有一个 Application。
		// 从它拿到主窗口,再拿到底层 GLFWwindow* —— ImGui 的 GLFW 后端需要这个原生指针。
		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// static_cast<目标类型>(源):C++ 的安全类型转换。这里把 void* 转回 GLFWwindow*。
		// ImGui_ImplGlfw_InitForOpenGL:让 GLFW 后端从指定窗口接收输入并喂给 ImGui。
		//   参数 true 表示要安装自己的回调,并在调用原回调前先让 ImGui 处理事件。
		// ImGui_ImplOpenGL3_Init:告诉渲染后端用 "#version 410" 的 GLSL 着色器画 UI。
		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		// 关闭顺序与初始化相反:先关渲染后端、再关平台后端、最后销毁 ImGui 上下文。
		// 这样避免在还有东西要渲染时把上下文提前销毁导致的崩溃。
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		// 每帧开头三步,顺序不能乱:
		// 1) 渲染后端准备新一帧(它可能要更新顶点缓冲等);
		ImGui_ImplOpenGL3_NewFrame();
		// 2) 平台后端准备新一帧(读取当前窗口尺寸、鼠标位置、时间等);
		ImGui_ImplGlfw_NewFrame();
		// 3) ImGui 核心"开新帧"——之后业务层就能调用 ImGui::Begin/Text/Button 等画 UI 了。
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		// DisplaySize 必须告诉 ImGui 当前可绘制区域大小,否则布局会错。
		// 这里用主窗口的宽高来设置。(float) 是 C 风格的强制转换,把整数转成 float。
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();                                  // 把这一帧所有 UI 命令整理成"绘制数据(DrawData)"
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  // 渲染后端真正把 DrawData 画上屏

		// 如果开了多视口,还要处理那些飘在主窗口之外的独立窗口。
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			// 多视口渲染需要切换 OpenGL 上下文,先备份当前上下文,等会恢复。
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();              // 更新所有平台窗口的状态
			ImGui::RenderPlatformWindowsDefault();       // 依次渲染每个平台窗口
			glfwMakeContextCurrent(backup_current_context);  // 恢复原来的上下文,避免后续渲染错乱
		}
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		// EventDispatcher:把一个抽象的 Event 根据真实类型分发给对应的处理函数。
		EventDispatcher dispatcher(event);
		// Dispatch<具体事件类型>(处理函数):如果 event 确实是 MouseButtonPressedEvent,
		// 就调用本类的 OnMouseButtonPressed 来处理它。
		// GLCORE_BIND_EVENT_FN 是个宏,把成员函数绑成可调用对象时顺便传进 this。
		dispatcher.Dispatch<MouseButtonPressedEvent>(GLCORE_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressed));
	}

	bool ImGuiLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		// ImGuiIO 里 WantCaptureMouse 表示"这次鼠标事件 ImGui 想要"。
		// 返回 true 表示事件被 ImGui 消费掉了,层栈就不再把它往下传给游戏逻辑——
		// 这样点击调试面板时,角色不会误开火。
		ImGuiIO io = ImGui::GetIO();
		return io.WantCaptureMouse;
	}

}
