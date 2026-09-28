#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

// ============================================================
//  Tube Screamer Plugin - PluginProcessor.h
//  Modela o circuito do Ibanez TS-808 / TS-9
//
//  Cadeia de sinal:
//  Input -> Highpass (buffer) -> Clipper (diodos + op-amp) ->
//           Tone (RC lowpass) -> Volume -> Output
// ============================================================

class TubeScreamerAudioProcessor : public juce::AudioProcessor
{
public:
    TubeScreamerAudioProcessor();
    ~TubeScreamerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- Parâmetros (0.0 a 1.0) ----
    juce::AudioParameterFloat* driveParam;   // Ganho do clipper
    juce::AudioParameterFloat* toneParam;    // Filtro de timbre
    juce::AudioParameterFloat* levelParam;   // Volume de saída

private:
    double fs = 44100.0; // sample rate

    // ---- Estado dos filtros (por canal) ----

    // 1) High-pass de entrada (remove DC, simula C1 ~0.047uF + R1 10k)
    //    y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    float hpX[2] = {}, hpY[2] = {};
    float hpAlpha = 0.997f;

    // 2) Low-pass de saída do clipper (simula C3 ~51pF dentro do loop de feedback)
    //    Suaviza as harmônicas muito agudas do clipping
    float lpClipX[2] = {}, lpClipY[2] = {};
    float lpClipCoef = 0.0f; // calculado no prepare()

    // 3) Filtro de tone (lowpass variável, simula pot de 20k + C4 0.022uF)
    float toneX[2] = {}, toneY[2] = {};

    // 4) High-pass de acoplamento de saída (remove DC residual)
    float outHpX[2] = {}, outHpY[2] = {};
    float outHpAlpha = 0.995f;

    // ---- Soft-clipping: saturação assimétrica de diodo ----
    // Modela dois diodos 1N914 em anti-paralelo (como no TS808)
    // Usa tanh + assimetria leve para comportamento mais orgânico
    inline float tubeSaturate (float x, float drive) noexcept
    {
        // Ganho do estágio inversor do op-amp: 1 + Rf/Rin
        // Drive vai de ~1 a ~500 (como no circuito real)
        float gain = 1.0f + drive * 499.0f;
        float v = x * gain;

        // Limite de tensão dos diodos (~0.7V)
        // Assimetria suave: diodo positivo clippa levemente antes
        float posClip = std::tanh(v * 0.8f);
        float negClip = std::tanh(v * 1.0f);
        float clipped = (posClip + negClip) * 0.5f;

        // Normaliza de volta (evita saturação de saída)
        return clipped / (gain * 0.002f + 1.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubeScreamerAudioProcessor)
};
