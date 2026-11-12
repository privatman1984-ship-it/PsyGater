#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int maximumDelaySeconds = 2;
constexpr float maximumFeedback = 0.95f;

struct Preset
{
    float depth;
    int rate;
    float width;
    float attackMs;
    float releaseMs;
    float delayMs;
    float feedback;
    float delayMix;
    float outputMix;
};

constexpr std::array<Preset, PsyGaterAudioProcessor::presetCount> presets {{
    { 0.82f, 2, 0.50f, 2.0f, 35.0f, 375.0f, 0.42f, 0.28f, 1.0f }, // Goa Pulse
    { 0.96f, 3, 0.38f, 0.5f, 18.0f, 250.0f, 0.32f, 0.18f, 1.0f }, // Psy Chop
    { 0.62f, 1, 0.58f, 8.0f, 75.0f, 500.0f, 0.68f, 0.52f, 0.9f }, // Echo Ritual
    { 0.72f, 2, 0.64f, 12.0f, 110.0f, 625.0f, 0.55f, 0.38f, 0.92f } // Forest Flow
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

    layout.add(std::make_unique<Float>(juce::ParameterID { "depth", 1 }, "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.82f));
    layout.add(std::make_unique<Choice>(juce::ParameterID { "rate", 1 }, "Rate",
        juce::StringArray { "1/4", "1/8", "1/16", "1/32", "4 Hz", "8 Hz", "12 Hz" }, 2));
    layout.add(std::make_unique<Float>(juce::ParameterID { "width", 1 }, "Width",
        juce::NormalisableRange<float>(0.05f, 0.95f), 0.50f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "attack", 1 }, "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.0f, 0.35f), 2.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "release", 1 }, "Release",
        juce::NormalisableRange<float>(1.0f, 500.0f, 0.0f, 0.35f), 35.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "delayTime", 1 }, "Delay",
        juce::NormalisableRange<float>(20.0f, 1500.0f, 0.0f, 0.5f), 375.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "feedback", 1 }, "Feedback",
        juce::NormalisableRange<float>(0.0f, maximumFeedback), 0.42f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "delayMix", 1 }, "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.28f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "outputMix", 1 }, "Output",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    layout.add(std::make_unique<Choice>(juce::ParameterID { "preset", 1 }, "Preset",
        juce::StringArray { "Goa Pulse", "Psy Chop", "Echo Ritual", "Forest Flow" }, 0));
    return layout;
}

bool PsyGaterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& output = layouts.getMainOutputChannelSet();
    if (output != juce::AudioChannelSet::mono() && output != juce::AudioChannelSet::stereo())
        return false;
    return output == layouts.getMainInputChannelSet();
}

void PsyGaterAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    const auto capacity = static_cast<size_t>(std::max(2.0, sampleRate * maximumDelaySeconds));
    for (auto& channel : delay)
    {
        channel.samples.assign(capacity, 0.0f);
        channel.writePosition = 0;
    }
    gatePhase = 0.0;
    gateEnvelope = 1.0f;
}

void PsyGaterAudioProcessor::releaseResources()
{
    for (auto& channel : delay)
    {
        channel.samples.clear();
        channel.writePosition = 0;
    }
}

float PsyGaterAudioProcessor::getParameter(const char* parameterID) const
{
    if (auto* value = parameters.getRawParameterValue(parameterID))
        return value->load();
    return 0.0f;
}

void PsyGaterAudioProcessor::setParameter(const char* parameterID, float value)
{
    if (auto* parameter = parameters.getParameter(parameterID))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

void PsyGaterAudioProcessor::applyPreset(int presetIndex)
{
    const auto index = juce::jlimit(0, presetCount - 1, presetIndex);
    const auto& preset = presets[static_cast<size_t>(index)];
    setParameter("depth", preset.depth);
    setParameter("rate", static_cast<float>(preset.rate));
    setParameter("width", preset.width);
    setParameter("attack", preset.attackMs);
    setParameter("release", preset.releaseMs);
    setParameter("delayTime", preset.delayMs);
    setParameter("feedback", preset.feedback);
    setParameter("delayMix", preset.delayMix);
    setParameter("outputMix", preset.outputMix);
    setParameter("preset", static_cast<float>(index));
}

void PsyGaterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();
    for (int channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    const int activeChannels = juce::jmin(2, inputChannels, buffer.getNumChannels());
    if (activeChannels == 0 || currentSampleRate <= 0.0)
        return;

    const float depth = getParameter("depth");
    const int rateChoice = juce::jlimit(0, 6, juce::roundToInt(getParameter("rate")));
    const float width = getParameter("width");
    const float attackMs = getParameter("attack");
    const float releaseMs = getParameter("release");
    const float delayMs = getParameter("delayTime");
    const float feedback = juce::jlimit(0.0f, maximumFeedback, getParameter("feedback"));
    const float delayMix = getParameter("delayMix");
    const float outputMix = getParameter("outputMix");

    double bpm = 120.0;
    if (const auto position = getPlayHead() != nullptr ? getPlayHead()->getPosition() : juce::Optional<juce::AudioPlayHead::PositionInfo> {})
        if (const auto hostBpm = position->getBpm())
            bpm = juce::jlimit(20.0, 300.0, *hostBpm);

    const double rates[] { bpm / 60.0, bpm / 30.0, bpm / 15.0, bpm / 7.5, 4.0, 8.0, 12.0 };
    const double phaseIncrement = rates[rateChoice] / currentSampleRate;
    const auto& firstDelay = delay[0].samples;
    if (firstDelay.size() < 2)
        return;
    const int delaySamples = juce::jlimit(1, static_cast<int>(firstDelay.size()) - 1,
        juce::roundToInt(delayMs * 0.001 * currentSampleRate));
    const float attackCoefficient = std::exp(-1.0f / (0.001f * attackMs * static_cast<float>(currentSampleRate)));
    const float releaseCoefficient = std::exp(-1.0f / (0.001f * releaseMs * static_cast<float>(currentSampleRate)));

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float targetGate = (gatePhase < width) ? 1.0f : (1.0f - depth);
        const float coefficient = targetGate > gateEnvelope ? attackCoefficient : releaseCoefficient;
        gateEnvelope = targetGate + coefficient * (gateEnvelope - targetGate);

        const float inLeft = buffer.getSample(0, sample);
        const float inRight = activeChannels > 1 ? buffer.getSample(1, sample) : inLeft;
        float out[2] { inLeft, inRight };

        for (int channel = 0; channel < activeChannels; ++channel)
        {
            auto& line = delay[static_cast<size_t>(channel)];
            const int size = static_cast<int>(line.samples.size());
            const int readPosition = (line.writePosition + size - delaySamples) % size;
            const float delayed = line.samples[static_cast<size_t>(readPosition)];
            const float source = channel == 0 ? inLeft : inRight;
            const float crossFeedback = activeChannels > 1
                ? delay[static_cast<size_t>(1 - channel)].samples[static_cast<size_t>(readPosition)]
                : delayed;
            line.samples[static_cast<size_t>(line.writePosition)] = source + crossFeedback * feedback;

            const float gated = source * gateEnvelope;
            const float processed = gated * (1.0f - delayMix) + delayed * delayMix;
            out[channel] = source * (1.0f - outputMix) + processed * outputMix;
            line.writePosition = (line.writePosition + 1) % size;
        }

        buffer.setSample(0, sample, out[0]);
        if (activeChannels > 1)
            buffer.setSample(1, sample, out[1]);

        gatePhase += phaseIncrement;
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
