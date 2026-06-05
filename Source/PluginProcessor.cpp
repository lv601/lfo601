#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;

    static float clampBipolar (float v) noexcept
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
            case LFOToolAudioProcessor::sixteenth:        return 0.25f;
            case LFOToolAudioProcessor::sixteenthTriplet: return 1.0f / 6.0f;
            case LFOToolAudioProcessor::sixteenthDotted:  return 0.375f;
            case LFOToolAudioProcessor::eighth:           return 0.5f;
            case LFOToolAudioProcessor::eighthTriplet:    return 1.0f / 3.0f;
            case LFOToolAudioProcessor::eighthDotted:     return 0.75f;
            case LFOToolAudioProcessor::quarter:          return 1.0f;
            case LFOToolAudioProcessor::quarterTriplet:   return 2.0f / 3.0f;
            case LFOToolAudioProcessor::quarterDotted:    return 1.5f;
            case LFOToolAudioProcessor::half:             return 2.0f;
            case LFOToolAudioProcessor::oneBar:           return 4.0f;
            case LFOToolAudioProcessor::twoBars:          return 8.0f;
            case LFOToolAudioProcessor::free:
            default:                                      return 1.0f;
        }
    }

    static float smoothstep (float x) noexcept
    {
        x = juce::jlimit (0.0f, 1.0f, x);
        return x * x * (3.0f - 2.0f * x);
    }
}

LFOToolAudioProcessor::LFOToolAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Stereo In",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("LFO CV Out", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    resetDefaultCurve();
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
                                                               juce::StringArray { "Sine", "Triangle", "Saw", "Reverse Saw", "Square", "S&H", "Curve" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "polarity", 1 },
                                                               "Polarity",
                                                               juce::StringArray { "Bipolar", "Unipolar" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "syncDivision", 1 },
                                                               "Sync",
                                                               juce::StringArray { "Free", "1/16", "1/16T", "1/16D", "1/8", "1/8T", "1/8D", "1/4", "1/4T", "1/4D", "1/2", "1 Bar", "2 Bar" },
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
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "curveMode", 1 },
                                                               "Curve Mode",
                                                               juce::StringArray { "Smooth", "Step", "Bezier" },
                                                               0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "outputMode", 1 },
                                                               "Output Mode",
                                                               juce::StringArray { "Add LFO to Input", "CV Only" },
                                                               1));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "phaseLock", 1 },
                                                               "Phase Lock",
                                                               juce::StringArray { "Off", "On" },
                                                               1));

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
    lastPpqPosition = -1.0;
}

void LFOToolAudioProcessor::updateTransportInfo()
{
    bool playing = false;
    double ppq = -1.0;

    if (auto* currentPlayHead = getPlayHead())
    {
        if (auto pos = currentPlayHead->getPosition())
        {
            if (auto bpm = pos->getBpm())
                hostBpm.store (static_cast<float> (*bpm));

            playing = pos->getIsPlaying();

            if (auto currentPpq = pos->getPpqPosition())
                ppq = *currentPpq;
        }
    }

    const auto syncDivision = static_cast<int> (parameters.getRawParameterValue ("syncDivision")->load() + 0.5f);
    const auto retrig = static_cast<int> (parameters.getRawParameterValue ("retrig")->load() + 0.5f);
    const auto phaseLock = static_cast<int> (parameters.getRawParameterValue ("phaseLock")->load() + 0.5f);

    if (playing && ppq >= 0.0 && syncDivision != free)
    {
        const auto beatsPerCycle = static_cast<double> (beatsPerCycleForDivision (syncDivision));
        const auto syncedPhase = std::fmod (ppq / beatsPerCycle, 1.0);

        if (phaseLock == 1)
        {
            phase = syncedPhase < 0.0 ? syncedPhase + 1.0 : syncedPhase;
            phaseDirection = (static_cast<int> (std::floor (ppq / beatsPerCycle)) % 2 == 0) ? 1 : -1;
        }
        else if (retrig == 1 && (playing && ! lastTransportPlaying))
        {
            phase = syncedPhase < 0.0 ? syncedPhase + 1.0 : syncedPhase;
        }
        else if (retrig == 1 && lastPpqPosition >= 0.0 && std::abs (ppq - lastPpqPosition) > 0.25)
        {
            // Transport jump/relocation in the host: snap LFO phase to musical position.
            phase = syncedPhase < 0.0 ? syncedPhase + 1.0 : syncedPhase;
        }
    }
    else if (retrig == 1 && playing && ! lastTransportPlaying)
    {
        resetLfo();
    }

    lastTransportPlaying = playing;
    lastPpqPosition = ppq;
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

    return polarity == unipolar ? juce::jlimit (0.0f, 1.0f, v) : clampBipolar (v);
}

float LFOToolAudioProcessor::applyDepthOffsetPolarity (float rawBipolar, float depth, float offset, int polarity) const noexcept
{
    const auto v = polarity == unipolar ? (rawBipolar * 0.5f + 0.5f) * depth + offset
                                        : rawBipolar * depth + offset;
    return polarity == unipolar ? juce::jlimit (0.0f, 1.0f, v) : clampBipolar (v);
}

float LFOToolAudioProcessor::renderSample (float rateHz, float depth, float offset, float smooth, float phaseDeg,
                                           int waveform, int polarity, int mode, int curveMode, bool running)
{
    if (! running)
    {
        smoothedValue += (0.0f - smoothedValue) * 0.02f;
        currentOutput.store (smoothedValue);
        currentPhase.store (static_cast<float> (phase));
        return smoothedValue;
    }

    const double inc = static_cast<double> (rateHz) / currentSampleRate;
    const auto phaseLock = static_cast<int> (parameters.getRawParameterValue ("phaseLock")->load() + 0.5f);
    const auto syncDivision = static_cast<int> (parameters.getRawParameterValue ("syncDivision")->load() + 0.5f);
    const bool hostPhaseLocked = phaseLock == 1 && syncDivision != free && lastTransportPlaying;

    if (! hostPhaseLocked)
    {
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
    }

    const double displayPhase = phase + static_cast<double> (phaseDeg) / 360.0;
    float target = 0.0f;

    if (waveform == customCurve)
    {
        const auto y = evaluateUserCurve (displayPhase, curveMode); // 0..1
        target = applyDepthOffsetPolarity (y * 2.0f - 1.0f, depth, offset, polarity);
    }
    else
    {
        target = shapeValue (waveform, depth, offset, polarity, displayPhase, rngState, sampleHoldValue, previousSHPhase);
    }

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

    const auto rateHz     = getRateHzFromParameters();
    const auto depth      = parameters.getRawParameterValue ("depth")->load();
    const auto offset     = parameters.getRawParameterValue ("offset")->load();
    const auto smooth     = parameters.getRawParameterValue ("smooth")->load();
    const auto phaseDeg   = parameters.getRawParameterValue ("phase")->load();
    const auto waveform   = static_cast<int> (parameters.getRawParameterValue ("waveform")->load() + 0.5f);
    const auto polarity   = static_cast<int> (parameters.getRawParameterValue ("polarity")->load() + 0.5f);
    const auto mode       = static_cast<int> (parameters.getRawParameterValue ("mode")->load() + 0.5f);
    const auto curveMode  = static_cast<int> (parameters.getRawParameterValue ("curveMode")->load() + 0.5f);
    const auto outputMode = static_cast<int> (parameters.getRawParameterValue ("outputMode")->load() + 0.5f);
    const auto running    = parameters.getRawParameterValue ("running")->load() > 0.5f;

    const auto totalInputChannels  = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();

    for (int ch = totalInputChannels; ch < totalOutputChannels; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto lfo = renderSample (rateHz, depth, offset, smooth, phaseDeg, waveform, polarity, mode, curveMode, running);

        for (int ch = 0; ch < totalOutputChannels; ++ch)
        {
            const auto in = ch < totalInputChannels ? buffer.getReadPointer (ch)[sample] : 0.0f;
            buffer.getWritePointer (ch)[sample] = outputMode == outputAdd ? in + lfo : lfo;
        }
    }
}

int LFOToolAudioProcessor::getCurvePointCount() const noexcept
{
    return juce::jlimit (2, maxCurvePoints, curvePointCount.load());
}

juce::Point<float> LFOToolAudioProcessor::getCurvePoint (int index) const noexcept
{
    const auto i = juce::jlimit (0, getCurvePointCount() - 1, index);
    return { curveX[static_cast<size_t> (i)].load(), curveY[static_cast<size_t> (i)].load() };
}

void LFOToolAudioProcessor::sortCurvePoints()
{
    const auto count = getCurvePointCount();
    std::array<juce::Point<float>, maxCurvePoints> points;

    for (int i = 0; i < count; ++i)
        points[static_cast<size_t> (i)] = getCurvePoint (i);

    std::sort (points.begin(), points.begin() + count,
               [] (juce::Point<float> a, juce::Point<float> b) { return a.x < b.x; });

    for (int i = 0; i < count; ++i)
    {
        curveX[static_cast<size_t> (i)].store (points[static_cast<size_t> (i)].x);
        curveY[static_cast<size_t> (i)].store (points[static_cast<size_t> (i)].y);
    }
}

void LFOToolAudioProcessor::setCurvePoint (int index, float x, float y)
{
    const auto i = juce::jlimit (0, getCurvePointCount() - 1, index);
    curveX[static_cast<size_t> (i)].store (juce::jlimit (0.0f, 1.0f, x));
    curveY[static_cast<size_t> (i)].store (juce::jlimit (0.0f, 1.0f, y));
    sortCurvePoints();
    saveCurveToState();
}

int LFOToolAudioProcessor::addCurvePoint (float x, float y)
{
    auto count = getCurvePointCount();
    if (count >= maxCurvePoints)
        return count - 1;

    curveX[static_cast<size_t> (count)].store (juce::jlimit (0.0f, 1.0f, x));
    curveY[static_cast<size_t> (count)].store (juce::jlimit (0.0f, 1.0f, y));
    curvePointCount.store (count + 1);
    sortCurvePoints();
    saveCurveToState();

    // Find the newly inserted point after sorting.
    const auto targetX = juce::jlimit (0.0f, 1.0f, x);
    int nearest = 0;
    float best = 2.0f;
    for (int i = 0; i < getCurvePointCount(); ++i)
    {
        const auto d = std::abs (curveX[static_cast<size_t> (i)].load() - targetX);
        if (d < best)
        {
            best = d;
            nearest = i;
        }
    }
    return nearest;
}

void LFOToolAudioProcessor::removeCurvePoint (int index)
{
    auto count = getCurvePointCount();
    if (count <= 2)
        return;

    const auto idx = juce::jlimit (0, count - 1, index);
    for (int i = idx; i < count - 1; ++i)
    {
        curveX[static_cast<size_t> (i)].store (curveX[static_cast<size_t> (i + 1)].load());
        curveY[static_cast<size_t> (i)].store (curveY[static_cast<size_t> (i + 1)].load());
    }

    curvePointCount.store (count - 1);
    sortCurvePoints();
    saveCurveToState();
}

void LFOToolAudioProcessor::resetDefaultCurve()
{
    curvePointCount.store (4);
    curveX[0].store (0.0f);  curveY[0].store (0.5f);
    curveX[1].store (0.25f); curveY[1].store (1.0f);
    curveX[2].store (0.75f); curveY[2].store (0.0f);
    curveX[3].store (1.0f);  curveY[3].store (0.5f);

    for (int i = 4; i < maxCurvePoints; ++i)
    {
        curveX[static_cast<size_t> (i)].store (0.0f);
        curveY[static_cast<size_t> (i)].store (0.5f);
    }

    saveCurveToState();
}

float LFOToolAudioProcessor::evaluateUserCurve (double phase01, int curveMode) const noexcept
{
    auto p = std::fmod (phase01, 1.0);
    if (p < 0.0)
        p += 1.0;

    const auto count = getCurvePointCount();

    if (count <= 1)
        return 0.5f;

    std::array<float, maxCurvePoints> xs {};
    std::array<float, maxCurvePoints> ys {};

    for (int i = 0; i < count; ++i)
    {
        xs[static_cast<size_t> (i)] = curveX[static_cast<size_t> (i)].load();
        ys[static_cast<size_t> (i)] = curveY[static_cast<size_t> (i)].load();
    }

    if (p <= xs[0])
        return ys[0];

    for (int i = 0; i < count - 1; ++i)
    {
        const auto x0 = xs[static_cast<size_t> (i)];
        const auto x1 = xs[static_cast<size_t> (i + 1)];
        if (p >= x0 && p <= x1)
        {
            const auto width = juce::jmax (0.0001f, x1 - x0);
            auto t = static_cast<float> ((p - x0) / width);

            if (curveMode == curveStep)
                return ys[static_cast<size_t> (i)];

            if (curveMode == curveBezier)
                t = smoothstep (t);

            return juce::jmap (t, ys[static_cast<size_t> (i)], ys[static_cast<size_t> (i + 1)]);
        }
    }

    return ys[static_cast<size_t> (count - 1)];
}

void LFOToolAudioProcessor::saveCurveToState()
{
    parameters.state.setProperty ("curvePointCount", getCurvePointCount(), nullptr);
    for (int i = 0; i < maxCurvePoints; ++i)
    {
        parameters.state.setProperty (juce::String ("curveX") + juce::String (i), curveX[static_cast<size_t> (i)].load(), nullptr);
        parameters.state.setProperty (juce::String ("curveY") + juce::String (i), curveY[static_cast<size_t> (i)].load(), nullptr);
    }
}

void LFOToolAudioProcessor::loadCurveFromState()
{
    if (! parameters.state.hasProperty ("curvePointCount"))
    {
        resetDefaultCurve();
        return;
    }

    const int count = juce::jlimit (2, maxCurvePoints, static_cast<int> (parameters.state.getProperty ("curvePointCount")));
    curvePointCount.store (count);

    for (int i = 0; i < maxCurvePoints; ++i)
    {
        const auto defaultX = i == 0 ? 0.0f : (i == 1 ? 0.25f : (i == 2 ? 0.75f : (i == 3 ? 1.0f : 0.0f)));
        const auto defaultY = i == 0 ? 0.5f : (i == 1 ? 1.0f  : (i == 2 ? 0.0f  : 0.5f));
        curveX[static_cast<size_t> (i)].store (static_cast<float> (parameters.state.getProperty (juce::String ("curveX") + juce::String (i), defaultX)));
        curveY[static_cast<size_t> (i)].store (static_cast<float> (parameters.state.getProperty (juce::String ("curveY") + juce::String (i), defaultY)));
    }

    sortCurvePoints();
}

void LFOToolAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    saveCurveToState();
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LFOToolAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
    {
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        loadCurveFromState();
    }
}

juce::AudioProcessorEditor* LFOToolAudioProcessor::createEditor()
{
    return new LFOToolAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LFOToolAudioProcessor();
}
