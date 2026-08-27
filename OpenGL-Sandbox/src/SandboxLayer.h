/*
 * 文件: SandboxLayer.h
 * 角色: 定义 SandboxLayer 类,它是 OpenGL-Sandbox 客户端的自定义层。
 *       目前是一个"空壳模板"——各个生命周期回调里都还没有具体实现(只有注释占位),
 *       方便你把它当作实验起点:在对应回调里填代码即可开始试新东西。
 *       这正好也是学习一个 Layer 都有哪些生命周期函数的好例子。
 * 关键依赖:
 *   - GLCore.h: 引擎核心(Layer 基类、Event、Timestep 等)
 *   - GLCoreUtils.h: 工具层(EnableGLDebugging、Shader、相机控制器等,供你按需使用)
 * 设计意图: 与 ExampleLayer 结构完全一致,但留空,让你照着 ExampleLayer 的写法自己填。
 */

#pragma once

#include <GLCore.h>        // 引擎核心头文件
#include <GLCoreUtils.h>   // 引擎工具头文件:Shader、相机控制器等

// SandboxLayer 公有继承自 GLCore::Layer。
// Layer 是引擎定义的层基类,提供若干虚函数(OnAttach/OnDetach/OnUpdate/OnEvent/OnImGuiRender),
// 子类重写它们即可插入到引擎每帧的循环里。"public Layer" 表示公有继承,保留基类接口。
class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();                       // 构造函数(目前为空)
	virtual ~SandboxLayer();              // 虚析构函数:保证通过基类指针删除子类时调用正确析构(初学者要点)

	// 下面这些函数都是 Layer 基类里声明的虚函数,这里用 "override" 关键字显式标记重写。
	// override 的作用:让编译器检查基类里确实存在同名同签名的虚函数,写错会报错,是一种安全保护。
	virtual void OnAttach() override;                          // 层被压入栈时调用一次:分配资源、初始化 GL 状态
	virtual void OnDetach() override;                          // 层被移出栈时调用一次:释放资源
	virtual void OnEvent(GLCore::Event& event) override;       // 有输入事件时调用:处理鼠标/键盘/窗口事件
	virtual void OnUpdate(GLCore::Timestep ts) override;       // 每帧调用:ts 是本帧耗时(秒),这里做更新 + 渲染
	virtual void OnImGuiRender() override;                     // 每帧的 ImGui 渲染阶段调用:画调试面板
private:
	// 沙盒层目前没有成员。当你照着 ExampleLayer 写时,通常会在私有区
	// 声明 Shader*、VAO/VBO/IBO 句柄、相机控制器、颜色等成员。
};
