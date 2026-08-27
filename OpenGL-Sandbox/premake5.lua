--[[
  文件: OpenGL-Sandbox/premake5.lua
  角色: 定义 "OpenGL-Sandbox" 客户端项目。它编译出一个可执行文件(.exe),
        就是 SandboxApp.cpp + SandboxLayer.cpp 的产物,作为一个最小沙盒/实验台。
        配置和 OpenGL-Examples/premake5.lua 基本一致(同样的 ConsoleApp、C++17、静态运行时、
        链接 OpenGL-Core),只是项目名不同。
  关键概念注释:
    - project "OpenGL-Sandbox": 项目名。
    - kind "ConsoleApp": 可执行程序(带控制台窗口,方便看日志)。
    - language "C++" / cppdialect "C++17": C++17。
    - staticruntime "on": 静态运行时,必须和 OpenGL-Core 一致。
    - files: 只编译本目录 src 下的 .h/.cpp。
    - includedirs: 用相对路径指向 OpenGL-Core 的各依赖目录。
    - links: 只链接 "OpenGL-Core";Core 作为静态库会把 GLFW/Glad/ImGui/opengl32 一并传递过来。
]]

project "OpenGL-Sandbox"
	kind "ConsoleApp"          -- 可执行程序(ConsoleApp,带控制台窗口,方便看日志)
	language "C++"             -- C++ 语言
	cppdialect "C++17"         -- C++17 标准
	staticruntime "on"         -- 静态运行时,必须和 OpenGL-Core 一致

	-- 输出目录与中间目录,均用顶层 outputdir 变量拼接。
	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	-- 参与编译的源文件:本目录 src 下所有 .h/.cpp。
	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	-- 头文件搜索路径:直接用相对路径指向 OpenGL-Core 里的各依赖目录。
	-- 包含 spdlog(日志)、引擎 src、引擎 vendor、以及 glm/Glad/ImGui 的头目录。
	includedirs
	{
		"../OpenGL-Core/vendor/spdlog/include",
		"../OpenGL-Core/src",
		"../OpenGL-Core/vendor",
		"../OpenGL-Core/%{IncludeDir.glm}",
		"../OpenGL-Core/%{IncludeDir.Glad}",
		"../OpenGL-Core/%{IncludeDir.ImGui}"
	}

	-- 链接的库:只需链接 OpenGL-Core 这一个静态库;
	-- 因为 Core 已 links 了 GLFW/Glad/ImGui/opengl32,作为静态库依赖会被一并传递给本客户端。
	links
	{
		"OpenGL-Core"
	}

	-- Windows 平台设置。
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GLCORE_PLATFORM_WINDOWS"
		}

	-- Debug 配置:定义调试宏、用 Debug 运行时、开调试符号。
	filter "configurations:Debug"
		defines "GLCORE_DEBUG"
		runtime "Debug"
		symbols "on"

	-- Release 配置:定义发布宏、用 Release 运行时、开优化。
	filter "configurations:Release"
		defines "GLCORE_RELEASE"
		runtime "Release"
        optimize "on"
