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

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider&) override;
};

class CurveEditor final : public juce::Component, private juce::Timer
{
public:
    explicit CurveEditor (LFOToolAudioProcessor&);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::Rectangle<float> getCurveArea() const;
    juce::Point<float> pointToPixel (juce::Point<float> point) const;
    juce::Point<float> pixelToPoint (juce::Point<float> pixel) const;
    int findPointNear (juce::Point<float> pixel) const;
    void makeCurveWaveformActive();

    LFOToolAudioProcessor& processor;
    int selectedPoint = -1;
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
    void setScaleFactor (float newScale) override;

private:
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureKnob (juce::Slider&, juce::Label&, const juce::String& name);
    void configureButton (juce::TextButton&, const juce::String& text);
    void configureCombo (juce::ComboBox&);
    void setChoiceParameter (const juce::String& parameterID, int choiceIndex);
    void setBoolParameter (const juce::String& parameterID, bool value);
    int getChoice (const juce::String& parameterID) const;
    bool getBool (const juce::String& parameterID) const;
    void updateButtonStates();
    void timerCallback() override;

    LFOToolAudioProcessor& processor;
    RetroLookAndFeel lookAndFeel;

    CurveEditor curveEditor;
    LevelMeter levelMeter;

    juce::Slider rateSlider, depthSlider, offsetSlider, smoothSlider, phaseSlider, cvOutSlider;
    juce::Label rateLabel, depthLabel, offsetLabel, smoothLabel, phaseValueLabel, bpmValueLabel, cvOutValueLabel;
    juce::Label rateNameLabel, depthNameLabel, offsetNameLabel, smoothNameLabel;
    juce::Label syncNameLabel, outputNameLabel, curveHintLabel;

    std::array<juce::TextButton, 7> waveformButtons;
    std::array<juce::TextButton, 3> curveModeButtons;
    juce::ComboBox syncCombo;
    juce::TextButton retrigOffButton, retrigOnButton;
    juce::TextButton bipolarButton, unipolarButton;
    juce::TextButton phaseLockOffButton, phaseLockOnButton;
    juce::TextButton outputAddButton, outputCVButton;
    juce::TextButton loopButton, oneShotButton, pingPongButton;
    juce::TextButton stopButton;

    std::unique_ptr<SliderAttachment> rateAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> offsetAttachment;
    std::unique_ptr<SliderAttachment> smoothAttachment;
    std::unique_ptr<SliderAttachment> phaseAttachment;
    std::unique_ptr<ComboBoxAttachment> syncAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOToolAudioProcessorEditor)
};
