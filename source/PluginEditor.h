#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

// ============================================================
//  PluginEditor.h  —  GUI do Tube Screamer
//  Visual inspirado no pedal verde TS-808 original
// ============================================================

// Knob customizado com visual de pedal analógico
class PedalKnob : public juce::Slider
{
public:
    PedalKnob (const juce::String& name)
    {
        setName(name);
        setSliderStyle (juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                             juce::MathConstants<float>::pi * 2.8f,
                             true);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float r  = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        // Sombra externa
        g.setColour (juce::Colour(0x44000000));
        g.fillEllipse (cx - r + 3, cy - r + 3, r * 2, r * 2);

        // Corpo do knob (cinza escuro metálico)
        juce::ColourGradient bodyGrad (
            juce::Colour(0xff555555), cx - r * 0.4f, cy - r * 0.4f,
            juce::Colour(0xff222222), cx + r * 0.4f, cy + r * 0.4f,
            true);
        g.setGradientFill(bodyGrad);
        g.fillEllipse (cx - r, cy - r, r * 2, r * 2);

        // Borda
        g.setColour (juce::Colour(0xff111111));
        g.drawEllipse (cx - r, cy - r, r * 2, r * 2, 1.5f);

        // Indicador (linha branca)
        auto normVal = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
        float startAngle = juce::MathConstants<float>::pi * 1.2f;
        float endAngle   = juce::MathConstants<float>::pi * 2.8f;
        float angle = startAngle + (float)normVal * (endAngle - startAngle);

        float lineLen = r * 0.65f;
        float lx1 = cx + std::sin(angle) * r * 0.35f;
        float ly1 = cy - std::cos(angle) * r * 0.35f;
        float lx2 = cx + std::sin(angle) * lineLen;
        float ly2 = cy - std::cos(angle) * lineLen;

        g.setColour (juce::Colours::white);
        g.drawLine (lx1, ly1, lx2, ly2, 2.5f);

        // Ponto brilhante
        g.setColour (juce::Colour(0x88ffffff));
        g.fillEllipse (cx - r * 0.25f, cy - r * 0.65f, r * 0.22f, r * 0.18f);
    }
};

// -------------------------------------------------------
class TubeScreamerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TubeScreamerAudioProcessorEditor (TubeScreamerAudioProcessor&);
    ~TubeScreamerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TubeScreamerAudioProcessor& audioProcessor;

    PedalKnob driveKnob  { "Drive" };
    PedalKnob toneKnob   { "Tone"  };
    PedalKnob levelKnob  { "Level" };

    juce::Label driveLabel, toneLabel, levelLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment* driveAttach = nullptr;
    juce::AudioProcessorValueTreeState::SliderAttachment* toneAttach  = nullptr;
    juce::AudioProcessorValueTreeState::SliderAttachment* levelAttach = nullptr;

    // Attachment manual (sem ValueTreeState)
    std::unique_ptr<juce::SliderParameterAttachment> driveSliderAttach;
    std::unique_ptr<juce::SliderParameterAttachment> toneSliderAttach;
    std::unique_ptr<juce::SliderParameterAttachment> levelSliderAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubeScreamerAudioProcessorEditor)
};
