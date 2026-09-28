#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginEditor.h"

// ============================================================
//  PluginProcessor.cpp  —  Tube Screamer DSP
// ============================================================

TubeScreamerAudioProcessor::TubeScreamerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Registra os três parâmetros do pedal
    addParameter (driveParam = new juce::AudioParameterFloat (
        "drive", "Drive", 0.0f, 1.0f, 0.5f));

    addParameter (toneParam  = new juce::AudioParameterFloat (
        "tone",  "Tone",  0.0f, 1.0f, 0.5f));

    addParameter (levelParam = new juce::AudioParameterFloat (
        "level", "Level", 0.0f, 1.0f, 0.7f));
}

TubeScreamerAudioProcessor::~TubeScreamerAudioProcessor() {}

// -------------------------------------------------------
void TubeScreamerAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    fs = sampleRate;

    // ---- Coeficiente do low-pass interno do clipper ----
    // Simula C3 no loop de feedback (~51pF em paralelo com Rf=51k)
    // fc = 1 / (2*pi*R*C) = 1 / (2*pi * 51k * 51pF) ≈ 61 kHz
    // (Limita as harmônicas ultra-agudas do clipping)
    double fc_clip = 61000.0;
    double wc = 2.0 * juce::MathConstants<double>::pi * fc_clip / sampleRate;
    lpClipCoef = (float)(1.0 - std::exp(-wc));   // coef do IIR de 1ª ordem

    // ---- Zera estados dos filtros ----
    for (int ch = 0; ch < 2; ++ch)
    {
        hpX[ch] = hpY[ch] = 0.0f;
        lpClipX[ch] = lpClipY[ch] = 0.0f;
        toneX[ch] = toneY[ch] = 0.0f;
        outHpX[ch] = outHpY[ch] = 0.0f;
    }
}

void TubeScreamerAudioProcessor::releaseResources() {}

// -------------------------------------------------------
void TubeScreamerAudioProcessor::processBlock (
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    const float drive = driveParam->get();   // 0..1
    const float tone  = toneParam->get();    // 0..1
    const float level = levelParam->get();   // 0..1

    // ---- Coeficiente do tone filter ----
    // Simula R_tone + C_tone: pot 20k (0..20k) + C = 0.022uF
    // Quando tone=0 -> R grande -> fc baixo (escuro)
    // Quando tone=1 -> R pequeno -> fc alto (brilhante)
    // fc varia de ~360 Hz a ~7.2 kHz
    double rTone   = 220.0 + (1.0 - (double)tone) * 20000.0;
    double cTone   = 0.022e-6;
    double wTone   = 1.0 / (rTone * cTone);
    double alphaTone = (float)(wTone / (wTone + 2.0 * fs));  // bilinear
    float  toneCoef = (float)alphaTone;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int n = 0; n < numSamples; ++n)
        {
            float x = data[n];

            // ---- 1. High-pass de entrada (C1/R1, fc ≈ 338 Hz) ----
            // Remove DC e frequências abaixo de 338 Hz antes do clipper
            // Isso é o que dá o "mid-hump" característico do TS
            float hpOut = hpAlpha * (hpY[ch] + x - hpX[ch]);
            hpX[ch] = x;
            hpY[ch] = hpOut;

            // ---- 2. Clipping assimétrico (estágio op-amp + diodos) ----
            float clipped = tubeSaturate(hpOut, drive);

            // ---- 3. Low-pass do loop de feedback (C3, fc ≈ 61kHz) ----
            // Suaviza as harmônicas ultra-agudas geradas pelo clipping
            float lpOut = lpClipY[ch] + lpClipCoef * (clipped - lpClipY[ch]);
            lpClipY[ch] = lpOut;

            // ---- 4. Tone filter (low-pass variável, R_tone + C4) ----
            // O TS não tem um treble cut/boost como outros pedais —
            // ele simplesmente atenua graves antes do clipper (hp) e
            // agudos depois (este lp), criando o "mid-boost" famoso
            float toneOut = toneY[ch] + toneCoef * (lpOut - toneY[ch]);
            toneY[ch] = toneOut;

            // ---- 5. High-pass de saída (remove DC residual) ----
            float outHp = outHpAlpha * (outHpY[ch] + toneOut - outHpX[ch]);
            outHpX[ch] = toneOut;
            outHpY[ch] = outHp;

            // ---- 6. Volume (level pot: 0..500k, saída linear) ----
            // Curva suave tipo potenciômetro de áudio (log)
            float vol = level * level * 4.0f;  // 0..4x
            data[n] = outHp * vol;
        }
    }
}

// -------------------------------------------------------
void TubeScreamerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, true);
    stream.writeFloat (*driveParam);
    stream.writeFloat (*toneParam);
    stream.writeFloat (*levelParam);
}

void TubeScreamerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, (size_t)sizeInBytes, false);
    *driveParam = stream.readFloat();
    *toneParam  = stream.readFloat();
    *levelParam = stream.readFloat();
}

// -------------------------------------------------------
juce::AudioProcessorEditor* TubeScreamerAudioProcessor::createEditor()
{
    return new TubeScreamerAudioProcessorEditor (*this);
}

// -------------------------------------------------------
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TubeScreamerAudioProcessor();
}

// Nota: createEditor() implementado em PluginEditor.cpp
// mas precisa ser definido aqui para o linker encontrar
