cmake_minimum_required(VERSION 3.22) 

project(TERRAIN VERSION 0.0.1)

find_package(OpenGL REQUIRED)
add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE

juce_add_plugin(WaveTerrainSynth
    VERSION "0.0.1"                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "Aaron Anderson"                          # Specify the name of the plugin's author
    IS_SYNTH TRUE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE              # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
    VST3_COPY_DIR "." 
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Aaro               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE JEe9                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS Standalone VST3 #Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Terrain"        # The name of the final executable, which can differ from the target name
    
)    

set_target_properties(WaveTerrainSynth PROPERTIES 
    CXX_STANDARD 17
    COMPILE_WARNING_AS_ERROR ON
)

target_sources(WaveTerrainSynth
    PRIVATE
        Source/MainEditor.cpp
        Source/MainProcessor.cpp)

target_compile_definitions(WaveTerrainSynth
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)
        
# juce_add_binary_data(BinaryData SOURCES
#     Source/Interface/Renderer/shaders/TrajectoryPoint.frag 
#     Source/Interface/Renderer/shaders/TrajectoryPoint.vert
#     Source/Interface/Renderer/shaders/Terrain.frag 
#     Source/Interface/Renderer/shaders/Terrain.vert
# )

if(WIN32) 
    target_link_libraries(WaveTerrainSynth 
    PRIVATE
        glu32.lib   
    )
endif()

target_link_libraries(WaveTerrainSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)