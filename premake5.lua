workspace "NetworkLibrary"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

project "NetworkLibrary"
	kind "ConsoleApp"
	location "NetworkLibrary"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	files
	{
		"%{prj.name}/src/Core/**.h",
		"%{prj.name}/src/Core/**.cpp",
		"%{prj.name}/src/Utils/**.h",
		"%{prj.name}/src/Utils/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src/Core/",
		"%{prj.name}/src/Utils/"
	}

	filter "system:Windows"
		links
		{
			"Ws2_32"
		}

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"