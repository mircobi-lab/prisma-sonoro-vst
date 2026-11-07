// Library.h — frequenze verificate e libreria degli strumenti (stessi dati di Prisma Sonoro web)
#pragma once
#include <cstdio>
#include "dsp/PrismaDSP.h"

namespace prisma {

// Livelli di evidenza: A fatto fisico · B studi con risultati · C studi preliminari · D tradizione o dichiarazione non verificata
struct FreqItem { const char* code; float hz; const char* name; const char* level; const char* info; };

inline const FreqItem kFrequencies[] = {
    {"R01", 0.1f,   "Respirazione a risonanza", "B",   "6 respiri/min: usa il cursore Respiro"},
    {"W01", 2.0f,   "Delta",                     "A/C", "Banda EEG 0,5–4 Hz: usa come battimento"},
    {"W02", 6.0f,   "Theta",                     "A/C", "Banda EEG 4–8 Hz: usa come battimento"},
    {"S01", 7.83f,  "Risonanza di Schumann",     "A/D", "Valore geofisico; effetti sul corpo non dimostrati"},
    {"W03", 10.0f,  "Alfa",                      "A/C", "Banda EEG 8–13 Hz: usa come battimento"},
    {"W04", 20.0f,  "Beta",                      "A/C", "Banda EEG 13–30 Hz"},
    {"G01", 40.0f,  "Gamma 40 Hz (GENUS)",       "B/C", "Ricerca MIT; non per autotrattamento"},
    {"X01", 110.0f, "110 Hz \"stomaco\"",        "D",   "Nessuna fonte attendibile"},
    {"F01", 174.0f, "Solfeggio 174",             "D",   "Tradizione"},
    {"N02", 243.0f, "Neuro Rune 243",            "D",   "Non trovata sul sito ufficiale"},
    {"F02", 285.0f, "Solfeggio 285",             "D",   "Tradizione"},
    {"F03", 396.0f, "Solfeggio 396",             "D",   "Tradizione · Muladhara"},
    {"F04", 417.0f, "Solfeggio 417",             "D",   "Tradizione · Svadhisthana"},
    {"T01", 432.0f, "La 432",                    "C",   "Studio pilota 432 vs 440"},
    {"T02", 440.0f, "La 440",                    "A",   "Diapason standard ISO 16"},
    {"F05", 528.0f, "Solfeggio 528",             "D",   "Tradizione; claim DNA senza supporto"},
    {"N01", 628.0f, "Neuro Rune 628",            "D",   "Dichiarazione commerciale non verificabile"},
    {"F06", 639.0f, "Solfeggio 639",             "D",   "Tradizione · Anahata"},
    {"F07", 741.0f, "Solfeggio 741",             "D",   "Tradizione; claim detox smentito"},
    {"F08", 852.0f, "Solfeggio 852",             "D",   "Tradizione · Ajna"},
    {"F09", 963.0f, "Solfeggio 963",             "D",   "Tradizione · Sahasrara"},
};

// Libreria: frequenze caratteristiche (valori assoluti in Hz, note calcolate con La = 440 Hz)
// Fonti: Terwagne & Bush (arXiv 1106.6348) e NPUST per le campane; Meinl / H. Cousto per i gong planetari;
// voce "Vibraphone" (estensione Fa3–Fa6, parziali 1:4:10); J. Wolfe, UNSW, per il didgeridoo (60–80 Hz).
struct LibItem { const char* group; const char* name; float hz; int model; const char* level; };

inline const LibItem kLibrary[] = {
    {"Campane tibetane", "Grande, diametro 72 cm",  66.76f, TibetanBowl, "A"},
    {"Campane tibetane", "Media, diametro 52 cm",  118.40f, TibetanBowl, "A"},
    {"Campane tibetane", "Tibet 2",                187.00f, TibetanBowl, "A"},
    {"Campane tibetane", "Tibet 1",                236.00f, TibetanBowl, "A"},
    {"Campane tibetane", "Tibet 3",                347.00f, TibetanBowl, "A"},
    {"Campane tibetane", "Tibet 4",                428.00f, TibetanBowl, "A"},
    {"Campane tibetane", "Do4 · Muladhara",        261.63f, TibetanBowl, "D"},
    {"Campane tibetane", "Re4 · Svadhisthana",     293.66f, TibetanBowl, "D"},
    {"Campane tibetane", "Mi4 · Manipura",         329.63f, TibetanBowl, "D"},
    {"Campane tibetane", "Fa4 · Anahata",          349.23f, TibetanBowl, "D"},
    {"Campane tibetane", "Sol4 · Vishuddha",       392.00f, TibetanBowl, "D"},
    {"Campane tibetane", "La4 · Ajna",             440.00f, TibetanBowl, "D"},
    {"Campane tibetane", "Si4 · Sahasrara",        493.88f, TibetanBowl, "D"},
    {"Gong planetari", "Sole",                126.22f, Gong, "D"},
    {"Gong planetari", "Terra, anno (OM)",    136.10f, Gong, "D"},
    {"Gong planetari", "Plutone",             140.25f, Gong, "D"},
    {"Gong planetari", "Mercurio",            141.27f, Gong, "D"},
    {"Gong planetari", "Marte",               144.72f, Gong, "D"},
    {"Gong planetari", "Saturno",             147.85f, Gong, "D"},
    {"Gong planetari", "Anno platonico",      172.06f, Gong, "D"},
    {"Gong planetari", "Chirone",             172.56f, Gong, "D"},
    {"Gong planetari", "Sedna",               181.60f, Gong, "D"},
    {"Gong planetari", "Giove",               183.58f, Gong, "D"},
    {"Gong planetari", "Giorno siderale",     194.71f, Gong, "D"},
    {"Gong planetari", "Urano",               207.36f, Gong, "D"},
    {"Gong planetari", "Luna sinodica",       210.42f, Gong, "D"},
    {"Gong planetari", "Nettuno",             211.44f, Gong, "D"},
    {"Gong planetari", "Venere",              221.23f, Gong, "D"},
    {"Gong planetari", "Luna siderale",       227.43f, Gong, "D"},
    {"Vibrafono", "Fa3 · nota più grave",  174.61f, Vibraphone, "A"},
    {"Vibrafono", "La3",                   220.00f, Vibraphone, "A"},
    {"Vibrafono", "Do4",                   261.63f, Vibraphone, "A"},
    {"Vibrafono", "Fa4",                   349.23f, Vibraphone, "A"},
    {"Vibrafono", "La4",                   440.00f, Vibraphone, "A"},
    {"Vibrafono", "Do5",                   523.25f, Vibraphone, "A"},
    {"Vibrafono", "Fa5",                   698.46f, Vibraphone, "A"},
    {"Vibrafono", "Fa6 · nota più acuta", 1396.91f, Vibraphone, "A"},
    {"Campane di cristallo", "Do4 · Muladhara",    261.63f, CrystalBowl, "D"},
    {"Campane di cristallo", "Re4 · Svadhisthana", 293.66f, CrystalBowl, "D"},
    {"Campane di cristallo", "Mi4 · Manipura",     329.63f, CrystalBowl, "D"},
    {"Campane di cristallo", "Fa4 · Anahata",      349.23f, CrystalBowl, "D"},
    {"Campane di cristallo", "Sol4 · Vishuddha",   392.00f, CrystalBowl, "D"},
    {"Campane di cristallo", "La4 · Ajna",         440.00f, CrystalBowl, "D"},
    {"Campane di cristallo", "Si4 · Sahasrara",    493.88f, CrystalBowl, "D"},
    {"Diapason", "128 Hz · sensibilità vibratoria", 128.0f, TuningFork, "A"},
    {"Diapason", "256 Hz · Do4 scala fisica",       256.0f, TuningFork, "A"},
    {"Diapason", "512 Hz · test dell'udito",        512.0f, TuningFork, "A"},
    {"Diapason", "136,10 Hz · OM (Cousto)",         136.10f, TuningFork, "D"},
    {"Diapason", "432 Hz",                          432.0f, TuningFork, "C"},
    {"Diapason", "528 Hz",                          528.0f, TuningFork, "D"},
    {"Didgeridoo", "Si1",                61.74f, Didgeridoo, "A"},
    {"Didgeridoo", "Do2",                65.41f, Didgeridoo, "A"},
    {"Didgeridoo", "Do#2",               69.30f, Didgeridoo, "A"},
    {"Didgeridoo", "Re2 · il più comune", 73.42f, Didgeridoo, "A"},
    {"Didgeridoo", "Mi2",                82.41f, Didgeridoo, "A"},
    {"Monocordo", "La1",                  55.00f, Monochord, "A"},
    {"Monocordo", "Do2",                  65.41f, Monochord, "A"},
    {"Monocordo", "Re2",                  73.42f, Monochord, "A"},
    {"Monocordo", "OM un'ottava sotto",   68.05f, Monochord, "D"},
    {"Shruti box", "Re3",                146.83f, ShrutiBox, "A"},
    {"Shruti box", "Do3",                130.81f, ShrutiBox, "A"},
    {"Anelli di Lakhovsky", "Base 432 Hz", 432.0f, LakhovskyRings, "D"},
    {"Anelli di Lakhovsky", "Base 136,10 Hz", 136.10f, LakhovskyRings, "D"},
};

// Nota temperata più vicina e scarto in cent, con La di riferimento
inline std::string noteName(double f, double a4 = 440.0) {
    if (f < 16.0) return "sub-audio (ritmo)";
    static const char* N[] = {"Do","Do#","Re","Re#","Mi","Fa","Fa#","Sol","Sol#","La","La#","Si"};
    double n = 12.0 * std::log2(f / a4) + 69.0; int r = (int) std::lround(n); double c = (n - r) * 100.0;
    char buf[64]; std::snprintf(buf, sizeof buf, "%s%d %+.1f c", N[((r % 12) + 12) % 12], r / 12 - 1, c);
    std::string s(buf); for (auto& ch : s) if (ch == '.') ch = ',';
    return s;
}

// Colore "ottava di luce": raddoppio della frequenza fino alla banda visibile (400–790 THz), λ = c/f, RGB con l'approssimazione di D. Bruton
inline uint32_t octaveColour(double f, double* lambdaNm = nullptr) {
    double x = std::max(0.01, f); while (x < 400e12) x *= 2; while (x > 790e12) x /= 2;
    double w = 299792458.0 / x * 1e9, r = 0, g = 0, b = 0;
    if (w < 440) { r = -(w - 440) / 60; b = 1; } else if (w < 490) { g = (w - 440) / 50; b = 1; } else if (w < 510) { g = 1; b = -(w - 510) / 20; }
    else if (w < 580) { r = (w - 510) / 70; g = 1; } else if (w < 645) { r = 1; g = -(w - 645) / 65; } else r = 1;
    double k = w < 420 ? 0.3 + 0.7 * (w - 380) / 40 : w > 700 ? 0.3 + 0.7 * (780 - w) / 80 : 1;
    auto h = [&](double v) { return v <= 0 ? 0u : (uint32_t) std::lround(255 * std::pow(v * k, 0.8)); };
    if (lambdaNm) *lambdaNm = w;
    return 0xff000000u | (h(r) << 16) | (h(g) << 8) | h(b);
}

} // namespace prisma
