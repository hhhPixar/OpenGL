--[[
  文件: OpenGL-Core/premake5.lua
  角色: 定义引擎静态库项目 "OpenGL-Core"。它是整个引擎的核心,被两个客户端(Examples/Sandbox)链接使用。
        编译产物是一个 .lib/.a 静态库,里面包含 Application、Layer、事件系统、ImGui 层、
        工具(Shader、相机控制器)等实现。客户端通过链接它来"用引擎"。
  关键概念注释:
    - project "OpenGL-Core": 声明一个名为 OpenGL-Core 的项目。
    - kind "StaticLib": 静态库(编译产出是 .lib/.a,不是 .exe)。
    - language "C++" / cppdialect "C++17": 用 C++17 标准。
    - staticruntime "on": 用静态 CRT 运行库(避免客户端机器缺 vcruntime 报错,各项目要一致)。
    - targetdir / objdir: 用顶层的 outputdir 变量统一组织产物和中间文件目录。
    - pchheader/pchsource: 预编译头(把一个常用头预先编译成二进制,加快编译);glpch.h 是 PCH 头,glpch.cpp 是触发源。
    - files: 要编译进库的源文件(本目录 src 下所有 .h/.cpp + 内置的 stb_image/glm)。
    - defines: 预处理宏;_CRT_SECURE_NO_WARNINGS 关掉某些旧 CRT 函数的安全警告。
    - includedirs: 头文件搜索路径;既包含自己的 src,也包含各依赖库的头目录。
    - links: 链接的库;OpenGL-Core 作为引擎,需要 GLFW(窗口)、Glad(GL 加载)、ImGui、以及系统 OpenGL 库。
    - filter: 按条件(系统/配置)做差异化设置;Windows 下定义平台宏、Debug 下定义 GLCORE_DEBUG 等。
]]

project "OpenGL-Core"
	kind "StaticLib"           -- 静态库:产出 .lib/.a,供客户端链接
	language "C++"              -- 使用 C++ 语言
	cppdialect "C++17"          -- C++ 语言标准为 C++17
	staticruntime "on"          -- 使用静态运行时库(/MT 或 -static),需和所有依赖/客户端保持一致

	-- 输出目录:用顶层定义的 outputdir(形如 "Debug-windows-x64")拼出完整路径。
	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	-- 中间对象目录:同上,组织到 bin-int 下,避免不同配置互相覆盖。
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	-- 预编译头(PCH):把 glpch.h 里一堆常用头(如 glad、imgui、glm)预先编译成二进制,
	-- 每个源文件只要 #include "glpch.h" 就能复用,大幅加快编译速度。
	pchheader "glpch.h"
	pchsource "src/glpch.cpp"

	-- 要参与编译的文件清单:本目录 src 下所有 .h/.cpp,以及内置进 vendor 的 stb_image、glm 源文件。
	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
	}

	-- 预处理宏:_CRT_SECURE_NO_WARNINGS 让使用 strcpy 等旧 CRT 函数时不报安全警告。
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	-- 头文件搜索路径:自己的 src,以及各依赖库的头目录(用顶层 IncludeDir 表拼接)。
	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	-- OpenGL-Core 自身要链接的库:它用了 GLFW(窗口/输入)、Glad(GL 函数)、ImGui(调试 UI),
	-- 以及 Windows 系统的 opengl32.lib(系统 OpenGL 实现)。客户端链接 OpenGL-Core 时也会继承这些依赖。
	links
	{
		"GLFW",
		"Glad",
		"ImGui"
	}

	-- filter:按"系统"做差异化设置。仅 Windows 平台下的配置。
	filter "system:windows"
		systemversion "latest"   -- 用最新版 Windows SDK

		defines
		{
			"GLCORE_PLATFORM_WINDOWS",   -- 平台宏,代码里用来判断是否 Windows
			"GLFW_INCLUDE_NONE"          -- 让 GLFW 不自动包含系统 GL 头,交给 Glad 来管理 GL 头
		}

		links
		{
			"opengl32"
		}

	-- macOS 平台配置:定义平台宏;系统 OpenGL/Cocoa 框架由最终可执行文件(Sandbox/Examples)链接。
	filter "system:macosx"
		defines
		{
			"GLCORE_PLATFORM_MACOS",
			"GLFW_INCLUDE_NONE"
		}

	-- filter:按"配置"做差异化设置。Debug 配置。
	filter "configurations:Debug"
		defines "GLCORE_DEBUG"   -- 调试宏,代码里可用来打开更详细的日志/断言
		runtime "Debug"           -- 使用 Debug 运行时库
		symbols "on"              -- 生成调试符号(.pdb),支持断点调试

	-- filter:Release 配置。
	filter "configurations:Release"
		defines "GLCORE_RELEASE"  -- 发布宏
		runtime "Release"          -- 使用 Release 运行时库
		optimize "on"              -- 开启优化
