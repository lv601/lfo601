#include "PluginEditor.h"

namespace
{
    const auto bg        = juce::Colour (0xff111116);
    const auto panel     = juce::Colour (0xff1c1c22);
    const auto panel2    = juce::Colour (0xff0d0d12);
    const auto border    = juce::Colour (0xff3a3942);
    const auto grid      = juce::Colour (0xff24242c);
    const auto yellow    = juce::Colour (0xffffdd35);
    const auto mint      = juce::Colour (0xff49e6b2);
    const auto mutedText = juce::Colour (0xff8d8ca7);
    const auto whiteText = juce::Colour (0xfff7f7ff);

    juce::Font smallFont()
    {
        return juce::Font (juce::FontOptions ("Courier New", 10.0f, juce::Font::plain));
    }

    juce::Font buttonFont()
    {
        return juce::Font (juce::FontOptions ("Courier New", 14.0f, juce::Font::bold));
    }

    juce::Font titleFont()
    {
        return juce::Font (juce::FontOptions ("Courier New", 13.0f, juce::Font::bold));
    }

    void drawTinyLabel (juce::Graphics& g, const juce::String& text, int x, int y, int w = 90)
    {
        g.setColour (mutedText);
        g.setFont (smallFont());
        g.drawText (text.toUpperCase(), x, y, w, 14, juce::Justification::left);
    }

    float normaliseOutput (float v) noexcept
    {
        return juce::jlimit (0.0f, 1.0f, (v + 1.0f) * 0.5f);
    }
}

//==============================================================================
RetroLookAndFeel::RetroLookAndFeel()
{
    setColour (juce::Slider::thumbColourId, yellow);
    setColour (juce::Slider::rotarySliderFillColourId, yellow);
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff30303a));
    setColour (juce::Slider::textBoxTextColourId, yellow);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::TextButton::textColourOffId, whiteText);
    setColour (juce::TextButton::textColourOnId, whiteText);
}

void RetroLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider&)
{
    auto r = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                     static_cast<float> (width), static_cast<float> (height)).reduced (7.0f);
    r.setHeight (juce::jmin (r.getHeight(), r.getWidth()));
    r.setWidth  (r.getHeight());
    r.setCentre (static_cast<float> (x) + width * 0.5f, static_cast<float> (y) + r.getHeight() * 0.5f + 4.0f);

    const auto radius = r.getWidth() * 0.5f;
    const auto centre = r.getCentre();
    const auto lineW = 4.5f;
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path outline;
    outline.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff30303a));
    g.strokePath (outline, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (yellow);
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colour (0xff171720));
    g.fillEllipse (r.reduced (8.0f));
    g.setColour (juce::Colour (0xff2b2b34));
    g.drawEllipse (r.reduced (8.0f), 2.0f);

    auto dot = centre + juce::Point<float> (std::cos (angle - juce::MathConstants<float>::halfPi) * (radius - 4.0f),
                                           std::sin (angle - juce::MathConstants<float>::halfPi) * (radius - 4.0f));
    g.setColour (yellow);
    g.fillEllipse (dot.x - 2.7f, dot.y - 2.7f, 5.4f, 5.4f);
}

void RetroLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                             bool highlighted, bool down)
{
    auto area = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();

    g.setColour (on ? juce::Colour (0xff22232a) : panel);
    if (down)
        g.setColour (juce::Colour (0xff2b2c34));
    else if (highlighted)
        g.setColour (juce::Colour (0xff24252d));

    g.fillRoundedRectangle (area, 7.0f);
    g.setColour (on ? yellow : juce::Colour (0xff5a5965));
    g.drawRoundedRectangle (area, 7.0f, on ? 1.4f : 1.0f);
}

void RetroLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                       bool, bool)
{
    auto area = button.getLocalBounds().reduced (3, 2);
    const auto maxHeight = juce::jmax (9.0f, static_cast<float> (button.getHeight()) * 0.34f);
    g.setFont (buttonFont().withHeight (juce::jmin (13.0f, maxHeight)));
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId));
    g.drawFittedText (button.getButtonText(), area, juce::Justification::centred, 2, 0.85f);
}

void RetroLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         const juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto track = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y) + height * 0.5f - 2.0f,
                                         static_cast<float> (width), 4.0f);

    g.setColour (juce::Colour (0xff30303a));
    g.fillRoundedRectangle (track, 2.0f);

    if (slider.getName() == "cvOut")
    {
        auto n = normaliseOutput (static_cast<float> (slider.getValue()));
        auto fill = track.withWidth (track.getWidth() * n);
        g.setColour (mint);
        g.fillRoundedRectangle (fill, 2.0f);
        return;
    }

    g.setColour (juce::Colour (0xff55555f));
    g.drawRoundedRectangle (track, 2.0f, 1.0f);
    g.setColour (juce::Colour (0xff393942));
    g.fillEllipse (sliderPos - 9.0f, track.getCentreY() - 9.0f, 18.0f, 18.0f);
    g.setColour (juce::Colour (0xff66666f));
    g.drawEllipse (sliderPos - 9.0f, track.getCentreY() - 9.0f, 18.0f, 18.0f, 1.0f);
}

//==============================================================================
WaveformView::WaveformView (LFOToolAudioProcessor& p) : processor (p)
{
    startTimerHz (30);
}

void WaveformView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (panel2);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (border.withAlpha (0.85f));
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);

    auto area = bounds.reduced (1.0f);
    g.setColour (grid.withAlpha (0.7f));
    for (int i = 1; i < 4; ++i)
    {
        const auto x = area.getX() + area.getWidth() * static_cast<float> (i) / 4.0f;
        g.drawVerticalLine (juce::roundToInt (x), area.getY(), area.getBottom());
    }
    g.drawHorizontalLine (juce::roundToInt (area.getCentreY()), area.getX(), area.getRight());

    g.setColour (mutedText);
    g.setFont (smallFont());
    g.drawText ("OUTPUT", 10, 2, 90, 16, juce::Justification::left);

    const int waveform = static_cast<int> (processor.parameters.getRawParameterValue ("waveform")->load() + 0.5f);
    const int polarity = static_cast<int> (processor.parameters.getRawParameterValue ("polarity")->load() + 0.5f);
    const auto depth = processor.parameters.getRawParameterValue ("depth")->load();
    const auto offset = processor.parameters.getRawParameterValue ("offset")->load();
    const auto phaseDeg = processor.parameters.getRawParameterValue ("phase")->load();

    constexpr double cycles = 3.0;
    juce::Path path;
    uint32_t rng = 0xCAFE1234;
    float held = 0.0f;
    double previous = 0.0;

    for (int i = 0; i < juce::roundToInt (area.getWidth()); ++i)
    {
        const auto xf = static_cast<float> (i) / juce::jmax (1.0f, area.getWidth() - 1.0f);
        const auto phase = static_cast<double> (xf) * cycles + static_cast<double> (phaseDeg) / 360.0;
        const auto v = LFOToolAudioProcessor::shapeValue (waveform, depth, offset, polarity, phase, rng, held, previous);
        const auto x = area.getX() + xf * area.getWidth();
        const auto y = area.getCentreY() - v * area.getHeight() * 0.43f;

        if (i == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    g.setColour (yellow.withAlpha (0.22f));
    g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (yellow);
    g.strokePath (path, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto ph = processor.getCurrentPhase();
    const auto markerX = area.getX() + std::fmod (static_cast<double> (ph), 1.0) / cycles * area.getWidth();
    const auto markerY = area.getCentreY() - processor.getCurrentOutput() * area.getHeight() * 0.43f;
    g.setColour (mint);
    g.fillEllipse (static_cast<float> (markerX) - 4.5f, markerY - 4.5f, 9.0f, 9.0f);
}

void WaveformView::timerCallback()
{
    repaint();
}

//==============================================================================
LevelMeter::LevelMeter (LFOToolAudioProcessor& p) : processor (p)
{
    startTimerHz (20);
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().reduced (1);
    const int bars = 5;
    const int gap = 5;
    const int barW = (r.getWidth() - gap * (bars - 1)) / bars;
    const auto level = std::abs (processor.getCurrentOutput());

    for (int i = 0; i < bars; ++i)
    {
        auto b = juce::Rectangle<int> (r.getX() + i * (barW + gap), r.getY(), barW, r.getHeight()).toFloat();
        const bool lit = level >= static_cast<float> (i) / static_cast<float> (bars);
        g.setColour (lit ? mint.withMultipliedAlpha (1.0f - i * 0.12f) : juce::Colour (0xff20202a));
        g.fillRoundedRectangle (b, 2.0f);
    }
}

void LevelMeter::timerCallback()
{
    repaint();
}

//==============================================================================
LFOToolAudioProcessorEditor::LFOToolAudioProcessorEditor (LFOToolAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), waveformView (p), levelMeter (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (720, 508);

    addAndMakeVisible (waveformView);
    addAndMakeVisible (levelMeter);

    configureKnob (rateSlider, rateNameLabel, "RATE");
    configureKnob (depthSlider, depthNameLabel, "DEPTH");
    configureKnob (offsetSlider, offsetNameLabel, "OFFSET");
    configureKnob (smoothSlider, smoothNameLabel, "SMOOTH");

    rateSlider.textFromValueFunction = [] (double v) { return juce::String (v, 2) + " Hz"; };
    depthSlider.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + "%"; };
    offsetSlider.textFromValueFunction = [] (double v) { return juce::String (v, 2); };
    smoothSlider.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + "%"; };

    auto setupValueLabel = [] (juce::Label& label)
    {
        label.setColour (juce::Label::textColourId, yellow);
        label.setFont (smallFont());
        label.setJustificationType (juce::Justification::centred);
    };

    for (auto* label : { &rateLabel, &depthLabel, &offsetLabel, &smoothLabel })
    {
        setupValueLabel (*label);
        addAndMakeVisible (*label);
    }

    phaseSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    phaseSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (phaseSlider);

    phaseValueLabel.setColour (juce::Label::textColourId, yellow);
    phaseValueLabel.setFont (smallFont());
    phaseValueLabel.setJustificationType (juce::Justification::right);
    addAndMakeVisible (phaseValueLabel);

    cvOutSlider.setName ("cvOut");
    cvOutSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    cvOutSlider.setRange (-1.0, 1.0, 0.001);
    cvOutSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    cvOutSlider.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (cvOutSlider);

    cvOutValueLabel.setColour (juce::Label::textColourId, mint);
    cvOutValueLabel.setFont (juce::Font (juce::FontOptions ("Courier New", 18.0f, juce::Font::bold)));
    cvOutValueLabel.setJustificationType (juce::Justification::right);
    addAndMakeVisible (cvOutValueLabel);

    const juce::StringArray waveTexts { "SIN", "TRI", "SAW", "REV", "SQR", "S&H" };
    for (size_t i = 0; i < waveformButtons.size(); ++i)
    {
        configureButton (waveformButtons[i], waveTexts[static_cast<int> (i)]);
        waveformButtons[i].onClick = [this, i] { setChoiceParameter ("waveform", static_cast<int> (i)); };
    }

    const juce::StringArray syncTexts { "FREE", "1/8", "1/4", "1/2", "1\nBAR", "2\nBAR" };
    for (size_t i = 0; i < syncButtons.size(); ++i)
    {
        configureButton (syncButtons[i], syncTexts[static_cast<int> (i)]);
        syncButtons[i].onClick = [this, i] { setChoiceParameter ("syncDivision", static_cast<int> (i)); };
    }

    configureButton (retrigOffButton, "OFF");
    configureButton (retrigOnButton, "ON");
    retrigOffButton.onClick = [this] { setChoiceParameter ("retrig", 0); };
    retrigOnButton.onClick  = [this] { setChoiceParameter ("retrig", 1); };

    configureButton (bipolarButton, "BIPOLAR");
    configureButton (unipolarButton, "UNIPOLAR");
    bipolarButton.onClick  = [this] { setChoiceParameter ("polarity", 0); };
    unipolarButton.onClick = [this] { setChoiceParameter ("polarity", 1); };

    configureButton (loopButton, "LOOP");
    configureButton (oneShotButton, "ONE SHOT");
    configureButton (pingPongButton, "PING PONG");
    loopButton.onClick     = [this] { setChoiceParameter ("mode", 0); processor.resetLfo(); };
    oneShotButton.onClick  = [this] { setChoiceParameter ("mode", 1); processor.resetLfo(); };
    pingPongButton.onClick = [this] { setChoiceParameter ("mode", 2); processor.resetLfo(); };

    configureButton (stopButton, "STOP");
    stopButton.onClick = [this]
    {
        const auto running = getBool ("running");
        setBoolParameter ("running", ! running);
        if (! running)
            processor.resetLfo();
    };

    bpmValueLabel.setColour (juce::Label::backgroundColourId, panel);
    bpmValueLabel.setColour (juce::Label::outlineColourId, border);
    bpmValueLabel.setColour (juce::Label::textColourId, yellow);
    bpmValueLabel.setFont (buttonFont());
    bpmValueLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bpmValueLabel);

    rateAttachment   = std::make_unique<SliderAttachment> (processor.parameters, "rate",   rateSlider);
    depthAttachment  = std::make_unique<SliderAttachment> (processor.parameters, "depth",  depthSlider);
    offsetAttachment = std::make_unique<SliderAttachment> (processor.parameters, "offset", offsetSlider);
    smoothAttachment = std::make_unique<SliderAttachment> (processor.parameters, "smooth", smoothSlider);
    phaseAttachment  = std::make_unique<SliderAttachment> (processor.parameters, "phase",  phaseSlider);

    startTimerHz (30);
    updateButtonStates();
    resized();
}

LFOToolAudioProcessorEditor::~LFOToolAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void LFOToolAudioProcessorEditor::setScaleFactor (float newScale)
{
    // FL Studio can report a host scale factor even though the wrapper already
    // allocates the editor at the requested pixel size.  The default JUCE
    // behaviour applies an additional transform, which makes controls appear
    // oversized and clipped in the FL wrapper.  Keep the editor at 1:1 pixels.
    juce::ignoreUnused (newScale);
    setTransform (juce::AffineTransform());
}

void LFOToolAudioProcessorEditor::configureKnob (juce::Slider& s, juce::Label& label, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setRotaryParameters (juce::MathConstants<float>::pi * 1.24f,
                           juce::MathConstants<float>::pi * 2.76f,
                           true);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (s);

    label.setText (name, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, mutedText);
    label.setFont (smallFont());
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void LFOToolAudioProcessorEditor::configureButton (juce::TextButton& b, const juce::String& text)
{
    b.setButtonText (text);
    b.setClickingTogglesState (false);
    b.setColour (juce::TextButton::textColourOffId, whiteText);
    b.setColour (juce::TextButton::textColourOnId, whiteText);
    b.setConnectedEdges (0);
    b.setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (b);
}

void LFOToolAudioProcessorEditor::setChoiceParameter (const juce::String& parameterID, int choiceIndex)
{
    if (auto* param = processor.parameters.getParameter (parameterID))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (choiceIndex)));
        param->endChangeGesture();
    }
}

void LFOToolAudioProcessorEditor::setBoolParameter (const juce::String& parameterID, bool value)
{
    if (auto* param = processor.parameters.getParameter (parameterID))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (value ? 1.0f : 0.0f);
        param->endChangeGesture();
    }
}

int LFOToolAudioProcessorEditor::getChoice (const juce::String& parameterID) const
{
    if (auto* v = processor.parameters.getRawParameterValue (parameterID))
        return static_cast<int> (v->load() + 0.5f);
    return 0;
}

bool LFOToolAudioProcessorEditor::getBool (const juce::String& parameterID) const
{
    if (auto* v = processor.parameters.getRawParameterValue (parameterID))
        return v->load() > 0.5f;
    return false;
}

void LFOToolAudioProcessorEditor::updateButtonStates()
{
    const auto wave = getChoice ("waveform");
    for (size_t i = 0; i < waveformButtons.size(); ++i)
        waveformButtons[i].setToggleState (static_cast<int> (i) == wave, juce::dontSendNotification);

    const auto sync = getChoice ("syncDivision");
    for (size_t i = 0; i < syncButtons.size(); ++i)
        syncButtons[i].setToggleState (static_cast<int> (i) == sync, juce::dontSendNotification);

    const auto retrig = getChoice ("retrig");
    retrigOffButton.setToggleState (retrig == 0, juce::dontSendNotification);
    retrigOnButton.setToggleState  (retrig == 1, juce::dontSendNotification);

    const auto polarity = getChoice ("polarity");
    bipolarButton.setToggleState  (polarity == 0, juce::dontSendNotification);
    unipolarButton.setToggleState (polarity == 1, juce::dontSendNotification);

    const auto mode = getChoice ("mode");
    loopButton.setToggleState     (mode == 0, juce::dontSendNotification);
    oneShotButton.setToggleState  (mode == 1, juce::dontSendNotification);
    pingPongButton.setToggleState (mode == 2, juce::dontSendNotification);

    const auto running = getBool ("running");
    stopButton.setButtonText (running ? "STOP" : "START");
    stopButton.setToggleState (! running, juce::dontSendNotification);
}

void LFOToolAudioProcessorEditor::timerCallback()
{
    updateButtonStates();

    const auto phase = processor.parameters.getRawParameterValue ("phase")->load();
    phaseValueLabel.setText (juce::String (juce::roundToInt (phase)) + " deg", juce::dontSendNotification);

    rateLabel.setText   (juce::String (rateSlider.getValue(), 2) + " Hz", juce::dontSendNotification);
    depthLabel.setText  (juce::String (juce::roundToInt (depthSlider.getValue() * 100.0)) + "%", juce::dontSendNotification);
    offsetLabel.setText (juce::String (offsetSlider.getValue(), 2), juce::dontSendNotification);
    smoothLabel.setText (juce::String (juce::roundToInt (smoothSlider.getValue() * 100.0)) + "%", juce::dontSendNotification);

    bpmValueLabel.setText (juce::String (juce::roundToInt (processor.getHostBpm())), juce::dontSendNotification);

    const auto cv = processor.getCurrentOutput();
    cvOutSlider.setValue (cv, juce::dontSendNotification);
    cvOutValueLabel.setText (juce::String (cv, 2), juce::dontSendNotification);
}

void LFOToolAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a0d));

    auto root = getLocalBounds().reduced (0).toFloat();
    g.setColour (bg);
    g.fillRoundedRectangle (root.reduced (10.0f), 8.0f);
    g.setColour (border);
    g.drawRoundedRectangle (root.reduced (10.5f), 8.0f, 1.0f);

    g.setColour (panel.withAlpha (0.7f));
    g.fillRect (juce::Rectangle<int> (getWidth() - 190, 39, 180, getHeight() - 49));
    g.setColour (border.withAlpha (0.8f));
    g.drawVerticalLine (getWidth() - 190, 39.0f, static_cast<float> (getHeight() - 10));
    g.drawHorizontalLine (39, 10.0f, static_cast<float> (getWidth() - 10));

    g.setFont (titleFont());
    g.setColour (yellow);
    g.drawText ("LFO TOOL", 27, 17, 130, 16, juce::Justification::left);
    g.setColour (mutedText);
    g.drawText ("CV MOD UNIT", getWidth() - 117, 17, 100, 16, juce::Justification::right);

    drawTinyLabel (g, "SHAPE", 27, 203);
    drawTinyLabel (g, "PHASE", 27, 336);
    drawTinyLabel (g, "SYNC", 27, 360);
    drawTinyLabel (g, "CV OUT", 27, 439);

    const int rx = getWidth() - 178;
    drawTinyLabel (g, "LEVEL", rx, 51);
    drawTinyLabel (g, "RETRIG", rx, 112);
    drawTinyLabel (g, "POLARITY", rx, 184);
    drawTinyLabel (g, "MODE", rx, 254);
    drawTinyLabel (g, "BPM", rx, 402);
}

void LFOToolAudioProcessorEditor::resized()
{
    const int rightX = getWidth() - 190;
    waveformView.setBounds (27, 51, rightX - 37, 132);

    const int waveY = 195;
    const int waveButtonW = 44;
    const int waveButtonH = 32;
    const int waveStartX = 73;
    for (size_t i = 0; i < waveformButtons.size(); ++i)
        waveformButtons[i].setBounds (waveStartX + static_cast<int> (i) * 50, waveY, waveButtonW, waveButtonH);

    const int knobY = 238;
    const int knobSize = 62;
    const std::array<int, 4> centres { 90, 216, 342, 468 };
    rateSlider.setBounds   (centres[0] - knobSize / 2, knobY, knobSize, 78);
    depthSlider.setBounds  (centres[1] - knobSize / 2, knobY, knobSize, 78);
    offsetSlider.setBounds (centres[2] - knobSize / 2, knobY, knobSize, 78);
    smoothSlider.setBounds (centres[3] - knobSize / 2, knobY, knobSize, 78);

    rateNameLabel.setBounds   (centres[0] - 36, 294, 72, 14);
    depthNameLabel.setBounds  (centres[1] - 36, 294, 72, 14);
    offsetNameLabel.setBounds (centres[2] - 36, 294, 72, 14);
    smoothNameLabel.setBounds (centres[3] - 36, 294, 72, 14);

    rateLabel.setBounds   (centres[0] - 42, 310, 84, 14);
    depthLabel.setBounds  (centres[1] - 42, 310, 84, 14);
    offsetLabel.setBounds (centres[2] - 42, 310, 84, 14);
    smoothLabel.setBounds (centres[3] - 42, 310, 84, 14);

    phaseSlider.setBounds (78, 332, rightX - 115, 28);
    phaseValueLabel.setBounds (rightX - 45, 336, 32, 14);

    const int syncY = 376;
    const int syncH = 50;
    const int syncGap = 7;
    const int syncStartX = 27;
    const int syncW = (rightX - syncStartX - 27 - syncGap * 5) / 6;
    for (size_t i = 0; i < syncButtons.size(); ++i)
        syncButtons[i].setBounds (syncStartX + static_cast<int> (i) * (syncW + syncGap), syncY, syncW, syncH);

    cvOutSlider.setBounds (78, 438, rightX - 150, 22);
    cvOutValueLabel.setBounds (rightX - 78, 435, 62, 26);

    const int rx = rightX + 12;
    levelMeter.setBounds (rx, 73, 166, 29);

    retrigOffButton.setBounds (rx, 137, 80, 34);
    retrigOnButton.setBounds  (rx + 86, 137, 80, 34);

    bipolarButton.setBounds  (rx, 208, 80, 34);
    unipolarButton.setBounds (rx + 86, 208, 80, 34);

    loopButton.setBounds     (rx, 277, 166, 34);
    oneShotButton.setBounds  (rx, 316, 166, 34);
    pingPongButton.setBounds (rx, 355, 166, 34);

    bpmValueLabel.setBounds (rx, 425, 166, 34);
    stopButton.setBounds (rx, 471, 166, 34);
}
