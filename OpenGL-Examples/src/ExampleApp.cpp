/*
 * 文件: ExampleApp.cpp
 * 角色: 这是 OpenGL-Examples 客户端(可执行程序)的入口文件。
 *       它演示了"如何用 GLCore 引擎搭一个应用"的标准模板:
 *       1) 定义一个继承自 GLCore::Application 的子类 Example;
 *       2) 在构造函数里调用 PushLayer 把自定义的 ExampleLayer 压入引擎层栈;
 *       3) 在 main() 里用 std::make_unique 创建应用实例并调用 Run() 启动主循环。
 * 关键依赖: GLCore.h(引擎公开头文件,提供 Application 类)、ExampleLayer.h(本客户端自定义层)。
 */

#include "GLCore.h"          // 引擎公开 API:Application、Layer、Event、Run() 等
#include "ExampleLayer.h"    // 本客户端自己写的渲染层,负责画彩色四边形

// using namespace 是"命名空间别名"用法:把 GLCore 命名空间里的名字直接引入,
// 这样下面写 Application 就不用写 GLCore::Application,代码更简洁。
// 初学者提示:头文件里一般不这么用(避免污染),.cpp 里用比较安全。
using namespace GLCore;

// 定义 Example 类,公有继承自 GLCore::Application。
// "public Application" 表示这是一个"是一个(is-a)"关系:Example 就是一个 Application。
// 引擎会通过基类指针管理它、调用它的 Run()。子类只需要在构造函数里把需要的 Layer 压进去即可。
class Example : public Application
{
public:
	// 构造函数:初始化列表 ": Application("OpenGL Examples")" 表示先调用基类构造,
	// 把窗口标题传进去(基类会用这个名字创建窗口)。初学者提示:初始化列表比在函数体里赋值更高效,
	// 因为它直接构造成员,而不是先默认构造再赋值。
	Example()
		: Application("OpenGL Examples")
	{
		// PushLayer 把一个 Layer 压入引擎的层栈。引擎每帧会依次调用各层的 OnUpdate/OnImGuiRender 等。
		// 这里 new 出来的 ExampleLayer 的所有权交给引擎,引擎会在合适时机负责释放。
		PushLayer(new ExampleLayer());
	}
};

// 程序入口。每个可执行客户端都有自己的 main()。
int main()
{
	// std::make_unique<Example>() 在堆上构造一个 Example 对象,返回 std::unique_ptr<Example>。
	// unique_ptr 是智能指针:超出作用域自动 delete,避免内存泄漏(现代 C++ 推荐写法,替代裸 new/delete)。
	// 这里用基类指针也可以,但用具体类型 Example 同样能调用 Run()(Run 是 Application 的成员)。
	std::unique_ptr<Example> app = std::make_unique<Example>();
	// Run() 进入引擎主循环:处理事件、更新层、渲染、交换缓冲,直到窗口关闭才返回。
	app->Run();
}
