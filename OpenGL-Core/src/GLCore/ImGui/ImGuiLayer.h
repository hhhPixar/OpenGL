/*
 * ImGuiLayer.h
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     定义了 GLCore 引擎里的 ImGuiLayer 类。ImGuiLayer 是引擎与 Dear ImGui
 *     (一个即时模式 GUI 库,游戏开发里常用作调试面板)之间的"胶水层"。
 *
 * 在引擎里的角色:
 *     引擎采用"层栈(LayerStack)"架构,各功能以 Layer 形式叠在一起。ImGuiLayer
 *     作为一个特殊的 overlay(覆盖层)放在栈的最顶端,负责初始化、驱动、销毁
 *     整个 ImGui 系统。每一帧由 Application 在合适时机调用 Begin() 开启新帧、
 *     各业务层在 OnImGuiRender() 里画自己的调试面板、最后由 End() 提交渲染。
 *
 * 关键依赖:
 *     - Layer              : 引擎层基类,ImGuiLayer 继承它才能进层栈
 *     - 各种 Event         : 事件系统,ImGuiLayer 需要拦截鼠标事件(点 GUI 时别传到游戏)
 *     - Dear ImGui         : 实际的 GUI 库(本头文件里不直接 include imgui.h,在 .cpp 里 include)
 */

#pragma once

// #include 的作用:把指定头文件的内容在预处理阶段"复制粘贴"到本文件这里。
// 这样本文件就能使用别的文件里声明的类/函数。
#include "GLCore/Core/Layer.h"

#include "GLCore/Events/ApplicationEvent.h"
#include "GLCore/Events/KeyEvent.h"
#include "GLCore/Events/MouseEvent.h"

// namespace(命名空间):把代码包进一个有名字的作用域,避免跟别的库同名类冲突。
// 引擎所有代码都在 GLCore 这个命名空间下,因此外面用的时候要写 GLCore::ImGuiLayer。
namespace GLCore {

	// class A : public B —— 公有继承。表示 "A 是一种 B"。
	// ImGuiLayer 是一种 Layer,所以继承 Layer,从而能被加进层栈统一管理。
	// 子类会自动拥有父类 Layer 的成员,并可重写(override)其中的虚函数。
	class ImGuiLayer : public Layer
	{
	public:
		// 构造函数:对象创建时调用。这里把父类 Layer 的名字标签设为 "ImGuiLayer"。
		ImGuiLayer();
		// 析构函数:对象销毁时调用。= default 表示让编译器自动生成一个空实现,
		// 因为 ImGuiLayer 自己没有什么需要手动释放的资源(ImGui 的清理在 OnDetach 里做)。
		~ImGuiLayer() = default;

		// virtual(虚函数):允许子类重写该函数,并且通过父类指针/引用调用时能"多态"地命中子类版本。
		// override:明确告诉编译器"我要重写父类的虚函数",如果父类没有同名虚函数就报错——
		// 这是个安全检查,能防止你不小心拼错函数名而悄悄变成"新函数"。
		// OnAttach 在层被加进栈时调用一次,这里用来初始化 ImGui 和它的平台/渲染后端。
		virtual void OnAttach() override;
		// OnDetach 在层被移出栈时调用一次,这里用来反向关闭/销毁 ImGui。
		virtual void OnDetach() override;

		// Begin():每帧开始时调用,通知 ImGui 后端"准备新一帧的数据",然后开一个新帧。
		// 每个想画 GUI 的层会在 Begin() 之后、End() 之前画自己的面板。
		void Begin();
		// End():每帧结束时调用,让 ImGui 真正渲染出这一帧所有 UI。
		void End();

		// 事件回调:当窗口/输入系统产生事件(如鼠标点击)时,层栈会自顶向下把事件传给各层。
		// ImGuiLayer 用它拦截鼠标按下事件——如果点击发生在 ImGui 窗口内,就"吃掉"事件不往下传。
		// 注意:这里 OnEvent 不是 override(基类 Layer 的 OnEvent 已是虚函数),且带了类名前缀写法,
		// 属于声明风格,实际仍为成员函数。
		virtual void OnEvent(Event& event) override;
		// 专门处理"鼠标按下"事件。返回 true 表示事件已被 ImGui 处理,不应再传给下面的游戏层。
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
	private:
		// m_Time 保存一个时间值,可用于以时间为基准的 UI 动画/计算(本引擎目前主要留给子类扩展)。
		float m_Time = 0.0f;
	};

}
