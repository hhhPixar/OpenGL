/*
 * 文件: SandboxApp.cpp
 * 角色: 这是 OpenGL-Sandbox 客户端(可执行程序)的入口文件。
 *       Sandbox 是"沙盒"——一个最小可运行的实验台,方便你在这里试新东西而不影响 Examples。
 *       它复用了和 ExampleApp 一模一样的"用引擎搭应用"模板:
 *       1) 定义继承自 GLCore::Application 的子类 Sandbox;
 *       2) 构造函数里 PushLayer 压入自定义的 SandboxLayer;
 *       3) main() 里 make_unique 创建并 Run() 启动主循环。
 *       和 ExampleApp 唯一不同的是:这里没有给 Application 传名字,
 *       所以窗口会用引擎默认标题。
 * 关键依赖: GLCore.h(引擎公开头文件)、SandboxLayer.h(沙盒自定义层)。
 */

#include "GLCore.h"          // 引擎公开 API:Application、Layer、Run() 等
#include "SandboxLayer.h"     // 沙盒自己定义的空壳层(各个回调里暂时只有注释)

// using namespace 是"命名空间别名"用法:把 GLCore 命名空间里的名字直接引入,
// 这样下面写 Application 就不必加 GLCore:: 前缀。.cpp 里用比较安全。
using namespace GLCore;

// 定义 Sandbox 类,公有继承自 GLCore::Application。
// "public Application" 表示 is-a 关系:Sandbox 就是一个 Application,
// 引擎通过基类接口管理它、调用 Run()。
class Sandbox : public Application
{
public:
	// 构造函数:没有像 Example 那样在初始化列表里传窗口名字(走引擎默认标题)。
	// 在函数体里 PushLayer 把自定义层压入层栈。引擎每帧会依次调用各层的 OnUpdate/OnImGuiRender 等。
	// new 出来的 SandboxLayer 所有权交给引擎。
	Sandbox()
	{
		PushLayer(new SandboxLayer());
	}
};

// 程序入口。每个可执行客户端都有自己的 main()。
int main()
{
	// std::make_unique<Sandbox>() 在堆上构造 Sandbox 对象,返回 std::unique_ptr。
	// unique_ptr 是智能指针:超出作用域自动 delete,避免内存泄漏(现代 C++ 推荐写法)。
	std::unique_ptr<Sandbox> app = std::make_unique<Sandbox>();
	// Run() 进入引擎主循环:处理事件、更新层、渲染、交换缓冲,直到窗口关闭才返回。
	app->Run();
}
