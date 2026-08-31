// 文件: test.frag.glsl
// 角色: 片元着色器(Fragment Shader)。每个生成片元(可粗略理解为"候选像素")
//       都会执行一次本程序,它的职责是:决定这个片元最终输出什么颜色。
// 关键依赖: 由 ExampleLayer 用 Shader::FromGLSLTextFiles 从本文件加载并编译。
//          外部(C++ 端)会设置 uniform vec4 u_Color。
// GLSL 版本: #version 410 core 表示用 OpenGL 4.1 的核心模式 GLSL(macOS 最高支持 4.1)。

#version 410 core

// layout(location = 0) 指定这个片元输出写入第 0 号颜色附件
// (对应默认帧缓冲,即屏幕)。out 表示这是"片元输出变量";
// 变量名 o_Color 是约定俗成的命名(o_ 表示 output 输出)。
layout (location = 0) out vec4 o_Color;

// uniform 是"对所有片元都一样"的全局变量,由 C++ 端用 glUniform4fv 设置。
// 这里 u_Color 是四分量(RGBA)颜色,整个四边形都用同一种颜色 ——
// 所以画面是"纯色四边形"。分量范围 0~1。
uniform vec4 u_Color;

// 片元着色器的入口。每个片元执行一次。
void main()
{
	// 直接把 C++ 传进来的颜色作为该片元的输出颜色。
	// 因为没有任何光照/纹理/渐变计算,所有片元输出同一个颜色 -> 纯色填充。
	o_Color = u_Color;
}
