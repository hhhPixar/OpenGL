// 文件: test.vert.glsl
// 角色: 顶点着色器(Vertex Shader)。每个顶点都会执行一次本程序,
//       它的职责是:把顶点的"模型空间位置"用视图投影矩阵变换后,
//       写入到内建输出变量 gl_Position —— 这就是顶点最终在屏幕上的位置。
// 关键依赖: 由 ExampleLayer 用 Shader::FromGLSLTextFiles 从本文件加载并编译。
//          外部(C++ 端)会设置 uniform mat4 u_ViewProjection。
// GLSL 版本: #version 410 core 表示用 OpenGL 4.1 的核心模式 GLSL(macOS 最高支持 4.1)。

#version 410 core

// layout(location = 0) 显式指定这个输入属性的位置为 0,与 C++ 端
// glEnableVertexAttribArray(0) / glVertexAttribPointer(0, ...) 对应。
// in 表示这是"顶点属性输入";vec3 是三分量浮点向量(x,y,z)。
// 变量名 a_Position 是约定俗成的命名(a_ 表示 attribute 输入)。
layout (location = 0) in vec3 a_Position;

// uniform 是"对所有顶点/片元都一样"的全局变量,值由 C++ 端用 glUniform* 设置。
// mat4 是 4x4 浮点矩阵。这里的 u_ViewProjection 是"视图*投影"组合矩阵,
// 用来把顶点从世界空间变换到裁剪空间(再由硬件透视除法到屏幕空间)。
uniform mat4 u_ViewProjection;

// 顶点着色器的入口。每个顶点执行一次。
void main()
{
	// gl_Position 是内建的 vec4 输出,表示顶点在裁剪空间的位置(必须 4 分量)。
	// a_Position 是 vec3,所以用 vec4(a_Position, 1.0f) 补上 w=1,变成齐次坐标(点),
	// 再左乘 u_ViewProjection 矩阵完成变换。w=1 表示"点",w=0 才表示"方向向量"。
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
}
