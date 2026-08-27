/*
 * Shader.cpp —— 着色器类的实现
 *
 * 在引擎中的角色：实现 Shader.h 里声明的流程——
 *   1) ReadFileAsString 把磁盘上的 .vert.glsl / .frag.glsl 文本读进 std::string；
 *   2) CompileShader 用 glCreateShader/glShaderSource/glCompileShader 编译单个着色器，
 *      并用 glGetShaderiv 检查 GL_COMPILE_STATUS，失败则打印错误日志；
 *   3) LoadFromGLSLTextFiles 把顶点/片元两个 shader 用 glAttachShader 挂到一个 program，
 *      再 glLinkProgram 链接，用 glGetProgramiv 检查 GL_LINK_STATUS；成功后存 m_RendererID；
 *   4) 析构时 glDeleteProgram 释放资源。
 *
 * 关键依赖：glad（OpenGL 函数）、<fstream>（文件读取）、Log.h（日志宏）。
 * 关于 OpenGL 着色器流水线的整体概念，见 Shader.h 的文件头与各函数注释。
 */
#include "glpch.h"
#include "Shader.h"

#include <fstream>

namespace GLCore::Utils {

	// 文件级辅助函数：把 filepath 指定的文本文件一次性读进一个 std::string 返回。
	// 用 "static" 限定作用域只在本 .cpp 内，避免链接时和其它文件同名函数冲突。
	// 参数用 const std::string&（常量引用）：避免拷贝整串字符串，又不能用 const char* 因为 std::string 更好管理内存。
	static std::string ReadFileAsString(const std::string& filepath)
	{
		// 用来装整个文件内容的字符串，先空着，后面 resize 扩容。
		std::string result;
		// 以二进制模式打开。文本模式下某些平台会改写换行符，二进制模式保证读到的是原始字节。
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			// 先定位到文件末尾，tellg() 得到文件大小（字节数）。
			in.seekg(0, std::ios::end);
			// 按大小把 string 预先扩容，resize 后内部缓冲区就有了可写空间。
			result.resize((size_t)in.tellg());
			// 再回到开头，准备从头读取。
			in.seekg(0, std::ios::beg);
			// 一次性把整个文件读进 result 的内存缓冲区。
			in.read(&result[0], result.size());
			// 关闭文件句柄。
			in.close();
		}
		else
		{
			// 打不开文件时只记日志、返回空串；上层 CompileShader 拿到空源码会在编译阶段报错。
			LOG_ERROR("Could not open file '{0}'", filepath);
		}

		// 返回读到的内容（成功）或空串（失败）。
		return result;
	}

	// 析构函数：对象销毁时让 OpenGL 删除这个 program，回收显存。
	Shader::~Shader()
	{
		// glDeleteProgram 释放 GPU 端的 program 对象。释放后 m_RendererID 不再有效。
		glDeleteProgram(m_RendererID);
	}

	// 编译单个着色器对象。
	// type：GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER；source：GLSL 源码文本。
	// 返回编译成功的 shader id；失败时打印日志并返回一个（可能无效的）id 供上层处理。
	GLuint Shader::CompileShader(GLenum type, const std::string& source)
	{
		// 1) 创建一个空着色器对象，OpenGL 返回它的 id。
		GLuint shader = glCreateShader(type);

		// 2) 把源码字符串挂上去。glShaderSource 要的是 const GLchar**（二级指针，指向多个字符串）。
		//    这里只传 1 段字符串，所以取 sourceCStr 的地址 &sourceCStr。最后那个 0 表示自动以 \ 计算长度。
		const GLchar* sourceCStr = source.c_str();
		// 把 GLSL 源码交给 shader 对象保存，下一步才真正编译。
		glShaderSource(shader, 1, &sourceCStr, 0);

		// 3) 真正编译。编译在 GPU 驱动里完成，结果会写进 shader 对象的状态里。
		glCompileShader(shader);

		// 4) 查询编译状态。glGetShaderiv 用来读着色器对象的各种整型参数，
		//    GL_COMPILE_STATUS 为 GL_FALSE 表示编译失败。
		GLint isCompiled = 0;
		// 取出编译结果标志。
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			// 先查日志长度。
			GLint maxLength = 0;
			// glGetShaderiv 配 GL_INFO_LOG_LENGTH 取出日志字符数。
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			// 分配正好够大的缓冲区装日志。
			std::vector<GLchar> infoLog(maxLength);
			// 把日志内容拷进 infoLog。
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

			// 删除这个失败的 shader 对象，避免资源泄漏。
			glDeleteShader(shader);

			// 打印编译错误日志（data() 返回底层字符数组指针，给 LOG_ERROR 的格式串用）。
			LOG_ERROR("{0}", infoLog.data());
			// 原项目(Hazel)这里会断言崩溃，本工程暂时只记日志，方便继续运行。
			// HZ_CORE_ASSERT(false, "Shader compilation failure!");
		}

		// 返回编译好的 shader id（供上层 glAttachShader 使用）。
		return shader;
	}

	// 静态工厂方法：在堆上 new 一个 Shader，调用 LoadFromGLSLTextFiles 完成编译链接，返回指针。
	// 之所以用工厂而不是构造函数，是因为构造函数不能返回错误码，而工厂可以在创建流程里集中处理失败。
	Shader* Shader::FromGLSLTextFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		// 在堆上创建一个空 Shader 对象（私有构造，只有本类方法能 new）。
		Shader* shader = new Shader();
		// 真正的编译链接在这里完成，结果存进 shader->m_RendererID。
		shader->LoadFromGLSLTextFiles(vertexShaderPath, fragmentShaderPath);
		// 返回编译链接好的 Shader 指针。
		return shader;
	}
	
	// 真正的编译链接主流程：读两份文件 -> 各自编译 -> 挂到 program -> 链接 -> 清理临时 shader。
	void Shader::LoadFromGLSLTextFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		// 读顶点着色器 GLSL 源码文本。
		std::string vertexSource = ReadFileAsString(vertexShaderPath);
		// 读片元着色器 GLSL 源码文本。
		std::string fragmentSource = ReadFileAsString(fragmentShaderPath);

		// 创建一个空的 program 对象（最终 glUseProgram 用的就是它）。
		GLuint program = glCreateProgram();
		// 计数器（本实现未实际使用，保留以兼容 Hazel 原结构）。
		int glShaderIDIndex = 0;
			
		// 编译顶点着色器。GL_VERTEX_SHADER 是顶点阶段，处理每个顶点位置。
		GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
		// 把顶点着色器挂到 program 上，等待链接。
		glAttachShader(program, vertexShader);
		// 编译片元着色器。GL_FRAGMENT_SHADER 是片元阶段，决定每个像素颜色。
		GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
		glAttachShader(program, fragmentShader);

		// 链接：把挂上来的多个 shader 拼成一个可执行的整体 program。
		glLinkProgram(program);

		// 查询链接状态。GL_LINK_STATUS 为 GL_FALSE 表示链接失败（通常是顶点和片元的 varying 接口对不上）。
		GLint isLinked = 0;
		// 取出链接结果标志。
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			// 先查链接日志长度。
			GLint maxLength = 0;
			// 取出链接日志字符数。
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// 分配缓冲区装链接日志。
			std::vector<GLchar> infoLog(maxLength);
			// 把链接日志内容拷进 infoLog。
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// 删除失败的 program。
			glDeleteProgram(program);

			// 删除顶点着色器对象（链接失败分支）。
			glDeleteShader(vertexShader);
			// 删除片元着色器对象（链接失败分支）。
			glDeleteShader(fragmentShader);

			// 打印链接错误日志。
			LOG_ERROR("{0}", infoLog.data());
			// 原项目(Hazel)这里会断言崩溃，本工程暂时只记日志。
			// HZ_CORE_ASSERT(false, "Shader link failure!");
		}
		
		// 链接成功后：着色器对象已经"焊"进 program，可以安全地解绑并删除它们，
		// program 本身不受影响（这是 OpenGL 推荐的清理方式，能省显存）。
		glDetachShader(program, vertexShader);
		// 解绑片元着色器对象。
		glDetachShader(program, fragmentShader);
		// 删除顶点着色器对象（已焊进 program，删除不影响 program）。
		glDeleteShader(vertexShader);
		// 删除片元着色器对象（已焊进 program）。
		glDeleteShader(fragmentShader);

		// 把最终 program id 存起来，外部通过 GetRendererID() 拿到它去用。
		m_RendererID = program;
	}

}