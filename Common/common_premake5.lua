local project_data = PROJECT_DATA.COMMON

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
		project_data.PATH .. "src/"
	}
