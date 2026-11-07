#include "PluginEditor.h"
#include "BinaryData.h"

using namespace juce;

static String U(const char* s) { return String::fromUTF8(s); }
static String num(double v, int dec) { return (dec <= 0 ? String((int) std::lround(v)) : String(v, dec)).replace(".", ","); }

// ------------------------------------------------------------------ aspetto
static Typeface::Ptr tfSans, tfSansBold, tfMono;

PrismaLookAndFeel::PrismaLookAndFeel() {
    if (!tfSans)     tfSans     = Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedRegular_ttf, BinaryData::BarlowCondensedRegular_ttfSize);
    if (!tfSansBold) tfSansBold = Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedSemiBold_ttf, BinaryData::BarlowCondensedSemiBold_ttfSize);
    if (!tfMono)     tfMono     = Typeface::createSystemTypefaceFor(BinaryData::ShareTechMonoRegular_ttf, BinaryData::ShareTechMonoRegular_ttfSize);
    setDefaultSansSerifTypeface(tfSans);
    setColour(ResizableWindow::backgroundColourId, pal::ink);
    setColour(Label::textColourId, pal::paper);
    setColour(Slider::textBoxTextColourId, pal::paper);
    setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour(Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour(ComboBox::backgroundColourId, pal::deep);
    setColour(ComboBox::outlineColourId, pal::line);
    setColour(ComboBox::textColourId, pal::paper);
    setColour(ComboBox::arrowColourId, pal::glow);
    setColour(PopupMenu::backgroundColourId, pal::panel);
    setColour(PopupMenu::textColourId, pal::paper);
    setColour(PopupMenu::highlightedBackgroundColourId, pal::glow);
    setColour(PopupMenu::highlightedTextColourId, pal::ink);
    setColour(PopupMenu::headerTextColourId, pal::glow);
    setColour(TextButton::buttonColourId, pal::deep);
    setColour(TextButton::textColourOffId, pal::paper);
    setColour(TextButton::textColourOnId, pal::ink);
    setColour(ToggleButton::textColourId, pal::paper);
    setColour(TextEditor::backgroundColourId, pal::deep);
    setColour(TextEditor::textColourId, pal::paper);
    setColour(TextEditor::highlightColourId, pal::glow.withAlpha(0.4f));
    setColour(CaretComponent::caretColourId, pal::glow);
}

Font PrismaLookAndFeel::sans(float h, bool bold) { return Font(FontOptions(bold ? tfSansBold : tfSans).withHeight(h)); }
Font PrismaLookAndFeel::mono(float h) { return Font(FontOptions(tfMono).withHeight(h)); }

void PrismaLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider& s) {
    auto r = Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(4.0f);
    float d = jmin(r.getWidth(), r.getHeight()); auto c = r.withSizeKeepingCentre(d, d);
    float ang = a0 + pos * (a1 - a0), th = 3.0f;
    Path bg; bg.addCentredArc(c.getCentreX(), c.getCentreY(), d / 2 - th, d / 2 - th, 0, a0, a1, true);
    g.setColour(pal::line); g.strokePath(bg, PathStrokeType(th, PathStrokeType::curved, PathStrokeType::rounded));
    Path fg; fg.addCentredArc(c.getCentreX(), c.getCentreY(), d / 2 - th, d / 2 - th, 0, a0, ang, true);
    g.setColour(s.isEnabled() ? pal::glow : pal::muted); g.strokePath(fg, PathStrokeType(th, PathStrokeType::curved, PathStrokeType::rounded));
    Point<float> p = c.getCentre().getPointOnCircumference(d / 2 - th - 6, ang);
    g.setColour(pal::paper); g.drawLine(Line<float>(c.getCentre().getPointOnCircumference(d * 0.12f, ang), p), 2.0f);
}

void PrismaLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int w, int h, float pos, float, float, Slider::SliderStyle, Slider&) {
    float cy = y + h * 0.5f;
    g.setColour(pal::line); g.fillRoundedRectangle((float) x, cy - 2, (float) w, 4, 2);
    g.setColour(pal::glow); g.fillRoundedRectangle((float) x, cy - 2, pos - x, 4, 2);
    g.setColour(pal::paper); g.fillEllipse(pos - 7, cy - 7, 14, 14);
}

void PrismaLookAndFeel::drawToggleButton(Graphics& g, ToggleButton& b, bool hl, bool) {
    auto r = b.getLocalBounds().toFloat().reduced(1);
    bool on = b.getToggleState();
    g.setColour(on ? pal::paper : (hl ? pal::line.brighter(0.2f) : pal::deep)); g.fillRoundedRectangle(r, r.getHeight() / 2);
    g.setColour(on ? pal::paper : pal::line); g.drawRoundedRectangle(r, r.getHeight() / 2, 1);
    g.setColour(on ? pal::ink : pal::paper); g.setFont(sans(15.0f, true)); g.drawText(b.getButtonText(), r, Justification::centred);
}

void PrismaLookAndFeel::drawButtonBackground(Graphics& g, Button& b, const Colour&, bool hl, bool down) {
    auto r = b.getLocalBounds().toFloat().reduced(1);
    bool accent = b.getProperties()["accent"];
    g.setColour(accent ? (down ? pal::glow.darker(0.2f) : pal::glow) : (down ? pal::line : hl ? pal::line.withAlpha(0.6f) : pal::deep));
    g.fillRoundedRectangle(r, r.getHeight() / 2);
    g.setColour(accent ? pal::glow : pal::line); g.drawRoundedRectangle(r, r.getHeight() / 2, 1);
}

// ------------------------------------------------------------------ forma d'onda disegnabile
void WaveView::paint(Graphics& g) {
    auto r = getLocalBounds().toFloat();
    g.setColour(pal::deep); g.fillRoundedRectangle(r, 6);
    g.setColour(pal::line); g.drawRoundedRectangle(r.reduced(0.5f), 6, 1);
    g.setColour(pal::line.withAlpha(0.6f)); g.drawHorizontalLine((int) r.getCentreY(), r.getX() + 8, r.getRight() - 8);
    for (int i = 1; i < 4; ++i) g.drawVerticalLine((int) (r.getX() + r.getWidth() * i / 4), r.getY() + 8, r.getBottom() - 8);
    auto a = r.reduced(10, 14); Path p; const int N = (int) cyc.size();
    for (int i = 0; i <= N; ++i) { float x = a.getX() + a.getWidth() * i / N, y = a.getCentreY() - cyc[(size_t) (i % N)] * a.getHeight() * 0.5f; i ? p.lineTo(x, y) : p.startNewSubPath(x, y); }
    g.setColour(colour.withAlpha(0.25f)); g.strokePath(p, PathStrokeType(6.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(colour); g.strokePath(p, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(pal::muted); g.setFont(PrismaLookAndFeel::mono(12.0f));
    g.drawText(drawnModel ? U("FORMA DISEGNATA · trascina per modificare") : U("UN CICLO · trascina per disegnare una forma tua"), r.reduced(12, 6), Justification::topLeft);
}
void WaveView::apply(const MouseEvent& e) {
    auto a = getLocalBounds().toFloat().reduced(10, 14); const int N = (int) cyc.size();
    int idx = jlimit(0, N - 1, (int) ((e.position.x - a.getX()) / a.getWidth() * N));
    float val = jlimit(-1.0f, 1.0f, (a.getCentreY() - e.position.y) / (a.getHeight() * 0.5f));
    if (lastIdx < 0) lastIdx = idx, lastVal = val;
    int i0 = jmin(lastIdx, idx), i1 = jmax(lastIdx, idx);
    for (int i = i0; i <= i1; ++i) { float t = i1 == i0 ? 1.0f : float(i - lastIdx) / float(idx - lastIdx); cyc[(size_t) i] = lastVal + (val - lastVal) * jlimit(0.0f, 1.0f, t); }
    lastIdx = idx; lastVal = val; repaint();
    auto now = Time::getMillisecondCounter(); if (onEdit && now - lastSend > 60) { lastSend = now; onEdit(cyc, false); }
}
void WaveView::mouseDown(const MouseEvent& e) { if (!drawnModel && onStartDraw) onStartDraw(); lastIdx = -1; apply(e); }
void WaveView::mouseDrag(const MouseEvent& e) { apply(e); }
void WaveView::mouseUp(const MouseEvent&) { lastIdx = -1; if (onEdit) onEdit(cyc, true); }

// ------------------------------------------------------------------ armonici
void HarmView::paint(Graphics& g) {
    auto r = getLocalBounds().toFloat();
    g.setColour(pal::deep); g.fillRoundedRectangle(r, 6); g.setColour(pal::line); g.drawRoundedRectangle(r.reduced(0.5f), 6, 1);
    auto a = r.reduced(10, 22).withTrimmedBottom(-8); float mx = 1e-9f;
    for (int k = 0; k < table.count; ++k) mx = jmax(mx, table.p[(size_t) k].amp);
    auto hOf = [&](float amp) { float db = amp > 0 ? 20.0f * std::log10(amp / mx) : -60.0f; return jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f); };
    g.setFont(PrismaLookAndFeel::mono(11.0f));
    if (editable) {
        float bw = a.getWidth() / prisma::kMaxPartials;
        for (int k = 0; k < table.count; ++k) {
            float h = hOf(table.p[(size_t) k].amp) * a.getHeight(); int idx = (int) std::lround(table.p[(size_t) k].ratio) - 1; if (idx < 0 || idx >= prisma::kMaxPartials) continue;
            g.setColour(colour.withAlpha(0.35f + 0.65f * h / a.getHeight())); g.fillRect(a.getX() + idx * bw + 1, a.getBottom() - h, bw - 2, h);
        }
        g.setColour(pal::muted); g.drawText(U("ARMONICI 1–64 (scala −60…0 dB) · trascina per cambiarli"), r.reduced(12, 4), Justification::topLeft);
    } else {
        auto X = [&](float ratio) { return a.getX() + a.getWidth() * std::log2(jmax(1.0f, ratio)) / 6.0f; }; // 1…64 in scala logaritmica
        for (int o = 0; o <= 6; ++o) { g.setColour(pal::line); g.drawVerticalLine((int) X(std::pow(2.0f, (float) o)), a.getY(), a.getBottom());
            g.setColour(pal::muted); g.drawText(String((int) std::pow(2, o)) + U("×"), (int) X(std::pow(2.0f, (float) o)) + 2, (int) a.getBottom() - 12, 30, 12, Justification::left); }
        for (int k = 0; k < table.count; ++k) { const auto& p = table.p[(size_t) k]; float h = hOf(p.amp) * a.getHeight();
            g.setColour(colour.withAlpha(0.4f + 0.6f * h / a.getHeight())); g.fillRect(X(p.ratio) - 1.5f, a.getBottom() - h, 3.0f, h); }
        g.setColour(pal::muted); g.drawText(U("PARZIALI (rapporti in scala logaritmica) · modello inarmonico: usa «Copia nel disegno» per modificarlo"), r.reduced(12, 4), Justification::topLeft);
    }
}
void HarmView::edit(const MouseEvent& e) {
    if (!editable || !onEdit) return;
    auto a = getLocalBounds().toFloat().reduced(10, 22).withTrimmedBottom(-8);
    int k = jlimit(0, prisma::kMaxPartials - 1, (int) ((e.position.x - a.getX()) / a.getWidth() * prisma::kMaxPartials));
    float h = jlimit(0.0f, 1.0f, (a.getBottom() - e.position.y) / a.getHeight());
    float mx = 1e-9f; for (int i = 0; i < table.count; ++i) mx = jmax(mx, table.p[(size_t) i].amp);
    onEdit(k, h <= 0.01f ? 0.0f : mx * std::pow(10.0f, (h * 60.0f - 60.0f) / 20.0f));
}

// ------------------------------------------------------------------ oscilloscopio
void ScopeView::paint(Graphics& g) {
    auto r = getLocalBounds().toFloat();
    g.setColour(pal::deep); g.fillRoundedRectangle(r, 6); g.setColour(pal::line); g.drawRoundedRectangle(r.reduced(0.5f), 6, 1);
    auto xy = r.removeFromLeft(r.getHeight()).reduced(10); auto tw = r.reduced(10);
    const int W = proc.scopeWrite.load(), M = PrismaProcessor::kScope - 1;
    float pk = 1e-4f; for (int i = 0; i < 4096; ++i) { int j = (W - 1 - i) & M; pk = jmax(pk, std::abs(proc.scopeL[j]), std::abs(proc.scopeR[j])); }
    const float G = 0.92f / pk;
    g.setColour(pal::line); g.drawHorizontalLine((int) xy.getCentreY(), xy.getX(), xy.getRight()); g.drawVerticalLine((int) xy.getCentreX(), xy.getY(), xy.getBottom());
    int nXY = jlimit(256, 4096, (int) (48000.0 / jmax(20.0, refHz) * 2.2));
    Path p; for (int i = 0; i < nXY; ++i) { int j = (W - nXY + i) & M; float x = xy.getCentreX() + proc.scopeL[j] * G * xy.getWidth() * 0.5f, y = xy.getCentreY() - proc.scopeR[j] * G * xy.getHeight() * 0.5f; i ? p.lineTo(x, y) : p.startNewSubPath(x, y); }
    g.setColour(colour.withAlpha(0.25f)); g.strokePath(p, PathStrokeType(4.0f)); g.setColour(colour); g.strokePath(p, PathStrokeType(1.3f));
    // forma nel tempo con trigger sul passaggio per lo zero in salita
    int span = jlimit(256, 3000, (int) (48000.0 / jmax(20.0, refHz) * 3)), start = (W - span - 1) & M;
    for (int i = 0; i < 2000; ++i) { int j0 = (W - span - 2000 + i) & M, j1 = (j0 + 1) & M; if (proc.scopeL[j0] <= 0 && proc.scopeL[j1] > 0) start = j1; }
    g.setColour(pal::line); g.drawHorizontalLine((int) tw.getCentreY(), tw.getX(), tw.getRight());
    Path q; for (int i = 0; i < span; ++i) { int j = (start + i) & M; float x = tw.getX() + tw.getWidth() * i / span, y = tw.getCentreY() - proc.scopeL[j] * G * tw.getHeight() * 0.5f; i ? q.lineTo(x, y) : q.startNewSubPath(x, y); }
    g.setColour(colour.withAlpha(0.25f)); g.strokePath(q, PathStrokeType(4.0f)); g.setColour(colour); g.strokePath(q, PathStrokeType(1.3f));
    g.setColour(pal::muted); g.setFont(PrismaLookAndFeel::mono(11.0f));
    g.drawText(U("XY  L→X  R→Y"), xy.toNearestInt().translated(0, -4), Justification::topLeft);
    g.drawText("USCITA SINISTRA", tw.toNearestInt().translated(0, -4), Justification::topLeft);
}

// ------------------------------------------------------------------ editor
PrismaEditor::Knob* PrismaEditor::makeKnob(const char* id, const char* label, bool linear) {
    auto k = std::make_unique<Knob>();
    k->s.setSliderStyle(linear ? Slider::LinearHorizontal : Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle(linear ? Slider::TextBoxRight : Slider::TextBoxBelow, false, linear ? 86 : 70, 18);
    k->s.setColour(Slider::textBoxTextColourId, pal::paper);
    k->l.setText(U(label), dontSendNotification); k->l.setJustificationType(linear ? Justification::centredLeft : Justification::centred);
    k->l.setFont(PrismaLookAndFeel::sans(14.0f)); k->l.setColour(Label::textColourId, pal::muted);
    addAndMakeVisible(k->s); addAndMakeVisible(k->l);
    k->a = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, id, k->s);
    k->s.setLookAndFeel(&lnf);
    knobs.push_back(std::move(k)); return knobs.back().get();
}

PrismaEditor::PrismaEditor(PrismaProcessor& p) : AudioProcessorEditor(&p), proc(p), scope(p) {
    setLookAndFeel(&lnf);

    kFixed = makeKnob("fixedHz", "Frequenza", true); kA4 = makeKnob("a4", "Diapason La", true);
    kTilt = makeKnob("tilt", "Inclinazione"); kInharm = makeKnob("inharm", "Inarmonicità"); kBeat = makeKnob("beatHz", "Battimento Hz");
    kBreath = makeKnob("breath", "Respiro 0,1 Hz"); kTremR = makeKnob("tremRate", "Tremolo Hz"); kTremD = makeKnob("tremDepth", "Tremolo");
    kNoise = makeKnob("noiseDb", "Rumore dB"); kFrom = makeKnob("sweepFrom", "Da Hz"); kTo = makeKnob("sweepTo", "A Hz"); kMin = makeKnob("sweepMin", "Minuti");
    kA = makeKnob("attack", "Attacco"); kD = makeKnob("decay", "Decadim."); kS = makeKnob("sustain", "Sostegno"); kR = makeKnob("release", "Rilascio"); kLevel = makeKnob("level", "Livello dB");

    auto combo = [this](ComboBox& c, const char* id, const StringArray& items) {
        c.addItemList(items, 1); addAndMakeVisible(c);
        if (id) comboAtt.push_back(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, id, c)); };
    StringArray models; for (int m = 0; m < prisma::NumModels; ++m) models.add(U(prisma::modelName(m)));
    combo(modeBox, "mode", {"Note MIDI", "Hz fisso"});
    combo(modelBox, "model", models);
    combo(beatBox, "beatMode", {"Battimento spento", "Binaurale (cuffie)", "Monoaurale (casse)", "Isocronico"});
    combo(noiseBox, "noiseType", {"Rumore spento", "Bianco", "Rosa", "Marrone", "Grigio", "Blu"});
    combo(dirBox, "sweepDir", {"Sale", "Scende", "Sale e scende"});

    // tabella delle 21 frequenze verificate
    freqBox.setTextWhenNothingSelected(U("Frequenze verificate (21) · livello di evidenza"));
    int id = 1; for (const auto& f : prisma::kFrequencies)
        freqBox.addItem(num(f.hz, f.hz < 10 ? 2 : 0) + U(" Hz · ") + U(f.name) + U(" · ") + f.level, id++);
    addAndMakeVisible(freqBox);
    freqBox.onChange = [this] {
        int i = freqBox.getSelectedItemIndex(); if (i < 0) return; const auto& f = prisma::kFrequencies[i];
        if (f.hz < 0.5f) proc.setParam("breath", 0.6f);                                   // 0,1 Hz = respiro guidato
        else if (f.hz < 30.0f && f.hz != 20.0f) { proc.setParam("beatHz", f.hz); if (pv("beatMode") < 0.5f) proc.setParam("beatMode", 1); proc.setParam("mode", 1); if (pv("fixedHz") < 60.0f) proc.setParam("fixedHz", 200.0f); }
        else { proc.setParam("mode", 1); proc.setParam("fixedHz", f.hz); }
        readInfo = String(f.code) + U(" · livello ") + f.level + U(" · ") + U(f.info);
    };

    // libreria strumenti con frequenze caratteristiche
    libBox.setTextWhenNothingSelected(U("Libreria strumenti · frequenze caratteristiche"));
    { String grp; int i = 1; for (const auto& it : prisma::kLibrary) { if (grp != U(it.group)) { grp = U(it.group); libBox.addSectionHeading(grp); }
        libBox.addItem(U(it.name) + U(" · ") + num(it.hz, 2) + U(" Hz · ") + it.level, i++); } }
    addAndMakeVisible(libBox);
    libBox.onChange = [this] {
        int sel = libBox.getSelectedId(); if (sel <= 0) return; const auto& it = prisma::kLibrary[sel - 1];
        proc.setParam("mode", 1); proc.setParam("model", (float) it.model); proc.setParam("fixedHz", it.hz);
        readInfo = U(it.group) + U(" · ") + U(it.name) + U(" · livello ") + it.level;
        auto t = prisma::makeModel(it.model); if (t.percussive) proc.requestStrike();
    };

    for (auto* b : {&droneBtn, &sweepBtn, &loopBtn}) addAndMakeVisible(*b);
    btnAtt.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, "drone", droneBtn));
    btnAtt.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, "sweepOn", sweepBtn));
    btnAtt.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, "sweepLoop", loopBtn));

    strikeBtn.getProperties().set("accent", true); strikeBtn.setColour(TextButton::textColourOffId, pal::ink);
    for (auto* b : {&strikeBtn, &a440Btn, &a432Btn, &copyBtn, &smoothBtn, &sineBtn, &invertBtn}) addAndMakeVisible(*b);
    strikeBtn.onClick = [this] { proc.requestStrike(); };
    a440Btn.onClick = [this] { proc.setParam("a4", 440.0f); };
    a432Btn.onClick = [this] { proc.setParam("a4", 432.0f); };

    auto toDrawn = [this](std::vector<float> c) {   // la forma corrente diventa il disegno; inclinazione e inarmonicità sono già incluse
        proc.setUserCycle(c); proc.setParam("tilt", 0.0f); proc.setParam("inharm", 0.0f); proc.setParam("model", (float) prisma::Drawn); refreshWave(true); };
    copyBtn.onClick = [this, toDrawn] { toDrawn(currentModelCycle()); };
    smoothBtn.onClick = [this, toDrawn] { auto c = proc.userCycle; const int N = (int) c.size(); std::vector<float> o(c.size());
        for (int i = 0; i < N; ++i) { float s = 0; for (int d = -3; d <= 3; ++d) s += c[(size_t) ((i + d + N) % N)]; o[(size_t) i] = s / 7.0f; } toDrawn(o); };
    sineBtn.onClick = [toDrawn] { std::vector<float> c(256); for (int i = 0; i < 256; ++i) c[(size_t) i] = std::sin(MathConstants<float>::twoPi * i / 256.0f); toDrawn(c); };
    invertBtn.onClick = [this, toDrawn] { auto c = proc.userCycle; for (auto& v : c) v = -v; toDrawn(c); };

    wave.onStartDraw = [this] { auto c = currentModelCycle(); wave.cyc = c; proc.setParam("tilt", 0.0f); proc.setParam("inharm", 0.0f); proc.setParam("model", (float) prisma::Drawn); wave.drawnModel = true; };
    wave.onEdit = [this](const std::vector<float>& c, bool) { proc.setUserCycle(c); harm.table = prisma::harmonicsFromCycle(c); harm.editable = true; harm.repaint(); };
    harm.onEdit = [this](int k, float amp) {
        if ((int) pv("model") != prisma::Drawn) { proc.setUserCycle(currentModelCycle()); proc.setParam("tilt", 0.0f); proc.setParam("inharm", 0.0f); proc.setParam("model", (float) prisma::Drawn); }
        auto t = prisma::harmonicsFromCycle(proc.userCycle); t.p[(size_t) k].amp = amp; if (t.p[(size_t) k].ratio != float(k + 1)) t.p[(size_t) k].ratio = float(k + 1);
        proc.setUserHarmonics(t); refreshWave(true); };
    addAndMakeVisible(wave); addAndMakeVisible(harm); addAndMakeVisible(scope);

    setSize(1120, 810);
    refreshWave(true);
    startTimerHz(30);
}

PrismaEditor::~PrismaEditor() {
    stopTimer();
    for (auto& k : knobs) k->s.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

std::vector<float> PrismaEditor::currentModelCycle() const {
    int m = (int) pv("model");
    if (m == prisma::Drawn) return proc.userCycle;
    return prisma::cycleFromTable(prisma::makeModel(m), 256, pv("tilt"), pv("inharm"));
}

void PrismaEditor::refreshWave(bool force) {
    int m = (int) pv("model"); float tl = pv("tilt"), ih = pv("inharm");
    size_t h = 0; for (size_t i = 0; i < proc.userCycle.size(); i += 8) h = h * 31 + (size_t) (proc.userCycle[i] * 10000.0f);
    if (!force && m == lastModel && tl == lastTilt && ih == lastInharm && (m != prisma::Drawn || h == lastCycleHash)) return;
    lastModel = m; lastTilt = tl; lastInharm = ih; lastCycleHash = h;
    if (m == prisma::Drawn) { wave.cyc = proc.userCycle; harm.table = prisma::harmonicsFromCycle(proc.userCycle); harm.editable = true; }
    else { auto t = prisma::makeModel(m); wave.cyc = prisma::cycleFromTable(t, 256, tl, ih);
        for (int k = 0; k < t.count; ++k) t.p[(size_t) k].amp *= std::pow(jmax(1.0f, t.p[(size_t) k].ratio), tl / 6.0206f);
        harm.table = t; harm.editable = t.harmonic; }
    wave.drawnModel = (m == prisma::Drawn);
    wave.repaint(); harm.repaint();
}

void PrismaEditor::timerCallback() {
    refreshWave();
    const bool fixed = pv("mode") > 0.5f; const float a4 = pv("a4");
    float hz = fixed ? (pv("sweepOn") > 0.5f ? proc.currentHz.load() : pv("fixedHz")) : 0.0f;
    if (fixed && hz <= 0) hz = pv("fixedHz");
    if (fixed) {
        double lam = 0; readColour = Colour(prisma::octaveColour(hz, &lam));
        readHz = num(hz, hz < 1000 ? 2 : 1) + " Hz";
        readNote = String(prisma::noteName(hz, 440.0)) + U("  ·  luce ") + num(lam, 0) + " nm";
    } else {
        readColour = pal::glow; readHz = "Note MIDI"; readNote = "La = " + num(a4, 2) + " Hz";
    }
    const int bm = (int) pv("beatMode");
    if (fixed && hz < 20.0f && (bm == 0 || bm == 3)) readNote += U("  ·  sub-audio: volume ridotto, usa Binaurale o Monoaurale");
    progress = proc.sweepProgress.load();
    wave.colour = harm.colour = scope.colour = readColour;
    scope.refHz = fixed ? jmax(20.0f, hz) : 220.0;
    scope.repaint(); repaint(headerArea); repaint(secSweep);
}

void PrismaEditor::paint(Graphics& g) {
    g.fillAll(pal::ink);
    // intestazione
    auto h = headerArea.toFloat();
    g.setColour(pal::paper); g.setFont(PrismaLookAndFeel::sans(34.0f, true)); g.drawText("PRISMA SONORO", h.withWidth(250).translated(0, -6), Justification::centredLeft);
    g.setColour(pal::muted); g.setFont(PrismaLookAndFeel::sans(14.0f)); g.drawText(U("MBStudio91 · sintesi additiva per il benessere"), h.withWidth(300).translated(0, 18), Justification::centredLeft);
    auto rd = h.withTrimmedLeft(300);
    g.setColour(readColour); g.fillEllipse(rd.getX(), rd.getCentreY() - 9, 18, 18);
    g.setFont(PrismaLookAndFeel::mono(30.0f)); g.drawText(readHz, rd.withTrimmedLeft(28).withWidth(230), Justification::centredLeft);
    g.setColour(pal::paper); g.setFont(PrismaLookAndFeel::mono(13.0f)); g.drawText(readNote, rd.withTrimmedLeft(270).withHeight(h.getHeight() / 2), Justification::bottomLeft);
    g.setColour(pal::muted); g.drawText(readInfo, rd.withTrimmedLeft(270).withTrimmedTop(h.getHeight() / 2 + 2), Justification::topLeft);
    g.setColour(pal::line); g.drawHorizontalLine((int) h.getBottom() + 4, 16.0f, (float) getWidth() - 16);
    // sezioni
    auto section = [&](Rectangle<int> r, const char* t) {
        g.setColour(pal::panel); g.fillRoundedRectangle(r.toFloat(), 6); g.setColour(pal::line); g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6, 1);
        g.setColour(pal::muted); g.setFont(PrismaLookAndFeel::sans(13.0f, true)); g.drawText(String(t).toUpperCase(), r.reduced(10, 4).withHeight(16), Justification::topLeft); };
    section(secSource, "Sorgente"); section(secTimbre, "Timbro e battimento"); section(secMod, "Respiro, tremolo e rumore");
    section(secSweep, "Sweep logaritmico"); section(secEnv, "Inviluppo e uscita");
    if (progress > 0) { auto pr = secSweep.reduced(12).removeFromBottom(4).toFloat(); g.setColour(pal::deep); g.fillRoundedRectangle(pr, 2); g.setColour(pal::glow); g.fillRoundedRectangle(pr.withWidth(pr.getWidth() * progress), 2); }
    g.setColour(pal::muted); g.setFont(PrismaLookAndFeel::sans(13.0f));
    g.drawText(U("Esperienza d'ascolto per il rilassamento: non sostituisce cure o consigli medici. Binaurale solo in cuffia, volume moderato, mai alla guida. "
               "Livelli: A fatto fisico · B studi con risultati · C studi preliminari · D tradizione."), Rectangle<int>(16, getHeight() - 28, getWidth() - 32, 22), Justification::centredLeft);
}

void PrismaEditor::resized() {
    auto r = getLocalBounds().reduced(16, 10);
    headerArea = r.removeFromTop(58);
    r.removeFromTop(12); r.removeFromBottom(26);
    auto left = r.removeFromLeft(640); r.removeFromLeft(14); auto right = r;

    wave.setBounds(left.removeFromTop(220)); left.removeFromTop(8);
    auto wb = left.removeFromTop(30);
    copyBtn.setBounds(wb.removeFromLeft(170)); wb.removeFromLeft(8); smoothBtn.setBounds(wb.removeFromLeft(90)); wb.removeFromLeft(8);
    sineBtn.setBounds(wb.removeFromLeft(100)); wb.removeFromLeft(8); invertBtn.setBounds(wb.removeFromLeft(90));
    left.removeFromTop(8); harm.setBounds(left.removeFromTop(140)); left.removeFromTop(8); scope.setBounds(left);

    // sorgente
    secSource = right.removeFromTop(214); right.removeFromTop(8);
    auto s = secSource.reduced(10).withTrimmedTop(18);
    auto row = s.removeFromTop(30); modeBox.setBounds(row.removeFromLeft(110)); row.removeFromLeft(6); modelBox.setBounds(row.removeFromLeft(178)); row.removeFromLeft(6); strikeBtn.setBounds(row);
    s.removeFromTop(6); freqBox.setBounds(s.removeFromTop(28)); s.removeFromTop(6); libBox.setBounds(s.removeFromTop(28)); s.removeFromTop(6);
    row = s.removeFromTop(30); kFixed->l.setBounds(row.removeFromLeft(70)); droneBtn.setBounds(row.removeFromRight(96)); row.removeFromRight(6); kFixed->s.setBounds(row);
    s.removeFromTop(4);
    row = s.removeFromTop(30); kA4->l.setBounds(row.removeFromLeft(70)); a432Btn.setBounds(row.removeFromRight(46)); row.removeFromRight(4); a440Btn.setBounds(row.removeFromRight(46)); row.removeFromRight(6); kA4->s.setBounds(row);

    auto knobRow = [](Rectangle<int> area, std::initializer_list<Knob*> ks) {
        const int w = 70; for (auto* k : ks) { auto c = area.removeFromLeft(w); k->l.setBounds(c.removeFromTop(16)); k->s.setBounds(c); area.removeFromLeft(4); } return area; };
    const int secH = 112;
    secTimbre = right.removeFromTop(secH); right.removeFromTop(8);
    { auto a = secTimbre.reduced(10).withTrimmedTop(16); auto rest = knobRow(a, {kTilt, kInharm, kBeat}); beatBox.setBounds(rest.withSizeKeepingCentre(rest.getWidth() - 4, 28)); }
    secMod = right.removeFromTop(secH); right.removeFromTop(8);
    { auto a = secMod.reduced(10).withTrimmedTop(16); auto rest = knobRow(a, {kBreath, kTremR, kTremD, kNoise}); noiseBox.setBounds(rest.withSizeKeepingCentre(rest.getWidth() - 4, 28)); }
    secSweep = right.removeFromTop(secH); right.removeFromTop(8);
    { auto a = secSweep.reduced(10).withTrimmedTop(16).withTrimmedBottom(6); auto rest = knobRow(a, {kFrom, kTo, kMin});
      auto c = rest.reduced(2, 4); auto r1 = c.removeFromTop(28); sweepBtn.setBounds(r1.removeFromLeft(r1.getWidth() / 2 - 3)); r1.removeFromLeft(6); loopBtn.setBounds(r1);
      c.removeFromTop(8); dirBox.setBounds(c.removeFromTop(28)); }
    secEnv = right.removeFromTop(secH);
    { auto a = secEnv.reduced(10).withTrimmedTop(16); knobRow(a, {kA, kD, kS, kR, kLevel}); }
}
