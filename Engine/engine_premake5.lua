local project_data = PROJECT_DATA.ENGINE
local common_project_data = PROJECT_DATA.COMMON

project (project_data.NAME)
	kind "StaticLib"
	location (project_data.PATH)
	language "C++"
	targetdir (project_data.PATH .. "bin/")
	targetname "%{prj.name}_%{cfg.buildcfg}"

	files
	{
		project_data.PATH .. "src/**.h",
		project_data.PATH .. "src/**.cpp",
		project_data.PATH .. "src/**.hpp"
	}

	includedirs
	{
		project_data.PATH .. "src/",
		common_project_data.PATH .. "src/",

		ROOT_PATH "vendor/entt/include/",
		ROOT_PATH "vendor/json/include/",

		ROOT_PATH "vendor/sdl2/SDL2-2.30.1/include/",
		ROOT_PATH "vendor/sdl2/SDL2_image-2.8.2/include/",
		ROOT_PATH "vendor/sdl2/SDL2_mixer-2.8.0/include/",
		ROOT_PATH "vendor/sdl2/SDL2_ttf-2.22.0/include/",
	}
