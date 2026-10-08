# Minimal test VST3 plugin (CI smoke target).
# Not shipped — only built in CI to exercise the VST3 host end-to-end.
#
# NOTE: juce_add_modules() is called by JUCE's top-level CMakeLists
# (via FetchContent at the host level) — do NOT call it again here.

juce_add_plugin(weft_smoke_plugin
    FORMATS VST3
    PRODUCT_NAME "WeftSmokePlugin"
    NAME "WeftSmokePlugin"
    MANUFACTURER "Weft"
    COMPANY_NAME "Weft"
    COMPANY_WEBSITE "https://github.com/iriserme-mva/weft"
    COMPANY_COPYRIGHT "Copyright (c) 2026 Weft"
    BUNDLE_ID "com.weft.smokeplugin"
    PLUGIN_CODE "WftS"
    VERSION "1.0.0"
    # juce_vst3_helper (writes moduleinfo.json post-build) crashes on the CI
    # images (Windows 0xC0000005 / macOS SIGSEGV) with JUCE 8.0.4. The smoke
    # host discovers VST3s via direct factory enumeration when moduleinfo.json
    # is absent (VST3PluginFormat::findAllTypesForFile slow path), so skip it.
    VST3_AUTO_MANIFEST FALSE)

target_sources(weft_smoke_plugin PRIVATE plugin.cpp)
target_link_libraries(weft_smoke_plugin PRIVATE juce::juce_audio_processors)

# Headless host: disable the web browser so the juce_gui_extra module sources
# (compiled into this target) don't drag in GTK (Linux/BSD) or WebKit (macOS).
# Matches the way JUCE's own AudioPluginHost is configured.
target_compile_definitions(weft_smoke_plugin PRIVATE JUCE_WEB_BROWSER=0)

# We use no networking. JUCE_USE_CURL defaults to 1 (juce_core.h), which
# #includes <curl/curl.h> on Linux/BSD; CI runners lack libcurl dev headers.
# Linking curl is separately opt-in (NEEDS_CURL, default off) — nothing to
# unlink, this only stops the header include.
target_compile_definitions(weft_smoke_plugin PRIVATE JUCE_USE_CURL=0)

# Must come AFTER juce_add_plugin: it writes into the target's generated
# sources dir. Produces JuceHeader.h (module includes + JucePlugin_CreateInstance).
juce_generate_juce_header(weft_smoke_plugin)
