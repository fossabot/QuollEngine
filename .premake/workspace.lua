
workspace "LiquidEngine"
    basedir "../workspace/"
    language "C++"
    cppdialect "C++17"
    architecture "x86_64"

    -- Set editor as starting project
    startproject "LiquidEditor"
    
    setupLibraryDirectories{}
    setupPlatformDefines{}
    linkPlatformLibraries{}
    setupToolsetOptions{}
    setupTestingOptions{}
    
    includedirs {
        "../engine/src",
        "../engine/rhi/core/include",
        "../engine/platform-tools/include"
    }
    
    configurations { "Debug", "Release", "Profile", "Test" }

    filter { "toolset:msc-*" }
        flags { "FatalCompileWarnings" }

    filter {"configurations:Debug or configurations:Test"}
        defines { "LIQUID_DEBUG" }
        symbols "On"

    filter {"configurations:Release or configurations:Profile"}
        defines { "LIQUID_RELEASE" }
        optimize "On"

    filter {"configurations:Profile"}
        defines { "LIQUID_PROFILER" }

    defines { "configurations:Test" }
        defines { "LIQUID_TEST" }
