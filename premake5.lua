--[[
  文件: premake5.lua (顶层)
  角色: 这是整个项目的"顶层"构建脚本。premake 是一个用 Lua 写配置的构建系统生成器,
        它会读取本文件,再根据里面的配置生成对应平台的工程文件(如 Visual Studio .sln/.vcxproj)。
  关键设计: 本文件定义了"两个独立的 workspace":
        1) "OpenGL-Sandbox" workspace —— 把 Sandbox 当主程序(startproject),
           并用 include 把 OpenGL-Core、Sandbox、以及 GLFW/Glad/ImGui 三个依赖项目都纳入编译。
        2) "OpenGL-Examples" workspace —— 把 Examples 当主程序,但用 includeexternal 引用
           已存在的 Core/依赖工程(避免重复编译,提升打开速度)。
    这意味着你可以单独打开其中一个 workspace 来只构建/调试对应的客户端。
  关键概念注释:
    - workspace: 顶级容器,对应一个 IDE 解决方案(.sln)。
    - architecture "x64": 指定目标平台为 64 位。
    - startproject: 生成工程后默认启动/调试的项目(按 F5 跑哪个 exe)。
    - configurations {Debug, Release}: 定义两种构建配置。
    - flags {MultiProcessorCompile}: 开启多处理器并行编译,加快编译。
    - outputdir: 一个变量,形如 "Debug-windows-x64",用于统一组织输出目录。
    - IncludeDir 表: 把各依赖库的头文件目录路径集中存放,供各项目复用。
    - group "Dependencies": 在 IDE 里把依赖项目折叠进一个分组显示,清爽。
    - include: 把指定子项目的 premake5.lua 纳入构建(会参与编译)。
    - includeexternal: 引用一个"外部"项目(假定已存在/已构建),不重复编译。
]]

-- OpenGL-Sandbox
workspace "OpenGL-Sandbox"
	architecture "x64"
	startproject "OpenGL-Sandbox"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

-- outputdir 变量:把"配置-系统-架构"拼成一段字符串,例如 "Debug-windows-x64"。
-- 之后各项目用它来组织输出目录,保证不同配置/平台互不覆盖。
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to OpenGL-Core
-- IncludeDir 是一张"表"(Lua 的 table),把各依赖库头文件目录的路径存起来,供各项目引用。
-- 这些路径都相对于 OpenGL-Core 目录。
IncludeDir = {}
IncludeDir["GLFW"] = "vendor/GLFW/include"
IncludeDir["Glad"] = "vendor/Glad/include"
IncludeDir["ImGui"] = "vendor/imgui"
IncludeDir["glm"] = "vendor/glm"
IncludeDir["stb_image"] = "vendor/stb_image"

-- Projects
-- group "Dependencies" 把下面三个依赖项目在 IDE 里折叠到一个"Dependencies"分组,看起来更清爽。
group "Dependencies"
	include "OpenGL-Core/vendor/GLFW"     -- GLFW: 窗口/输入库,被引擎用来创建窗口、处理键鼠事件
	include "OpenGL-Core/vendor/Glad"    -- Glad: OpenGL 函数加载器,运行时加载 GL 扩展函数指针
	include "OpenGL-Core/vendor/imgui"   -- ImGui: 即时模式 GUI,用来画调试/调色面板
group ""  -- 结束分组,后面 include 的项目回到顶层显示

include "OpenGL-Core"        -- 把引擎静态库项目纳入构建
include "OpenGL-Sandbox"     -- 把沙盒客户端项目纳入构建

-- OpenGL-Examples
-- 第二个 workspace:再定义一个独立解决方案,只针对 Examples 客户端。
workspace "OpenGL-Examples"
    startproject "OpenGL-Examples"
    architecture "x64"
    startproject "OpenGL-Examples"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

-- 与上面同样的 outputdir 变量(这里重新定义,因为换了 workspace 上下文)。
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to OpenGL-Core
-- 同样把依赖库头文件目录路径存入 IncludeDir 表。
IncludeDir = {}
IncludeDir["GLFW"] = "vendor/GLFW/include"
IncludeDir["Glad"] = "vendor/Glad/include"
IncludeDir["ImGui"] = "vendor/imgui"
IncludeDir["glm"] = "vendor/glm"
IncludeDir["stb_image"] = "vendor/stb_image"

-- Projects
-- 这里用 includeexternal(而非 include)引用 Core 和依赖项目:
-- 表示假定它们已经在别的 workspace 里编译好了,本 workspace 只引用、不重复编译,打开更快。
group "Dependencies"
    includeexternal "OpenGL-Core/vendor/GLFW"
    includeexternal "OpenGL-Core/vendor/Glad"
    includeexternal "OpenGL-Core/vendor/imgui"
group ""

includeexternal "OpenGL-Core"   -- 引用引擎静态库(外部,不重编)
include "OpenGL-Examples"       -- 本 workspace 的主项目,纳入构建
