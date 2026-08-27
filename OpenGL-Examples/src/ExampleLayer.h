/*
 * 文件: ExampleLayer.h
 * 角色: 定义 ExampleLayer 类,它是 OpenGL-Examples 客户端的自定义渲染层。
 *       该层用 OpenGL 画一个四边形(由两个三角形组成),颜色可通过鼠标点击切换、
 *       通过 ImGui 调色板调整。这是学习 OpenGL 渲染管线的核心示例。
 * 关键依赖:
 *   - GLCore.h: 引擎核心(Application、Layer、Event、Timestep 等基础设施)
 *   - GLCoreUtils.h: 工具层(Shader、OrthographicCameraController、EnableGLDebugging 等)
 *   - glm: 数学库,提供 vec4/mat4 等向量和矩阵类型
 * 设计意图: 演示一个典型 Layer 的生命周期 —— OnAttach 分配 GPU 资源,
 *           OnUpdate 每帧渲染,OnEvent 处理输入,OnDetach 释放资源。
 */

#pragma once

#include <GLCore.h>        // 引擎核心头文件
#include <GLCoreUtils.h>   // 引擎工具头文件:Shader、相机控制器等

// ExampleLayer 公有继承自 GLCore::Layer。
// Layer 是引擎定义的抽象层基类,提供若干虚函数(OnAttach/OnDetach/OnUpdate/OnEvent/OnImGuiRender),
// 子类重写它们即可插入到引擎每帧的循环里。"public Layer" 表示公有继承,保留基类接口。
class ExampleLayer : public GLCore::Layer
{
public:
	ExampleLayer();                       // 构造函数:初始化相机控制器(设定宽高比)
	virtual ~ExampleLayer();              // 虚析构函数:保证通过基类指针删除子类对象时调用正确的析构(初学者要点)

	// 下面这些函数都是 Layer 基类里声明的虚函数,这里用 "override" 关键字显式标记重写。
	// override 的作用:让编译器检查基类里确实存在同名同签名的虚函数,写错了会报错,是种安全保护。
	virtual void OnAttach() override;                          // 层被压入栈时调用一次:在这里分配 VAO/VBO/IBO、加载着色器
	virtual void OnDetach() override;                          // 层被移出栈时调用一次:在这里 glDelete 释放 GPU 资源
	virtual void OnEvent(GLCore::Event& event) override;       // 有输入事件(鼠标/键盘/窗口)时调用:交给相机控制器、处理换色
	virtual void OnUpdate(GLCore::Timestep ts) override;       // 每帧调用:ts 是本帧耗时(秒),这里清屏 + 画四边形
	virtual void OnImGuiRender() override;                    // 每帧的 ImGui 渲染阶段调用:画调试/调色面板
private:
	// 着色器程序指针。Shader::FromGLSLTextFiles 从文本文件加载并编译链接成一个 GLSL program。
	// 用裸指针是因为该类手动管理生命周期(在 OnAttach 创建、OnDetach 释放);引擎层示例风格如此。
	GLCore::Utils::Shader* m_Shader;
	// 正交相机控制器:封装了平移/缩放/旋转的输入处理 + 一个正交相机,GetCamera() 取相机,GetViewProjectionMatrix() 取视图投影矩阵。
	GLCore::Utils::OrthographicCameraController m_CameraController;

	// OpenGL 对象句柄(GLuint 是无符号整数,代表一个 GPU 对象的名字/ID):
	//   m_QuadVA: VAO(Vertex Array Object,顶点数组对象)—— 记录"顶点属性如何从 VBO 里取出"的状态。
	//   m_QuadVB: VBO(Vertex Buffer Object,顶点缓冲对象)—— 在 GPU 上存储顶点数据(位置)。
	//   m_QuadIB: IBO(Index Buffer Object,索引缓冲对象,绑到 GL_ELEMENT_ARRAY_BUFFER)—— 存顶点索引,决定用哪些顶点、以什么顺序拼三角形。
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;

	// 四边形的"基础色"与"按下鼠标时切换的备用色"。glm::vec4 是四分量浮点向量,这里 (R,G,B,A),分量范围 0~1。
	// 成员默认初始化(类内直接给初值):即使构造函数不显式赋值,这些成员也有确定的初值,避免未初始化读脏数据。
	glm::vec4 m_SquareBaseColor = { 0.8f, 0.2f, 0.3f, 1.0f };
	glm::vec4 m_SquareAlternateColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	// 当前实际使用的颜色:默认等于基础色,鼠标按下时切到备用色、松开时切回基础色,也可由 ImGui 调色板改写。
	glm::vec4 m_SquareColor = m_SquareBaseColor;
};
