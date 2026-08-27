/*
 * Log.cpp —— 日志系统实现
 *
 * 职责：定义静态成员 s_Logger 并实现 Init()，创建一个输出到控制台、带颜色的 spdlog logger。
 *
 * 在引擎里的角色：由 Application 构造函数在最开头调用 Log::Init()，确保后续所有日志都能正常输出。
 *
 * 关键依赖：
 *   - glpch.h                 —— 预编译头，已包含常用 STL 与 Log.h
 *   - Log.h                    —— 声明 Log 类
 *   - spdlog/sinks/stdout_color_sinks.h —— 提供彩色标准输出 sink（输出目的地）
 */
#include "glpch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace GLCore {

	/*
	 * 静态成员的定义：头文件里只"声明"了 s_Logger，真正分配存储必须在一个 .cpp 里写一次。
	 * 这里相当于 std::shared_ptr<spdlog::logger> Log::s_Logger; 初始化为空指针，
	 * 之后 Init() 会真正赋值。注意只能定义一次，否则会链接重复符号错误。
	 */
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	/*
	 * Init() —— 创建并配置全局 logger，在 Application 构造时调用。
	 *
	 * 步骤：
	 *   1) set_pattern 设置日志格式：
	 *        %^  开始颜色段   %T  时间(时分秒)   %n  logger 名字
	 *        %v  日志正文     %$  结束颜色段
	 *      效果："[14:23:01] GLCORE: 这是一条日志" 带颜色高亮
	 *   2) stdout_color_mt 创建一个输出到 stdout、带颜色的多线程 logger，名叫 "GLCORE"
	 *   3) set_level(trace) 把最低级别设为 trace，因此所有级别都会输出
	 */
	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_Logger = spdlog::stdout_color_mt("GLCORE");
		s_Logger->set_level(spdlog::level::trace);
	}

}
