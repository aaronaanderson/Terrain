cmake_minimum_required(VERSION 3.22)

project(TERRAIN VERSION 1.2.2)

# Show nested target folders in IDEs that support it (VS, CLion, Xcode).
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenGL REQUIRED)

add_subdirectory(JUCE)
add_subdirectory(clap-juce-extensions EXCLUDE_FROM_ALL)
add_subdirectory(morphlib)


# ------------------------------------------------------------------------------
# Plugin target
# ------------------------------------------------------------------------------
juce_add_plugin(WaveTerrainSynth
    VERSION "1.2.2"
    COMPANY_NAME "Aaron Anderson"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    IS_MIDI_EFFECT FALSE
    BUNDLE_ID com.AaronAnderson.WaveTerrainSynth
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Aaro
    PLUGIN_CODE JEe9
    APP_SANDBOX_ENABLED TRUE
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "Terrain"
    VST3_AUTO_MANIFEST FALSE
)

clap_juce_extensions_plugin(TARGET WaveTerrainSynth
    CLAP_ID "com.AaronAnderson.WaveTerrainSynth"
    CLAP_FEATURES instrument synthesizer
)

set_target_properties(WaveTerrainSynth PROPERTIES
    CXX_STANDARD 17
    COMPILE_WARNING_AS_ERROR OFF
    FOLDER "Apps"
)

# --------------------------------------------------------------------------
# Custom build type: ReleaseWithDebugInfo
# --------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES Debug Release ReleaseWithDebugInfo CACHE STRING "" FORCE)

# Compiler flags for MSVC
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASEWITHDEBUGINFO "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_C_FLAGS_RELEASEWITHDEBUGINFO    "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASEWITHDEBUGINFO
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /INCREMENTAL:NO")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASEWITHDEBUGINFO
        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /INCREMENTAL:NO")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASEWITHDEBUGINFO
        "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG /INCREMENTAL:NO")
endif()

# Compiler flags for GCC/Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASEWITHDEBUGINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")
    set(CMAKE_C_FLAGS_RELEASEWITHDEBUGINFO    "${CMAKE_C_FLAGS_RELEASE} -g")
endif()

# Necessary for VS 2022
if (MSVC)
    set_target_properties(WaveTerrainSynth_VST3 PROPERTIES
        VS_DEBUGGER_COMMAND "C:/Program Files/REAPER (x64)/reaper.exe"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()



# ------------------------------------------------------------------------------
# glm module 
# ------------------------------------------------------------------------------
juce_add_module(glm_module)
set_target_properties(glm_module PROPERTIES
    COMPILE_WARNING_AS_ERROR OFF
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -W"
    FOLDER "ThirdParty"
)

# ------------------------------------------------------------------------------
# Collect sources/headers and mirror on-disk folder structure in the IDE
# ------------------------------------------------------------------------------
# All C/C++ sources under Source/
file(GLOB_RECURSE TERRAIN_CPP_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c
)
file(GLOB_RECURSE TERRAIN_HEADER_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)
set(TERRAIN_ALL_FILES ${TERRAIN_CPP_FILES} ${TERRAIN_HEADER_FILES})

# Attach files to the plugin target
target_sources(WaveTerrainSynth PRIVATE ${TERRAIN_ALL_FILES})

# Create nested “virtual folders” that match the filesystem under Source/
# (Works in Visual Studio, CLion, Xcode; VS Code’s CMake Tools uses best-effort display.)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source
             PREFIX "Source"
             FILES ${TERRAIN_ALL_FILES})

# ------------------------------------------------------------------------------
# Binary data (shaders, fonts) + grouping
# ------------------------------------------------------------------------------
set(TERRAIN_ASSETS
    Source/Interface/Renderer/Shaders/TrajectoryPoint.frag
    Source/Interface/Renderer/Shaders/TrajectoryPoint.vert
    Source/Interface/Renderer/Shaders/Terrain.frag
    Source/Interface/Renderer/Shaders/Terrain.vert
    Source/Interface/Paul-le1V.ttf
)
juce_add_binary_data(BinaryData SOURCES ${TERRAIN_ASSETS})
set_target_properties(BinaryData PROPERTIES FOLDER "Assets")

# Group asset source files under a neat tree, too (on the BinaryData target).
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source
             PREFIX "Assets"
             FILES ${TERRAIN_ASSETS})

# ------------------------------------------------------------------------------
# Third-party libraries
# ------------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # necessary for GCC linking

add_library(MTS-ESP STATIC MTS-ESP/Client/libMTSClient.cpp)
target_include_directories(MTS-ESP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/MTS-ESP/Client)
set_target_properties(MTS-ESP PROPERTIES FOLDER "ThirdParty")

# If morphlib is defined in its subdir, optionally fold it too
# (safe even if morphlib already sets its own FOLDER)
set_target_properties(morphlib PROPERTIES FOLDER "ThirdParty")

# ------------------------------------------------------------------------------
# Includes / defs / links
# ------------------------------------------------------------------------------
target_include_directories(WaveTerrainSynth PRIVATE
    PerlinNoise
    MTS-ESP/Client
    glm/glm
)

target_compile_definitions(WaveTerrainSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

if (WIN32)
    target_link_libraries(WaveTerrainSynth PRIVATE glu32.lib)
endif()

target_link_libraries(WaveTerrainSynth
    PRIVATE
        BinaryData
        glm_module
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
        MTS-ESP
        morphlib
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
