// Mostly from Hazel
#pragma once

/*
 * Core.h —— 引擎核心工具宏定义头文件
 *
 * 职责：本文件不包含类或函数，只定义整个引擎复用的一些"预处理宏"，
 *       主要有：断言宏 GLCORE_ASSERT、位标志宏 BIT、事件绑定宏 GLCORE_BIND_EVENT_FN。
 *       这些宏被 Events/Event.h、Application.h 等广泛使用，是最底层的公共依赖。
 *
 * 关键依赖：
 *   - <memory>            —— 为 GLCORE_BIND_EVENT_FN 中 std::bind 的使用做准备（实际依赖 std::placeholders，由 glpch.h 提供）
 *   - Log.h（间接）        —— GLCORE_ASSERT 在断言失败时会调用 LOG_ERROR 宏打印日志
 *   - GLCORE_DEBUG 宏      —— 由编译配置（CMake）定义，用来开启或关闭断言
 *
 * 设计说明：这些宏集中放在一个头文件里，方便统一维护和 #include。
 */

#include <memory>

/*
 * 断言开关：只有在 Debug 配置下（CMake 定义了 GLCORE_DEBUG）才会启用断言。
 * 这样在 Release 构建里可以完全去掉断言检查，零性能开销。
 */
#ifdef GLCORE_DEBUG
	#define GLCORE_ENABLE_ASSERTS
#endif

/*
 * GLCORE_ASSERT —— 断言宏
 *
 * 用法：GLCORE_ASSERT(条件表达式, "出错信息", 可选参数...);
 *
 *   - x 为"条件"，为 false 时表示程序出现了不该出现的状态
 *   - __VA_ARGS__ 是 C 语言可变参数宏，接收 printf 风格的格式串和参数
 *   - __debugbreak() 是编译器内置函数：触发调试断点，让程序停下来便于排查
 *
 * 为什么用宏而不是函数？因为宏可以直接使用 __debugbreak() 打断调试器，
 * 并且可以在 Release 下被完全替换为空（第二个 #define 分支），没有任何调用开销。
 */
#ifdef GLCORE_ENABLE_ASSERTS
	#define GLCORE_ASSERT(x, ...) { if(!(x)) { LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define GLCORE_ASSERT(x, ...)
#endif

/*
 * BIT(x) —— 位标志宏，把序号 x 转成 1 << x 的位掩码
 *
 *   例如 BIT(0)=1, BIT(1)=2, BIT(2)=4, BIT(3)=8 ...
 *
 * 为什么用位掩码？一个事件可能同时属于多个类别（如"鼠标按键"既是 Input 类又是 Mouse 类），
 * 用二进制位组合可以高效地用一个 int 表示多个分类标志，再用位与运算(&)判断"是否包含某类"。
 * 在 Event.h 的 EventCategory 枚举里就会用到它。
 */
#define BIT(x) (1 << x)

/*
 * GLCORE_BIND_EVENT_FN —— 事件回调绑定宏
 *
 *   用法：GLCORE_BIND_EVENT_FN(Application::OnEvent)
 *   展开为：std::bind(&Application::OnEvent, this, std::placeholders::_1)
 *
 *   为什么需要它？Application 的成员函数 OnEvent 不能直接作为 GLFW 的回调，
 *   因为成员函数需要 this 指针才能调用。std::bind 把"成员函数指针 + this + 占位符"
 *   绑成一个无参数签名的可调用对象(std::function)，其中 _1 表示"保留第一个参数"
 *   （即将来事件发生时传入的 Event&），这样就可以把一个"对象方法"变成一个"普通函数"传给底层。
 *
 *   注意 Application.cpp 内部还定义了一个 BIND_EVENT_FN 宏，思路一样但绑的是本类的函数，
 *   写法略不同，两者会合起来讲。
 *
 *   this 必须存在，所以这个宏只能在类的非静态成员函数里使用。
 */
#define GLCORE_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
