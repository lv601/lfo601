#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;

    static float clamp1 (float v) noexcept
    {
        return juce::jlimit (-1.0f, 1.0f, v);
    }

    static uint32_t xorshift32 (uint32_t& s) noexcept
    {
        if (s == 0)
            s = 0xDEADBEEF;

        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return s;
    }

    static float randomBipolar (uint32_t& s) noexcept
    {
        return (static_cast<float> (xorshift32 (s) & 0x7fffffff) / static_cast<float> (0x7fffffff)) * 2.0f - 1.0f;
    }

    static float beatsPerCycleForDivision (int syncDivision) noexcept
    {
        switch (syncDivision)
        {
            case LFOToolAudioProcessor::eighth:  return 0.5f; // 1/8 note
            case LFOToolAudioProcessor::quarter: return 1.0f; // 1/4 note
            case LFOToolAudioProcessor::half:    return 2.0f; // 1/2 note
            case LFOToolAudioProcessor::oneBar:  return 4.0f; // 4/4 bar
            case LFOToolAudioProcessor::twoBars: return 8.0f;
            case LFOToolAudioProcessor::free:
            default:                             return 1.0f;
        }
    }
}

LFOToolAudioProcessor::LFOToolAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Stereo In",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("LFO CV Out", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout LFOToolAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto rateRange = juce::NormalisableRange<float> (0.01f, 20.0f, 0.001f, 0.35f);
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "rate", 1 },
                                                              "Rate", rateRange, 1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "depth", 1 },
                                                              "Depth", juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "offset", 1 },
                                                              "Offset", juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "smooth", 1 },
                                                              "Smooth", juce::NormalisableRange<float> (0.0f, 0.99f), 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "phase", 1 },
                                                              "Phase", juce::NormalisableRange<float> (0.0f, 360.0f), 0.0f));

    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "waveform", 1 },
                                                               "Waveform",
                                                               juce::StringArray { "Sine", "Triangle", "Saw", "Reverse Saw", "Square", "S&H" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "polarity", 1 },
                                                               "Polarity",
                                                               juce::StringArray { "Bipolar", "Unipolar" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "syncDivision", 1 },
                                                               "Sync",
                                                               juce::StringArray { "Free", "1/8", "1/4", "1/2", "1 Bar", "2 Bar" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "retrig", 1 },
                                                               "Retrig",
                                                               juce::StringArray { "Off", "On" },
                                                               1));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "mode", 1 },
                                                               "Mode",
                                                               juce::StringArray { "Loop", "One Shot", "Ping Pong" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "running", 1 },
                                                             "Run / Stop", true));

    return { p.begin(), p.end() };
}

void LFOToolAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    resetLfo();
}

void LFOToolAudioProcessor::releaseResources()
{
}

bool LFOToolAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    const auto& in  = layouts.getMainInputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    if (! in.isDisabled() && in != out)
        return false;

    return true;
}

void LFOToolAudioProcessor::resetLfo()
{
    phase = 0.0;
    phaseDirection = 1;
    smoothedValue = 0.0f;
    sampleHoldValue = 0.0f;
    previousSHPhase = 0.0;
}

void LFOToolAudioProcessor::updateTransportInfo()
{
    bool playing = false;

    if (auto* currentPlayHead = getPlayHead())
    {
        if (auto pos = currentPlayHead->getPosition())
        {
            if (auto bpm = pos->getBpm())
                hostBpm.store (static_cast<float> (*bpm));

            playing = pos->getIsPlaying();
        }
    }

    const auto retrig = static_cast<int> (parameters.getRawParameterValue ("retrig")->load() + 0.5f);
    if (retrig == 1 && playing && ! lastTransportPlaying)
        resetLfo();

    lastTransportPlaying = playing;
}

float LFOToolAudioProcessor::getRateHzFromParameters() const
{
    const auto syncDivision = static_cast<int> (parameters.getRawParameterValue ("syncDivision")->load() + 0.5f);

    if (syncDivision == free)
        return parameters.getRawParameterValue ("rate")->load();

    const float bpm = juce::jmax (1.0f, hostBpm.load());
    const float beatsPerCycle = beatsPerCycleForDivision (syncDivision);
    return (bpm / 60.0f) / beatsPerCycle;
}

float LFOToolAudioProcessor::waveValue (int waveform, double phase01, uint32_t& rng, float& held, double& previousPhase)
{
    auto p = std::fmod (phase01, 1.0);
    if (p < 0.0)
        p += 1.0;

    switch (waveform)
    {
        case sine:
            return std::sin (static_cast<float> (p) * 2.0f * pi);
        case triangle:
            return p < 0.5 ? static_cast<float> (p * 4.0 - 1.0) : static_cast<float> (3.0 - p * 4.0);
        case saw:
            return static_cast<float> (p * 2.0 - 1.0);
        case reverseSaw:
            return static_cast<float> (1.0 - p * 2.0);
        case square:
            return p < 0.5 ? 1.0f : -1.0f;
        case sampleHold:
            if (previousPhase > 0.9 && p < 0.1)
                held = randomBipolar (rng);
            previousPhase = p;
            return held;
        default:
            return 0.0f;
    }
}

float LFOToolAudioProcessor::shapeValue (int waveform, float depth, float offset, int polarity, double phase01,
                                         uint32_t& rng, float& held, double& previousPhase)
{
    auto raw = waveValue (waveform, phase01, rng, held, previousPhase);
    auto v = polarity == unipolar ? (raw * 0.5f + 0.5f) * depth + offset
                                  : raw * depth + offset;
    return clamp1 (v);
}

float LFOToolAudioProcessor::renderSample (float rateHz, float depth, float offset, float smooth, float phaseDeg,
                                           int waveform, int polarity, int mode, bool running)
{
    if (! running)
    {
        smoothedValue += (0.0f - smoothedValue) * 0.02f;
        currentOutput.store (smoothedValue);
        currentPhase.store (static_cast<float> (phase));
        return smoothedValue;
    }

    const double inc = static_cast<double> (rateHz) / currentSampleRate;

    if (mode == pingPong)
    {
        phase += inc * static_cast<double> (phaseDirection);
        if (phase >= 1.0)
        {
            phase = 2.0 - phase;
            phaseDirection = -1;
        }
        else if (phase <= 0.0)
        {
            phase = -phase;
            phaseDirection = 1;
        }
    }
    else if (mode == oneShot)
    {
        phase += inc;
        if (phase > 1.0)
            phase = 1.0;
    }
    else
    {
        phase += inc;
        while (phase >= 1.0)
            phase -= 1.0;
    }

    const double displayPhase = phase + static_cast<double> (phaseDeg) / 360.0;
    const auto target = shapeValue (waveform, depth, offset, polarity, displayPhase, rngState, sampleHoldValue, previousSHPhase);
    const auto coefficient = juce::jlimit (0.001f, 1.0f, 1.0f - smooth);
    smoothedValue += (target - smoothedValue) * coefficient;

    currentOutput.store (smoothedValue);
    currentPhase.store (static_cast<float> (phase));
    return smoothedValue;
}

void LFOToolAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateTransportInfo();

    const auto rateHz   = getRateHzFromParameters();
    const auto depth    = parameters.getRawParameterValue ("depth")->load();
    const auto offset   = parameters.getRawParameterValue ("offset")->load();
    const auto smooth   = parameters.getRawParameterValue ("smooth")->load();
    const auto phaseDeg = parameters.getRawParameterValue ("phase")->load();
    const auto waveform = static_cast<int> (parameters.getRawParameterValue ("waveform")->load() + 0.5f);
    const auto polarity = static_cast<int> (parameters.getRawParameterValue ("polarity")->load() + 0.5f);
    const auto mode     = static_cast<int> (parameters.getRawParameterValue ("mode")->load() + 0.5f);
    const auto running  = parameters.getRawParameterValue ("running")->load() > 0.5f;

    const auto totalInputChannels  = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();

    for (int ch = totalInputChannels; ch < totalOutputChannels; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto lfo = renderSample (rateHz, depth, offset, smooth, phaseDeg, waveform, polarity, mode, running);

        for (int ch = 0; ch < totalOutputChannels; ++ch)
        {
            const auto in = ch < totalInputChannels ? buffer.getReadPointer (ch)[sample] : 0.0f;
            buffer.getWritePointer (ch)[sample] = in + lfo;
        }
    }
}

void LFOToolAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LFOToolAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* LFOToolAudioProcessor::createEditor()
{
    return new LFOToolAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LFOToolAudioProcessor();
}
