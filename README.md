Prisma Sonoro · plug-in VST3 / AU
MBStudio91 · Piedmont Electronic Sound. Strumento virtuale (synth) per Ableton Live e qualsiasi programma che apra VST3 o Audio Unit. Raccoglie tutto il lavoro fatto per la pagina Prisma Sonoro e lo porta nel DAW.

Cosa fa
Sintesi additiva fino a 64 parziali. Ogni parziale ha rapporto con la fondamentale, ampiezza, fase, attacco, decadimento, doppietto (due seni vicinissimi che battono, come nelle campane) e un respiro d'ampiezza proprio. La sintesi additiva non produce aliasing: le parziali sopra 0,45 × frequenza di campionamento vengono escluse.

Forme d'onda complesse in tre modi

Modelli pronti (14): Sinusoide, Dente di sega, Quadra, Triangolo, Campana tibetana, Campana di cristallo, Gong, Vibrafono, Diapason, Didgeridoo, Monocordo, Shruti box, Anelli di Lakhovsky, Forma disegnata.
Disegno col mouse: trascini sul riquadro del ciclo e la forma diventa suono (64 armonici calcolati con la trasformata di Fourier, fasi comprese). «Copia nel disegno» parte da qualunque modello; «Liscia», «Sinusoide» e «Inverti» aiutano a rifinire.
Armonici: trascini le barre 1–64 (scala da −60 a 0 dB).
Più due controlli che trasformano qualsiasi forma: Inclinazione spettrale (dB per ottava) e Inarmonicità (rapporti stirati come in una corda di pianoforte, r' = r·√(1 + B·r²)).

Due modi di suonare

Note MIDI: polifonico a 8 voci, diapason regolabile (415–466 Hz, tasti rapidi 440 e 432).
Hz fisso: tutto suona alla frequenza esatta scelta (0,1–20 000 Hz). «Bordone» lo tiene acceso senza note; ogni nota MIDI o il pulsante «Colpo» ribatte lo strumento alla stessa frequenza. Gli strumenti percussivi (campane, gong, vibrafono, diapason, monocordo) risuonano fino alla fine anche se si alza il tasto.
Frequenze verificate (21) con livello di evidenza, e libreria strumenti con le frequenze caratteristiche: campane tibetane misurate e set chakra, 16 gong planetari, vibrafono Fa3–Fa6, campane di cristallo, diapason (128 e 512 Hz clinici, 256, OM 136,10, 432, 528), didgeridoo 61,7–82,4 Hz, monocordo, shruti box, anelli di Lakhovsky.

Battimenti: binaurale (in cuffia), monoaurale (per le casse), isocronico (impulsi a bordi morbidi), 0,5–40 Hz.

Modulazioni: respiro guidato a 0,1 Hz (6 respiri al minuto), tremolo 0–12 Hz (il motore del vibrafono).

Rumori: bianco, rosa, marrone, grigio, blu.

Sweep logaritmico: da/a in Hz, durata 0,1–60 minuti, sale / scende / sale e scende, ripeti.

Inviluppo ADSR e livello d'uscita, con limitatore di picco a −1 dBFS e filtro anti-continua a 5 Hz sempre attivi.

Sicurezza: un tono fisso sotto i 20 Hz senza battimento viene abbassato di 12 dB, perché non si sente e affatica i woofer.

Visualizzazione: oscilloscopio XY (sinistra su X, destra su Y) e forma nel tempo, colore "ottava di luce" della frequenza con lunghezza d'onda in nanometri.

Tutti i parametri sono automatizzabili in Ableton. La forma disegnata viene salvata col progetto.

Collaudi già fatti (versione Linux)
pluginval (lo strumento di validazione standard dei plug-in), livello di severità 8: superato, compresi stato, automazione, sicurezza dei parametri tra thread e test casuali. Test dell'interfaccia: superati.
Analisi dei suoni resi:
528 Hz esatti; binaurale 200 / 207,83 Hz; impulsi isocronici a 10,00 Hz.
Dente di sega a 1760 Hz con l'energia fuori dagli armonici a −112 dB (nessun aliasing).
Campana a 236 Hz con parziali a 2,95× e 5,7× e battimento del doppietto a 1,8 Hz.
Forma disegnata ricostruita con correlazione 0,9993.
Sweep 55→880 Hz che segue la curva logaritmica entro l'1%.
Limitatore fermo a −1,00 dBFS.
Carico massimo (8 voci × 64 armonici, binaurale): circa il 4% di un core.
Come ottenerlo sul Mac
Il plug-in va compilato su macOS. Ci sono due strade.

A. Senza installare niente: GitHub Actions
Crea un repository su GitHub e caricaci questa cartella, compresa la cartella nascosta .github.
GitHub compila da solo su un Mac (scheda Actions, flusso «Build macOS») e lo valida con pluginval.
A fine esecuzione scarica PrismaSonoro-mac dalla sezione Artifacts. Dentro trovi tre zip: VST3, AU e app.
Installa:
Prisma Sonoro.vst3 in ~/Library/Audio/Plug-Ins/VST3/
Prisma Sonoro.component in ~/Library/Audio/Plug-Ins/Components/
I file scaricati da internet vengono bloccati da macOS. Togli il blocco dal Terminale:
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"Prisma Sonoro.vst3"
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"Prisma Sonoro.component"
B. Sul tuo Mac
Terminale: xcode-select --install (Strumenti da riga di comando di Xcode, gratuiti).
Installa CMake da cmake.org, oppure brew install cmake.
Nella cartella del progetto: ./build_mac.sh. Scarica JUCE, compila in versione universale (Apple Silicon e Intel) e copia VST3 e AU nelle cartelle giuste.
In Ableton Live 12
Impostazioni → Plug-in: attiva «Usa le cartelle di sistema dei plug-in VST3» e/o «Usa Audio Units», poi Riesamina. Prisma Sonoro compare tra gli strumenti, sotto MBStudio91.

Primo uso consigliato
Trascina Prisma Sonoro su una traccia MIDI.
Scegli Hz fisso, una voce da Libreria strumenti (per esempio Gong planetari → Terra, OM, 136,10 Hz) e premi Colpo.
Per un tappeto: modello Campana di cristallo o Shruti box, accendi Bordone, aggiungi rumore rosa a −30 dB e Respiro 0,1 Hz al 50%.
Per disegnare: trascina sul riquadro del ciclo. Le barre degli armonici e il suono cambiano subito.
Note di metodo
Frequenze della libreria: sono valori assoluti in Hz. I nomi delle note sono calcolati con La = 440. Il diapason regolabile agisce sul modo Note MIDI.
Fonti dei modelli:
campane tibetane: rapporti 2,95× e 5,7× (misure NPUST), doppietti (Terwagne & Bush, arXiv 1106.6348);
didgeridoo: bordone 60–80 Hz, toot 2,6–2,9×, formanti 500 / 1500 / 2600 Hz (J. Wolfe, UNSW);
vibrafono: parziali 1 : 4 : 10, Fa3–Fa6;
gong planetari: valori di H. Cousto usati da Meinl.
Livelli di evidenza: A fatto fisico · B studi con risultati · C studi preliminari · D tradizione o dichiarazione non verificata.
Esperienza d'ascolto per il rilassamento: non sostituisce cure o consigli medici. Il binaurale si ascolta in cuffia, a volume moderato, mai alla guida.
Licenza
Il codice di Prisma Sonoro è tuo. Usa il framework JUCE 8, che ha una doppia licenza: AGPLv3 oppure licenza JUCE commerciale. La licenza JUCE «Starter» è gratuita fino a 20 000 $ l'anno di ricavi legati a JUCE (donazioni comprese). Se un giorno lo vendi o lo distribuisci, verifica quale delle due fa per te.

I font Barlow Condensed e Share Tech Mono sono SIL Open Font License (testo in Resources/OFL.txt).

Struttura
CMakeLists.txt              progetto (JUCE 8.0.4 scaricato in automatico)
Source/dsp/PrismaDSP.h      motore audio indipendente (C++17)
Source/Library.h            21 frequenze verificate, libreria strumenti, nomi delle note, colori
Source/PluginProcessor.*    parametri, MIDI, stato salvato
Source/PluginEditor.*       interfaccia
tests/render_test.cpp       collaudo offline (rende i WAV di prova)
tests/analyze.py            analisi FFT dei WAV di prova
.github/workflows/          compilazione automatica per Mac
build_mac.sh                compilazione e installazione sul tuo Mac
