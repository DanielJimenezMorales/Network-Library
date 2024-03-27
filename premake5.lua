workspace "NetworkLibrary"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

project "Common"
	kind "StaticLib"
	location "Common"
	language "C++"
	targetdir "%{prj.name}/bin"
	targetname "%{prj.name}_%{cfg.buildcfg}"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"%{prj.name}/src/"
	}

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"

project "NetworkLibrary"
	kind "StaticLib"
	location "NetworkLibrary"
	language "C++"
	targetdir "%{prj.name}/bin"
	targetname "%{prj.name}_%{cfg.buildcfg}"

	files
	{
		"%{prj.name}/src/Core/**.h",
		"%{prj.name}/src/Core/**.cpp",
		"%{prj.name}/src/Utils/**.h",
		"%{prj.name}/src/Utils/**.cpp"
	}

	includedirs
	{
		"Common/src/",
		"%{prj.name}/src/Core/",
		"%{prj.name}/src/Utils/"
	}

	dependson
	{
		"Common"
	}

	libdirs
	{
		"Common/bin"
	}

	links
	{
		"Common_%{cfg.buildcfg}"
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

project "DemoGame"
	kind "ConsoleApp"
	location "DemoGame"
	language "C++"
	targetdir "%{prj.name}/bin"
	targetname "%{prj.name}_%{cfg.buildcfg}"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Common/src/",
		"NetworkLibrary/src/Core/",
		"NetworkLibrary/src/Utils/",
		"%{prj.name}/src/"
	}

	dependson
	{
		"Common",
		"NetworkLibrary"
	}

	libdirs
	{
		"Common/bin",
		"NetworkLibrary/bin"
	}

	links
	{
		"Common_%{cfg.buildcfg}",
		"NetworkLibrary_%{cfg.buildcfg}"
	}

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"