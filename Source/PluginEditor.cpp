#include "PluginEditor.h"

namespace
{
juce::String rateText(double value)
{
    const juce::StringArray names { "1/4", "1/8", "1/16", "1/32", "4 Hz", "8 Hz", "12 Hz" };
    return names[juce::jlimit(0, 6, juce::roundToInt(value))];
}
}

PsyGaterAudioProcessorEditor::PsyGaterAudioProcessorEditor(PsyGaterAudioProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse), processor(processorToUse)
{
    setSize(760, 470);
    setResizable(true, true);
    setResizeLimits(680, 420, 1080, 680);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab5c2));
    presetLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(presetLabel);

    for (int i = 0; i < PsyGaterAudioProcessor::presetCount; ++i)
        presetBox.addItem(PsyGaterAudioProcessor::presetNames[static_cast<size_t>(i)], i + 1);

    presetBox.onChange = [this]
    {
        const int index = presetBox.getSelectedItemIndex();
        if (index >= 0)
            processor.applyPreset(index);
    };
    addAndMakeVisible(presetBox);

    patternLabel.setText("GATE PATTERN", juce::dontSendNotification);
    patternLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab5c2));
    patternLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(patternLabel);
    patternBox.addItemList({ "Straight", "Goa", "Offbeat", "Stutter" }, 1);
    addAndMakeVisible(patternBox);
    patternAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "pattern", patternBox);

    configureSlider(0, "DEPTH", "depth");
    configureSlider(1, "RATE", "rate");
    configureSlider(2, "WIDTH", "width");
    configureSlider(3, "ATTACK", "attack", true);
    configureSlider(4, "RELEASE", "release", true);
    configureSlider(5, "DELAY TIME", "delayTime", true);
    configureSlider(6, "FEEDBACK", "feedback");
    configureSlider(7, "DRY / WET", "dryWet");
    configureSlider(8, "OUTPUT", "output");
    configureSlider(9, "SWING", "swing");

    if (auto* value = processor.parameters.getRawParameterValue("preset"))
        presetBox.setSelectedItemIndex(juce::jlimit(0, PsyGaterAudioProcessor::presetCount - 1,
            juce::roundToInt(value->load())), juce::dontSendNotification);
}

void PsyGaterAudioProcessorEditor::configureSlider(size_t index, const juce::String& labelText,
                                                    const juce::String& parameterID, bool milliseconds)
{
    auto& slider = sliders[index];
    auto& label = labels[index];

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff7a30));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffc16a));

    if (parameterID == "rate")
    {
        slider.setRange(0.0, 6.0, 1.0);
        slider.textFromValueFunction = [](double value) { return rateText(value); };
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
    else
    {
        slider.setTextValueSuffix(" %");
        slider.setNumDecimalPlacesToDisplay(0);
        slider.textFromValueFunction = [](double value) { return juce::String(juce::roundToInt(value * 100.0)); };
        slider.valueFromTextFunction = [](const juce::String& text) { return text.getDoubleValue() / 100.0; };
    }

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffd5dce5));
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(label);
    addAndMakeVisible(slider);

    sliderAttachments[index] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, parameterID, slider);
}

void PsyGaterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10151d));
    g.setColour(juce::Colour(0xffff7a30));
    g.setFont(juce::Font(30.0f, juce::Font::bold));
    g.drawText("PSYGATER", 24, 15, 310, 38, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8795a5));
    g.setFont(juce::Font(12.0f));
    g.drawText("RHYTHMIC GATE  /  PING-PONG ECHO", 26, 54, 450, 20, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff293440));
    g.drawHorizontalLine(83, 24.0f, static_cast<float>(getWidth() - 24));

    g.setColour(juce::Colour(0xff19232d));
    g.fillRoundedRectangle(16.0f, 78.0f, static_cast<float>(getWidth() - 32), 48.0f, 8.0f);
}

void PsyGaterAudioProcessorEditor::resized()
{
    const int scale = getWidth();
    const int margin = 24;

    presetLabel.setBounds(margin, 90, 62, 24);
    presetBox.setBounds(margin + 68, 88, 190, 28);
    patternLabel.setBounds(margin + 300, 90, 100, 24);
    patternBox.setBounds(margin + 405, 88, 145, 28);

    const int columns = 5;
    const int cellWidth = (scale - 2 * margin) / columns;
    const int cellHeight = (getHeight() - 142) / 2;
    const int startY = 138;

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const int column = static_cast<int>(i % columns);
        const int row = static_cast<int>(i / columns);
        const int x = margin + column * cellWidth;
        const int y = startY + row * cellHeight;
        labels[i].setBounds(x, y, cellWidth - 8, 20);
        sliders[i].setBounds(x + 4, y + 22, cellWidth - 16, juce::jmax(90, cellHeight - 22));
    }
}
