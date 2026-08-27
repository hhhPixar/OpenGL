/* ============================================================================
 * WindowsInput.cpp —— WindowsInput 的具体实现(平台层 / Windows + GLFW)
 *
 * 这里实现 Input 抽象基类里那些 *Impl 纯虚函数,用 GLFW 的轮询 API 查询
 * 当前键盘/鼠标状态。同时,这里也是单例实例 s_Instance 被 new 出来的地方
 * (文件级静态变量初始化)。
 *
 * 实现特点:本类不保存任何状态,每次查询都实时问 GLFW。要拿到 GLFWwindow*,
 * 就通过 Application 单例 -> Window -> GetNativeWindow() 取回原生窗口指针,
 * 再 static_cast 回 GLFWwindow*。
 *
 * 关键依赖:
 *   - WindowsInput.h          : 本类声明
 *   - Application.h           : Application::Get() 单例,用来拿窗口
 *   - GLFW/glfw3.h            : glfwGetKey/glfwGetMouseButton/glfwGetCursorPos
 * ========================================================================== */
#include "glpch.h"
#include "WindowsInput.h"

#include "GLCore/Core/Application.h"
#include <GLFW/glfw3.h>

namespace GLCore {

	/* 单例实例化:这行是整个输入系统的"出生点"。它是文件级静态变量,程序
	 * 启动时(进入 main 之前)就会被初始化——new 一个 WindowsInput 对象,把
	 * 它的地址赋给基类的静态指针 s_Instance。之后所有 Input::IsKeyPressed(...)
	 * 静态调用,内部 s_Instance->xxxImpl 多态调到的就是这个对象。
	 * new 出来的对象这里不 delete(进程结束就回收),单例本就常驻。 */
	Input* Input::s_Instance = new WindowsInput();

	/* 查询某键是否被按下。glfwGetKey 返回 GLFW_PRESS / GLFW_RELEASE / GLFW_REPEAT。
	 * 把 PRESS 和 REPEAT 都算"按下中"(REPEAT 是按住不放的连续状态)。
	 * Application::Get() 是应用单例,GetWindow() 拿 Window 抽象,GetNativeWindow()
	 * 拿 void* 原生窗口,static_cast 回 GLFWwindow* 给 GLFW 用。 */
	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	/* 查询某鼠标键是否按下。glfwGetMouseButton 返回 PRESS/RELEASE,这里只认 PRESS。 */
	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	/* 查询鼠标位置,返回 (x, y) 像素坐标,左上角为原点。glfwGetCursorPos
	 * 用 double* 输出参数(double xpos, ypos; 传 &xpos, &ypos 进去),取回后
	 * 转 float。return { ... } 是 C++11 的花括号初始化,直接构造 std::pair 返回。 */
	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	/* 只取鼠标 x。复用 GetMousePositionImpl,auto[x, y] 是 C++17 结构化绑定,
	 * 把 pair 拆成 x 和 y 两个变量。 */
	float WindowsInput::GetMouseXImpl()
	{
		auto[x, y] = GetMousePositionImpl();
		return x;
	}

	/* 只取鼠标 y,同上。 */
	float WindowsInput::GetMouseYImpl()
	{
		auto[x, y] = GetMousePositionImpl();
		return y;
	}

}