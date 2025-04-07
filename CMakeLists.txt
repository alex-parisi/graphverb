cmake_minimum_required(VERSION 3.30)

project(GraphVerb VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent to grab JUCE
include(FetchContent)
FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG master
)
FetchContent_MakeAvailable(juce)

################################################################################
# GraphVerb Plugin                                                             #
# ------------                                                                 #
################################################################################

juce_add_plugin(GraphVerb
        COMPANY_NAME "Scribe Now LLC"
        PRODUCT_NAME "GraphVerb"
        BUNDLE_ID "com.scribenow.graphverb"
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE MJPL
        FORMATS VST3 AU Standalone
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(GraphVerb PRIVATE
        Components/SpectralAnalyzer/src/SpectralAnalyzer.cpp
        Components/SpectralGraph/src/SpectralGraph.cpp
        Components/CommunityClustering/src/CommunityClustering.cpp
        Components/UI/Knob/src/KnobComponent.cpp
        Components/UI/Button/src/ButtonComponent.cpp
        Components/UI/ClusterVisualizer/src/ClusterVisualizer.cpp
        Components/UI/ClusterEnergy/src/ClusterEnergy.cpp
        GraphVerb/src/GraphVerb.cpp
        GraphVerb/src/GraphVerbEditor.cpp
)

target_include_directories(GraphVerb PRIVATE
        GraphVerb/inc
        Components/SpectralAnalyzer/inc
        Components/SpectralGraph/inc
        Components/CommunityClustering/inc
        Components/CommunityReverb/inc
        Components/UI/Knob/inc
        Components/UI/Button/inc
        Components/UI/Scope/inc
        Components/UI/ClusterEnergy/inc
        Components/UI/ClusterVisualizer/inc
)

target_compile_definitions(GraphVerb PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

target_link_libraries(GraphVerb PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_dsp
)
