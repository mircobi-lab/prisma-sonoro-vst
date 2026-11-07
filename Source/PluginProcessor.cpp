#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

static juce::String U(const char* s) { return juce::String::fromUTF8(s); }

// testo dei valori all'italiana: virgola decimale e unità; dec < 0 = decimali automatici, dec == -2 = percentuale
static juce::AudioParameterFloatAttributes fmt(int dec, const char* unit) {
    juce::String u = U(unit);
    return juce::AudioParameterFloatAttributes().withLabel(u)
        .withStringFromValueFunction([dec, u](float v, int) {
            if (dec == -2) return juce::String(juce::roundToInt(v * 100.0f)) + " %";
            int d = dec >= 0 ? dec : (v < 10.0f ? 3 : v < 1000.0f ? 2 : 1);
            auto t = (d == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, d)).replace(".", ",");
            return u.isEmpty() ? t : t + " " + u; })
        .withValueFromStringFunction([dec](const juce::String& t) { float v = t.replace(",", ".").getFloatValue(); return dec == -2 ? v / 100.0f : v; });
}

static juce::NormalisableRange<float> logRange(float lo, float hi, float centre) {
    juce::NormalisableRange<float> r(lo, hi); r.setSkewForCentre(centre); return r;
}

APVTS::ParameterLayout PrismaProcessor::createLayout() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto pid = [](const char* s) { return ParameterID {s, 1}; };
    StringArray models; for (int m = 0; m < prisma::NumModels; ++m) models.add(U(prisma::modelName(m)));

    p.push_back(std::make_unique<AudioParameterChoice>(pid("mode"), "Modo", StringArray {"Note MIDI", "Hz fisso"}, 1));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("fixedHz"), "Frequenza fissa", logRange(0.1f, 20000.0f, 440.0f), 528.0f,
        fmt(-1, "Hz")));
    p.push_back(std::make_unique<AudioParameterBool>(pid("drone"), "Bordone", false));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("a4"), "Diapason La", NormalisableRange<float>(415.0f, 466.0f, 0.01f), 440.0f,
        fmt(2, "Hz")));
    p.push_back(std::make_unique<AudioParameterChoice>(pid("model"), "Modello", models, 0));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("tilt"), "Inclinazione spettrale", NormalisableRange<float>(-12.0f, 6.0f, 0.1f), 0.0f,
        fmt(1, "dB/ott")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("inharm"), U("Inarmonicità"), logRange(0.0f, 0.01f, 0.001f), 0.0f, fmt(5, "")));
    p.push_back(std::make_unique<AudioParameterChoice>(pid("beatMode"), "Battimento", StringArray {"Spento", "Binaurale", "Monoaurale", "Isocronico"}, 0));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("beatHz"), "Frequenza battimento", logRange(0.5f, 40.0f, 8.0f), 6.0f,
        fmt(2, "Hz")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("breath"), "Respiro 0,1 Hz", NormalisableRange<float>(0.0f, 1.0f), 0.0f, fmt(-2, "")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("tremRate"), U("Tremolo velocità"), NormalisableRange<float>(0.0f, 12.0f, 0.01f), 0.0f,
        fmt(2, "Hz")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("tremDepth"), U("Tremolo profondità"), NormalisableRange<float>(0.0f, 1.0f), 0.0f, fmt(-2, "")));
    p.push_back(std::make_unique<AudioParameterChoice>(pid("noiseType"), "Rumore", StringArray {"Spento", "Bianco", "Rosa", "Marrone", "Grigio", "Blu"}, 0));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("noiseDb"), "Livello rumore", NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -30.0f,
        fmt(1, "dB")));
    p.push_back(std::make_unique<AudioParameterBool>(pid("sweepOn"), "Sweep attivo", false));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("sweepFrom"), "Sweep da", logRange(1.0f, 20000.0f, 440.0f), 55.0f, fmt(-1, "Hz")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("sweepTo"), "Sweep a", logRange(1.0f, 20000.0f, 440.0f), 880.0f, fmt(-1, "Hz")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("sweepMin"), "Sweep durata", logRange(0.1f, 60.0f, 5.0f), 5.0f, fmt(1, "min")));
    p.push_back(std::make_unique<AudioParameterChoice>(pid("sweepDir"), "Sweep direzione", StringArray {"Sale", "Scende", "Sale e scende"}, 0));
    p.push_back(std::make_unique<AudioParameterBool>(pid("sweepLoop"), "Sweep ripeti", false));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("attack"), "Attacco", logRange(0.001f, 10.0f, 0.2f), 0.01f, fmt(3, "s")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("decay"), "Decadimento", logRange(0.001f, 10.0f, 0.5f), 0.3f, fmt(3, "s")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("sustain"), "Sostegno", NormalisableRange<float>(0.0f, 1.0f), 1.0f, fmt(-2, "")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("release"), "Rilascio", logRange(0.005f, 30.0f, 1.5f), 1.5f, fmt(2, "s")));
    p.push_back(std::make_unique<AudioParameterFloat>(pid("level"), "Livello", NormalisableRange<float>(-48.0f, 0.0f, 0.1f), -6.0f, fmt(1, "dB")));
    return {p.begin(), p.end()};
}

PrismaProcessor::PrismaProcessor()
    : AudioProcessor(BusesProperties().withOutput("Uscita", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PRISMA", createLayout()) {
    userCycle.resize(256);
    for (int i = 0; i < 256; ++i) userCycle[(size_t) i] = std::sin(juce::MathConstants<float>::twoPi * i / 256.0f);
    pushTable(prisma::harmonicsFromCycle(userCycle));
}

bool PrismaProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PrismaProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    sr = sampleRate; engine.prepare(sampleRate);
    tmpSize = juce::jmax(samplesPerBlock, 4096); tmpL.allocate((size_t) tmpSize, true); tmpR.allocate((size_t) tmpSize, true);
    tableDirty = true;
}

prisma::Params PrismaProcessor::readParams() const {
    auto v = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    prisma::Params P;
    P.mode = (int) v("mode"); P.fixedHz = v("fixedHz"); P.droneOn = v("drone") > 0.5f; P.a4 = v("a4");
    P.model = (int) v("model"); P.tiltDb = v("tilt"); P.inharm = v("inharm");
    P.beatMode = (int) v("beatMode"); P.beatHz = v("beatHz"); P.breathDepth = v("breath");
    P.tremRate = v("tremRate"); P.tremDepth = v("tremDepth"); P.noiseType = (int) v("noiseType"); P.noiseDb = v("noiseDb");
    P.sweepOn = v("sweepOn") > 0.5f; P.sweepFrom = v("sweepFrom"); P.sweepTo = v("sweepTo"); P.sweepMin = v("sweepMin");
    P.sweepDir = (int) v("sweepDir"); P.sweepLoop = v("sweepLoop") > 0.5f;
    P.attack = v("attack"); P.decay = v("decay"); P.sustain = v("sustain"); P.release = v("release"); P.levelDb = v("level");
    return P;
}

void PrismaProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (n > tmpSize) { tmpSize = n; tmpL.allocate((size_t) n, true); tmpR.allocate((size_t) n, true); }

    if (tableDirty.load()) { const juce::SpinLock::ScopedTryLockType tl(tableLock); if (tl.isLocked()) { engine.setUserTable(pendingTable); tableDirty = false; } }
    engine.setParams(readParams());

    // colpo manuale dal pulsante: nota 60, rilasciata dopo 1,5 s (i modelli percussivi continuano a risuonare)
    if (strikeRequests.exchange(0) > 0) { engine.noteOn(60, 0.9f); strikeOffSamples = (int) (1.5 * sr); }
    if (strikeOffSamples >= 0) { strikeOffSamples -= n; if (strikeOffSamples < 0) engine.noteOff(60); }

    // MIDI a precisione di campione: il blocco viene diviso agli eventi
    int pos = 0;
    for (const auto meta : midi) {
        const auto msg = meta.getMessage(); const int at = juce::jlimit(0, n, meta.samplePosition);
        if (at > pos) { engine.process(tmpL.get() + pos, tmpR.get() + pos, at - pos); pos = at; }
        if (msg.isNoteOn()) engine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff()) engine.noteOff(msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine.allNotesOff();
    }
    if (pos < n) engine.process(tmpL.get() + pos, tmpR.get() + pos, n - pos);

    buffer.clear();
    buffer.copyFrom(0, 0, tmpL.get(), n);
    if (buffer.getNumChannels() > 1) buffer.copyFrom(1, 0, tmpR.get(), n);

    int w = scopeWrite.load();
    for (int i = 0; i < n; ++i) { scopeL[w] = tmpL[i]; scopeR[w] = tmpR[i]; w = (w + 1) & (kScope - 1); }
    scopeWrite.store(w);
    const auto& P = engine.params();
    currentHz.store(P.mode == 1 ? (float) engine.currentFixed() : 0.0f);
    sweepProgress.store(P.sweepOn ? (float) engine.sweepProgress() : 0.0f);
}

void PrismaProcessor::pushTable(const prisma::PartialTable& t) {
    const juce::SpinLock::ScopedLockType l(tableLock); pendingTable = t; tableDirty = true;
}

void PrismaProcessor::setUserCycle(const std::vector<float>& cyc) {
    userCycle = cyc; pushTable(prisma::harmonicsFromCycle(userCycle));
}

void PrismaProcessor::setUserHarmonics(const prisma::PartialTable& t) {
    userCycle = prisma::cycleFromTable(t, 256); pushTable(t);
}

void PrismaProcessor::setParam(const juce::String& id, float plainValue) {
    if (auto* p = apvts.getParameter(id)) {
        p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(plainValue)); p->endChangeGesture();
    }
}

void PrismaProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    juce::MemoryBlock mb(userCycle.data(), userCycle.size() * sizeof(float));
    state.setProperty("userCycle", mb.toBase64Encoding(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, destData);
}

void PrismaProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        auto state = juce::ValueTree::fromXml(*xml);
        if (state.hasType(apvts.state.getType())) {
            juce::MemoryBlock mb;
            if (mb.fromBase64Encoding(state.getProperty("userCycle").toString()) && mb.getSize() == 256 * sizeof(float)) {
                std::vector<float> cyc(256); std::memcpy(cyc.data(), mb.getData(), mb.getSize()); setUserCycle(cyc);
            }
            apvts.replaceState(state);
        }
    }
}

juce::AudioProcessorEditor* PrismaProcessor::createEditor() { return new PrismaEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PrismaProcessor(); }
