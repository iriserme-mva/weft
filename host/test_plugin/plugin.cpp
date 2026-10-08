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
        dryMixParameter = std::make_unique<juce::AudioParameterFloat>(
            "dryMix", "DryMix", juce::NormalisableRange<float>(0.f, 1.f), 0.8f);
        modeParameter = std::make_unique<juce::AudioParameterChoice>(
            "mode", "Mode", juce::StringArray{"Soft", "Hard", "Off"}, 0);
        addParameter(dryMixParameter.get());
        addParameter(modeParameter.get());
    }

    static juce::AudioProcessor* createPluginFilter() {
        return new SmokeProcessor();
    }

    const juce::String getName() const override { return "WeftSmokePlugin"; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        // The param is constructed at its default, so the current (normalized)
        // value IS the default. Use the public get() accessor — getValue() and
        // getDefaultValue() are private in JUCE 8.
        dryMix = dryMixParameter->get();
        juce::ignoreUnused(sampleRate, samplesPerBlock);
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&) override {
        if (dryMix < 1.0001f)
            buffer.applyGain(dryMix);  // all channels at once
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
    std::unique_ptr<juce::AudioParameterFloat> dryMixParameter;
    std::unique_ptr<juce::AudioParameterChoice> modeParameter;
    float dryMix = 0.8f;
};

}  // namespace weft_smoke

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return weft_smoke::SmokeProcessor::createPluginFilter();
}

// JucePlugin_CreateInstance is provided by the generated JuceHeader.h.
