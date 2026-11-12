#pragma once

#include "PluginProcessor.h"

class PsyGaterAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PsyGaterAudioProcessorEditor(PsyGaterAudioProcessor& processor);
    ~PsyGaterAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    PsyGaterAudioProcessor& processor;
    juce::ComboBox presetBox;
    juce::Label presetLabel;
    std::array<juce::Slider, 9> sliders;
    std::array<juce::Label, 9> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 9> attachments;

    void configureSlider(size_t index, const juce::String& labelText, const juce::String& parameterID, bool milliseconds = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsyGaterAudioProcessorEditor)
};
