#include "PluginEditor.h"

// ============================================================
//  PluginEditor.cpp  —  GUI do Tube Screamer
// ============================================================

TubeScreamerAudioProcessorEditor::TubeScreamerAudioProcessorEditor (
    TubeScreamerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (360, 220);

    // ---- Configura os knobs ----
    auto setupKnob = [&](juce::Slider& knob, juce::Label& label,
                         const juce::String& text)
    {
        addAndMakeVisible(knob);
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font("Arial", 11.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colour(0xffeeeecc));
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupKnob(driveKnob, driveLabel, "Drive");
    setupKnob(toneKnob,  toneLabel,  "Tone");
    setupKnob(levelKnob, levelLabel, "Level");

    // ---- Vincula knobs aos parâmetros ----
    driveSliderAttach = std::make_unique<juce::SliderParameterAttachment>(
        *audioProcessor.driveParam, driveKnob);
    toneSliderAttach  = std::make_unique<juce::SliderParameterAttachment>(
        *audioProcessor.toneParam,  toneKnob);
    levelSliderAttach = std::make_unique<juce::SliderParameterAttachment>(
        *audioProcessor.levelParam, levelKnob);
}

TubeScreamerAudioProcessorEditor::~TubeScreamerAudioProcessorEditor() {}

// -------------------------------------------------------
void TubeScreamerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // ---- Fundo verde característico do TS-808 ----
    juce::ColourGradient bg (
        juce::Colour(0xff2d6e2d), 0,   0,
        juce::Colour(0xff1a4a1a), 0, (float)getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);

    // Borda metálica
    g.setColour(juce::Colour(0xff888866));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.5f), 12.0f, 2.5f);

    // ---- Parafusos nos cantos ----
    auto drawScrew = [&](float x, float y)
    {
        g.setColour(juce::Colour(0xff999977));
        g.fillEllipse(x - 5, y - 5, 10, 10);
        g.setColour(juce::Colour(0xff555544));
        g.drawEllipse(x - 5, y - 5, 10, 10, 1.0f);
        g.setColour(juce::Colour(0xff333322));
        g.drawLine(x - 2.5f, y, x + 2.5f, y, 1.5f);
        g.drawLine(x, y - 2.5f, x, y + 2.5f, 1.5f);
    };
    drawScrew(20, 20);
    drawScrew(getWidth() - 20, 20);
    drawScrew(20, getHeight() - 20);
    drawScrew(getWidth() - 20, getHeight() - 20);

    // ---- Título ----
    g.setFont(juce::Font("Arial", 20.0f, juce::Font::bold | juce::Font::italic));
    g.setColour(juce::Colour(0xfff0e080));
    g.drawText("TScreamer", 0, 14, getWidth(), 24,
               juce::Justification::centred);

    // Subtítulo
    g.setFont(juce::Font("Arial", 9.5f, juce::Font::plain));
    g.setColour(juce::Colour(0xffaabbaa));
    g.drawText("Overdrive / TS-808", 0, 36, getWidth(), 14,
               juce::Justification::centred);

    // ---- Linha decorativa ----
    g.setColour(juce::Colour(0x55f0e080));
    g.drawLine(40, 54, getWidth() - 40, 54, 1.0f);

    // ---- Jacks de entrada/saída ----
    auto drawJack = [&](float x, float y, const juce::String& label)
    {
        g.setColour(juce::Colour(0xff333322));
        g.fillEllipse(x - 7, y - 7, 14, 14);
        g.setColour(juce::Colour(0xff888866));
        g.drawEllipse(x - 7, y - 7, 14, 14, 1.5f);
        g.setColour(juce::Colour(0xff111100));
        g.fillEllipse(x - 3, y - 3, 6, 6);
        g.setFont(juce::Font("Arial", 8.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xffaabb99));
        g.drawText(label, (int)(x - 15), (int)(y + 9), 30, 12,
                   juce::Justification::centred);
    };
    drawJack(40,  getHeight() - 28, "IN");
    drawJack(getWidth() - 40, getHeight() - 28, "OUT");

    // ---- LED verde ----
    float ledX = getWidth() * 0.5f;
    float ledY = getHeight() - 28.0f;
    juce::ColourGradient ledGrad (
        juce::Colour(0xff88ff44), ledX - 3, ledY - 3,
        juce::Colour(0xff225500), ledX + 3, ledY + 3,
        true);
    g.setGradientFill(ledGrad);
    g.fillEllipse(ledX - 5, ledY - 5, 10, 10);
    g.setColour(juce::Colour(0xff004400));
    g.drawEllipse(ledX - 5, ledY - 5, 10, 10, 1.0f);
}

// -------------------------------------------------------
void TubeScreamerAudioProcessorEditor::resized()
{
    const int w   = getWidth();
    const int knobSize = 72;
    const int labelH   = 16;
    const int knobY    = 62;
    const int labelY   = knobY + knobSize + 2;

    int spacing = w / 4;

    driveKnob.setBounds (spacing * 1 - knobSize / 2, knobY, knobSize, knobSize);
    toneKnob .setBounds (spacing * 2 - knobSize / 2, knobY, knobSize, knobSize);
    levelKnob.setBounds (spacing * 3 - knobSize / 2, knobY, knobSize, knobSize);

    driveLabel.setBounds (spacing * 1 - 30, labelY, 60, labelH);
    toneLabel .setBounds (spacing * 2 - 30, labelY, 60, labelH);
    levelLabel.setBounds (spacing * 3 - 30, labelY, 60, labelH);
}
