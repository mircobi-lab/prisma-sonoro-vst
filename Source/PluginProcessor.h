#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "dsp/PrismaDSP.h"
#include "Library.h"

class PrismaProcessor : public juce::AudioProcessor {
public:
    PrismaProcessor();
    ~PrismaProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Prisma Sonoro"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 30.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Predefinito"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // forma disegnata (thread dei messaggi): 256 punti di un ciclo
    std::vector<float> userCycle;
    void setUserCycle(const std::vector<float>& cyc);      // chiamata dall'interfaccia
    void setUserHarmonics(const prisma::PartialTable& t);   // modifica diretta degli armonici

    // colpo manuale (pulsante dell'interfaccia)
    void requestStrike() { strikeRequests.fetch_add(1); }

    // dati per la visualizzazione
    static constexpr int kScope = 8192;
    float scopeL[kScope] {}, scopeR[kScope] {};
    std::atomic<int> scopeWrite {0};
    std::atomic<float> currentHz {0.0f}, sweepProgress {0.0f};
    std::atomic<int> activeVoices {0};

    void setParam(const juce::String& id, float plainValue);

private:
    prisma::Params readParams() const;
    void pushTable(const prisma::PartialTable& t);

    prisma::Engine engine;
    juce::SpinLock tableLock; prisma::PartialTable pendingTable; std::atomic<bool> tableDirty {false};
    std::atomic<int> strikeRequests {0}; int strikeOffSamples = -1;
    double sr = 48000.0;
    juce::HeapBlock<float> tmpL, tmpR; int tmpSize = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrismaProcessor)
};
