#pragma once
/*
 * KeyEvent.h —— 键盘事件
 * ============================================================
 * 定义键盘相关事件,继承自 Event 基类。
 *
 * 继承层次:
 *   Event
 *     └─ KeyEvent              (中间基类,存 m_KeyCode + 归类为"键盘|输入")
 *         ├─ KeyPressedEvent   (按键按下,带 repeatCount 重复计数)
 *         ├─ KeyReleasedEvent  (按键松开)
 *         └─ KeyTypedEvent     (字符输入,用于文本框等)
 *
 * 按键码 keycode 是 GLFW 定义的整数常量(如 GLFW_KEY_A、GLFW_KEY_SPACE)。
 *
 * 依赖:Event.h(基类与 EVENT_CLASS_TYPE / EVENT_CLASS_CATEGORY 宏)。
 */

#include "Event.h"

namespace GLCore {

	/*
	 * KeyEvent:所有键盘事件的共同基类(中间层)。
	 * 它统一存放 m_KeyCode,并把大类设为"键盘 | 输入"。
	 * 注意它本身不绑定具体 EventType——具体类型由各子类用 EVENT_CLASS_TYPE 设置。
	 * 构造函数是 protected,只有子类能调用,外部不能直接 new KeyEvent。
	 */
	class KeyEvent : public Event
	{
	public:
		// 获取按键码。GLFW 的 keycode 是整数(如 GLFW_KEY_A、GLFW_KEY_SPACE)。
		inline int GetKeyCode() const { return m_KeyCode; }

		// 键盘事件统一归类为"键盘 + 输入"两大类(位或 |)。
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		// protected 构造:仅供子类调用。用初始化列表给 m_KeyCode 赋值。
		// 【C++概念】protected:成员对派生类可见,但类外部不可访问,所以外部无法直接 new。
		KeyEvent(int keycode)
			: m_KeyCode(keycode) {}

		// 按键码成员。protected 表示子类(如 KeyPressedEvent)能直接访问它。
		int m_KeyCode;
	};

	/*
	 * 按键按下事件。除 keycode 外还带 repeatCount:
	 * 长按一个键时,操作系统会反复发送"按下"事件,repeatCount 记录这是第几次重复
	 * (首次按下 repeatCount=0,之后持续按住会 1,2,3……递增)。
	 */
	class KeyPressedEvent : public KeyEvent
	{
	public:
		// 构造:先调基类 KeyEvent 构造设 keycode,再初始化 m_RepeatCount。
		KeyPressedEvent(int keycode, int repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		// 获取重复计数(仅长按时才有意义)。
		inline int GetRepeatCount() const { return m_RepeatCount; }

		// 拼出 "KeyPressedEvent: <keycode> (<n> repeats)" 用于日志。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		// 绑定具体类型为 KeyPressed(宏原理见 Event.h,用到了 # 与 ##)。
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		// 重复计数成员。
		int m_RepeatCount;
	};

	// 按键松开事件。只关心是哪个键松开了,没有额外数据。
	class KeyReleasedEvent : public KeyEvent
	{
	public:
		// 构造:把按键码交给基类 KeyEvent。
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {}

		// 拼出 "KeyReleasedEvent: <keycode>"。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		// 绑定类型 KeyReleased。
		EVENT_CLASS_TYPE(KeyReleased)
	};

	/*
	 * 字符输入事件。与 KeyPressed 不同:它代表"用户输入了一个字符"
	 * (可能由组合键产生,主要给文本输入框用)。本引擎里复用 keycode 字段存放字符码。
	 */
	class KeyTypedEvent : public KeyEvent
	{
	public:
		// 构造:把字符码作为 keycode 交给基类。
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode) {}

		// 拼出 "KeyTypedEvent: <keycode>"。
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		// 绑定类型 KeyTyped。
		EVENT_CLASS_TYPE(KeyTyped)
	};
}