#!/bin/bash
# Compila Prisma Sonoro sul tuo Mac e lo installa nelle cartelle dei plug-in dell'utente.
# Servono: Strumenti da riga di comando di Xcode (xcode-select --install) e CMake (cmake.org o "brew install cmake").
set -e
cd "$(dirname "$0")"
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target PrismaSonoro_VST3 PrismaSonoro_AU PrismaSonoro_Standalone -j "$(sysctl -n hw.ncpu)"
OUT="build/PrismaSonoro_artefacts/Release"
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
rm -rf ~/Library/Audio/Plug-Ins/VST3/"Prisma Sonoro.vst3" ~/Library/Audio/Plug-Ins/Components/"Prisma Sonoro.component"
cp -R "$OUT/VST3/Prisma Sonoro.vst3"     ~/Library/Audio/Plug-Ins/VST3/
cp -R "$OUT/AU/Prisma Sonoro.component"  ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep -s - ~/Library/Audio/Plug-Ins/VST3/"Prisma Sonoro.vst3" ~/Library/Audio/Plug-Ins/Components/"Prisma Sonoro.component"
killall -9 AudioComponentRegistrar 2>/dev/null || true   # fa rileggere gli Audio Unit al sistema
echo "Installato. In Ableton Live: Impostazioni > Plug-in > Riesamina."
