/*
 * GLCoreUtils.h
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     引擎的"工具聚合头"。和 GLCore.h 搭配,客户端一行 include 就能用上引擎
 *     提供的各类辅助工具类。
 *
 * 在引擎里的角色:
 *     GLCore.h 给的是"引擎核心"(Application、层、事件等基础设施);
 *     本头则把引擎里"偏应用层的工具类"集中起来——着色器、正交相机、相机控制器、
 *     OpenGL 调试辅助。客户端典型写法:
 *         #include <GLCore.h>
 *         #include <GLCoreUtils.h>
 *
 * 聚合头的便利与代价:
 *     便利——客户端不用记各工具类的具体路径;升级引擎时只要这个头不变,客户端代码
 *     基本不受影响。
 *     代价——把多个模块打包,可能引入用不到的依赖、让编译变慢;引擎内部模块之间
 *     仍应精确 include 自己需要的头,而不是走聚合头。
 *
 * 关键依赖(均为引擎内部的工具类):
 *     - GLCore/Util/Shader.h                   : 着色器加载/编译/绑定的封装
 *     - GLCore/Util/OrthographicCamera.h       : 正交(2D 风格)相机
 *     - GLCore/Util/OrthographicCameraController.h : 正交相机控制器(用输入控制移动/缩放)
 *     - GLCore/Util/OpenGLDebug.h              : OpenGL 调试辅助(如错误检查、GL 标签)
 */

#pragma once

// Utility header file - include into application for access to utility classes/functions

#include "GLCore/Util/Shader.h"
#include "GLCore/Util/OrthographicCamera.h"
#include "GLCore/Util/OrthographicCameraController.h"
#include "GLCore/Util/OpenGLDebug.h"
