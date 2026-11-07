cmake_minimum_required(VERSION 3.22)
project(PrismaSonoro VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Mac: binario universale (Apple Silicon + Intel), macOS 11 o successivo
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

# JUCE: da una cartella locale (-DJUCE_DIR=...) oppure scaricato automaticamente
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

set(PRISMA_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND PRISMA_FORMATS AU)
endif()

juce_add_plugin(PrismaSonoro
    COMPANY_NAME "MBStudio91"
    COMPANY_WEBSITE "https://mbstudio91.it"
    BUNDLE_ID "it.mbstudio91.prismasonoro"
    PLUGIN_MANUFACTURER_CODE Mbs9
    PLUGIN_CODE Prsm
    FORMATS ${PRISMA_FORMATS}
    PRODUCT_NAME "Prisma Sonoro"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice)

juce_add_binary_data(PrismaFonts SOURCES
    Resources/BarlowCondensed-Regular.ttf
    Resources/BarlowCondensed-SemiBold.ttf
    Resources/ShareTechMono-Regular.ttf)

target_sources(PrismaSonoro PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)

target_compile_definitions(PrismaSonoro PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PrismaSonoro
    PRIVATE PrismaFonts juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
