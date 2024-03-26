workspace "NetworkLibrary"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

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
		"%{prj.name}/src/",
		"NetworkLibrary/src/Core/",
		"NetworkLibrary/src/Utils/"
	}

	dependson {"NetworkLibrary"}

	libdirs
	{
		"NetworkLibrary/bin"
	}

	links
	{
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