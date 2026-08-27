/*
 * OpenGLDebug.h —— OpenGL 调试输出模块的声明
 *
 * 在引擎中的角色：开发期排错利器。OpenGL 是个 C 风格的状态机 API，调用错了默认不会抛异常，
 * 只是默默地把错误码塞进某个内部标志里，开发者很难发现到底哪一行出问题。
 * 本模块利用 GL_KHR_debug 扩展（核心 OpenGL 4.3+ 自带）注册一个回调，
 * 让显卡驱动在发生错误/警告时主动调用我们的 OpenGLLogMessage，把信息打印到日志。
 *
 * 关键依赖：
 *   - glad/glad.h：提供 GLenum/GLuint 等 GL 类型和 glEnable/glDebugMessageCallback。
 *   - GLCore/Core/Log.h：LOG_ERROR/WARN/INFO/TRACE 宏，把消息送往引擎日志系统。
 *
 * 使用方式：在 OpenGL 上下文创建后调用一次 EnableGLDebugging()，之后所有 GL 错误会自动打印。
 * 用 SetGLDebugLogLevel 控制想看多详细的信息（HighAssert 模式下高严重度错误会直接断言崩溃，方便定位）。
 */
#pragma once

#include <glad/glad.h>

#include "GLCore/Core/Log.h"

namespace GLCore::Utils {

	// 调试日志的详细程度档位。值越大表示越啰嗦（连低优先级信息也打）。
	// 用 enum class（强类型枚举）而不是普通 enum，避免它的成员名污染外层作用域，
	// 也防止和别的整型隐式转换。比较时拿 (int) 强转一下即可。
	enum class DebugLogLevel
	{
		// 档位数值含义见 OpenGLLogMessage 里的判断逻辑：None 全关，HighAssert 高危即崩，依次到 Notification 全开。
		None = 0, HighAssert = 1, High = 2, Medium = 3, Low = 4, Notification = 5
	};

	// 启用 GL 调试：注册回调 + 打开 GL_DEBUG_OUTPUT（见 .cpp 实现）。
	void EnableGLDebugging();
	// 设置想看多详细的信息，搭配上面枚举使用。
	void SetGLDebugLogLevel(DebugLogLevel level);
	// GL_KHR_debug 回调函数签名（GLDEBUGPROC 类型）。
	// 驱动在出错时调用它，把 source/type/id/severity/消息文本传进来。
	// length 是 message 长度，userParam 是注册时透传的自定义指针（本引擎没用，传 nullptr）。
	// 注意这个声明是导出给外部/驱动按 C 约定调用的，参数类型都是 OpenGL 定义好的 GL* 类型。
	void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

}