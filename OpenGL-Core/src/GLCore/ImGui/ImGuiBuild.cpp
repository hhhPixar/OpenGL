/*
 * ImGuiBuild.cpp
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     看起来奇怪:它只有三行 #include 和一个 #define,没有任何"逻辑代码"。
 *     但这是有意为之的设计。
 *
 * 在引擎里的角色:
 *     Dear ImGui 的"平台后端"(imgui_impl_glfw.cpp)和"渲染后端"
 *     (imgui_impl_opengl3.cpp)需要被编译进最终的库。把它们集中到这一个
 *     翻译单元(translation unit,即一个 .cpp)里 include 进来,由 premake
 *     脚本把本文件单独作为一个编译目标(ImGuiBuild)处理。
 *
 * 为什么这么写:
 *     1. 避免重复编译:如果多个 cpp 都各自 #include "imgui_impl_glfw.cpp",
 *        这些后端代码会被编很多次,既慢又可能产生符号重复定义。
 *        集中在一个地方 include,整个工程只编译一次。
 *     2. 配合 premake:构建脚本里把 ImGuiBuild 作为独立目标,产出一个小静态库,
 *        主引擎再链接它即可,职责清晰。
 *     3. 控制宏:在 include 之前 #define IMGUI_IMPL_OPENGL_LOADER_GLAD,告诉
 *        ImGui 的 OpenGL3 后端"我们用 glad 加载 OpenGL 函数",这样它会选 glad 分支编译。
 *
 * 关键依赖:
 *     - glpch.h                        : 预编译头,复用常用 STL 与 Log
 *     - imgui_impl_opengl3.cpp         : ImGui 的 OpenGL3 渲染后端实现
 *     - imgui_impl_glfw.cpp            : ImGui 的 GLFW 平台后端实现
 */

#include "glpch.h"

// 在 include 后端代码前先定义这个宏,告诉 OpenGL3 后端:
// 本工程用 glad 作为 OpenGL 函数加载器(而不是 GL3W / GLEW 等)。
// 宏是在预处理阶段做文本替换,所以这行必须写在两个 #include 之前才会生效。
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
// 直接 #include 一个 .cpp?没错——这里是把 ImGui 后端的实现源码"插"进本翻译单元,
// 让它们和本文件一起被编译成一个目标。这正是集中编译后端的常用做法。
#include "examples/imgui_impl_opengl3.cpp"
#include "examples/imgui_impl_glfw.cpp"
