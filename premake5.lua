-- Makes a path relative to the folder containing this script file.
ROOT_PATH = function(path)
    return string.format("%s/%s", _MAIN_SCRIPT_DIR, path)
end

-- Global project variables
PROJECT_DATA = 
{
	COMMON =
	{
		NAME = "Common",
		PATH = ROOT_PATH "common/",
		PREMAKE_PATH = ROOT_PATH "common/common_premake5.lua"
	},
	ENGINE =
	{
		NAME = "Engine",
		PATH = ROOT_PATH "Engine/",
		PREMAKE_PATH = ROOT_PATH "Engine/engine_premake5.lua"
	},
	NETWORK_LIBRARY =
	{
		NAME = "NetworkLibrary",
		PATH = ROOT_PATH "NetworkLibrary/",
		PREMAKE_PATH = ROOT_PATH "NetworkLibrary/network_library_premake5.lua"
	},
	CLIENT_GAME =
	{
		NAME = "ClientGame",
		PATH = ROOT_PATH "DemoGame/",
		PREMAKE_PATH = ROOT_PATH "DemoGame/client_game_premake5.lua"
	},
	SERVER_GAME =
	{
		NAME = "ServerGame",
		PATH = ROOT_PATH "DemoGame/",
		PREMAKE_PATH = ROOT_PATH "DemoGame/server_game_premake5.lua"
	},
	LAUNCHER_GAME =
	{
		NAME = "Launcher",
		PATH = ROOT_PATH "DemoGame/",
		PREMAKE_PATH = ROOT_PATH "DemoGame/launcher_game_premake5.lua"
	},
	TEST_GAME =
	{
		NAME = "TestDemoGame",
		PATH = ROOT_PATH "TestDemoGame/",
		PREMAKE_PATH = ROOT_PATH "TestDemoGame/test_game_premake5.lua"
	}
}

-- Global workspace variables
WORKSPACE_DATA = 
{
	NAME = "NetworkLibrary",
	STARTUP_PROJECT = PROJECT_DATA.LAUNCHER_GAME.NAME
}

workspace (WORKSPACE_DATA.NAME)
	architecture "x64"
	-- Entt requires C++ to be version 17
	cppdialect "C++17"
	startproject(WORKSPACE_DATA.STARTUP_PROJECT)

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

	filter "configurations:Debug"
		defines
		{
			"LOG_ENABLED"
		}
		symbols "On"

	filter "configurations:Release"
		optimize "On"

include (PROJECT_DATA.COMMON.PREMAKE_PATH)
include (PROJECT_DATA.ENGINE.PREMAKE_PATH)
include (PROJECT_DATA.NETWORK_LIBRARY.PREMAKE_PATH)

group "DemoGame"
include (PROJECT_DATA.CLIENT_GAME.PREMAKE_PATH)
include (PROJECT_DATA.SERVER_GAME.PREMAKE_PATH)
include (PROJECT_DATA.LAUNCHER_GAME.PREMAKE_PATH)

group "Tests"
include (PROJECT_DATA.TEST_GAME.PREMAKE_PATH)
