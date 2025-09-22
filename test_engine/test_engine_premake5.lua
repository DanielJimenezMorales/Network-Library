local project_data = PROJECT_DATA.TEST_ENGINE
local common_project_data = PROJECT_DATA.COMMON
local engine_project_data = PROJECT_DATA.ENGINE

project (project_data.NAME)
	kind "ConsoleApp"
	location (project_data.PATH)
	language "C++"
	targetdir (project_data.PATH .. "bin/")
	targetname (project_data.NAME .. "_%{cfg.buildcfg}")

	files
	{
		ROOT_PATH "vendor/googletest/googletest/src/gtest-all.cc",

		project_data.PATH .. "src/**.h",
		project_data.PATH .. "src/**.cpp"
	}

	includedirs
	{
		project_data.PATH .. "src",
		common_project_data.PATH .. "src",
		engine_project_data.PATH .. "src",

		ROOT_PATH "vendor/googletest/googletest/include",
		ROOT_PATH "vendor/googletest/googletest",
		ROOT_PATH "vendor/entt/include"
	}

	libdirs
	{
	}

	links
	{
		common_project_data.NAME,
		engine_project_data.NAME
	}
