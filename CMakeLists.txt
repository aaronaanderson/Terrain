# cmake_minimum_required(VERSION 3.22) 

# project(TERRAIN VERSION 1.2.2)

# find_package(OpenGL REQUIRED)
# add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE
# add_subdirectory(clap-juce-extensions EXCLUDE_FROM_ALL)

# # set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
# # set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment target" FORCE)

# juce_add_plugin(WaveTerrainSynth
#     VERSION "1.2.2"                               # Set this if the plugin version is different to the project version
#     # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
#     # ICON_SMALL ...
#     COMPANY_NAME "Aaron Anderson"                          # Specify the name of the plugin's author
#     IS_SYNTH TRUE                       # Is this a synth or an effect?
#     NEEDS_MIDI_INPUT TRUE              # Does the plugin need midi input?
#     # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
#     IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
#     # VST3_COPY_DIR "./vst3" 
#     BUNDLE_ID com.AaronAnderson.WaveTerrainSynth
#     COPY_PLUGIN_AFTER_BUILD TRUE
#     PLUGIN_MANUFACTURER_CODE Aaro               # A four-character manufacturer id with at least one upper-case character
#     PLUGIN_CODE JEe9                        # A unique four-character plugin id with exactly one upper-case character
#                                                 # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
#     APP_SANDBOX_ENABLED TRUE        # Needed for GarageBand
#     FORMATS Standalone VST3 AU     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
#     PRODUCT_NAME "Terrain"         # The name of the final executable, which can differ from the target name    
#     APP_SANDBOX_FILE_ACCESS_HOME_RW "Library/Audio/Presets/Aaron Anderson/Terrain"
# )    

# clap_juce_extensions_plugin(TARGET WaveTerrainSynth
#     CLAP_ID "com.AaronAnderson.WaveTerrainSynth"
#     CLAP_FEATURES instrument synthesizer
# )

# # if (APPLE)
# #    target_link_options(WaveTerrainSynth PUBLIC -Wl -ld_classic)
# # endif()

# # target_compile_options(WaveTerrainSynth PRIVATE -fsanitize=thread)
# # target_link_options(WaveTerrainSynth PRIVATE -fsanitize=thread)

# set_target_properties(WaveTerrainSynth PROPERTIES 
#     CXX_STANDARD 17
#     COMPILE_WARNING_AS_ERROR OFF
# )

# juce_add_module(glm_module)
# set_target_properties(glm_module PROPERTIES 
#     COMPILE_WARNING_AS_ERROR OFF,
#     COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -W"
# )
# # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-lto")
# # target_sources(WaveTerrainSynth
# #     PRIVATE
# #         Source/MainEditor.cpp
# #         Source/MainProcessor.cpp
# #         Source/DSP/MPEVoiceData.cpp)
# file(GLOB_RECURSE TERRAIN_CPP_FILES CONFIGURE_DEPENDS
#     Source/*.cpp
#     Source/*.c
# )

# file(GLOB_RECURSE TERRAIN_HEADER_FILES CONFIGURE_DEPENDS
#     Source/*.h
#     Source/*.hpp
# )

# target_sources(WaveTerrainSynth
#     PRIVATE
#         ${TERRAIN_CPP_FILES}
#         ${TERRAIN_HEADER_FILES}
# )

# set(CMAKE_POSITION_INDEPENDENT_CODE ON) #necessary for GCC linking
# add_library(MTS-ESP STATIC MTS-ESP/Client/libMTSClient.cpp)

# target_include_directories(WaveTerrainSynth PRIVATE 
#     PerlinNoise 
#     MTS-ESP/Client
#     glm/glm)

# target_compile_definitions(WaveTerrainSynth
#     PUBLIC
#         # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
#         JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
#         JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
#         )

# juce_add_binary_data(BinaryData SOURCES
#     Source/Interface/Renderer/Shaders/TrajectoryPoint.frag 
#     Source/Interface/Renderer/Shaders/TrajectoryPoint.vert
#     Source/Interface/Renderer/Shaders/Terrain.frag 
#     Source/Interface/Renderer/Shaders/Terrain.vert
#     Source/Interface/Paul-le1V.ttf
# )

# if(WIN32) 
#     target_link_libraries(WaveTerrainSynth 
#     PRIVATE
#         glu32.lib   
#     )
# endif()

# add_subdirectory(morphlib)

# target_link_libraries(WaveTerrainSynth
#     PRIVATE
#         BinaryData
#         glm_module
#         juce::juce_audio_utils
#         juce::juce_dsp
#         juce::juce_opengl
#         MTS-ESP
#         morphlib
#     PUBLIC
#         juce::juce_recommended_config_flags
#         juce::juce_recommended_lto_flags
#         juce::juce_recommended_warning_flags)
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
    VST3_COPY_DIR "C:\\Program Files\\Common Files\\VST3\\"
    PLUGIN_MANUFACTURER_CODE Aaro
    PLUGIN_CODE JEe9
    APP_SANDBOX_ENABLED TRUE
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "Terrain"
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

# ------------------------------------------------------------------------------
# glm module (as in your original)
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
