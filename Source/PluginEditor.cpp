#include "PluginEditor.h"

PsyGaterAudioProcessorEditor::PsyGaterAudioProcessorEditor(PsyGaterAudioProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse), processor(processorToUse)
{
    setSize(760, 440);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab5c2));
    presetLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(presetLabel);

    for (int i = 0; i < PsyGaterAudioProcessor::presetCount; ++i)
        presetBox.addItem(PsyGaterAudioProcessor::presetNames[static_cast<size_t>(i)], i + 1);
    presetBox.setSelectedItemIndex(juce::roundToInt(processor.parameters.getRawParameterValue("preset")->load()), juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int index = presetBox.getSelectedItemIndex();
        if (index >= 0)
            processor.applyPreset(index);
    };
    addAndMakeVisible(presetBox);

    configureSlider(0, "DEPTH", "depth");
    configureSlider(1, "RATE", "rate");
    configureSlider(2, "WIDTH", "width");
    configureSlider(3, "ATTACK", "attack", true);
    configureSlider(4, "RELEASE", "release", true);
    configureSlider(5, "DELAY (ms)", "delayTime", true);
    configureSlider(6, "FEEDBACK", "feedback");
    configureSlider(7, "DELAY MIX", "delayMix");
    configureSlider(8, "OUTPUT", "outputMix");
}

void PsyGaterAudioProcessorEditor::configureSlider(size_t index, const juce::String& labelText,
                                                    const juce::String& parameterID, bool milliseconds)
{
    auto& slider = sliders[index];
    auto& label = labels[index];
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff7a30));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffc16a));
    if (parameterID == "rate")
    {
        slider.setRange(0.0, 6.0, 1.0);
        slider.textFromValueFunction = [](double value)
        {
            const juce::StringArray names { "1/4", "1/8", "1/16", "1/32", "4 Hz", "8 Hz", "12 Hz" };
            return names[juce::jlimit(0, 6, juce::roundToInt(value))];
        };
        slider.valueFromTextFunction = [](const juce::String& text)
        {
            const juce::StringArray names { "1/4", "1/8", "1/16", "1/32", "4 Hz", "8 Hz", "12 Hz" };
            return static_cast<double>(juce::jmax(0, names.indexOf(text)));
        };
    }
    else if (milliseconds)
    {
        slider.setTextValueSuffix(" ms");
    }
    else if (parameterID != "outputMix")
    {
        slider.setTextValueSuffix(" %");
        slider.setNumDecimalPlacesToDisplay(0);
        slider.setTextFromValueFunction([](double value) { return juce::String(juce::roundToInt(value * 100.0)); });
        slider.setValueFromTextFunction([](const juce::String& text) { return text.getDoubleValue() / 100.0; });
    }

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffd5dce5));
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(label);
    addAndMakeVisible(slider);
    attachments[index] = std::make_unique<SliderAttachment>(processor.parameters, parameterID, slider);
}

void PsyGaterAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff10151d));
    graphics.setColour(juce::Colour(0xffff7a30));
    graphics.setFont(juce::Font(29.0f, juce::Font::bold));
    graphics.drawText("PSYGATER", 24, 16, 300, 38, juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(0xff788798));
    graphics.setFont(juce::Font(12.0f));
    graphics.drawText("GOA / PSYTRANCE RHYTHMIC GATE + STEREO DELAY", 26, 54, 500, 20, juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(0xff27313d));
    graphics.drawHorizontalLine(83, 24.0f, static_cast<float>(getWidth() - 24));
}

void PsyGaterAudioProcessorEditor::resized()
{
    presetLabel.setBounds(26, 91, 72, 24);
    presetBox.setBounds(104, 89, 190, 28);

    constexpr int columns = 5;
    constexpr int cellWidth = 140;
    constexpr int cellHeight = 145;
    const int startX = 25;
    const int startY = 138;
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const int column = static_cast<int>(i % columns);
        const int row = static_cast<int>(i / columns);
        const int x = startX + column * cellWidth;
        const int y = startY + row * cellHeight;
        labels[i].setBounds(x, y, 128, 20);
        sliders[i].setBounds(x + 4, y + 22, 120, 116);
    }
}
