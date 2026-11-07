// PrismaDSP.h — motore audio di Prisma Sonoro (C++17, senza dipendenze esterne)
// MBStudio91 · Piedmont Electronic Sound
//
// Sintesi additiva fino a 64 parziali, ognuna con rapporto, ampiezza, fase, attacco,
// decadimento, doppietto (due seni vicinissimi che battono) e modulazione d'ampiezza lenta.
// Essendo additiva, ogni forma d'onda è priva di aliasing: le parziali sopra 0,45·fs vengono escluse.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace prisma {

constexpr int    kMaxPartials = 64;
constexpr int    kMaxOsc      = 128;   // un doppietto occupa due oscillatori
constexpr int    kMaxVoices   = 8;
constexpr int    kSub         = 16;    // campioni per sotto-blocco di controllo
constexpr double kTwoPi       = 6.283185307179586476925;

// ---------------------------------------------------------------- parziali e modelli
struct Partial {
    float ratio   = 1.0f;  // rispetto alla fondamentale
    float amp     = 0.0f;  // lineare, prima della normalizzazione
    float phase   = 0.0f;  // radianti (base seno)
    float attack  = 0.004f;// secondi, rampa lineare
    float decay   = 0.0f;  // secondi a −60 dB; 0 = sostenuto
    float split   = 0.0f;  // doppietto: scarto relativo (frazione della frequenza della parziale)
    float amRate  = 0.0f;  // Hz, modulazione d'ampiezza lenta della singola parziale
    float amDepth = 0.0f;  // 0..1
};

struct PartialTable {
    std::array<Partial, kMaxPartials> p {};
    int  count      = 0;
    bool percussive = false;   // la voce si spegne da sola quando tutte le parziali sono decadute
    bool harmonic   = true;    // rapporti interi: il ciclo singolo è rappresentabile
};

enum Model : int {
    Sine = 0, Saw, Square, Triangle, Drawn,
    TibetanBowl, CrystalBowl, Gong, Vibraphone, TuningFork,
    Didgeridoo, Monochord, ShrutiBox, LakhovskyRings,
    NumModels
};

inline const char* modelName(int m) {
    static const char* names[] = {
        "Sinusoide", "Dente di sega", "Quadra", "Triangolo", "Forma disegnata",
        "Campana tibetana", "Campana di cristallo", "Gong", "Vibrafono", "Diapason",
        "Didgeridoo", "Monocordo", "Shruti box", "Anelli di Lakhovsky" };
    return (m >= 0 && m < NumModels) ? names[m] : "?";
}

// generatore pseudo-casuale deterministico (stessi modelli a ogni caricamento)
struct Lcg { uint32_t s; explicit Lcg(uint32_t seed) : s(seed) {}
    float next() { s = s * 1664525u + 1013904223u; return (s >> 8) * (1.0f / 16777216.0f); }
    float range(float a, float b) { return a + (b - a) * next(); } };

inline void add(PartialTable& t, Partial p) { if (t.count < kMaxPartials) t.p[t.count++] = p; }

// Costruisce la tabella di parziali di un modello.
// Fonti: rapporti delle campane tibetane ≈ 2,95× e 5,7× (misure NPUST); doppietti di pochi Hz (Terwagne & Bush);
// vibrafono 1:4:10; diapason con modo "clang" ≈ 6,27×; didgeridoo: formanti 500 / 1500 / 2600 Hz (J. Wolfe, UNSW).
inline PartialTable makeModel(int model) {
    PartialTable t;
    const float pi = 3.14159265f;
    switch (model) {
    case Sine: add(t, {1, 1}); break;
    case Saw:      for (int k = 1; k <= kMaxPartials; ++k) add(t, {float(k), 1.0f / k, (k % 2) ? 0.0f : pi}); break;
    case Square:   for (int k = 1; k <= kMaxPartials * 2 - 1 && t.count < kMaxPartials; k += 2) add(t, {float(k), 1.0f / k}); break;
    case Triangle: for (int k = 1; k <= kMaxPartials * 2 - 1 && t.count < kMaxPartials; k += 2) add(t, {float(k), 1.0f / (k * k), ((k / 2) % 2) ? pi : 0.0f}); break;
    case Drawn: add(t, {1, 1}); break; // sostituita dalla forma dell'utente
    case TibetanBowl:
        t.percussive = true; t.harmonic = false;
        add(t, {1.00f, 1.00f, 0, 0.006f, 40, 0.008f});
        add(t, {2.95f, 0.45f, 0, 0.006f, 18, 0.010f});
        add(t, {5.70f, 0.20f, 0, 0.006f,  8, 0.012f});
        add(t, {12.6f, 0.05f, 0, 0.002f, 0.12f});              // colpo del battente
        break;
    case CrystalBowl:
        t.harmonic = false;
        add(t, {1.0f, 1.00f, 0, 3.5f, 0, 0.0006f});
        add(t, {2.0f, 0.04f, 0, 3.5f, 0});
        add(t, {3.0f, 0.012f, 0, 3.5f, 0});
        break;
    case Gong: {
        t.percussive = true; t.harmonic = false; Lcg r(7);
        add(t, {1.0f, 0.42f, 0, 0.015f, 32});
        for (int k = 0; k < 40; ++k) {
            float ratio = std::pow(2.0f, 0.4f + r.next() * 5.2f);
            float bloom = ratio > 5 ? r.range(0.5f, 2.6f) : 0.02f;   // gli acuti "sbocciano" dopo il colpo
            add(t, {ratio, 0.3f * std::pow(1.0f / ratio, 0.5f) * r.range(0.5f, 1.0f), r.range(0, 2 * pi), bloom,
                    r.range(8, 26) * std::pow(1.0f / ratio, 0.25f)});
        }
        break; }
    case Vibraphone:
        t.percussive = true; t.harmonic = false;
        add(t, {1,  0.42f, 0, 0.002f, 6.0f});
        add(t, {4,  0.11f, 0, 0.002f, 1.8f});
        add(t, {10, 0.03f, 0, 0.002f, 0.6f});
        break;
    case TuningFork:
        t.percussive = true; t.harmonic = false;
        add(t, {1.00f, 0.45f, 0, 0.002f, 15});
        add(t, {6.27f, 0.08f, 0, 0.001f, 0.25f});
        break;
    case Didgeridoo: {
        // dente di sega a 73,4 Hz (Re2) pesato da tre formanti della bocca; AM lente = vocali che cambiano
        const float f0 = 73.42f; const float F[3] = {500, 1500, 2600}, B[3] = {130, 220, 320}, G[3] = {1.0f, 0.55f, 0.3f};
        Lcg r(11);
        for (int k = 1; k <= 48; ++k) {
            float f = f0 * k, w = 0.25f;
            for (int i = 0; i < 3; ++i) { float d = (f - F[i]) / B[i]; w += G[i] * std::exp(-0.5f * d * d) * 2.2f; }
            bool near = w > 0.6f;
            add(t, {float(k), w / k, (k % 2) ? 0.0f : pi, 0.04f, 0, 0, near ? r.range(0.3f, 0.9f) : 0, near ? 0.35f : 0});
        }
        break; }
    case Monochord: {
        // corda pizzicata a 1/8 della lunghezza: |sin(kπβ)|/k²; gli armonici alti si spengono prima; doppietti = molte corde
        t.percussive = true;
        for (int k = 1; k <= 40; ++k) {
            float a = std::fabs(std::sin(k * pi * 0.125f)) / (k * 0.7f + 0.3f);
            add(t, {float(k), a, 0, 0.002f, 9.0f / (1.0f + 0.15f * k), 0.0015f});
        }
        break; }
    case ShrutiBox: {
        const float f0 = 146.8f; // ance: dente di sega con passa-basso morbido a 1,4 kHz, mantice lento
        for (int k = 1; k <= 40; ++k) {
            float f = f0 * k, roll = 1.0f / (1.0f + (f / 1400.0f) * (f / 1400.0f));
            add(t, {float(k), roll / k, (k % 2) ? 0.0f : pi, 0.25f, 0, 0, 0.13f, 0.22f});
        }
        break; }
    case LakhovskyRings: {
        // 7 anelli: l'esterno suona la base, l'anello k-esimo base·7/k; ognuno con un respiro lento proprio
        t.harmonic = false; const float rates[7] = {.031f, .047f, .059f, .071f, .083f, .097f, .113f};
        for (int k = 7; k >= 1; --k) { float ratio = 7.0f / k; add(t, {ratio, 0.5f / ratio, 0, 1.2f, 0, 0, rates[7 - k], 0.9f}); }
        break; }
    default: add(t, {1, 1}); break;
    }
    return t;
}

// Forma d'onda a ciclo singolo -> 64 armonici (ampiezza e fase), con DFT diretta.
inline PartialTable harmonicsFromCycle(const std::vector<float>& cyc) {
    PartialTable t; const int N = (int) cyc.size(); if (N < 8) { add(t, {1, 1}); return t; }
    double mean = 0; for (float v : cyc) mean += v; mean /= N;
    for (int k = 1; k <= kMaxPartials; ++k) {
        double s = 0, c = 0;
        for (int n = 0; n < N; ++n) { double th = kTwoPi * k * n / N; s += (cyc[n] - mean) * std::sin(th); c += (cyc[n] - mean) * std::cos(th); }
        s *= 2.0 / N; c *= 2.0 / N;
        add(t, {float(k), float(std::sqrt(s * s + c * c)), float(std::atan2(c, s))});
    }
    return t;
}

// Ciclo singolo dalla tabella (per il disegno). Per i modelli inarmonici mostra un periodo della fondamentale.
inline std::vector<float> cycleFromTable(const PartialTable& t, int N, float tiltDb = 0, float inharm = 0) {
    std::vector<float> out(N, 0.0f); float pk = 1e-9f;
    for (int n = 0; n < N; ++n) {
        double th = kTwoPi * n / N, v = 0;
        for (int k = 0; k < t.count; ++k) {
            const auto& p = t.p[k]; double r = p.ratio * std::sqrt(1.0 + inharm * p.ratio * p.ratio);
            v += p.amp * std::pow(std::max(1.0f, p.ratio), tiltDb / 6.0206f) * std::sin(r * th + p.phase);
        }
        out[n] = (float) v; pk = std::max(pk, (float) std::fabs(v));
    }
    for (auto& v : out) v /= pk;
    return out;
}

// ---------------------------------------------------------------- parametri
struct Params {
    int   mode = 0;            // 0 = note MIDI, 1 = Hz fisso
    float fixedHz = 528.0f;
    bool  droneOn = false;
    float a4 = 440.0f;
    int   model = Sine;
    float tiltDb = 0.0f;       // inclinazione spettrale in dB/ottava
    float inharm = 0.0f;       // inarmonicità tipo corda: r' = r·√(1 + B·r²)
    int   beatMode = 0;        // 0 spento, 1 binaurale, 2 monoaurale, 3 isocronico
    float beatHz = 6.0f;
    float breathDepth = 0.0f;  // respiro guidato a 0,1 Hz (6 respiri/min)
    float tremRate = 0.0f, tremDepth = 0.0f; // tremolo (motore del vibrafono)
    int   noiseType = 0;       // 0 spento, 1 bianco, 2 rosa, 3 marrone, 4 grigio, 5 blu
    float noiseDb = -30.0f;
    bool  sweepOn = false; float sweepFrom = 55, sweepTo = 880, sweepMin = 5; int sweepDir = 0; bool sweepLoop = false;
    float attack = 0.01f, decay = 0.3f, sustain = 1.0f, release = 1.5f;
    float levelDb = -6.0f;
};

// ---------------------------------------------------------------- filtri di supporto
struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    float process(float x) { double y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return (float) y; }
    // RBJ cookbook
    void shelf(bool low, double sr, double f, double gainDb) {
        double A = std::pow(10, gainDb / 40), w = kTwoPi * f / sr, c = std::cos(w), s = std::sin(w), al = s / 2 * std::sqrt(2.0), sA = 2 * std::sqrt(A) * al;
        double B0, B1, B2, A0, A1, A2;
        if (low) { B0 = A*((A+1)-(A-1)*c+sA); B1 = 2*A*((A-1)-(A+1)*c); B2 = A*((A+1)-(A-1)*c-sA); A0 = (A+1)+(A-1)*c+sA; A1 = -2*((A-1)+(A+1)*c); A2 = (A+1)+(A-1)*c-sA; }
        else     { B0 = A*((A+1)+(A-1)*c+sA); B1 = -2*A*((A-1)+(A+1)*c); B2 = A*((A+1)+(A-1)*c-sA); A0 = (A+1)-(A-1)*c+sA; A1 = 2*((A-1)-(A+1)*c); A2 = (A+1)-(A-1)*c-sA; }
        set(B0, B1, B2, A0, A1, A2); }
    void peak(double sr, double f, double q, double gainDb) {
        double A = std::pow(10, gainDb / 40), w = kTwoPi * f / sr, al = std::sin(w) / (2 * q), c = std::cos(w);
        set(1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A); }
    void set(double B0, double B1, double B2, double A0, double A1, double A2) { b0 = B0/A0; b1 = B1/A0; b2 = B2/A0; a1 = A1/A0; a2 = A2/A0; }
};

struct NoiseGen {
    uint32_t s = 22222u; double b[7] {}; double brown = 0, prev = 0; Biquad g1, g2, g3;
    void prepare(double sr, uint32_t seed) { s = seed; g1.shelf(true, sr, 250, 10); g2.peak(sr, 3500, 0.9, -9); g3.shelf(false, sr, 9000, 4); }
    float white() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s * (1.0f / 4294967296.0f)) * 2.0f - 1.0f; }
    float next(int type) {
        float w = white();
        switch (type) {
        case 1: return w * 0.5f;
        case 2: { b[0]=0.99886*b[0]+w*0.0555179; b[1]=0.99332*b[1]+w*0.0750759; b[2]=0.96900*b[2]+w*0.1538520;
                  b[3]=0.86650*b[3]+w*0.3104856; b[4]=0.55000*b[4]+w*0.5329522; b[5]=-0.7616*b[5]-w*0.0168980;
                  double p = b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+w*0.5362; b[6] = w*0.115926; return float(p * 0.11); }
        case 3: { brown = (brown + 0.02 * w) / 1.02; return float(brown * 3.2); }
        case 4: return g3.process(g2.process(g1.process(w * 0.16f)));   // grigio: curva inversa di uguale intensità (approssimata)
        case 5: { float d = float((w - prev) * 0.35); prev = w; return d; }
        default: return 0.0f;
        }
    }
};

// ---------------------------------------------------------------- banco di oscillatori (un insieme)
struct OscSet {
    int n = 0;
    alignas(32) float re[kMaxOsc] {}, im[kMaxOsc] {}, wr[kMaxOsc] {}, wi[kMaxOsc] {}, g[kMaxOsc] {}, dg[kMaxOsc] {};
    float ratio[kMaxOsc] {}, amp[kMaxOsc] {}, mask[kMaxOsc] {}, att[kMaxOsc] {}, dec[kMaxOsc] {}, amR[kMaxOsc] {}, amD[kMaxOsc] {};
    float offsetHz = 0.0f;

    void build(const PartialTable& t, float tiltDb, float inharm, float norm, bool keepPhase) {
        int m = 0;
        for (int k = 0; k < t.count && m < kMaxOsc - 1; ++k) {
            const Partial& p = t.p[k]; if (p.amp <= 0.0f) continue;
            float r = p.ratio * std::sqrt(1.0f + inharm * p.ratio * p.ratio);
            float a = p.amp * std::pow(std::max(1.0f, p.ratio), tiltDb / 6.0206f) * norm;
            auto put = [&](float rr, float aa, float ph) {
                if (!keepPhase || m >= n) { re[m] = std::cos(ph); im[m] = std::sin(ph); g[m] = 0; }
                ratio[m] = rr; amp[m] = aa; att[m] = std::max(0.001f, p.attack); dec[m] = p.decay; amR[m] = p.amRate; amD[m] = p.amDepth; ++m; };
            if (p.split > 0.0f) { put(r * (1 - p.split * 0.5f), a * 0.5f, p.phase); put(r * (1 + p.split * 0.5f), a * 0.5f, p.phase); }
            else put(r, a, p.phase);
        }
        for (int k = std::max(0, m); k < std::min(n, kMaxOsc); ++k) g[k] = 0;
        n = m;
    }
    void setFreq(double f0, double sr) {
        for (int k = 0; k < n; ++k) {
            double f = f0 * ratio[k] + offsetHz, w = kTwoPi * f / sr;
            wr[k] = (float) std::cos(w); wi[k] = (float) std::sin(w);
            mask[k] = (f > 0.45 * sr || f <= 0) ? 0.0f : 1.0f;   // niente aliasing: sopra 0,45·fs la parziale tace
        }
    }
    // obiettivi di guadagno al prossimo confine di sotto-blocco
    void targets(double t, double sr) {
        for (int k = 0; k < n; ++k) {
            double e = std::min(1.0, t / att[k]);
            if (dec[k] > 0) e *= std::exp(-6.9078 * t / dec[k]);
            if (amD[k] > 0) e *= 1.0 - amD[k] * (0.5 - 0.5 * std::cos(kTwoPi * amR[k] * t));
            dg[k] = (float) ((amp[k] * mask[k] * e - g[k]) / kSub);
        }
        // rinormalizza le rotazioni (correzione al primo ordine)
        for (int k = 0; k < n; ++k) { float m2 = re[k] * re[k] + im[k] * im[k]; float c = 1.5f - 0.5f * m2; re[k] *= c; im[k] *= c; }
        (void) sr;
    }
    // somma un sotto-blocco in out[0..len)
    void render(float* out, int len) {
        for (int s = 0; s < len; ++s) {
            float acc = 0;
            for (int k = 0; k < n; ++k) {
                float nr = re[k] * wr[k] - im[k] * wi[k];
                float ni = re[k] * wi[k] + im[k] * wr[k];
                re[k] = nr; im[k] = ni; g[k] += dg[k]; acc += g[k] * ni;
            }
            out[s] += acc;
        }
    }
    bool silent(double t) const {   // per modelli percussivi: tutto sotto −80 dB
        for (int k = 0; k < n; ++k) if (dec[k] <= 0 || t < dec[k] * 1.35) return false;
        return true;
    }
};

// ---------------------------------------------------------------- voce
struct Voice {
    bool active = false, held = false, isDrone = false; int note = -1; float vel = 1; uint64_t age = 0;
    double t = 0, f0 = 440; OscSet A, B; bool useB = false; bool percussive = false;
    // ADSR
    int stage = 0; float env = 0; // 1 attacco, 2 decadimento, 3 sostegno, 4 rilascio
    float subGain = 1.0f;

    void start(double freq, float velocity, const PartialTable& tab, const Params& P, float norm, double sr, int beatMode) {
        active = true; held = true; t = 0; f0 = freq; vel = velocity; stage = 1; env = 0; percussive = tab.percussive;
        A.offsetHz = 0; A.n = 0; A.build(tab, P.tiltDb, P.inharm, norm, false); A.setFreq(f0, sr);
        useB = (beatMode == 1 || beatMode == 2);
        if (useB) { B.offsetHz = P.beatHz; B.n = 0; B.build(tab, P.tiltDb, P.inharm, norm, false); B.setFreq(f0, sr); }
        updateSub(P);
    }
    void rebuild(const PartialTable& tab, const Params& P, float norm, double sr, int beatMode) {
        percussive = tab.percussive;
        A.build(tab, P.tiltDb, P.inharm, norm, true); A.setFreq(f0, sr);
        bool nb = (beatMode == 1 || beatMode == 2);
        if (nb) { if (!useB) B.n = 0; B.offsetHz = P.beatHz; B.build(tab, P.tiltDb, P.inharm, norm, useB); B.setFreq(f0, sr); }
        useB = nb; updateSub(P);
    }
    void setFreq(double freq, float beatHz, double sr) { f0 = freq; A.setFreq(f0, sr); if (useB) { B.offsetHz = beatHz; B.setFreq(f0, sr); } }
    void updateSub(const Params& P) { subGain = (f0 < 20.0 && (P.beatMode == 0 || P.beatMode == 3)) ? 0.25f : 1.0f; } // sub-audio: non si sente e affatica i woofer
    void release() { held = false; stage = 4; }

    float adsrStep(const Params& P, double sr) {
        switch (stage) {
        case 1: env += float(1.0 / (std::max(0.001f, P.attack) * sr)); if (env >= 1) { env = 1; stage = 2; } break;
        case 2: env -= float((1.0 - P.sustain) / (std::max(0.001f, P.decay) * sr)); if (env <= P.sustain) { env = P.sustain; stage = 3; } break;
        case 3: env = P.sustain; break;
        case 4: env *= float(std::exp(-6.9078 / (std::max(0.005f, P.release) * sr))); if (env < 1e-4f) { env = 0; active = false; } break;
        }
        return env;
    }

    // rende len campioni (len ≤ kSub) e li somma in L/R
    void render(float* L, float* R, int len, const Params& P, double sr, int beatMode) {
        float a[kSub] {}, b[kSub] {};
        A.targets(t, sr); A.render(a, len);
        if (useB) { B.targets(t, sr); B.render(b, len); }
        float gv = vel * subGain;
        for (int s = 0; s < len; ++s) {
            float e = adsrStep(P, sr) * gv;
            if (beatMode == 1)      { L[s] += a[s] * e; R[s] += b[s] * e; }               // binaurale: un tono per orecchio
            else if (beatMode == 2) { float m = 0.5f * (a[s] + b[s]) * e; L[s] += m; R[s] += m; } // monoaurale
            else                    { L[s] += a[s] * e; R[s] += a[s] * e; }
        }
        t += len / sr;
        if (percussive && A.silent(t)) active = false;
    }
};

// ---------------------------------------------------------------- motore
class Engine {
public:
    void prepare(double sampleRate) {
        sr = sampleRate; for (auto& v : voices) v = Voice(); drone = Voice(); drone.isDrone = true;
        nL.prepare(sr, 12345u); nR.prepare(sr, 98765u); limEnv = 1; dcxL = dcyL = dcxR = dcyR = 0; clock = 0; sweepT = 0;
        dcR = float(std::exp(-kTwoPi * 5.0 / sr)); limRel = float(1.0 - std::exp(-1.0 / (0.12 * sr))); setTable(makeModel(P.model));
    }
    void setUserCycle(const std::vector<float>& cyc) { user = harmonicsFromCycle(cyc); userSet = true; if (P.model == Drawn) setTable(user); }
    void setUserTable(const PartialTable& t) { user = t; userSet = true; if (P.model == Drawn) { setTable(user);
        for (auto& v : voices) if (v.active) v.rebuild(tab, P, norm, sr, P.beatMode); if (drone.active) drone.rebuild(tab, P, norm, sr, P.beatMode); } }
    double droneHz() const { return drone.active ? drone.f0 : 0.0; }
    const PartialTable& table() const { return tab; }

    void setParams(const Params& np) {
        bool tableChange = np.model != P.model || np.tiltDb != P.tiltDb || np.inharm != P.inharm || np.beatMode != P.beatMode;
        bool beatChange  = np.beatHz != P.beatHz;
        bool sweepStart  = np.sweepOn && !P.sweepOn;
        bool modeChange  = np.mode != P.mode;
        P = np;
        if (sweepStart) sweepT = 0;
        if (modeChange) { allNotesOff(); }
        if (tableChange) {
            setTable(P.model == Drawn && userSet ? user : makeModel(P.model));
            for (auto& v : voices) if (v.active) v.rebuild(tab, P, norm, sr, P.beatMode);
            if (drone.active) drone.rebuild(tab, P, norm, sr, P.beatMode);
        } else if (beatChange) {
            for (auto& v : voices) if (v.active) v.setFreq(v.f0, P.beatHz, sr);
            if (drone.active) drone.setFreq(drone.f0, P.beatHz, sr);
        }
    }
    void noteOn(int note, float vel) {
        double f = P.mode == 1 ? currentFixed() : P.a4 * std::pow(2.0, (note - 69) / 12.0);
        Voice* v = nullptr;
        for (auto& x : voices) if (!x.active) { v = &x; break; }
        if (!v) { v = &voices[0]; for (auto& x : voices) if (x.age < v->age) v = &x; }   // ruba la voce più vecchia
        v->note = note; v->age = ++ageCounter; v->start(f, vel, tab, P, norm, sr, P.beatMode);
    }
    // i modelli percussivi (campane, gong, diapason…) continuano a risuonare dopo il rilascio del tasto, come lo strumento vero
    void noteOff(int note) { for (auto& v : voices) if (v.active && v.held && v.note == note) { if (v.percussive) v.held = false; else v.release(); } }
    void allNotesOff() { for (auto& v : voices) if (v.active) v.release(); }

    double currentFixed() const {
        if (!P.sweepOn) return P.fixedHz;
        double T = std::max(1.0, P.sweepMin * 60.0), a = std::max(0.1f, P.sweepFrom), b = std::max(0.1f, P.sweepTo);
        double u = sweepT / T; if (P.sweepLoop) u -= std::floor(u); else u = std::min(1.0, u);
        if (P.sweepDir == 1) std::swap(a, b);
        if (P.sweepDir == 2) u = u < 0.5 ? u * 2 : 2 - u * 2;
        return a * std::pow(b / a, u);
    }
    double sweepProgress() const { double T = std::max(1.0, P.sweepMin * 60.0); double u = sweepT / T; return P.sweepLoop ? u - std::floor(u) : std::min(1.0, u); }

    void process(float* L, float* R, int n) {
        std::fill(L, L + n, 0.0f); std::fill(R, R + n, 0.0f);
        // drone a frequenza fissa
        if (P.mode == 1 && P.droneOn && (!drone.active || !drone.held)) { drone.start(currentFixed(), 1.0f, tab, P, norm, sr, P.beatMode); }
        if ((P.mode != 1 || !P.droneOn) && drone.active && drone.held) drone.release();
        if (P.mode == 1) {
            double f = currentFixed();
            if (drone.active && std::fabs(f - drone.f0) > 1e-6) { drone.setFreq(f, P.beatHz, sr); drone.updateSub(P); }
            for (auto& v : voices) if (v.active && std::fabs(f - v.f0) > 1e-6) { v.setFreq(f, P.beatHz, sr); v.updateSub(P); }
        }
        for (int off = 0; off < n; off += kSub) {
            int len = std::min(kSub, n - off);
            for (auto& v : voices) if (v.active) v.render(L + off, R + off, len, P, sr, P.beatMode);
            if (drone.active) drone.render(L + off, R + off, len, P, sr, P.beatMode);
        }
        const float level = std::pow(10.0f, P.levelDb / 20.0f), nz = P.noiseType ? std::pow(10.0f, P.noiseDb / 20.0f) : 0.0f;
        for (int s = 0; s < n; ++s) {
            double tt = clock / sr; float g = 1.0f;
            if (P.beatMode == 3) g *= isoGate(std::fmod(tt * P.beatHz, 1.0));                       // impulsi isocronici a bordi morbidi
            if (P.tremDepth > 0 && P.tremRate > 0) g *= 1.0f - P.tremDepth * 0.5f * float(1.0 - std::cos(kTwoPi * P.tremRate * tt));
            float br = P.breathDepth > 0 ? 1.0f - P.breathDepth * float(0.5 - 0.5 * std::cos(kTwoPi * 0.1 * tt)) : 1.0f; // 6 respiri/min
            float l = L[s] * g * br, r = R[s] * g * br;
            if (nz > 0) { l += nL.next(P.noiseType) * nz * br; r += nR.next(P.noiseType) * nz * br; }
            l *= level; r *= level;
            // blocco della continua (passa-alto 5 Hz)
            float yl = l - dcxL + dcR * dcyL; dcxL = l; dcyL = yl; float yr = r - dcxR + dcR * dcyR; dcxR = r; dcyR = yr;
            // limitatore di picco a −1 dBFS
            float pk = std::max(std::fabs(yl), std::fabs(yr)), ceil = 0.891f, want = pk > ceil ? ceil / pk : 1.0f;
            if (want < limEnv) limEnv = want; else limEnv += (want - limEnv) * limRel;
            L[s] = std::clamp(yl * limEnv, -0.99f, 0.99f); R[s] = std::clamp(yr * limEnv, -0.99f, 0.99f);
            ++clock;
        }
        if (P.sweepOn) sweepT += n / sr;
    }
    const Params& params() const { return P; }

private:
    static float isoGate(double ph) {
        const double e = 0.1;
        if (ph < e) return float(0.5 - 0.5 * std::cos(3.14159265 * ph / e));
        if (ph < 0.5) return 1.0f;
        if (ph < 0.5 + e) return float(0.5 + 0.5 * std::cos(3.14159265 * (ph - 0.5) / e));
        return 0.0f;
    }
    void setTable(const PartialTable& t) {
        tab = t; double s2 = 0;
        for (int k = 0; k < tab.count; ++k) { double a = tab.p[k].amp * std::pow(std::max(1.0f, tab.p[k].ratio), P.tiltDb / 6.0206f); s2 += a * a; }
        norm = s2 > 0 ? float(0.2 / std::sqrt(s2 / 2.0)) : 1.0f;   // stessa intensità RMS (≈ −14 dBFS) per tutti i modelli
    }
    double sr = 48000; Params P; PartialTable tab, user; bool userSet = false; float norm = 1;
    std::array<Voice, kMaxVoices> voices {}; Voice drone; uint64_t ageCounter = 0;
    NoiseGen nL, nR; float limRel = 0.0002f, limEnv = 1, dcR = 0.999f, dcxL = 0, dcyL = 0, dcxR = 0, dcyR = 0; uint64_t clock = 0; double sweepT = 0;
};

} // namespace prisma
