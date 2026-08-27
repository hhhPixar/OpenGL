#pragma once

/*
 * Layer.h —— 层(Layer)抽象基类
 *
 * 职责：定义引擎里"一层"的概念，每层可以是游戏画面、调试 UI、ImGui 面板等。
 *       Layer 提供五个虚函数回调(OnAttach/OnDetach/OnUpdate/OnImGuiRender/OnEvent)，
 *       由 Application 在合适时机调用，子类重写它们实现自己的行为。
 *
 * 在引擎里的角色：LayerStack 管理多个 Layer，主循环每帧把更新/渲染/事件都
 *       按顺序派发给每一层，是引擎"组合式功能"的核心抽象。比如：
 *       - 游戏层 OnUpdate 更新世界
 *       - ImGui 层 OnImGuiRender 画调试面板
 *       - 事件层 OnEvent 处理鼠标键盘
 *
 * 关键依赖：
 *   - Core.h        —— 提供 GLCORE_BIND_EVENT_FN 等宏（这里只是间接用到）
 *   - Timestep.h    —— OnUpdate 接收的帧时间参数类型
 *   - Events/Event.h—— OnEvent 接收的事件参数类型
 *
 * 设计要点：这是"抽象基类 + 虚函数回调"的典型多态设计。
 *   - virtual 关键字：让该函数支持"动态绑定"，即调用时根据"对象实际类型"而非"指针/引用声明类型"
 *     找到正确版本。例如 Layer* 指向子类 ImGuiLayer，调用 OnAttach() 会跑子类的版本。
 *   - {} 空实现：不是纯虚函数(= 0)，而是给一个默认空体；子类可以"选择重写"，也可以不写。
 *   - = default：让编译器生成默认实现（这里析构函数用 default 表示"普通自动析构"）。
 */

#include "Core.h"
#include "Timestep.h"
#include "../Events/Event.h"

namespace GLCore {

	/*
	 * Layer —— 引擎里所有"层"的基类
	 *
	 * 子类只需要重写自己关心的几个回调，例如游戏层只重写 OnUpdate/OnImGuiRender。
	 * 五个回调由谁、何时调用：
	 *   OnAttach()        —— 加入层栈(Push)时由 LayerStack 调用一次，可用来初始化资源
	 *   OnDetach()        —— 从层栈移除(Pop)时调用一次，可用来清理资源
	 *   OnUpdate(ts)      —— 主循环每帧由 Application::Run 调用，参数是本帧耗时
	 *   OnImGuiRender()   —— 主循环每帧在 ImGui Begin/End 之间调用，用于画调试面板
	 *   OnEvent(e)        —— 有事件时由 Application::OnEvent 从层栈顶到底逆序调用
	 */
	class Layer
	{
	public:
		// 构造，name 只用于调试显示（默认参数"Layer"）
		Layer(const std::string& name = "Layer");

		/*
		 * 虚析构函数 = default。
		 * 必须 virtual：因为会通过 Layer* 基类指针 delete 子类对象，
		 * 如果不是虚函数，析构只走基类版本，子类资源会泄漏。
		 * = default 让编译器自动生成。
		 */
		virtual ~Layer() = default;

		/*
		 * 下列五个函数都是 virtual 且带空体 {}：
		 *   - virtual 让派生类重写时可"覆盖"
		 *   - {} 表示"默认什么都不做"，子类不重写就保持空行为
		 *   - 若写成 virtual void Foo() = 0; 就是"纯虚函数"，强制子类必须实现，但本类不能实例化
		 *     这里故意不写 = 0，让子类可以"挑感兴趣的写"，灵活一些。
		 */
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		/*
		 * GetName() —— 返回调试名字的 const 引用。
		 *
		 *   const std::string& —— 返回引用避免拷贝整个字符串；
		 *   函数后 const —— 表示"这个函数不修改对象"，const 对象也能调用；
		 *   inline —— 头文件里定义、允许内联展开。
		 */
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		/*
		 * protected：受保护成员，类自己和派生类可直接访问，外部不能。
		 * m_DebugName 只是用于日志/调试，子类可能想设它，所以放 protected。
		 */
		std::string m_DebugName;
	};

}
