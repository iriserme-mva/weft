# JUCE host layer (opt-in: -DWEFT_BUILD_HOST=ON)
#
# Fetches JUCE 8.0.4 (pinned by SHA256), builds the test VST3 plugin and
# the weft_smoke CLI (JuceBackend + weft_core). Core-only builds never
# touch JUCE, so the CI core job stays light.

include(FetchContent)
FetchContent_Declare(juce
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.tar.gz
    URL_HASH SHA256=0effe9823a6fd8a4f98e5c08be7d308350e417b2d15dcf34011e778fefbbf005)
FetchContent_MakeAvailable(juce)

# JUCE's top-level CMakeLists already calls juce_add_modules(ALIAS_NAMESPACE juce)
# — do not call it again.

add_subdirectory(test_plugin)

# ---- smoke CLI -------------------------------------------------------------
add_executable(weft_smoke
    src/smoke.cpp
    src/JuceBackend.cpp
)
target_link_libraries(weft_smoke
    PRIVATE
        weft_core
        juce::juce_core
        juce::juce_events
        juce::juce_audio_processors
)
# NOTE: we do NOT link juce::juce_audio_plugin_client into the smoke CLI.
# That is the *plugin-side* module; linking it would compile a second copy of
# the VST3 SDK base into the host executable, which conflicts (symbol
# interposition / ODR on the shared FUnknown/IPluginFactory base) with the SDK
# already compiled into the loaded .vst3 plugin and segfaults at instantiate
# time. JUCE's own AudioPluginHost links only juce_audio_processors (host side)
# for exactly this reason. JuceBackend.cpp / smoke.cpp use no client API.
# Headless host: disable the web browser so the juce_gui_extra module sources
# (compiled into this target) don't drag in GTK (Linux/BSD) or WebKit (macOS).
# Matches the way JUCE's own AudioPluginHost is configured.
target_compile_definitions(weft_smoke PRIVATE JUCE_WEB_BROWSER=0)
# No networking: JUCE_USE_CURL defaults to 1 and #includes <curl/curl.h> on
# Linux/BSD, where CI runners lack libcurl dev headers. See test_plugin.
target_compile_definitions(weft_smoke PRIVATE JUCE_USE_CURL=0)
# VST3 hosting: the AudioPluginFormatManager only registers the VST3 format
# when JUCE_PLUGINHOST_VST3=1 (per-target, like JUCE_USE_CURL). Without it
# the manager has zero formats and load() fails with "no plugin format can
# load this file". JUCE's own AudioPluginHost is configured this way.
target_compile_definitions(weft_smoke PRIVATE JUCE_PLUGINHOST_VST3=1)
if(WIN32)
    target_link_libraries(weft_smoke PRIVATE ws2_32 winmm)
endif()
