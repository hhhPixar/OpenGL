/*
 * LayerStack.cpp —— 层栈实现
 *
 * 职责：实现层栈的构造/析构/增删函数。重点：析构时 delete 全部层指针（所有权），
 *       以及用 m_LayerInsertIndex 把 layer 与 overlay 分两段插入/查找。
 *
 * 在引擎里的角色：被 Application 间接使用——Application 的 PushLayer/PushOverlay 都委托给这里。
 *
 * 关键依赖：
 *   - glpch.h      —— 预编译头（含 <algorithm> 用于 std::find，<vector> 等）
 *   - LayerStack.h —— 本类声明
 *   - Layer.h      —— OnAttach/OnDetach 回调
 *
 * 所有权说明：m_Layers 存的是裸指针 Layer*，析构函数里 delete 全部，
 *   也就是 LayerStack "拥有" 这些层对象。push 进来的必须是 new 出来的堆对象，
 *   不能传栈上对象或别的智能管过的对象，否则会双重释放。
 */
#include "glpch.h"
#include "LayerStack.h"

namespace GLCore {

	// 默认构造：m_LayerInsertIndex 已在头文件初始化为 0，函数体为空
	LayerStack::LayerStack()
	{
	}

	/*
	 * 析构：范围 for 遍历 m_Layers，对每个 Layer* 调用 delete。
	 *
	 *   for (Layer* layer : m_Layers) —— 范围 for：依次取每个元素到 layer。
	 *   delete layer —— 释放这个堆对象；因为是虚析构(基类 Layer 析构是 virtual)，
	 *     即使通过基类指针 delete 也会正确调用子类的析构链，不会泄漏子类资源。
	 *
	 *   这是"手动 RAII"——析构时清理，但不像 unique_ptr 那样自动，需要开发者记得写。
	 */
	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
			delete layer;
	}

	/*
	 * PushLayer —— 把 layer 加到"普通层段"末尾（overlay 段之前）。
	 *
	 *   m_LayerInsertIndex 是普通层段长度，也是 overlay 段起点。
	 *   emplace 在指定位置"原地构造插入"一个元素：插入到 begin() + m_LayerInsertIndex 处，
	 *   即"overlay 段的最前面"，把 overlay 往后挤一格——保证 overlay 永远在普通层之后。
	 *   之后 m_LayerInsertIndex++ 维护计数，再调用 OnAttach()。
	 */
	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
		layer->OnAttach();
	}

	/*
	 * PushOverlay —— 把 overlay 加到末尾（栈顶最后）。
	 *   emplace_back 在 vector 末尾追加，不需要动 m_LayerInsertIndex，再调用 OnAttach()。
	 */
	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	/*
	 * PopLayer —— 从普通层段[0, m_LayerInsertIndex)移除指定 layer。
	 *
	 *   std::find 在一段范围里查找等于 layer 的指针，返回迭代器 it。
	 *   auto —— 让编译器自动推断类型（这里是 std::vector<Layer*>::iterator），少打字。
	 *   若没找到，it == end（这里 end 用 begin + m_LayerInsertIndex 表示），什么都不做；
	 *   找到了就先 OnDetach()，再 erase(it) 删除元素，并把 m_LayerInsertIndex--。
	 *   注意：只 delete 数组里的指针，不 delete 对象本身——这里只是"从层栈摘下来"。
	 */
	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	/*
	 * PopOverlay —— 从 overlay 段[m_LayerInsertIndex, end)移除指定 overlay。
	 *   思路与 PopLayer 相同，只是搜索范围换到后半段，且不动 m_LayerInsertIndex。
	 */
	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	}

}
