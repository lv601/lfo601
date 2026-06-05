#include "PluginEditor.h"

#include <cmath>

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
        return juce::Font (juce::FontOptions ("Courier New", 13.0f, juce::Font::bold));
    }

    juce::Font titleFont()
    {
        return juce::Font (juce::FontOptions ("Courier New", 13.0f, juce::Font::bold));
    }

    void drawTinyLabel (juce::Graphics& g, const juce::String& text, int x, int y, int w = 110)
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
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::outlineColourId, border);
    setColour (juce::ComboBox::textColourId, whiteText);
    setColour (juce::ComboBox::arrowColourId, yellow);
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

void RetroLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
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
CurveEditor::CurveEditor (LFOToolAudioProcessor& p) : processor (p)
{
    startTimerHz (30);
}

juce::Rectangle<float> CurveEditor::getCurveArea() const
{
    return getLocalBounds().toFloat().reduced (1.0f);
}

juce::Point<float> CurveEditor::pointToPixel (juce::Point<float> point) const
{
    const auto a = getCurveArea().reduced (10.0f, 18.0f);
    return { a.getX() + point.x * a.getWidth(), a.getBottom() - point.y * a.getHeight() };
}

juce::Point<float> CurveEditor::pixelToPoint (juce::Point<float> pixel) const
{
    const auto a = getCurveArea().reduced (10.0f, 18.0f);
    return { juce::jlimit (0.0f, 1.0f, (pixel.x - a.getX()) / juce::jmax (1.0f, a.getWidth())),
             juce::jlimit (0.0f, 1.0f, (a.getBottom() - pixel.y) / juce::jmax (1.0f, a.getHeight())) };
}

int CurveEditor::findPointNear (juce::Point<float> pixel) const
{
    int best = -1;
    float bestDistance = 12.0f;

    for (int i = 0; i < processor.getCurvePointCount(); ++i)
    {
        const auto d = pixel.getDistanceFrom (pointToPixel (processor.getCurvePoint (i)));
        if (d < bestDistance)
        {
            bestDistance = d;
            best = i;
        }
    }

    return best;
}

void CurveEditor::makeCurveWaveformActive()
{
    if (auto* param = processor.parameters.getParameter ("waveform"))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (LFOToolAudioProcessor::customCurve)));
        param->endChangeGesture();
    }
}

void CurveEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (panel2);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (border.withAlpha (0.85f));
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);

    auto area = getCurveArea();
    auto curveArea = area.reduced (10.0f, 18.0f);

    g.setColour (grid.withAlpha (0.7f));
    for (int i = 1; i < 4; ++i)
    {
        const auto x = curveArea.getX() + curveArea.getWidth() * static_cast<float> (i) / 4.0f;
        g.drawVerticalLine (juce::roundToInt (x), curveArea.getY(), curveArea.getBottom());
    }
    for (int i = 1; i < 3; ++i)
    {
        const auto y = curveArea.getY() + curveArea.getHeight() * static_cast<float> (i) / 3.0f;
        g.drawHorizontalLine (juce::roundToInt (y), curveArea.getX(), curveArea.getRight());
    }

    g.setColour (mutedText);
    g.setFont (smallFont());
    g.drawText ("CURVE EDITOR", 10, 2, 110, 16, juce::Justification::left);
    g.drawText ("click add / drag move / double delete", 150, 2, 260, 16, juce::Justification::left);

    const int waveform = static_cast<int> (processor.parameters.getRawParameterValue ("waveform")->load() + 0.5f);
    const int curveMode = static_cast<int> (processor.parameters.getRawParameterValue ("curveMode")->load() + 0.5f);
    const int polarity = static_cast<int> (processor.parameters.getRawParameterValue ("polarity")->load() + 0.5f);
    const auto depth = processor.parameters.getRawParameterValue ("depth")->load();
    const auto offset = processor.parameters.getRawParameterValue ("offset")->load();
    const auto phaseDeg = processor.parameters.getRawParameterValue ("phase")->load();

    juce::Path path;
    uint32_t rng = 0xCAFE1234;
    float held = 0.0f;
    double previous = 0.0;

    for (int i = 0; i < juce::roundToInt (curveArea.getWidth()); ++i)
    {
        const auto xf = static_cast<float> (i) / juce::jmax (1.0f, curveArea.getWidth() - 1.0f);
        const auto phase = static_cast<double> (xf) + static_cast<double> (phaseDeg) / 360.0;
        float y01 = 0.5f;

        if (waveform == LFOToolAudioProcessor::customCurve)
            y01 = processor.evaluateUserCurve (phase, curveMode);
        else
        {
            const auto v = LFOToolAudioProcessor::shapeValue (waveform, depth, offset, polarity, phase, rng, held, previous);
            y01 = polarity == LFOToolAudioProcessor::unipolar ? v : v * 0.5f + 0.5f;
        }

        const auto x = curveArea.getX() + xf * curveArea.getWidth();
        const auto y = curveArea.getBottom() - y01 * curveArea.getHeight();

        if (i == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    g.setColour (yellow.withAlpha (0.22f));
    g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (yellow);
    g.strokePath (path, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    for (int i = 0; i < processor.getCurvePointCount(); ++i)
    {
        const auto p = pointToPixel (processor.getCurvePoint (i));
        g.setColour (i == selectedPoint ? yellow : mint);
        g.fillEllipse (p.x - 4.5f, p.y - 4.5f, 9.0f, 9.0f);
        g.setColour (panel2);
        g.drawEllipse (p.x - 4.5f, p.y - 4.5f, 9.0f, 9.0f, 1.0f);
    }

    const auto ph = processor.getCurrentPhase();
    const auto markerX = curveArea.getX() + std::fmod (static_cast<double> (ph), 1.0) * curveArea.getWidth();
    const auto markerY = curveArea.getBottom() - processor.evaluateUserCurve (ph, curveMode) * curveArea.getHeight();
    g.setColour (mint);
    g.drawLine (static_cast<float> (markerX), curveArea.getY(), static_cast<float> (markerX), curveArea.getBottom(), 1.0f);
    g.fillEllipse (static_cast<float> (markerX) - 3.5f, markerY - 3.5f, 7.0f, 7.0f);
}

void CurveEditor::mouseDown (const juce::MouseEvent& e)
{
    makeCurveWaveformActive();
    selectedPoint = findPointNear (e.position);

    if (selectedPoint < 0)
        selectedPoint = processor.addCurvePoint (pixelToPoint (e.position).x, pixelToPoint (e.position).y);

    repaint();
}

void CurveEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (selectedPoint >= 0)
    {
        const auto p = pixelToPoint (e.position);
        processor.setCurvePoint (selectedPoint, p.x, p.y);
        selectedPoint = findPointNear (e.position);
        repaint();
    }
}

void CurveEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    makeCurveWaveformActive();
    const auto near = findPointNear (e.position);
    if (near >= 0)
    {
        processor.removeCurvePoint (near);
        selectedPoint = -1;
        repaint();
    }
}

void CurveEditor::timerCallback()
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
    : AudioProcessorEditor (&p), processor (p), curveEditor (p), levelMeter (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (760, 610);

    addAndMakeVisible (curveEditor);
    addAndMakeVisible (levelMeter);

    configureKnob (rateSlider, rateNameLabel, "RATE");
    configureKnob (depthSlider, depthNameLabel, "DEPTH");
    configureKnob (offsetSlider, offsetNameLabel, "OFFSET");
    configureKnob (smoothSlider, smoothNameLabel, "SMOOTH");

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

    const juce::StringArray waveTexts { "SIN", "TRI", "SAW", "REV", "SQR", "S&H", "CURV" };
    for (size_t i = 0; i < waveformButtons.size(); ++i)
    {
        configureButton (waveformButtons[i], waveTexts[static_cast<int> (i)]);
        waveformButtons[i].onClick = [this, i] { setChoiceParameter ("waveform", static_cast<int> (i)); };
    }

    const juce::StringArray curveTexts { "SMOOTH", "STEP", "BEZIER" };
    for (size_t i = 0; i < curveModeButtons.size(); ++i)
    {
        configureButton (curveModeButtons[i], curveTexts[static_cast<int> (i)]);
        curveModeButtons[i].onClick = [this, i]
        {
            setChoiceParameter ("curveMode", static_cast<int> (i));
            setChoiceParameter ("waveform", LFOToolAudioProcessor::customCurve);
        };
    }

    configureCombo (syncCombo);
    const juce::StringArray syncTexts { "Free", "1/16", "1/16T", "1/16D", "1/8", "1/8T", "1/8D", "1/4", "1/4T", "1/4D", "1/2", "1 Bar", "2 Bar" };
    for (int i = 0; i < syncTexts.size(); ++i)
        syncCombo.addItem (syncTexts[i], i + 1);
    addAndMakeVisible (syncCombo);

    configureButton (retrigOffButton, "OFF");
    configureButton (retrigOnButton, "ON");
    retrigOffButton.onClick = [this] { setChoiceParameter ("retrig", 0); };
    retrigOnButton.onClick  = [this] { setChoiceParameter ("retrig", 1); };

    configureButton (bipolarButton, "BIPOLAR");
    configureButton (unipolarButton, "UNIPOLAR");
    bipolarButton.onClick  = [this] { setChoiceParameter ("polarity", 0); };
    unipolarButton.onClick = [this] { setChoiceParameter ("polarity", 1); };

    configureButton (phaseLockOffButton, "OFF");
    configureButton (phaseLockOnButton, "ON");
    phaseLockOffButton.onClick = [this] { setChoiceParameter ("phaseLock", 0); };
    phaseLockOnButton.onClick  = [this] { setChoiceParameter ("phaseLock", 1); };

    configureButton (outputAddButton, "ADD");
    configureButton (outputCVButton, "CV ONLY");
    outputAddButton.onClick = [this] { setChoiceParameter ("outputMode", 0); };
    outputCVButton.onClick  = [this] { setChoiceParameter ("outputMode", 1); };

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

    curveHintLabel.setColour (juce::Label::textColourId, mutedText);
    curveHintLabel.setFont (smallFont());
    curveHintLabel.setJustificationType (juce::Justification::left);
    curveHintLabel.setText ("CURVE MODE", juce::dontSendNotification);
    addAndMakeVisible (curveHintLabel);

    rateAttachment   = std::make_unique<SliderAttachment> (processor.parameters, "rate",   rateSlider);
    depthAttachment  = std::make_unique<SliderAttachment> (processor.parameters, "depth",  depthSlider);
    offsetAttachment = std::make_unique<SliderAttachment> (processor.parameters, "offset", offsetSlider);
    smoothAttachment = std::make_unique<SliderAttachment> (processor.parameters, "smooth", smoothSlider);
    phaseAttachment  = std::make_unique<SliderAttachment> (processor.parameters, "phase",  phaseSlider);
    syncAttachment   = std::make_unique<ComboBoxAttachment> (processor.parameters, "syncDivision", syncCombo);

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

void LFOToolAudioProcessorEditor::configureCombo (juce::ComboBox& c)
{
    c.setLookAndFeel (&lookAndFeel);
    c.setColour (juce::ComboBox::backgroundColourId, panel);
    c.setColour (juce::ComboBox::outlineColourId, border);
    c.setColour (juce::ComboBox::textColourId, whiteText);
    c.setColour (juce::ComboBox::arrowColourId, yellow);
    c.setJustificationType (juce::Justification::centred);
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

    const auto curveMode = getChoice ("curveMode");
    for (size_t i = 0; i < curveModeButtons.size(); ++i)
        curveModeButtons[i].setToggleState (static_cast<int> (i) == curveMode, juce::dontSendNotification);

    const auto retrig = getChoice ("retrig");
    retrigOffButton.setToggleState (retrig == 0, juce::dontSendNotification);
    retrigOnButton.setToggleState  (retrig == 1, juce::dontSendNotification);

    const auto polarity = getChoice ("polarity");
    bipolarButton.setToggleState  (polarity == 0, juce::dontSendNotification);
    unipolarButton.setToggleState (polarity == 1, juce::dontSendNotification);

    const auto phaseLock = getChoice ("phaseLock");
    phaseLockOffButton.setToggleState (phaseLock == 0, juce::dontSendNotification);
    phaseLockOnButton.setToggleState  (phaseLock == 1, juce::dontSendNotification);

    const auto outputMode = getChoice ("outputMode");
    outputAddButton.setToggleState (outputMode == 0, juce::dontSendNotification);
    outputCVButton.setToggleState  (outputMode == 1, juce::dontSendNotification);

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

    auto root = getLocalBounds().toFloat();
    g.setColour (bg);
    g.fillRoundedRectangle (root.reduced (10.0f), 8.0f);
    g.setColour (border);
    g.drawRoundedRectangle (root.reduced (10.5f), 8.0f, 1.0f);

    const int rightX = getWidth() - 200;
    g.setColour (panel.withAlpha (0.7f));
    g.fillRect (juce::Rectangle<int> (rightX, 39, 190, getHeight() - 49));
    g.setColour (border.withAlpha (0.8f));
    g.drawVerticalLine (rightX, 39.0f, static_cast<float> (getHeight() - 10));
    g.drawHorizontalLine (39, 10.0f, static_cast<float> (getWidth() - 10));

    g.setFont (titleFont());
    g.setColour (yellow);
    g.drawText ("LFOz TOOL 2.0", 27, 17, 170, 16, juce::Justification::left);
    g.setColour (mutedText);
    g.drawText ("CV MOD UNIT", getWidth() - 126, 17, 110, 16, juce::Justification::right);

    drawTinyLabel (g, "SHAPE", 27, 222);
    drawTinyLabel (g, "CURVE", 27, 262);
    drawTinyLabel (g, "PHASE", 27, 395);
    drawTinyLabel (g, "SYNC", 27, 426);
    drawTinyLabel (g, "CV OUT", 27, 545);

    const int rx = rightX + 12;
    drawTinyLabel (g, "LEVEL", rx, 51);
    drawTinyLabel (g, "RETRIG", rx, 109);
    drawTinyLabel (g, "POLARITY", rx, 177);
    drawTinyLabel (g, "PHASE LOCK", rx, 245);
    drawTinyLabel (g, "OUTPUT", rx, 313);
    drawTinyLabel (g, "MODE", rx, 381);
    drawTinyLabel (g, "BPM", rx, 545);
}

void LFOToolAudioProcessorEditor::resized()
{
    const int rightX = getWidth() - 200;
    curveEditor.setBounds (27, 51, rightX - 37, 158);

    const int waveY = 221;
    const int waveButtonW = 45;
    const int waveButtonH = 31;
    const int waveStartX = 80;
    for (size_t i = 0; i < waveformButtons.size(); ++i)
        waveformButtons[i].setBounds (waveStartX + static_cast<int> (i) * 49, waveY, waveButtonW, waveButtonH);

    curveHintLabel.setBounds (80, 257, 96, 16);
    const int curveModeY = 274;
    const int curveButtonW = 76;
    for (size_t i = 0; i < curveModeButtons.size(); ++i)
        curveModeButtons[i].setBounds (80 + static_cast<int> (i) * 83, curveModeY, curveButtonW, 29);

    const int knobY = 300;
    const int knobSize = 62;
    const std::array<int, 4> centres { 92, 218, 344, 470 };
    rateSlider.setBounds   (centres[0] - knobSize / 2, knobY, knobSize, 78);
    depthSlider.setBounds  (centres[1] - knobSize / 2, knobY, knobSize, 78);
    offsetSlider.setBounds (centres[2] - knobSize / 2, knobY, knobSize, 78);
    smoothSlider.setBounds (centres[3] - knobSize / 2, knobY, knobSize, 78);

    rateNameLabel.setBounds   (centres[0] - 36, 356, 72, 14);
    depthNameLabel.setBounds  (centres[1] - 36, 356, 72, 14);
    offsetNameLabel.setBounds (centres[2] - 36, 356, 72, 14);
    smoothNameLabel.setBounds (centres[3] - 36, 356, 72, 14);

    rateLabel.setBounds   (centres[0] - 42, 371, 84, 14);
    depthLabel.setBounds  (centres[1] - 42, 371, 84, 14);
    offsetLabel.setBounds (centres[2] - 42, 371, 84, 14);
    smoothLabel.setBounds (centres[3] - 42, 371, 84, 14);

    phaseSlider.setBounds (78, 391, rightX - 120, 28);
    phaseValueLabel.setBounds (rightX - 56, 395, 50, 14);

    syncCombo.setBounds (78, 424, 178, 34);

    cvOutSlider.setBounds (78, 542, rightX - 150, 22);
    cvOutValueLabel.setBounds (rightX - 78, 539, 62, 26);

    const int rx = rightX + 12;
    levelMeter.setBounds (rx, 73, 166, 27);

    retrigOffButton.setBounds (rx, 132, 80, 34);
    retrigOnButton.setBounds  (rx + 86, 132, 80, 34);

    bipolarButton.setBounds  (rx, 200, 80, 34);
    unipolarButton.setBounds (rx + 86, 200, 80, 34);

    phaseLockOffButton.setBounds (rx, 268, 80, 34);
    phaseLockOnButton.setBounds  (rx + 86, 268, 80, 34);

    outputAddButton.setBounds (rx, 336, 80, 34);
    outputCVButton.setBounds  (rx + 86, 336, 80, 34);

    loopButton.setBounds     (rx, 404, 166, 30);
    oneShotButton.setBounds  (rx, 438, 166, 30);
    pingPongButton.setBounds (rx, 472, 166, 30);

    bpmValueLabel.setBounds (rx, 570, 80, 28);
    stopButton.setBounds (rx + 86, 570, 80, 28);
}
