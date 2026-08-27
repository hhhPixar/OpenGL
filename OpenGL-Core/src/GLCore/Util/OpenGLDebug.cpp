/*
 * OpenGLDebug.cpp —— OpenGL 调试输出模块的实现
 *
 * 在引擎中的角色：把 OpenGL 驱动产生的调试消息转成引擎日志输出。
 *   - EnableGLDebugging：glDebugMessageCallback 注册回调 + glEnable(GL_DEBUG_OUTPUT)；
 *   - OpenGLLogMessage：按 severity（高/中/低/通知）分档打印日志，
 *     HighAssert 档位下高严重度错误会直接 GLCORE_ASSERT 崩掉，方便在开发期立即定位。
 *
 * 关键依赖：glad（GL 函数）、Log.h（日志与断言宏）。
 */

#include "glpch.h"
#include "OpenGLDebug.h"

namespace GLCore::Utils {

	// 当前调试日志档位。static 让它只在当前编译单元可见，外部通过 SetGLDebugLogLevel 修改。
	// 默认 HighAssert：开发期希望遇到高严重度 GL 错误就直接断住。
	static DebugLogLevel s_DebugLogLevel = DebugLogLevel::HighAssert;

	// 设置全局调试日志档位。
	void SetGLDebugLogLevel(DebugLogLevel level)
	{
		// 保存到静态变量，供回调函数 OpenGLLogMessage 读取判断。
		s_DebugLogLevel = level;
	}

	// GL 调试回调本体。驱动一调用就把这些参数填好。
	// 参数含义（GL_KHR_debug 标准）：
	//   source——消息来源（GL_DEBUG_SOURCE_* 之一）；
	//   type——消息类型（GL_DEBUG_TYPE_*，如 ERROR/DEPRECATED_BEHAVIOR 等）；
	//   id——驱动自定义的消息编号；
	//   severity——严重等级（见下方 switch）；
	//   length——message 字符数；
	//   message——以 \0 结尾的人类可读字符串；
	//   userParam——注册时透传的指针（本引擎未使用）。
	void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		// 按 severity 决定打印的日志级别。只有当前 s_DebugLogLevel 足够高时才打，避免噪音。
		switch (severity)
		{
		// 高严重度：通常是 OpenGL 错误（无效枚举、越界等）。
		case GL_DEBUG_SEVERITY_HIGH:
			// (int)s_DebugLogLevel > 0 表示档位至少是 HighAssert 或 High。
			if ((int)s_DebugLogLevel > 0)
			{
				// 把驱动给出的错误文本打印到引擎日志。
				LOG_ERROR("[OpenGL Debug HIGH] {0}", message);
				// HighAssert 档位下，断言触发让程序立即停下来，方便在调试器里看调用栈。
				if (s_DebugLogLevel == DebugLogLevel::HighAssert)
					// 断言失败 -> 程序中断在出错位置附近，便于定位是哪一行 GL 调用引发的错误。
					GLCORE_ASSERT(false, "GL_DEBUG_SEVERITY_HIGH");
			}
			break;
		// 中等：可能是不推荐的用法。档位 > 2（即 Low 或 Notification）才打。
		case GL_DEBUG_SEVERITY_MEDIUM:
			// 只在足够啰嗦的档位下才打印中等严重度。
			if ((int)s_DebugLogLevel > 2)
				// 以警告级别打印。
				LOG_WARN("[OpenGL Debug MEDIUM] {0}", message);
			break;
		// 低：冗余的性能/状态提示。档位 > 3 才打。
		case GL_DEBUG_SEVERITY_LOW:
			// 只在更啰嗦的档位下才打印低严重度。
			if ((int)s_DebugLogLevel > 3)
				// 以信息级别打印。
				LOG_INFO("[OpenGL Debug LOW] {0}", message);
			break;
		// 通知：最普通的日志。档位 > 4 才打（基本只在 Notification 全开时）。
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			// 只在全开档位下才打印通知级别。
			if ((int)s_DebugLogLevel > 4)
				// 以跟踪级别打印（最细粒度的日志）。
				LOG_TRACE("[OpenGL Debug NOTIFICATION] {0}", message);
			break;
		}
	}

	// 启用 OpenGL 调试输出。应在创建好 GL 上下文后尽早调用一次。
	void EnableGLDebugging()
	{
		// 注册回调：驱动有消息时就会调用 OpenGLLogMessage。第二个参数 userParam 传 nullptr。
		glDebugMessageCallback(OpenGLLogMessage, nullptr);
		// 打开调试输出开关。不开的话，驱动即使有错误也不会回调我们。
		glEnable(GL_DEBUG_OUTPUT);
		// 让驱动同步触发回调（在出错的 GL 调用之后立即调用，而不是异步批量回调）。
		// 同步模式下，断言命中时的调用栈正好停在出错位置附近，便于定位。
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

}