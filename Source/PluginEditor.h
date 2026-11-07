#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

namespace pal {
    const juce::Colour ink   {0xff0f1220}, panel {0xff171b2e}, line {0xff2a3050}, deep {0xff090b15};
    const juce::Colour paper {0xffece6d8}, muted {0xff9a9fb8}, glow {0xffe8b86a};
}

class PrismaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    PrismaLookAndFeel();
    static juce::Font sans(float h, bool bold = false);
    static juce::Font mono(float h);
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h, float pos, float minP, float maxP, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override { return sans(15.0f); }
    juce::Font getPopupMenuFont() override { return sans(15.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return sans(15.0f, true); }
    juce::Font getLabelFont(juce::Label& l) override { return l.getFont(); }
};

// Ciclo singolo: si può disegnare col mouse
class WaveView : public juce::Component {
public:
    std::vector<float> cyc = std::vector<float>(256, 0.0f);
    juce::Colour colour = pal::glow; bool drawnModel = false;
    std::function<void()> onStartDraw;                       // prima modifica: copia il modello nel disegno
    std::function<void(const std::vector<float>&, bool)> onEdit; // (ciclo, finale?)
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
private:
    int lastIdx = -1; float lastVal = 0; juce::uint32 lastSend = 0;
    void apply(const juce::MouseEvent&);
};

// Armonici: barre modificabili per le tabelle armoniche
class HarmView : public juce::Component {
public:
    prisma::PartialTable table; bool editable = false; juce::Colour colour = pal::glow;
    std::function<void(int k, float amp)> onEdit;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e) override { edit(e); }
    void mouseDrag(const juce::MouseEvent& e) override { edit(e); }
private:
    void edit(const juce::MouseEvent&);
};

// Oscilloscopio XY + forma nel tempo
class ScopeView : public juce::Component {
public:
    explicit ScopeView(PrismaProcessor& p) : proc(p) {}
    juce::Colour colour = pal::glow; double refHz = 200;
    void paint(juce::Graphics&) override;
private:
    PrismaProcessor& proc;
};

class PrismaEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit PrismaEditor(PrismaProcessor&);
    ~PrismaEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshWave(bool force = false);
    float pv(const char* id) const { return proc.apvts.getRawParameterValue(id)->load(); }
    std::vector<float> currentModelCycle() const;

    PrismaProcessor& proc;
    PrismaLookAndFeel lnf;

    struct Knob { juce::Slider s; juce::Label l; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; };
    std::vector<std::unique_ptr<Knob>> knobs;
    Knob* makeKnob(const char* id, const char* label, bool linear = false);
    Knob *kFixed, *kA4, *kTilt, *kInharm, *kBeat, *kBreath, *kTremR, *kTremD, *kNoise, *kFrom, *kTo, *kMin, *kA, *kD, *kS, *kR, *kLevel;

    juce::ComboBox modeBox, modelBox, freqBox, libBox, beatBox, noiseBox, dirBox;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    juce::ToggleButton droneBtn {"Bordone"}, sweepBtn {"Sweep"}, loopBtn {"Ripeti"};
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtt;
    juce::TextButton strikeBtn {"Colpo"}, a440Btn {"440"}, a432Btn {"432"}, copyBtn {"Copia nel disegno"}, smoothBtn {"Liscia"}, sineBtn {"Sinusoide"}, invertBtn {"Inverti"};

    WaveView wave; HarmView harm; ScopeView scope;

    // stato per l'aggiornamento
    int lastModel = -1; float lastTilt = 1e9f, lastInharm = 1e9f; size_t lastCycleHash = 0;
    juce::String readHz, readNote, readInfo; juce::Colour readColour = pal::glow; float progress = 0;
    juce::Rectangle<int> secSource, secTimbre, secMod, secSweep, secEnv, headerArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrismaEditor)
};
