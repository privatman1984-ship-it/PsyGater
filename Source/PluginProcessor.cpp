#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>

class PsyGaterAudioProcessor final : public juce::AudioProcessor
{
public:
    PsyGaterAudioProcessor();
    ~PsyGaterAudioProcessor() override = default;

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.6; }
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    void getStateInformation(juce::MemoryBlock& destination) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void applyPreset(int presetIndex);
    static constexpr int presetCount = 8;
    static const std::array<const char*, presetCount> presetNames;

    juce::AudioProcessorValueTreeState parameters;

private:
    struct DelayLine
    {
        std::vector<float> buffer;
        int writePosition = 0;
    };

    std::array<DelayLine, 2> delayLines;
    double sampleRate = 44100.0;
    double gatePhase = 0.0;
    uint64_t gateCycle = 0;
    float gateEnvelope = 1.0f;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void setValue(const char* parameterID, float value);
    float getValue(const char* parameterID) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsyGaterAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
