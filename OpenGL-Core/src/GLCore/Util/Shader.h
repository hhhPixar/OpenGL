/*
 * Shader.h —— 着色器类的声明
 *
 * 在引擎中的角色：工具模块(Util)里的核心资源之一。OpenGL 渲染需要"着色器程序(program)"，
 * 它由顶点着色器(vertex shader)和片元着色器(fragment shader)两段 GLSL 源码编译链接而成。
 * 这个类把"读文件 -> 编译 -> 链接 -> 得到一个 program id"整个流程封装好，
 * 外层应用拿到 Shader 对象后，用 GetRendererID() 拿到 OpenGL program id 就能 glUseProgram 启用它。
 *
 * 关键依赖：
 *   - glad/glad.h：OpenGL 函数指针的加载库，提供 GLuint、GLenum 等类型和所有 gl* 函数。
 *   - <string>：文件路径和 GLSL 源码都用 std::string 传递。
 *
 * 设计要点：构造函数是 private 的，强制外部走 FromGLSLTextFiles 静态工厂方法创建对象
 * （这样能保证一个 Shader 对象一定对应一个已链接好的 program，不会出现"半成品"对象）。
 */
#pragma once

#include <string>

#include <glad/glad.h>

namespace GLCore::Utils {

	// 命名空间嵌套：GLCore 是引擎总命名空间，Utils 是其中的工具子命名空间。
	// 把相关工具类都放进 GLCore::Utils，既能避免全局命名冲突，也方便使用 using namespace。

	// 着色器类：封装一个 OpenGL program 对象（顶点+片元 GLSL 编译链接后的产物）。
	class Shader
	{
	public:
		// 析构函数。对象销毁时调用 glDeleteProgram 释放 OpenGL 端的 program 资源，防止显存泄漏。

		~Shader();

		// 返回 OpenGL program 的 id（一个无符号整数句柄）。拿到后可传给 glUseProgram 使用。
		// 注意这里按值返回 GLuint（不是引用），因为 GLuint 是基本整型，拷贝很便宜。

		GLuint GetRendererID() { return m_RendererID; }

		// 静态工厂方法：传入顶点和片元两个 GLSL 文本文件路径，返回一个 new 出来的 Shader*。
		// static 成员函数：不依赖任何具体对象实例就能调用（属于类本身），常用于"创建对象"的工厂模式。
		// 返回裸指针 Shader*，调用方需自己管理生命周期（实际项目里通常会包进 std::unique_ptr）。

		static Shader* FromGLSLTextFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	private:
		// 默认构造函数设为 private：外部不能直接 new Shader() 或 Shader s;，
		// 必须走 FromGLSLTextFiles，从而保证对象创建时一定完成了编译链接。
		// "= default" 让编译器自动生成一个空实现（什么也不做，m_RendererID 未初始化，靠后续 Load 赋值）。

		Shader() = default;

		// 实际干活的函数：读两个文件、编译、链接、把最终 program id 存进 m_RendererID。
		void LoadFromGLSLTextFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		// 编译单个着色器：type 指定 GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER，source 是 GLSL 源码文本。
		// 返回编译好的 shader 对象 id（链接前还需要 glAttachShader 挂到 program 上）。
		GLuint CompileShader(GLenum type, const std::string& source);
	private:
		// OpenGL program 对象的 id。所有 glUseProgram / glUniform* 调用都要靠它。
		GLuint m_RendererID;
	};

}