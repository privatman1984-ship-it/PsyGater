#include "PluginEditor.h"

namespace
{
juce::String rateTextForValue(double value)
{
    const juce::StringArray names { "1/4", "1/8", "1/16", "1/32", "4Hz", "8Hz", "12Hz" };
    return names[juce::jlimit(0, 6, juce::roundToInt(value))];
}
}

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

    presetBox.setSelectedItemIndex(0, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        if (auto* presetParam = processor.parameters.getRawParameterValue("preset"))
            presetParam->setValueNotifyingHost(static_cast<float>(presetBox.getSelectedItemIndex()));
    };
    addAndMakeVisible(presetBox);

    configureSlider(0, "DEPTH", "depth");
    configureSlider(1, "RATE", "rate");
    configureSlider(2, "WIDTH", "width");
    configureSlider(3, "ATTACK", "attack", true);
    configureSlider(4, "RELEASE", "release", true);
    configureSlider(5, "DELAY", "delayTime", true);
    configureSlider(6, "FEEDBACK", "feedback");
    configureSlider(7, "DRY/WET", "dryWet");
    configureSlider(8, "OUTPUT", "output");

    if (auto* presetValue = processor.parameters.getRawParameterValue("preset"))
        presetBox.setSelectedItemIndex(juce::jlimit(0, PsyGaterAudioProcessor::presetCount - 1,
            juce::roundToInt(presetValue->load())), juce::dontSendNotification);
}

void PsyGaterAudioProcessorEditor::configureSlider(size_t index, const juce::String& labelText,
                                                    const juce::String& parameterID, bool milliseconds)
{
    auto& slider = sliders[index];
    auto& label = labels[index];

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff7a30));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffc16a));

    if (parameterID == "rate")
    {
        slider.setRange(0.0, 6.0, 1.0);
        slider.textFromValueFunction = [](double value)
        {
            return rateTextForValue(value);
        };
        slider.valueFromTextFunction = [](const juce::String& text)
        {
            const juce::StringArray names { "1/4", "1/8", "1/16", "1/32", "4Hz", "8Hz", "12Hz" };
            return static_cast<double>(juce::jlimit(0, 6, names.indexOf(text)));
        };
    }
    else if (milliseconds)
    {
        slider.setRange(0.1, 1000.0, 0.1);
        slider.setTextValueSuffix(" ms");
    }
    else
    {
        slider.setRange(0.0, 1.0, 0.01);
        slider.setTextValueSuffix(" %");
        slider.setNumDecimalPlacesToDisplay(0);
        slider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value * 100.0));
        };
        slider.valueFromTextFunction = [](const juce::String& text)
        {
            return text.getDoubleValue() / 100.0;
        };
    }

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffd5dce5));
    label.setFont(juce::Font(11.0f, juce::Font::bold));

    addAndMakeVisible(label);
    addAndMakeVisible(slider);
    attachments[index] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, parameterID, slider);
}

void PsyGaterAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff10151d));
    graphics.setColour(juce::Colour(0xffff7a30));
    graphics.setFont(juce::Font(29.0f, juce::Font::bold));
    graphics.drawText("PSYGATER", juce::Rectangle<float>(24.0f, 16.0f, 300.0f, 38.0f), juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xff788798));
    graphics.setFont(juce::Font(12.0f));
    graphics.drawText("GOA / PSYTRANCE RHYTHMIC GATE + STEREO DELAY",
        juce::Rectangle<float>(26.0f, 54.0f, 500.0f, 20.0f),
        juce::Justification::centredLeft);

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
