/*
 * GLCore.h
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     引擎的"聚合头"(umbrella header / aggregate header)。客户端(使用本引擎
 *     写自己游戏/程序的人)只要 #include <GLCore.h>,就能拿到引擎的全部核心能力。
 *
 * 在引擎里的角色:
 *     把引擎里最常用的几个外部库头 + 引擎主入口集中起来,客户端无需逐个手动
 *     include。通常和 GLCoreUtils.h 搭配使用:
 *         #include <GLCore.h>         // 引擎主功能(应用、层、事件、日志等)
 *         #include <GLCoreUtils.h>    // 工具类(着色器、相机、调试工具等)
 *
 * 聚合头的便利与代价:
 *     便利——客户端写起来省心,一个 include 搞定,不用记一堆路径。
 *     代价——它会把很多东西拉进来,可能编进一些用不到的代码、让单文件编译变慢、
 *     也可能引入意外的符号依赖。因此聚合头通常只给"外部使用者"用,引擎内部
 *     各模块之间还是精确 include 各自需要的头。
 *
 * 关键依赖:
 *     - glad/glad.h        : OpenGL 函数加载器,加载现代 OpenGL 的函数指针
 *     - glm/...            : 数学库(glm 是 OpenGL 生态里事实标准的数学库),
 *                             提供 vec2/vec3/mat4 等。这里 include 了它的核心、
 *                             type_ptr(把 glm 数据转成 OpenGL 能吃的指针)和
 *                             matrix_transform(平移/旋转/缩放等)
 *     - imgui.h            : Dear ImGui 核心头,让客户端也能直接用 ImGui 画调试 UI
 *     - GLCore/Core/Application.h : 引擎的 Application 单例,程序的"大管家",
 *                             客户端通过它 Run() 启动主循环
 */

#pragma once

// Main header file - include into application for complete access

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "GLCore/Core/Application.h"
