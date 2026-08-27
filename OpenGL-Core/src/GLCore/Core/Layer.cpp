/*
 * Layer.cpp —— Layer 基类构造函数实现
 *
 * 职责：实现 Layer 的构造函数，把传入的调试名存到 m_DebugName。
 *       五个虚函数回调都是空体（在头文件里直接 {}），所以本文件只有构造函数。
 *
 * 在引擎里的角色：每当用户 new 一个 Layer 子类（如 ExampleLayer）时，
 *       会先调用本基类构造初始化 m_DebugName，再执行子类自己的构造。
 *
 * 关键依赖：
 *   - glpch.h —— 预编译头，包含 <string> 等
 *   - Layer.h  —— 声明 Layer 类
 */
#include "glpch.h"
#include "Layer.h"

namespace GLCore {

	/*
	 * Layer 构造函数
	 *
	 *   debugName：仅用于调试显示的名字（默认参数在头文件声明里给出）。
	 *   : m_DebugName(debugName) —— 成员初始化列表：直接用 debugName 构造 m_DebugName 成员，
	 *                               比在函数体内赋值高效（少一次构造+赋值）。
	 */
	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
	}

}
