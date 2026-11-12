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
    double getTailLengthSeconds() const override { return 3.0; }
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

    juce::AudioProcessorValueTreeState parameters;
    void applyPreset(int presetIndex);

    static constexpr int presetCount = 4;
    static const std::array<const char*, presetCount> presetNames;

private:
    struct DelayChannel
    {
        std::vector<float> samples;
        int writePosition = 0;
    };

    std::array<DelayChannel, 2> delay;
    double currentSampleRate = 44100.0;
    double gatePhase = 0.0;
    float gateEnvelope = 1.0f;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float getParameter(const char* parameterID) const;
    void setParameter(const char* parameterID, float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsyGaterAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
