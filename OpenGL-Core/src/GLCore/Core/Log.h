#pragma once

/*
 * Log.h —— 日志系统头文件
 *
 * 职责：封装第三方日志库 spdlog，对外提供一个全局 logger，以及一系列
 *       LOG_TRACE/INFO/WARN/ERROR/CRITICAL 宏供全引擎打印日志。
 *
 * 在引擎里的角色：所有模块（Application、Event、Window、Layer 等）都通过
 *       这些宏输出调试信息、错误、断言失败提示等，是引擎运行时"看得到自己状态"的窗口。
 *
 * 关键依赖：
 *   - spdlog/spdlog.h         —— 真正的日志库，提供 logger 类和多种日志级别
 *   - spdlog/fmt/ostr.h       —— 让 spdlog 支持 << 输出复杂数据（如把对象格式化进日志）
 *   - Core.h                  —— 因为 GLCORE_ASSERT 宏会反过来调用 LOG_ERROR，形成依赖环
 *
 * 设计要点：Log 是一个"静态工具类"——所有方法都是 static，不需要 new 出实例，
 *           整个引擎共用一个 logger（s_Logger），通过 Init() 初始化一次。
 */

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

/*
 * namespace GLCore —— 命名空间，把引擎所有代码包在 GLCore 名字下，
 *   避免与项目里其它库的同名类/函数冲突（例如 Window、Log、Event 都很常见）。
 *   大括号包住的内容都属于 GLCore。
 */
namespace GLCore {

	/*
	 * Log —— 日志管理类（静态工具类）
	 *
	 * 设计意图：不需要创建 Log 对象，所有功能都是静态的：
	 *   - Init()           程序启动时调用一次，创建并配置 spdlog logger
	 *   - GetLogger()      返回全局 logger 引用，供各处宏调用
	 *   - s_Logger          静态成员指针，存放唯一的 logger
	 *
	 * 这是"单例/全局访问点"的简化写法：因为只有一个 logger，干脆用 static 全局化。
	 */
	class Log
	{
	public:
		/*
		 * Init() —— 初始化日志系统，只在 Application 构造时调用一次。
		 * 具体实现在 Log.cpp：设置日志格式、创建带颜色的 stdout logger。
		 */
		static void Init();

		/*
		 * GetLogger() —— 返回全局 logger 的引用(shared_ptr 的引用)，供 LOG_XXX 宏使用。
		 *
		 * inline —— 关键字表示"在头文件里定义、可被内联展开"，避免重复定义。
		 *   因为头文件会被多个 .cpp 包含，普通函数重复定义会链接报错；
		 *   inline 让编译器允许多份定义（取其一），并且可能直接把函数体嵌入调用处提升性能。
		 *
		 * std::shared_ptr<spdlog::logger> —— 智能指针的一种，带引用计数、可共享所有权。
		 *   返回引用(&)而不是值拷贝，避免每次调用都增加一次引用计数开销。
		 */
		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
	private:
		/*
		 * s_Logger —— 静态成员变量，唯一的 logger。
		 * 静态成员属于"类"而不属于某个对象，全引擎共享一份，需在 .cpp 里定义分配（见 Log.cpp）。
		 */
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

/*
 * 客户端日志宏 —— 直接转发到 spdlog 的对应级别方法
 *
 * 这些宏让业务代码写 LOG_INFO("x={0}", x) 就能打印，不需要每次写 ::GLCore::Log::GetLogger()->info(...)
 *
 * __VA_ARGS__ —— C 预处理可变参数宏，代表调用时传入的所有实参，
 *   例如 LOG_INFO("a={}, b={}", a, b) 里 __VA_ARGS__ = "a={}, b={}", a, b
 *
 * spdlog 的 info/trace/warn/error/critical 都支持 fmt 风格的 {} 占位符格式化。
 *
 * 级别从轻到重：TRACE < INFO < WARN < ERROR < CRITICAL。
 * Init() 里设置了最低输出级别为 trace，意味着所有级别都会打印。
 */
// Client log macros
#define LOG_TRACE(...)         ::GLCore::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)          ::GLCore::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)          ::GLCore::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)         ::GLCore::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)      ::GLCore::Log::GetLogger()->critical(__VA_ARGS__)
