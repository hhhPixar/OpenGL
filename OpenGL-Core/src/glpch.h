/*
 * glpch.h (预编译头,precompiled header)
 * ----------------------------------------------------------------------------
 * 这个文件是什么:
 *     本工程的预编译头(pch)。它把全工程到处都会用到的标准库头 + 引擎 Log 头
 *     集中起来,让编译器预先"消化"一次。
 *
 * 在引擎里的角色:
 *     几乎每个 cpp 第一行都是 #include "glpch.h"。意思是这些常用头的内容
 *     大家都要。如果每个 cpp 都重新解析一遍 <vector>、<string> 等大模板,
 *     编译会非常慢。
 *
 * 预编译头加速编译的原理:
 *     编译器把 glpch.h 编一次,把"解析后的符号表/语法树"存成一个二进制文件
 *     (比如 glpch.pch / glpch.h.gch)。之后任何 cpp 再 #include "glpch.h" 时,
 *     编译器直接加载那个二进制缓存,跳过重新解析,大幅节省时间。
 *     代价:只要 glpch.h 被改动,所有依赖它的 cpp 都得重新编译,所以
 *     预编译头里只放"几乎不变、又到处都用"的头。
 *
 * 关键依赖:
 *     - STL 常用头(iostream、memory、vector、string、unordered_map 等)
 *     - GLCore/Core/Log.h  : 引擎的日志系统,全工程都要打日志,适合放预编译头
 */

#pragma once
// pragma once 是编译器指令:保证本头文件在一个翻译单元里只被 include 一次,
// 避免"重复定义"错误。它比传统的 #ifndef 守卫写法更简洁、更可靠。

#include <iostream>
#include <memory>          // 智能指针(如 std::shared_ptr、std::unique_ptr),自动管理内存
#include <utility>         // std::pair、std::move、std::forward 等实用工具
#include <algorithm>       // std::sort、std::find 等通用算法
#include <functional>      // std::function、std::bind,用于回调/可调用对象

#include <string>          // std::string 字符串类
#include <sstream>         // std::stringstream 字符串流,方便把各种类型拼成字符串
#include <vector>          // std::vector 动态数组,引擎里最常用的容器
#include <unordered_map>   // std::unordered_map 哈希映射(键值对)
#include <unordered_set>   // std::unordered_set 哈希集合

// 引擎的日志系统头。日志到处都要用,放进预编译头能让所有文件直接打 LOG_XXX。
#include "GLCore/Core/Log.h"

// 平台判断宏:仅在 Windows 平台时引入 <Windows.h>(用于某些 Windows 特有 API)。
// 其他平台这一段被跳过,保证跨平台可编译。
#ifdef GLCORE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
