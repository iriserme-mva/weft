// Minimal test VST3 plugin for CI smoke tests.
// Two params: "DryMix" (continuous float, 0..1, default 0.8) and
// "Mode" (discrete choice: Soft/Hard/Off, default Soft).

#include "JuceHeader.h"

namespace weft_smoke {

class SmokeProcessor : public juce::AudioProcessor {
public:
    SmokeProcessor()
        : AudioProcessor(juce::AudioProcessor::BusesProperties()
                             .withInput("Input", juce::AudioChannelSet::stereo())
                             .withOutput("Output", juce::AudioChannelSet::stereo())) {
        // CRITICAL (JUCE 8.0.4): addParameter() transfers ownership of the
        // parameter to the AudioProcessor — the parameterTree deletes it in
        // ~AudioProcessor (juce_AudioProcessor.cpp, addParameter wraps the raw
        // pointer in a unique_ptr). Do NOT also hold it in a unique_ptr member:
        // that double-ownership is a use-after-free that segfaults at teardown
        // of any instance (e.g. the throwaway component a VST3 host creates in
        // findAllTypesForFile) on every OS. Canonical JUCE pattern: raw pointer
        // member owned by the processor (see examples/Plugins/GainPluginDemo.h).
        dryMixParameter = new juce::AudioParameterFloat(
            "dryMix", "DryMix", juce::NormalisableRange<float>(0.f, 1.f), 0.8f);
        addParameter(dryMixParameter);

        modeParameter = new juce::AudioParameterChoice(
            "mode", "Mode", juce::StringArray{"Soft", "Hard", "Off"}, 0);
        addParameter(modeParameter);
    }

    static juce::AudioProcessor* createPluginFilter() {
        return new SmokeProcessor();
    }

    const juce::String getName() const override { return "WeftSmokePlugin"; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        juce::ignoreUnused(sampleRate, samplesPerBlock);
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&) override {
        // Read the param live: an offline renderer (and any live host) pushes
        // config values *after* prepareToPlay, so a cached copy would silently
        // ignore them. This is what makes the CI render assertion meaningful —
        // it proves the config's `params:` block actually reaches the plugin.
        const float mix = dryMixParameter->get();
        if (mix < 1.0001f)
            buffer.applyGain(mix);
    }

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool isMidiEffect() const override { return false; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    // Owned by the AudioProcessor (see addParameter comment in ctor) — raw
    // pointer, never delete manually, never hold in a smart pointer.
    juce::AudioParameterFloat* dryMixParameter = nullptr;
    juce::AudioParameterChoice* modeParameter = nullptr;
};

}  // namespace weft_smoke

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return weft_smoke::SmokeProcessor::createPluginFilter();
}

// JucePlugin_CreateInstance is provided by the generated JuceHeader.h.
