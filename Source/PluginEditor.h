#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RetroLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider&) override;
};

class WaveformView final : public juce::Component, private juce::Timer
{
public:
    explicit WaveformView (LFOToolAudioProcessor&);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    LFOToolAudioProcessor& processor;
};

class LevelMeter final : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeter (LFOToolAudioProcessor&);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    LFOToolAudioProcessor& processor;
};

class LFOToolAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit LFOToolAudioProcessorEditor (LFOToolAudioProcessor&);
    ~LFOToolAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void configureKnob (juce::Slider&, juce::Label&, const juce::String& name);
    void configureButton (juce::TextButton&, const juce::String& text);
    void setChoiceParameter (const juce::String& parameterID, int choiceIndex);
    void setBoolParameter (const juce::String& parameterID, bool value);
    int getChoice (const juce::String& parameterID) const;
    bool getBool (const juce::String& parameterID) const;
    void updateButtonStates();
    void timerCallback() override;

    LFOToolAudioProcessor& processor;
    RetroLookAndFeel lookAndFeel;

    WaveformView waveformView;
    LevelMeter levelMeter;

    juce::Slider rateSlider, depthSlider, offsetSlider, smoothSlider, phaseSlider, cvOutSlider;
    juce::Label rateLabel, depthLabel, offsetLabel, smoothLabel, phaseValueLabel, bpmValueLabel, cvOutValueLabel;
    juce::Label rateNameLabel, depthNameLabel, offsetNameLabel, smoothNameLabel;

    std::array<juce::TextButton, 6> waveformButtons;
    std::array<juce::TextButton, 6> syncButtons;
    juce::TextButton retrigOffButton, retrigOnButton;
    juce::TextButton bipolarButton, unipolarButton;
    juce::TextButton loopButton, oneShotButton, pingPongButton;
    juce::TextButton stopButton;

    std::unique_ptr<SliderAttachment> rateAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> offsetAttachment;
    std::unique_ptr<SliderAttachment> smoothAttachment;
    std::unique_ptr<SliderAttachment> phaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOToolAudioProcessorEditor)
};
