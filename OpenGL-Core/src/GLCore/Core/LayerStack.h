#pragma once

/*
 * LayerStack.h —— 层栈容器
 *
 * 职责：管理一组 Layer 的添加/移除，并按"layer 段 + overlay 段"两段存储。
 *       普通层(layer)放在前半段，覆盖层(overlay，如 ImGui)放在后半段（栈顶最后）。
 *       提供迭代器让 Application::Run 可以用范围 for 遍历所有层。
 *
 * 在引擎里的角色：Application 持有一个 LayerStack 成员，主循环里遍历它调用 OnUpdate/OnImGuiRender；
 *       事件来时也通过它逆序（从后往前）分发事件给各层。
 *
 * 关键依赖：
 *   - Core.h —— 通用宏（间接依赖）
 *   - Layer.h —— Layer 类型定义
 *   - <vector> —— 动态数组容器，存 Layer* 指针
 *
 * 设计要点：
 *   - 这里存的是裸指针 Layer*，不是智能指针。所有权约定：LayerStack 在析构时 delete 全部指针，
 *     即"层栈拥有所有层对象"。所以外部 push 进来的层必须是用 new 创建在堆上的。
 *   - m_LayerInsertIndex 把数组分成两半：
 *       [0, m_LayerInsertIndex)  —— 普通层(layer)，按下标顺序排
 *       [m_LayerInsertIndex, end) —— 覆盖层(overlay)，栈顶是最后一个元素
 *     这样普通层 Push 时只在前半段插队、不影响 overlay 的位置。
 */
#include "Core.h"
#include "Layer.h"

#include <vector>

namespace GLCore {

	/*
	 * LayerStack —— 层栈
	 *
	 *   Push：把层加入容器（PushLayer 加到普通段，PushOverlay 加到末尾即栈顶）
	 *   Pop ：按指针移除指定层（先调用其 OnDetach）
	 *   begin()/end()：让本对象可以用范围 for（for (Layer* layer : m_LayerStack)）遍历
	 *
	 *   注意 begin/end 只覆盖整个 m_Layers（layer 段和 overlay 段都在内），
	 *   不单独暴露 m_LayerInsertIndex 给外部。
	 */
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		// 把 layer 加入"普通层段"（位于现有 overlay 之前），调用其 OnAttach
		void PushLayer(Layer* layer);
		// 把 overlay 加入到末尾（成为栈顶），调用其 OnAttach
		void PushOverlay(Layer* overlay);
		// 从普通层段移除 layer，调用其 OnDetach；找不到则什么都不做
		void PopLayer(Layer* layer);
		// 从 overlay 段移除 overlay，调用其 OnDetach；找不到则什么都不做
		void PopOverlay(Layer* overlay);

		/*
		 * begin()/end() —— 返回 std::vector 迭代器，使本对象可用于范围 for 循环。
		 *
		 *   迭代器(iterator)类似指针，可以 ++ 前进、* 解引用取元素，
		 *   begin() 指向首元素、end() 指向"尾后位置"（不存在的元素）。
		 *   范围 for：for (Layer* layer : stack) 等价于从 begin 到 end 遍历。
		 */
		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		// 容器：所有层指针（普通层 + 覆盖层）按顺序排在一起
		std::vector<Layer*> m_Layers;
		// 普通层段的"下一个插入位置"，也正好是 overlay 段的起点
		uint32_t m_LayerInsertIndex = 0;
	};

}
