workspace "NetworkLibrary"
	architecture "x64"
	-- Entt requires C++ to be version 17
	cppdialect "C++17"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile",
		-- "FatalWarnings",
		"NoImplicitLink",
		"NoIncrementalLink",
		"NoManifest",
		"RelativeLinks",
		"UndefinedIdentifiers"
	}

	defines
	{
		"_HAS_EXCEPTIONS = 0"
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
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Common/src/",
		"NetworkLibrary/src/Core/",
		"NetworkLibrary/src/Utils/",
		"%{prj.name}/src/",

		"vendor/sdl2/SDL2-2.30.1/include/",
		"vendor/sdl2/SDL2_image-2.8.2/include/",
		"vendor/sdl2/SDL2_mixer-2.8.0/include/",
		"vendor/sdl2/SDL2_ttf-2.22.0/include/",
		"vendor/entt/include/"
	}

	dependson
	{
		"Common",
		"NetworkLibrary"
	}

	libdirs
	{
		"Common/bin",
		"NetworkLibrary/bin",

		"vendor/sdl2/SDL2-2.30.1/lib/x64",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64"
	}

	links
	{
		"Common_%{cfg.buildcfg}",
		"NetworkLibrary_%{cfg.buildcfg}",

		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2",
		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2main",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64/SDL2_image",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64/SDL2_mixer",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64/SDL2_ttf"
	}

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"

project "Tests"
	kind "ConsoleApp"
	location "Tests"
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