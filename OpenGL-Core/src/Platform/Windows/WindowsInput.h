#pragma once
/* ============================================================================
 * WindowsInput.h —— Input 抽象接口的 GLFW 实现(平台层 / Windows)
 *
 * 这是 Input 的具体实现:用 glfwGetKey / glfwGetMouseButton / glfwGetCursorPos
 * 轮询当前键盘和鼠标状态。它本身不存任何状态——每次查询都直接问 GLFW 当下
 * 的实时状态,所以不需要保存历史。
 *
 * 这个类不直接被上层 new;它在 WindowsInput.cpp 里被一次性实例化并赋给
 * Input::s_Instance,之后所有 Input::IsKeyPressed(...) 静态调用都转调到这里。
 *
 * 关键依赖:
 *   - Input.h : 被实现的抽象基类(继承 Input)
 *   (glfw3.h 在 .cpp 里 include,这里不需要)
 * ========================================================================== */

#include "GLCore/Core/Input.h"

namespace GLCore {

	/* ---------------------------------------------------------------------------
	 * WindowsInput:Input 的 GLFW 实现。继承 Input,把所有 *Impl 纯虚函数
	 * override 一遍。访问方式:protected 继承来的构造(基类 protected),所以
	 * 只能由友元/同类内部 new(实际在 .cpp 的静态初始化里 new)。
	 * ------------------------------------------------------------------------- */
	class WindowsInput : public Input
	{
	protected:
		/* override:重写基类的纯虚函数。这里把 HZ_KEY_* 键码传给 glfwGetKey,
		 * 返回 GLFW_PRESS/GLFW_REPEAT 视为按下。 */
		virtual bool IsKeyPressedImpl(int keycode) override;

		/* 鼠标按键/位置查询,实现在 .cpp(那里 include 了 glfw3.h 和 Application,
		 * 能拿到 GLFWwindow* 并调 glfwGetMouseButton / glfwGetCursorPos)。 */
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};

}
