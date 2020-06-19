workspace "CHIP8"
	architecture "x86_64"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "CHIP8"
	location "CHIP8"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "CHIP8/src/pch.cpp"

	files
	{
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/dependencies/SDL2/include",
		"%{prj.name}/src"
	}

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter "system:windows"
		systemversion "latest"

		libdirs
		{
			"%{prj.name}/dependencies/SDL2/lib/x64"
		}
	
		links
		{
			"SDL2.lib",
			"SDL2main.lib"
		}	

		postbuildcommands
		{
			("{COPY} %{prj.location}/dependencies/SDL2/lib/x64/SDL2.dll %{cfg.buildtarget.directory}")
		}
	
	filter "system:linux"
		links { "SDL2" }
