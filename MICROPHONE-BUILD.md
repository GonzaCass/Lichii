# Fritchii v5.5 with microphone input

Download: https://github.com/GonzaCass/Lichii/releases/tag/fritchii-v5.5-microphone

Source: https://github.com/GonzaCass/Lichii/tree/microphone-v5.5

This is an unofficial Windows x64 build of Frizti/Fritchii v5.5, with the Switch 2 microphone support from zortos293/Lichii #497 and the unavailable-device fallback from #881. Credit for the original capture implementation belongs to rolling-not-trolling and the upstream contributors. Fritchii's v5.5 graphics and performance changes are retained.

## Download and use

Download the Windows ZIP from this repository's **Fritchii v5.5 + Microphone** release. GitHub's automatic "Source code" archives are not playable builds.

1. Extract the entire ZIP into a new folder on your SSD. Keep its DLLs and subfolders beside `lichii.exe`.
2. Run `lichii.exe` and use your own compatible game files. No game, keys, firmware, saves, or extracted game shaders are included.
3. Before starting emulation, open **Emulation > Configure > Controls > Microphone**. Select your input device, set its volume, and use **Start microphone test** to check the level meter.
4. Apply the selection, stop the preview test, and start the game. Humming, singing, or an instrument must reach the selected Windows input device.

The build uses the existing Eden/Fritchii user-data folder unless you explicitly configure portable mode. Back up your saves before switching builds. A newly selected release/driver can require shader preparation again. Only a metadata recipe is included; the emulator builds shaders from the player's own files.

If Windows cannot open or start the input, the game receives silence instead of a failed stream. This avoids the known null-input-stream panic, but silence is not working microphone capture: check the device and Windows microphone permissions if the meter does not move. Other causes of game crashes are outside this fix. Device and gain changes require stopped emulation. If you change the Audio backend, apply it and reopen Configure before testing the microphone.

The official automatic updater is disabled in this custom build so it cannot replace the microphone fix with an official binary that lacks it. Install later custom releases manually, or return to official Fritchii once it includes microphone support.

## Descargar y usar

DescargÃƒÂ¡ el ZIP de Windows de la release **Fritchii v5.5 + Microphone**, extraelo completo en el SSD y ejecutÃƒÂ¡ `lichii.exe`. Los archivos automÃƒÂ¡ticos "Source code" no contienen el programa compilado.

Con el juego detenido, entrÃƒÂ¡ en **Emulation > Configure > Controls > Microphone**, elegÃƒÂ­ tu entrada de Windows y usÃƒÂ¡ **Start microphone test**. El medidor debe responder a tu voz o instrumento. AplicÃƒÂ¡ el ajuste y abrÃƒÂ­ tu propio juego. Si el dispositivo no puede abrirse, el juego recibe silencio; revisÃƒÂ¡ la selecciÃƒÂ³n y los permisos de Windows.

No se incluyen juego, claves, firmware, partidas ni shaders extraÃƒÂ­dos del juego. ConservÃƒÂ¡ todos los DLL y subdirectorios del ZIP. RespaldÃƒÂ¡ tus partidas antes de cambiar de versiÃƒÂ³n. La preparaciÃƒÂ³n inicial de shaders puede repetirse al cambiar de versiÃƒÂ³n o controlador. Esta compilaciÃƒÂ³n no se actualiza automÃƒÂ¡ticamente a la versiÃƒÂ³n oficial para evitar perder el fix.

## Source and build

Base: `Frizti/Fritchii`, tag `fritchii-v5.5`, commit `4012e5271e278b21cf8f56b97481dac2b777cd0d`. The microphone and fallback ports are separate commits. Windows test-harness fixes are a separate commit and do not change the emulator's stack reserve or graphics behavior.

The downloadable build uses MSYS2 UCRT64, Clang/LLD 23.1.5, Release, the repository's `eglantine.profdata` PGO profile, ThinLTO, and system Qt 6.12. Follow the upstream dependency documentation. Configuration used `BUILD_TESTING=ON`, `YUZU_TESTS=ON`, `YUZU_CMD=OFF`, `YUZU_ROOM=OFF`, `YUZU_USE_BUNDLED_QT=OFF`, `YUZU_USE_BUNDLED_SDL3=OFF`, `YUZU_USE_BUNDLED_FFMPEG=OFF`, `YUZU_USE_BUNDLED_OPENSSL=OFF`, `ENABLE_UPDATE_CHECKER=OFF`, `ENABLE_WEB_SERVICE=OFF`, `USE_DISCORD_PRESENCE=OFF`, `ENABLE_LTO=ON`, and `ENABLE_WERROR=OFF`. Select `clang`/`clang++`, `llvm-ar`/`llvm-ranlib` for compiler archives, and LLD. Pass `-fprofile-use=<absolute path to eglantine.profdata>` to Clang along with `-fuse-ld=lld`, `-Wno-backend-plugin`, `-Wno-profile-instr-unprofiled`, and `-Wno-profile-instr-out-of-date`.

```sh
cmake --build build --target yuzu tests --parallel 6
build/bin/tests.exe '*Microphone*,*Audio input*,[input],[sysmix],[sysmix_sink]' --reporter compact
```

Use `windeployqt6` and include the non-system DLL dependencies when packaging a MinGW build; copying the executable alone is insufficient. Include licenses and the corresponding source. See `LICENSE.txt`, `LICENSES/`, and the upstream dependency manifests for licensing.

## Validation of this build

The Windows build completed and the focused audio/input/sysmix suite passed: 39 cases, 12,754 assertions. An initial run alongside shader preparation and compilation failed the existing output-concurrency case; the full rerun and three separate repeats of that case passed. No output implementation or test assertion was changed to suppress that result. The test executable uses the stack reserve configured in the Windows test-harness commit.

Real ocarina microphone capture was confirmed by the player on our previous v5.1 port of #497. This v5.5 build has automated coverage, including the unavailable-device fallback; a fresh manual gameplay confirmation on v5.5 is still pending. The release is published as a prerelease.
