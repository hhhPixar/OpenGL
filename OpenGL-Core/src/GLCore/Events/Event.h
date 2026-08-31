#pragma once
/*
 * Event.h —— 事件系统核心头文件
 * ============================================================
 * 这是整个引擎事件系统的地基。所有具体事件(键盘按键、鼠标移动、窗口关闭……)
 * 最终都继承自此文件定义的 Event 基类。
 *
 * 引擎采用"阻塞式事件"(blocking event)设计:事件一发生就立刻被分发,
 * 调用方必须当场处理完毕,而不是先丢进队列稍后再处理。下面原作者注释也提到,
 * 未来可以改成"事件总线缓冲 + 在 update 阶段统一处理"的做法,那样更平滑,
 * 但当前版本先用最简单直接的阻塞式。
 *
 * 本文件定义三大核心:
 *   1. EventType        —— "具体是哪种事件"的强类型枚举(按键?鼠标移动?窗口关闭?)
 *   2. EventCategory    —— "事件属于哪些大类"的位标志(可同时属于"鼠标"和"输入")
 *   3. Event + EventDispatcher —— 多态基类 + 模板分发器(运行时按类型把事件交给对应回调)
 *
 * 另有两个宏 EVENT_CLASS_TYPE / EVENT_CLASS_CATEGORY,让子类一行代码就能
 * 实现基类的纯虚函数,内部用到了 # (字符串化) 和 ## (标记拼接) 预处理运算符。
 *
 * 关键依赖:../Core/Core.h 里的 BIT(x) 宏 = (1 << x),用来生成二进制位标志。
 */

#include "glpch.h"
#include "../Core/Core.h"

namespace GLCore {

	// Events in Hazel are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	/*
	 * EventType:用 enum class(强类型枚举)罗列所有可能的具体事件类型。
	 * 【C++概念】enum class 比 enum 更安全:成员不会泄漏到外层作用域,
	 *   也不和整数隐式转换,必须写 EventType::KeyPressed 这样限定使用。
	 *   None=0 作为"无/无效"占位。每个具体事件类会通过宏绑定其中一个类型。
	 */
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	/*
	 * EventCategory:事件的"大类"标签。这里用普通 enum(而非 enum class),
	 * 因为要把它当"位标志"(bit flags)用,需要能和整数做位运算。
	 * 每个大类对应二进制的一位:BIT(0)=1、BIT(1)=2、BIT(2)=4……
	 * 一个事件可同时属于多个大类,用按位或 | 拼起来即可
	 *   (见 MouseEvent.h: EventCategoryMouse | EventCategoryInput)。
	 */
	enum EventCategory
	{
		// None=0:二进制 00000,表示"不属于任何类别"。
		None = 0,
		// BIT(0)=(1<<0)=二进制 00001,代表"应用程序/窗口"类事件。
		EventCategoryApplication    = BIT(0),
		// BIT(1)=(1<<1)=二进制 00010,代表"输入设备"类事件(键盘、鼠标统称)。
		EventCategoryInput          = BIT(1),
		// BIT(2)=(1<<2)=二进制 00100,代表"键盘"类事件。
		EventCategoryKeyboard       = BIT(2),
		// BIT(3)=(1<<3)=二进制 01000,代表"鼠标"类事件(移动、滚轮、按键)。
		EventCategoryMouse          = BIT(3),
		// BIT(4)=(1<<4)=二进制 10000,代表"鼠标按键"类事件(按下/松开)。
		EventCategoryMouseButton    = BIT(4)
	};

	/*
	 * EVENT_CLASS_TYPE 宏:给具体事件类"一键实现"基类的三个纯虚函数。
	 * 【C++宏运算符】
	 *   #type  —— 字符串化(stringification):把参数变字符串字面量。
	 *             type=WindowResize 时,#type 展开成 "WindowResize"(带引号),
	 *             于是 GetName() 能直接返回事件类型名字,方便日志。
	 *   ##type —— 标记拼接(token pasting):EventType::##type 拼成 EventType::WindowResize。
	 * 宏分多行写,行尾反斜杠 \ 是"续行符",告诉预处理器下一行还是本宏的一部分。
	 * 注意:\ 后必须紧跟换行,不能有空格或注释——所以这几行不能加行尾注释。
	 * 三行分别实现:
	 *   GetStaticType() —— 静态方法,无需对象即可调用,返回该类固定的类型;
	 *                      EventDispatcher 正是靠它做"运行时类型匹配"。
	 *   GetEventType()  —— 虚函数 override,运行时通过对象返回真实类型(多态)。
	 *   GetName()       —— 返回类型名字字符串,用于调试/日志。
	 */
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

	// EVENT_CLASS_CATEGORY 宏:一键实现 GetCategoryFlags(),返回该事件的大类位标志。
	// 参数 category 通常是几个 BIT 用 | 拼起来的结果,如 EventCategoryMouse | EventCategoryInput。
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	/*
	 * Event:所有事件的抽象基类。具体事件(按键、鼠标移动……)都继承自它。
	 * 【C++概念】
	 *   virtual ... = 0; —— 纯虚函数:无默认实现,强制子类实现;含它的类是"抽象类",
	 *                       不能直接实例化,只能实例化其具体子类。
	 *   virtual + override —— 子类重写基类虚函数,运行时调用到真实子类的版本(多态)。
	 *                       override 让编译器检查"确实重写了基类的某个虚函数",写错即报错。
	 *   const —— 加在成员函数末尾表示"不修改对象成员",const 对象也能调用它。
	 */
	class Event
	{
	public:
		// Handled:事件是否"已被处理"。EventDispatcher 分发时若回调返回 true,
		// 就把 Handled 置 true;后续可据此判断是否还要继续传递该事件。
		bool Handled = false;

		// 三个纯虚函数,子类必须实现:返回事件类型、类型名字、大类位标志。
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		// ToString():默认只返回名字;子类可重写以拼出带参数值的详细信息,
		// 配合下面的 operator<< 即可 cout << event 打印事件。
		virtual std::string ToString() const { return GetName(); }

		/*
		 * IsInCategory():判断本事件是否属于某个大类。
		 * 【C++概念】位标志判断用"按位与 &":
		 *   设 GetCategoryFlags()=01010(既属鼠标位3、又属输入位1),
		 *   传入 category=01000(只看鼠标位),01010 & 01000 = 01000,非0即真→属于;
		 *   传入 category=00001(应用位),01010 & 00001 = 0→假,不属于。
		 * inline:建议编译器把函数体直接展开到调用处,省去调用开销,适合短小函数。
		 */
		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	/*
	 * EventDispatcher:事件分发器。用引用把一个 Event 包起来,提供 Dispatch 模板方法。
	 * 它是阻塞式事件能"按类型路由到不同回调"的关键:
	 * 调用方写一连串 dispatcher.Dispatch<XxxEvent>(...),哪个类型匹配上了,
	 * 就把事件交给对应回调并返回 true,后续 Dispatch 不必再处理。
	 */
	class EventDispatcher
	{
	public:
		// 构造函数:保存事件的引用。m_Event 是引用类型,
		// 【C++概念】引用(&):必须在构造初始化列表里初始化,它是事件的"别名",不拷贝、不持有。
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}
		
		/*
		 * Dispatch<T,F>:事件分发的核心逻辑。
		 * 【C++概念】template<typename T, typename F> —— 模板。T=期望的事件类型(如 KeyPressedEvent),
		 *   F=回调的可调用对象类型;编译器据你传入的 func 自动推导 F,故调用时不必写 F。
		 * 流程:
		 *   1) T::GetStaticType() 拿到"回调想要的事件类型"(编译期已知的固定值);
		 *   2) 和 m_Event.GetEventType()(运行时才知的实际类型)比较;
		 *   3) 相等→类型匹配:把基类引用 m_Event 用 static_cast<T&> 安全转成具体子类引用,
		 *      传给回调 func;回调返回 bool(是否已处理),用它更新 Handled;返回 true。
		 *   4) 不等→返回 false,调用方继续尝试下一个类型的 Dispatch。
		 * static_cast<T&>:编译期类型转换,这里安全,因为已用 GetEventType() 确认真实类型就是 T。
		 */
		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		// m_Event:持有要分发的那个事件的引用(不拥有所有权,事件本身在外面)。
		Event& m_Event;
	};

	/*
	 * 重载 << 运算符,让事件可直接用流输出,如 std::cout << event;
	 * 它会调用 event.ToString(),于是日志里能打印出可读的事件描述。
	 * 【C++概念】运算符重载:给已有 << 赋予针对 Event 的新含义;返回 ostream& 以支持链式 <<。
	 */
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}

