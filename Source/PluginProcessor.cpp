#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float maxDelaySeconds = 2.0f;
constexpr float maxFeedback = 0.95f;

struct Preset
{
    float depth, rate, width, attackMs, releaseMs, delayMs, feedback, dryWet, output, pattern, swing;
};

constexpr std::array<Preset, PsyGaterAudioProcessor::presetCount> presets {{
    { 0.86f, 2.0f, 0.48f, 1.2f, 24.0f, 375.0f, 0.48f, 0.30f, 0.94f, 1.0f, 0.08f },
    { 0.96f, 3.0f, 0.34f, 0.4f, 12.0f, 250.0f, 0.34f, 0.22f, 0.96f, 3.0f, 0.02f },
    { 0.70f, 1.0f, 0.58f, 5.0f, 80.0f, 500.0f, 0.66f, 0.46f, 0.90f, 1.0f, 0.12f },
    { 0.76f, 2.0f, 0.64f, 9.0f, 110.0f, 625.0f, 0.56f, 0.38f, 0.94f, 2.0f, 0.18f },
    { 0.92f, 2.0f, 0.42f, 0.6f, 16.0f, 187.5f, 0.44f, 0.32f, 0.92f, 3.0f, 0.0f },
    { 0.78f, 3.0f, 0.55f, 3.0f, 42.0f, 750.0f, 0.72f, 0.56f, 0.88f, 1.0f, 0.10f },
    { 0.64f, 1.0f, 0.70f, 14.0f, 150.0f, 420.0f, 0.52f, 0.34f, 0.90f, 2.0f, 0.22f },
    { 0.98f, 4.0f, 0.28f, 0.2f, 8.0f, 125.0f, 0.28f, 0.18f, 0.98f, 3.0f, 0.0f }
}};
}

const std::array<const char*, PsyGaterAudioProcessor::presetCount> PsyGaterAudioProcessor::presetNames {{
    "Goa Pulse", "Psy Chop", "Echo Ritual", "Forest Flow",
    "Night Drive", "Crystal Echo", "Moss Temple", "Razor Stutter"
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

    layout.add(std::make_unique<Float>(juce::ParameterID { "depth", 1 }, "Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.86f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "rate", 1 }, "Rate", juce::NormalisableRange<float>(0.0f, 6.0f, 1.0f), 2.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "width", 1 }, "Width", juce::NormalisableRange<float>(0.05f, 0.95f), 0.48f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "attack", 1 }, "Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.0f, 0.35f), 1.2f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "release", 1 }, "Release", juce::NormalisableRange<float>(1.0f, 500.0f, 0.0f, 0.35f), 24.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "delayTime", 1 }, "Delay Time", juce::NormalisableRange<float>(20.0f, 1500.0f, 0.0f, 0.5f), 375.0f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "feedback", 1 }, "Feedback", juce::NormalisableRange<float>(0.0f, maxFeedback), 0.48f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "dryWet", 1 }, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f));
    layout.add(std::make_unique<Float>(juce::ParameterID { "output", 1 }, "Output", juce::NormalisableRange<float>(0.0f, 1.0f), 0.94f));
    layout.add(std::make_unique<Choice>(juce::ParameterID { "pattern", 1 }, "Pattern",
        juce::StringArray { "Straight", "Goa", "Offbeat", "Stutter" }, 1));
    layout.add(std::make_unique<Float>(juce::ParameterID { "swing", 1 }, "Swing", juce::NormalisableRange<float>(0.0f, 0.45f), 0.08f));
    layout.add(std::make_unique<Choice>(juce::ParameterID { "preset", 1 }, "Preset",
        juce::StringArray { "Goa Pulse", "Psy Chop", "Echo Ritual", "Forest Flow", "Night Drive", "Crystal Echo", "Moss Temple", "Razor Stutter" }, 0));
    return layout;
}

bool PsyGaterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();
    return (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo()) && input == output;
}

void PsyGaterAudioProcessor::prepareToPlay(double sr, int)
{
    sampleRate = sr;
    const auto capacity = static_cast<size_t>(std::max(2.0, sr * maxDelaySeconds));
    for (auto& line : delayLines)
    {
        line.buffer.assign(capacity, 0.0f);
        line.writePosition = 0;
    }
    gatePhase = 0.0;
    gateCycle = 0;
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
    if (auto* parameter = parameters.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

void PsyGaterAudioProcessor::applyPreset(int presetIndex)
{
    const int index = juce::jlimit(0, presetCount - 1, presetIndex);
    const auto& p = presets[static_cast<size_t>(index)];
    setValue("depth", p.depth);
    setValue("rate", p.rate);
    setValue("width", p.width);
    setValue("attack", p.attackMs);
    setValue("release", p.releaseMs);
    setValue("delayTime", p.delayMs);
    setValue("feedback", p.feedback);
    setValue("dryWet", p.dryWet);
    setValue("output", p.output);
    setValue("pattern", p.pattern);
    setValue("swing", p.swing);
    setValue("preset", static_cast<float>(index));
}

void PsyGaterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();
    for (int channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    const int channelCount = juce::jmin(2, inputChannels, outputChannels, buffer.getNumChannels());
    if (channelCount == 0 || sampleRate <= 0.0 || delayLines[0].buffer.size() < 2)
        return;

    const float depth = getValue("depth");
    const int rateIndex = juce::jlimit(0, 6, juce::roundToInt(getValue("rate")));
    const float width = getValue("width");
    const float attackMs = getValue("attack");
    const float releaseMs = getValue("release");
    const float delayMs = getValue("delayTime");
    const float feedback = juce::jlimit(0.0f, maxFeedback, getValue("feedback"));
    const float dryWet = getValue("dryWet");
    const float output = getValue("output");
    const int pattern = juce::jlimit(0, 3, juce::roundToInt(getValue("pattern")));
    const double swing = juce::jlimit(0.0, 0.45, static_cast<double>(getValue("swing")));

    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit(20.0, 300.0, *hostBpm);

    // Rate indices 0-3 are tempo-synced quarter through 32nd notes; 4-6 are free 4/8/12 Hz.
    const double rates[] { bpm / 60.0, bpm / 30.0, bpm / 15.0, bpm / 7.5, 4.0, 8.0, 12.0 };
    const double phaseStep = rates[rateIndex] / sampleRate;
    const int delaySize = static_cast<int>(delayLines[0].buffer.size());
    const int delaySamples = juce::jlimit(1, delaySize - 1,
        juce::roundToInt(delayMs * 0.001 * sampleRate));
    const float attackCoeff = std::exp(-1.0f / (0.001f * attackMs * static_cast<float>(sampleRate)));
    const float releaseCoeff = std::exp(-1.0f / (0.001f * releaseMs * static_cast<float>(sampleRate)));

    // A 16-cycle bar mask provides repeatable Goa, offbeat and stutter patterns.
    constexpr uint16_t masks[] { 0xffffu, 0xd6adu, 0xaaaau, 0xccccu };

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const int cycleInBar = static_cast<int>(gateCycle & 15u);
        const bool patternStepEnabled = (masks[pattern] & (static_cast<uint16_t>(1u) << cycleInBar)) != 0;
        const double localPhase = (gatePhase >= 0.5 && (cycleInBar & 1) != 0)
            ? juce::jlimit(0.0, 0.999999, gatePhase - swing * 0.5)
            : gatePhase;
        const float targetGate = patternStepEnabled && localPhase < width ? 1.0f : (1.0f - depth);
        const float coefficient = targetGate > gateEnvelope ? attackCoeff : releaseCoeff;
        gateEnvelope = targetGate + coefficient * (gateEnvelope - targetGate);

        const float inputL = buffer.getSample(0, sample);
        const float inputR = channelCount > 1 ? buffer.getSample(1, sample) : inputL;
        auto& left = delayLines[0];
        const int leftRead = (left.writePosition + delaySize - delaySamples) % delaySize;
        const float delayedL = left.buffer[static_cast<size_t>(leftRead)];
        float delayedR = delayedL;
        if (channelCount > 1)
        {
            auto& right = delayLines[1];
            const int rightRead = (right.writePosition + delaySize - delaySamples) % delaySize;
            delayedR = right.buffer[static_cast<size_t>(rightRead)];
        }

        const float gatedL = inputL * gateEnvelope;
        const float gatedR = inputR * gateEnvelope;
        left.buffer[static_cast<size_t>(left.writePosition)] = gatedL + (channelCount > 1 ? delayedR : delayedL) * feedback;
        if (channelCount > 1)
            delayLines[1].buffer[static_cast<size_t>(delayLines[1].writePosition)] = gatedR + delayedL * feedback;

        const float outL = (inputL * (1.0f - dryWet) + (gatedL + delayedL) * dryWet) * output;
        const float outR = (inputR * (1.0f - dryWet) + (gatedR + delayedR) * dryWet) * output;
        buffer.setSample(0, sample, outL);
        if (channelCount > 1)
            buffer.setSample(1, sample, outR);

        left.writePosition = (left.writePosition + 1) % delaySize;
        if (channelCount > 1)
            delayLines[1].writePosition = (delayLines[1].writePosition + 1) % delaySize;

        gatePhase += phaseStep;
        if (gatePhase >= 1.0)
        {
            gatePhase -= std::floor(gatePhase);
            ++gateCycle;
        }
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
