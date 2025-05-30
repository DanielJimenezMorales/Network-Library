local project_data = PROJECT_DATA.SERVER_GAME
local common_project_data = PROJECT_DATA.COMMON
local engine_project_data = PROJECT_DATA.ENGINE
local network_library_project_data = PROJECT_DATA.NETWORK_LIBRARY

project (project_data.NAME)
	kind "StaticLib"
	location (project_data.PATH)
	language "C++"
	targetdir (project_data.PATH .. "bin/")
	targetname (project_data.NAME .. "_%{cfg.buildcfg}")

	files
	{
		project_data.PATH .. "src/**.h",
		project_data.PATH .. "src/**.cpp"
	}

	removefiles
	{
		-- Remove client-side
		project_data.PATH .. "src/client/**.h",
		project_data.PATH .. "src/client/**.cpp",

		-- Remove launcher-side
		project_data.PATH .. "src/launcher/**.h",
		project_data.PATH .. "src/launcher/**.cpp"
	}

	includedirs
	{
		project_data.PATH .. "src",
		common_project_data.PATH .. "src",
		engine_project_data.PATH .. "src",
		network_library_project_data.PATH .. "src",

		ROOT_PATH "vendor/entt/include/",
		ROOT_PATH "vendor/json/include/",

		ROOT_PATH "vendor/sdl2/SDL2-2.30.1/include/",
		ROOT_PATH "vendor/sdl2/SDL2_image-2.8.2/include/",
		ROOT_PATH "vendor/sdl2/SDL2_mixer-2.8.0/include/",
		ROOT_PATH "vendor/sdl2/SDL2_ttf-2.22.0/include/",
	}

	libdirs
	{
		ROOT_PATH "vendor/sdl2/SDL2-2.30.1/lib/x64",
		ROOT_PATH "vendor/sdl2/SDL2_image-2.8.2/lib/x64",
		ROOT_PATH "vendor/sdl2/SDL2_mixer-2.8.0/lib/x64",
		ROOT_PATH "vendor/sdl2/SDL2_ttf-2.22.0/lib/x64"
	}

	links
	{
		common_project_data.NAME,
		engine_project_data.NAME,
		network_library_project_data.NAME,

		ROOT_PATH "vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2",
		ROOT_PATH "vendor/sdl2/SDL2-2.30.1/lib/x64/SDL2main",
		ROOT_PATH "vendor/sdl2/SDL2_image-2.8.2/lib/x64/SDL2_image",
		ROOT_PATH "vendor/sdl2/SDL2_mixer-2.8.0/lib/x64/SDL2_mixer",
		ROOT_PATH "vendor/sdl2/SDL2_ttf-2.22.0/lib/x64/SDL2_ttf"
	}
