--[[
  文件: OpenGL-Examples/premake5.lua
  角色: 定义 "OpenGL-Examples" 客户端项目。它编译出一个可执行文件(.exe),
        就是 ExampleApp.cpp + ExampleLayer.cpp 的产物,演示用引擎画彩色四边形。
  关键概念注释:
    - project "OpenGL-Examples": 项目名。
    - kind "ConsoleApp": 可执行程序(ConsoleApp 会带一个控制台窗口,方便看 spdlog 日志输出)。
    - language "C++" / cppdialect "C++17": C++17。
    - staticruntime "on": 静态运行时,和 OpenGL-Core 保持一致(否则链接会报错)。
    - targetdir / objdir: 用顶层 outputdir 统一组织产物路径。
    - files: 只编译本目录 src 下的 .h/.cpp。
    - includedirs: 这里直接写"相对路径"指向 OpenGL-Core 的各依赖目录(因为 Examples 不一定能继承 Core 的 include 设置)。
    - links: 只链接 "OpenGL-Core" 一个;OpenGL-Core 已经链接了 GLFW/Glad/ImGui/opengl32,
             作为静态库会把这些符号一起带过来给客户端(静态库传递依赖)。
    - filter system:windows 等:与 Core 类似的平台/配置宏设置。
]]

project "OpenGL-Examples"
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
