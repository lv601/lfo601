#pragma once

#include <JuceHeader.h>
#include <atomic>

class LFOToolAudioProcessor final : public juce::AudioProcessor
{
public:
    enum Waveform
    {
        sine = 0,
        triangle,
        saw,
        reverseSaw,
        square,
        sampleHold
    };

    enum SyncDivision
    {
        free = 0,
        eighth,
        quarter,
        half,
        oneBar,
        twoBars
    };

    enum Polarity
    {
        bipolar = 0,
        unipolar
    };

    enum Mode
    {
        loop = 0,
        oneShot,
        pingPong
    };

    LFOToolAudioProcessor();
    ~LFOToolAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static float waveValue (int waveform, double phase01, uint32_t& rngState, float& sampleHoldValue, double& previousSHPhase);
    static float shapeValue (int waveform, float depth, float offset, int polarity, double phase01,
                             uint32_t& rngState, float& sampleHoldValue, double& previousSHPhase);

    float getCurrentOutput() const noexcept { return currentOutput.load(); }
    float getCurrentPhase()  const noexcept { return currentPhase.load(); }
    float getHostBpm()       const noexcept { return hostBpm.load(); }

    void resetLfo();

    juce::AudioProcessorValueTreeState parameters;

private:
    float renderSample (float rateHz, float depth, float offset, float smooth, float phaseDeg,
                        int waveform, int polarity, int mode, bool running);
    float getRateHzFromParameters() const;
    void updateTransportInfo();

    double currentSampleRate = 44100.0;
    double phase = 0.0;
    int phaseDirection = 1;
    float smoothedValue = 0.0f;
    float sampleHoldValue = 0.0f;
    double previousSHPhase = 0.0;
    uint32_t rngState = 0xDEADBEEF;
    bool lastTransportPlaying = false;

    std::atomic<float> currentOutput { 0.0f };
    std::atomic<float> currentPhase  { 0.0f };
    std::atomic<float> hostBpm       { 120.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOToolAudioProcessor)
};
