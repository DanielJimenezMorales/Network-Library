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
		"_HAS_EXCEPTIONS=0"
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
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"Common/src/",
		"%{prj.name}/src/"
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

project "Engine"
	kind "StaticLib"
	location "Engine"
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
		"Common/src/",
		"%{prj.name}/src/",
		"vendor/entt/include/",

		"vendor/sdl2/SDL2-2.30.1/include/",
		"vendor/sdl2/SDL2_image-2.8.2/include/",
		"vendor/sdl2/SDL2_mixer-2.8.0/include/",
		"vendor/sdl2/SDL2_ttf-2.22.0/include/",
	}

	dependson
	{
		"Common"
	}

	libdirs
	{
		"Common/bin",

		"vendor/sdl2/SDL2-2.30.1/lib/x64",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64"
	}

	links
	{
		"Common_%{cfg.buildcfg}",

		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2",
		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2main",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64/SDL2_image",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64/SDL2_mixer",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64/SDL2_ttf"
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
		"NetworkLibrary/src/",
		"Engine/src",
		"%{prj.name}/src/",
		"vendor/entt/include/",

		"vendor/sdl2/SDL2-2.30.1/include/",
		"vendor/sdl2/SDL2_image-2.8.2/include/",
		"vendor/sdl2/SDL2_mixer-2.8.0/include/",
		"vendor/sdl2/SDL2_ttf-2.22.0/include/",
		"vendor/json/include/"
	}

	dependson
	{
		"Common",
		"NetworkLibrary",
		"Engine"
	}

	libdirs
	{
		"Common/bin",
		"NetworkLibrary/bin",
		"Engine/bin",

		"vendor/sdl2/SDL2-2.30.1/lib/x64",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64"
	}

	links
	{
		"Common_%{cfg.buildcfg}",
		"NetworkLibrary_%{cfg.buildcfg}",
		"Engine_%{cfg.buildcfg}",

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
		"NetworkLibrary/src/replication/",
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

group "Tests"
project "TestDemoGame"
	kind "ConsoleApp"
	location "TestDemoGame"
	language "C++"
	targetdir "%{prj.name}/bin"
	targetname "%{prj.name}_%{cfg.buildcfg}"

	files
	{
		"vendor/googletest/googletest/src/gtest-all.cc",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"Engine/src/**.cpp",
		"DemoGame/src/**.cpp"
	}

	removefiles
	{
		"DemoGame/src/main.cpp"
	}

	includedirs
	{
		--Delete later
		"vendor/json/include/",
		"NetworkLibrary/src/",
		"vendor/sdl2/SDL2-2.30.1/include/",
		"vendor/sdl2/SDL2_image-2.8.2/include/",
		"vendor/sdl2/SDL2_mixer-2.8.0/include/",
		"vendor/sdl2/SDL2_ttf-2.22.0/include/",

		"vendor/entt/include/",
		"vendor/googletest/googletest/include",
		"vendor/googletest/googletest",
		"DemoGame/src/",
		"Engine/src/",
		"Common/src/",
		"%{prj.name}/src/"
	}

	libdirs
	{
		--Delete later
		"NetworkLibrary/bin",
		"vendor/sdl2/SDL2-2.30.1/lib/x64",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64"
	}

	links
	{
		--Delete later
		"NetworkLibrary_%{cfg.buildcfg}",
		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2",
		"vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2main",
		"vendor/sdl2/SDL2_image-2.8.2/lib/x64/SDL2_image",
		"vendor/sdl2/SDL2_mixer-2.8.0/lib/x64/SDL2_mixer",
		"vendor/sdl2/SDL2_ttf-2.22.0/lib/x64/SDL2_ttf"
	}

	dependson
	{
		"DemoGame"
	}

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"