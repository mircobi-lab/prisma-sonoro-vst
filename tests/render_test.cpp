// Collaudo offline del motore: rende alcuni casi in WAV (float 32 bit, 48 kHz) per l'analisi
#include "../Source/dsp/PrismaDSP.h"
#include <cstdio>
#include <cstring>
#include <chrono>

using namespace prisma;

static void writeWav(const char* path, const std::vector<float>& L, const std::vector<float>& R, int sr) {
    FILE* f = std::fopen(path, "wb"); if (!f) return;
    uint32_t n = (uint32_t) L.size(), data = n * 2 * 4, riff = 36 + data; uint16_t fmt = 3, ch = 2, bits = 32, align = 8; uint32_t rate = sr, br = sr * 8, sz = 16;
    std::fwrite("RIFF", 1, 4, f); std::fwrite(&riff, 4, 1, f); std::fwrite("WAVEfmt ", 1, 8, f); std::fwrite(&sz, 4, 1, f);
    std::fwrite(&fmt, 2, 1, f); std::fwrite(&ch, 2, 1, f); std::fwrite(&rate, 4, 1, f); std::fwrite(&br, 4, 1, f); std::fwrite(&align, 2, 1, f); std::fwrite(&bits, 2, 1, f);
    std::fwrite("data", 1, 4, f); std::fwrite(&data, 4, 1, f);
    for (uint32_t i = 0; i < n; ++i) { std::fwrite(&L[i], 4, 1, f); std::fwrite(&R[i], 4, 1, f); }
    std::fclose(f);
}

struct Case { const char* name; Params p; int note; double seconds; bool useCycle; };

int main() {
    const int sr = 48000, block = 512;
    std::vector<Case> cases;
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 528; p.model = Sine; cases.push_back({"01_sine_528_fixed", p, -1, 4, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 200; p.beatMode = 1; p.beatHz = 7.83f; cases.push_back({"02_binaural_200_783", p, -1, 4, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 400; p.beatMode = 3; p.beatHz = 10; cases.push_back({"03_iso_400_10", p, -1, 4, false}); }
    { Params p; p.mode = 0; p.model = Saw; p.release = 0.3f; cases.push_back({"04_saw_A6_alias", p, 93, 3, false}); }             // La6 1760 Hz: verifica anti-aliasing
    { Params p; p.mode = 1; p.fixedHz = 236; p.model = TibetanBowl; p.sustain = 1; p.release = 8; cases.push_back({"05_bowl_236", p, 60, 10, false}); }
    { Params p; p.mode = 1; p.fixedHz = 136.10f; p.model = Gong; p.release = 10; cases.push_back({"06_gong_om_13610", p, 60, 10, false}); }
    { Params p; p.mode = 0; p.model = Vibraphone; p.tremRate = 4; p.tremDepth = 0.5f; cases.push_back({"07_vibes_A4_trem4", p, 69, 5, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 73.42f; p.model = Didgeridoo; cases.push_back({"08_didge_D2", p, -1, 5, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 432; p.model = LakhovskyRings; cases.push_back({"09_rings_432", p, -1, 6, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 110; p.model = Drawn; cases.push_back({"10_drawn_110", p, -1, 3, true}); }
    { Params p; p.mode = 1; p.droneOn = true; p.sweepOn = true; p.sweepFrom = 55; p.sweepTo = 880; p.sweepMin = 0.1f; p.model = Sine; cases.push_back({"11_sweep_55_880_6s", p, -1, 6, false}); }
    { Params p; p.mode = 1; p.droneOn = false; p.noiseType = 4; p.noiseDb = -12; cases.push_back({"12_grey_noise", p, -1, 4, false}); }
    { Params p; p.mode = 1; p.droneOn = true; p.fixedHz = 7.83f; p.model = Sine; cases.push_back({"13_sub_783_tone", p, -1, 3, false}); }
    { Params p; p.mode = 0; p.model = Saw; p.levelDb = 0; cases.push_back({"14_poly_chord_limiter", p, 48, 3, false}); }

    for (auto& c : cases) {
        Engine e; e.prepare(sr);
        if (c.useCycle) { // forma disegnata: onda "a pinna di squalo" + ripple, 256 punti
            std::vector<float> cyc(256); for (int i = 0; i < 256; ++i) { double x = i / 256.0; cyc[i] = float((x < 0.7 ? std::sin(x / 0.7 * 3.14159) : -std::sin((x - 0.7) / 0.3 * 3.14159) * 0.6) + 0.15 * std::sin(x * 2 * 3.14159 * 9)); }
            e.setUserCycle(cyc);
        }
        e.setParams(c.p);
        const int N = int(c.seconds * sr); std::vector<float> L(N), R(N);
        if (c.note >= 0) { e.noteOn(c.note, 0.9f); if (std::strstr(c.name, "chord")) { e.noteOn(52, .9f); e.noteOn(55, .9f); e.noteOn(60, .9f); e.noteOn(64, .9f); } }
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i += block) { int n = std::min(block, N - i); if (c.note >= 0 && i >= int(c.seconds * 0.6 * sr) && i < int(c.seconds * 0.6 * sr) + block) e.allNotesOff(); e.process(&L[i], &R[i], n); }
        double ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
        float pk = 0; for (int i = 0; i < N; ++i) pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i])));
        char path[256]; std::snprintf(path, sizeof path, "out/%s.wav", c.name); writeWav(path, L, R, sr);
        std::printf("%-26s picco %.3f  tempo CPU %.1f ms per %.0f s (%.2f%% realtime)\n", c.name, pk, ms, c.seconds, ms / (c.seconds * 10.0));
    }
    // carico peggiore: 8 voci, dente di sega da 64 armonici, binaurale (256 oscillatori per voce)
    { Engine e; e.prepare(sr); Params p; p.model = Saw; p.beatMode = 1; e.setParams(p); for (int k = 0; k < 8; ++k) e.noteOn(36 + k * 2, .8f);
      std::vector<float> L(block), R(block); auto t0 = std::chrono::high_resolution_clock::now(); int blocks = sr * 5 / block;
      for (int i = 0; i < blocks; ++i) e.process(L.data(), R.data(), block);
      double ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
      std::printf("CARICO MASSIMO 8 voci x 128 osc x 2 (binaurale): %.1f ms per 5 s = %.1f%% di un core\n", ms, ms / 50.0); }
    return 0;
}
