/*
 * 文件: SandboxLayer.cpp
 * 角色: SandboxLayer 类的实现。这是一个"空壳模板",各个生命周期回调里只有注释占位,
 *       等你照着 ExampleLayer 的写法填入实际逻辑。它同时是学习一个 Layer 生命周期的好例子:
 *       OnAttach(初始化) -> OnUpdate(每帧渲染) / OnEvent(输入) / OnImGuiRender(GUI)
 *       -> OnDetach(清理)。
 * 关键依赖: SandboxLayer.h(本层声明)、GLCore 工具(EnableGLDebugging 等,已包含在头文件)。
 * 用法提示: 想试新东西时,在对应回调里写代码即可,例如在 OnAttach 里创建 VAO/VBO、
 *           在 OnUpdate 里 glDrawElements 画几何体,参考 ExampleLayer.cpp 的完整范例。
 */

#include "SandboxLayer.h"

// 把两个命名空间的名字直接引入,后续写 Event、Timestep 等更简洁。
// using namespace 是"命名空间别名"用法,初学者提示:.cpp 里用比较安全,头文件里一般避免。
using namespace GLCore;
using namespace GLCore::Utils;

// 构造函数(目前为空)。照着 ExampleLayer 时,可用初始化列表给相机控制器传宽高比。
SandboxLayer::SandboxLayer()
{
}

// 析构函数(目前为空)。资源清理通常放在 OnDetach 里做,所以这里留空。
// 基类析构是 virtual,这里需要有个实现体。
SandboxLayer::~SandboxLayer()
{
}

// 层被加入层栈时调用一次。EnableGLDebugging 打开 GL 调试输出,方便发现错误。
// 想初始化 OpenGL 资源(着色器、缓冲等)就在 // Init here 处加代码。
void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

	// Init here
}

// 层被移出层栈时调用一次。想释放资源就在 // Shutdown here 处加代码
// (例如 glDeleteVertexArrays / glDeleteBuffers)。
void SandboxLayer::OnDetach()
{
	// Shutdown here
}

// 有输入事件时调用。想处理鼠标/键盘/窗口事件就在 // Events here 处加代码,
// 例如用 EventDispatcher 分派,或交给 OrthographicCameraController。
void SandboxLayer::OnEvent(Event& event)
{
	// Events here
}

// 每帧调用一次,ts 是本帧耗时(秒)。想渲染就在 // Render here 处加代码
// (例如 glClear -> glUseProgram -> 设 uniform -> glDrawElements)。
void SandboxLayer::OnUpdate(Timestep ts)
{
	// Render here
}

// 每帧的 ImGui 阶段调用。想画调试面板就在 // ImGui here 处加代码
// (例如 ImGui::Begin ... ImGui::End)。
void SandboxLayer::OnImGuiRender()
{
	// ImGui here
}
