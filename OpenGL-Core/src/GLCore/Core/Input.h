#pragma once
/* ============================================================================
 * Input.h —— 输入系统抽象接口(平台无关层)
 *
 * 这个文件定义了输入系统的抽象基类 Input,以及它的单例访问方式。和 Window
 * 一样,Input 是纯虚接口,具体实现(WindowsInput)在 Platform/Windows 下,
 * 通过静态指针 s_Instance + 一次 new 完成实例化,整个程序共用一个输入实例。
 *
 * 设计要点:
 *   - 静态外壳 + 虚函数内核:对外暴露的 IsKeyPressed 等都是 static,内部转调
 *     s_Instance->xxxImpl()(纯虚函数),由子类用 GLFW 实现。这样调用方写
 *     Input::IsKeyPressed(...) 就行,完全不用知道背后是哪个平台实现。
 *   - 单例:用 s_Instance 这个静态指针全局只持有一个对象,删除拷贝构造/赋值
 *     防止外部复制这个唯一实例。
 *
 * 关键依赖:Core.h(基础宏)。
 * ========================================================================== */

#include "Core.h"

namespace GLCore {

	/* ---------------------------------------------------------------------------
	 * Input:输入系统的抽象基类(接口 + 单例外壳)。
	 *
	 * 这是一种常见写法:对外是"静态门面",对内是"虚函数实现"。
	 * 上层调用 Input::IsKeyPressed(...)(静态),它内部转调 s_Instance 的纯虚
	 * xxxImpl;真正干活的是子类 WindowsInput,用 glfwGetKey/glfwGetMouseButton
	 * 等轮询当前按键/鼠标状态。
	 *
	 * 这种"轮询"方式和 Window 里的"事件回调"不同:事件回调是"按键按下时被
	 * 通知一次",轮询是"每帧主动问一次现在按没按"。两者互补,各有用途。
	 * ------------------------------------------------------------------------- */
	class Input
	{
	protected:
		/* protected 构造:只允许子类和本类构造,外部不能直接 new Input(因为它是
		 * 抽象类有纯虚函数,本来就不能实例化)。= default 用编译器默认实现。 */
		Input() = default;
	public:
		/* = delete 表示"删除"拷贝构造和拷贝赋值,编译器会拒绝任何尝试复制
		 * Input 对象的代码。单例模式常用这个手法保证全局只有一个实例。
		 * 这两条防止出现 Input a = *Input::... 这种意外的拷贝。 */
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		/* inline static:建议编译器把函数体直接展开到调用处(减少函数调用开销)。
		 * 静态方法可以不通过对象、直接用 Input::IsKeyPressed(...) 调用。
		 * 内部把请求转交给 s_Instance->IsKeyPressedImpl——s_Instance 指向真正
		 * 干活的子类对象,Impl 是纯虚函数,运行时多态调到 WindowsInput 的实现。 */
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

		/* 鼠标按键/位置查询,同上,全部走 s_Instance 转调到子类的 Impl。
		 * std::pair<float,float> 是"两个 float 打包"的类型,这里表示鼠标 x,y。 */
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
	protected:
		/* 纯虚函数(= 0):接口方法,子类必须实现。命名带 Impl 后缀,表示
		 * "真正干活的实现版",和静态门面版区分开。
		 * 返回 bool:按键/鼠标键是否处于按下状态。 */
		virtual bool IsKeyPressedImpl(int keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		/* 全局唯一实例指针。在 WindowsInput.cpp 里这样初始化:
		 *     Input* Input::s_Instance = new WindowsInput();
		 * 程序启动时(静态变量初始化阶段)就 new 出 WindowsInput 并赋给它,
		 * 之后所有静态调用都通过这个指针找到那个对象。这就是"单例"。 */
		static Input* s_Instance;
	};

}