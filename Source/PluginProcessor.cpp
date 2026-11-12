#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float maxDelaySeconds = 2.0f;
constexpr float maxFeedback = 0.95f;

struct Preset
{
    float depth;
    float rate;
    float width;
    float attackMs;
    float releaseMs;
    float delayMs;
    float feedback;
    float delayMix;
    float outputMix;
};

constexpr std::array<Preset, PsyGaterAudioProcessor::presetCount> presetValues {{
    { 0.82f, 1.0f, 0.52f, 2.0f, 35.0f, 360.0f, 0.42f, 0.28f, 1.0f },
    { 0.94f, 2.0f, 0.38f, 0.8f, 18.0f, 260.0f, 0.34f, 0.2f, 0.96f },
    { 0.65f, 3.0f, 0.60f, 8.0f, 80.0f, 520.0f, 0.68f, 0.52f, 0.9f },
    { 0.72f, 2.0f, 0.66f, 10.0f, 110.0f, 620.0f, 0.58f, 0.4f, 0.94f }
}};
}

const std::array<const char*, PsyGaterAudioProcessor::presetCount> PsyGaterAudioProcessor::presetNames {{
    "Goa Pulse", "Psy Chop", "Echo Ritual", "Forest Flow"
}};

PsyGaterAudioProcessor::PsyGaterAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PsyGaterState", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PsyGaterAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Choice = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<Float>("depth", "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.82f));
    layout.add(std::make_unique<Float>("rate", "Rate",
        juce::NormalisableRange<float>(0.0f, 6.0f), 1.0f));
    layout.add(std::make_unique<Float>("width", "Width",
        juce::NormalisableRange<float>(0.05f, 0.95f), 0.52f));
    layout.add(std::make_unique<Float>("attack", "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f), 2.0f));
    layout.add(std::make_unique<Float>("release", "Release",
        juce::NormalisableRange<float>(1.0f, 500.0f), 35.0f));
    layout.add(std::make_unique<Float>("delayTime", "Delay",
        juce::NormalisableRange<float>(20.0f, 1500.0f), 360.0f));
    layout.add(std::make_unique<Float>("feedback", "Feedback",
        juce::NormalisableRange<float>(0.0f, maxFeedback), 0.42f));
    layout.add(std::make_unique<Float>("delayMix", "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.28f));
    layout.add(std::make_unique<Float>("outputMix", "Output",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    layout.add(std::make_unique<Choice>("preset", "Preset",
        juce::StringArray { "Goa Pulse", "Psy Chop", "Echo Ritual", "Forest Flow" }, 0));

    return layout;
}

bool PsyGaterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& inputSet = layouts.getMainInputChannelSet();
    const auto& outputSet = layouts.getMainOutputChannelSet();
    return (inputSet == juce::AudioChannelSet::mono() || inputSet == juce::AudioChannelSet::stereo())
        && inputSet == outputSet;
}

void PsyGaterAudioProcessor::prepareToPlay(double sr, int)
{
    sampleRate = sr;
    const size_t size = static_cast<size_t>(std::max(2.0, sr * maxDelaySeconds));

    for (auto& line : delayLines)
    {
        line.buffer.assign(size, 0.0f);
        line.writePosition = 0;
    }

    gatePhase = 0.0;
    gateEnvelope = 1.0f;
}

void PsyGaterAudioProcessor::releaseResources()
{
    for (auto& line : delayLines)
    {
        line.buffer.clear();
        line.writePosition = 0;
    }
}

float PsyGaterAudioProcessor::getValue(const char* id) const
{
    if (auto* value = parameters.getRawParameterValue(id))
        return value->load();
    return 0.0f;
}

void PsyGaterAudioProcessor::setValue(const char* id, float value)
{
    if (auto* param = parameters.getRawParameterValue(id))
        param->setValueNotifyingHost(value);
}

void PsyGaterAudioProcessor::applyPreset(int presetIndex)
{
    const int index = juce::jlimit(0, presetCount - 1, presetIndex);
    const auto& preset = presetValues[static_cast<size_t>(index)];

    setValue("depth", preset.depth);
    setValue("rate", preset.rate);
    setValue("width", preset.width);
    setValue("attack", preset.attackMs);
    setValue("release", preset.releaseMs);
    setValue("delayTime", preset.delayMs);
    setValue("feedback", preset.feedback);
    setValue("delayMix", preset.delayMix);
    setValue("outputMix", preset.outputMix);
    if (auto* presetParam = parameters.getRawParameterValue("preset"))
        presetParam->setValueNotifyingHost(static_cast<float>(index));
}

void PsyGaterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();
    for (int ch = inputChannels; ch < outputChannels; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const int activeChannels = juce::jmin(2, inputChannels, outputChannels);
    if (activeChannels < 1 || sampleRate <= 0.0)
        return;

    const float depth = getValue("depth");
    const float rateValue = juce::jlimit(0.0f, 6.0f, getValue("rate"));
    const float width = getValue("width");
    const float attackMs = getValue("attack");
    const float releaseMs = getValue("release");
    const float delayMs = getValue("delayTime");
    const float feedback = juce::jlimit(0.0f, maxFeedback, getValue("feedback"));
    const float delayMix = getValue("delayMix");
    const float outputMix = getValue("outputMix");

    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit(20.0, 300.0, *hostBpm);

    const double divisionRates[] = { bpm / 60.0, bpm / 30.0, bpm / 15.0, bpm / 7.5, 4.0, 8.0, 12.0 };
    const int rateIndex = juce::jlimit(0, 6, juce::roundToInt(rateValue));
    const double gateRate = divisionRates[rateIndex];
    const double phaseStep = gateRate / sampleRate;

    const int maxDelaySamples = static_cast<int>(sampleRate * maxDelaySeconds);
    const int delaySamples = juce::jlimit(1, maxDelaySamples, juce::roundToInt(delayMs * 0.001 * sampleRate));

    const float attackCoeff = (attackMs > 0.0f) ? std::exp(-1.0f / (attackMs * 0.001f * sampleRate)) : 0.0f;
    const float releaseCoeff = (releaseMs > 0.0f) ? std::exp(-1.0f / (releaseMs * 0.001f * sampleRate)) : 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float targetGate = (gatePhase < width) ? 1.0f : (1.0f - depth);
        const float coeff = (targetGate > gateEnvelope) ? attackCoeff : releaseCoeff;
        gateEnvelope = targetGate + coeff * (gateEnvelope - targetGate);

        const float inputL = buffer.getSample(0, sample);
        const float inputR = activeChannels > 1 ? buffer.getSample(1, sample) : inputL;

        for (int ch = 0; ch < activeChannels; ++ch)
        {
            auto& line = delayLines[static_cast<size_t>(ch)];
            const int size = static_cast<int>(line.buffer.size());
            const int readPos = (line.writePosition + size - delaySamples) % size;
            const float delayed = line.buffer[static_cast<size_t>(readPos)];
            const float cross = (ch == 0 && activeChannels > 1)
                ? delayLines[1].buffer[static_cast<size_t>(readPos)]
                : ((ch == 1 && activeChannels > 1)
                    ? delayLines[0].buffer[static_cast<size_t>(readPos)]
                    : delayed);

            const float source = (ch == 0) ? inputL : inputR;
            const float gatedInput = source * gateEnvelope;

            line.buffer[static_cast<size_t>(line.writePosition)] = gatedInput + cross * feedback;

            const float wet = delayed * (1.0f - delayMix) + (gatedInput * delayMix);
            const float output = source * (1.0f - outputMix) + wet * outputMix;
            buffer.setSample(ch, sample, output);

            line.writePosition = (line.writePosition + 1) % size;
        }

        gatePhase += phaseStep;
        if (gatePhase >= 1.0)
            gatePhase -= std::floor(gatePhase);
    }
}

juce::AudioProcessorEditor* PsyGaterAudioProcessor::createEditor()
{
    return new PsyGaterAudioProcessorEditor(*this);
}

void PsyGaterAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto state = parameters.copyState(); auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void PsyGaterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PsyGaterAudioProcessor();
}
