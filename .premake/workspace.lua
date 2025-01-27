
workspace "QuollEngine"
    basedir "../workspace/"
    language "C++"
    cppdialect "C++20"
    architecture "x86_64"

    -- Set editor as starting project
    startproject "QuollEditor"
    
    setupLibraryDirectories{}
    setupPlatformDefines{}
    linkPlatformLibraries{}
    setupToolsetOptions{}
    setupTestingOptions{}
    
    includedirs {
        "../engine/src",
        "../engine/rhi/core/include",
        "../platform/base/include"
    }
    
    configurations { "Debug", "Release", "Profile", "Test" }

    staticruntime "On"

    filter { "toolset:msc-*" }
       defines { "_SILENCE_CXX20_CISO646_REMOVED_WARNING" }
       flags { "FatalCompileWarnings" }

    filter {"configurations:Debug or configurations:Test"}
        defines { "QUOLL_DEBUG" }
        symbols "On"

    filter {"configurations:Release or configurations:Profile"}
        defines { "QUOLL_RELEASE" }
        optimize "On"
        symbols "Off"

    filter {"configurations:Profile"}
        defines { "QUOLL_PROFILER" }

    defines { "configurations:Test" }
        defines { "QUOLL_TEST" }
