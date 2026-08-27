#pragma once

/*
 * Timestep.h —— 帧时间步长类
 *
 * 职责：把一个 float（一帧用了多少秒）包装成一个类型 Timestep，
 *       提供 GetSeconds()/GetMilliseconds() 两种访问方式，并且支持隐式转回 float。
 *
 * 在引擎里的角色：Application::Run() 每帧用 glfwGetTime() 算出"距上一帧过了多久"，
 *       把这个时间差作为 Timestep 传给各层 Layer::OnUpdate(ts)，让游戏逻辑可以
 *       "按时间推进"而不是"按帧数推进"，从而在不同帧率下行为一致（例如移动 1 秒 10 米）。
 *
 * 设计意图：用类包装 float 而不是直接传 float，类型更明确（避免和别的 float 混用），
 *   同时通过 operator float() 保留了"想当 float 用就能用"的便利性。
 *
 * 无依赖：纯值类型，只依赖内置 float。
 */

namespace GLCore {

	/*
	 * Timestep —— 帧间隔时间
	 *
	 * 用法：Timestep ts = now - last;  Layer::OnUpdate(ts);
	 *   - 可用 ts.GetSeconds() / ts.GetMilliseconds() 取秒/毫秒
	 *   - 也可直接当 float 用：float s = ts;（触发下面的隐式转换）
	 */
	class Timestep
	{
	public:
		/*
		 * 构造函数，默认 0.0f（默认参数：调用时不传参就用 0.0f，简化无参场景）。
		 * m_Time(time) 是"成员初始化列表"，比在函数体里赋值更高效，直接构造成员。
		 */
		Timestep(float time = 0.0f)
			: m_Time(time)
		{
		}

		/*
		 * operator float() —— 类型转换运算符（隐式转换）
		 *
		 *   作用：当某处需要 float 但你给的是 Timestep 时，编译器会自动调用它转成 float。
		 *   例如 float dt = ts;  或  position += velocity * ts;
		 *
		 *   const 表示"转换不改对象本身"，是只读操作。
		 *   这样既保留了类型安全，又用起来方便，不用到处写 ts.GetSeconds()。
		 */
		operator float() const { return m_Time; }

		// 取秒数（const 表示不修改对象）
		float GetSeconds() const { return m_Time; }

		// 取毫秒数：1 秒 = 100 毫秒
		float GetMilliseconds() const { return m_Time * 1000.0f; }
	private:
		// 实际存放的秒数，外部不可见（封装）
		float m_Time;
	};

}
