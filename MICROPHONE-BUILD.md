# Fritchii v5.5 + microphone + native DLSS (revision 2)

Unofficial Windows x64 build based on Frizti/Fritchii v5.5 (`4012e5271e278b21cf8f56b97481dac2b777cd0d`). Microphone capture is ported from zortos293/Lichii #497, with the unavailable-device fallback from #881. Credit belongs to rolling-not-trolling and the upstream contributors. The microphone PR is https://github.com/Frizti/Fritchii/pull/31.

## Download and use

Download the Windows ZIP from https://github.com/GonzaCass/Lichii/releases/tag/fritchii-v5.5-microphone-r2. Automatic GitHub source archives are not runnable builds.

1. Extract the complete ZIP into a fresh folder on your SSD. Do not mix DLLs with the earlier MinGW build. Run `vc_redist.x64.exe` if Windows needs the Microsoft C++ runtime.
2. Run `lichii.exe` and open your own compatible game files. No game, keys, firmware, saves or extracted game shaders are included. Only shader recipe metadata is included.
3. With emulation stopped, open **Emulation > Configure > Controls > Microphone**, choose your Windows input and use **Start microphone test**. Apply the selection and stop the preview before playing. Humming or an instrument must reach that Windows device.
4. On a supported NVIDIA GPU, native DLSS is available under **Emulation > Configure > Graphics > Use NVIDIA DLSS**. The local test used preset K. NVIDIA runtime components are managed by the emulator/NVIDIA App, not bundled in this ZIP. DLSS Super Resolution does not imply Frame Generation support.

The existing Eden/Fritchii user-data folder is reused unless you configure portable mode. Back up saves before switching builds. Shader preparation may repeat after a compiler, release or driver change. The custom automatic updater is disabled so it cannot silently replace the microphone build.

If the input device cannot open, the game receives silence instead of a failed stream. Silence is not working microphone capture: check Windows permissions and device selection. Device/gain changes require stopped emulation. After changing the Audio backend, apply it and reopen Configure before testing the microphone. This fix does not cover every possible game crash.

## Revision 2 correction

The first public package used MinGW. Although the microphone fix worked locally, that toolchain excluded the MSVC-only Streamline integration and omitted the DLSS checkbox. This was our packaging error. Revision 2 uses clang-cl with the MSVC ABI and explicitly enables `ENABLE_NVIDIA_DLSS=ON`; no renderer patch or third-party DLSS injection mod was added.

The source of the tested binary is `b07ab3985c8a22f6c29db36f74751233f68363d9` at https://github.com/GonzaCass/Lichii/tree/b07ab3985c8a22f6c29db36f74751233f68363d9. Later documentation commits do not change that binary. The PR contains only the microphone ports and Windows test-harness changes.

## Build

Windows x64; clang-cl/LLD 23.1.5; MSVC tools 14.44.35207; Windows SDK 10.0.26100; Qt 6.8.3 MSVC2022 x64; Release, repository PGO profile and ThinLTO. Install VS 2022 C++ Build Tools, Ninja, CMake, compatible LLVM and Qt. Use the MSVC environment loader in `tools/windows/load-msvc-env.ps1`.

The reference script is `tools/windows/build-microphone-dlss.ps1`. Supply LLVM, Qt, CMake and Ninja paths. Keep MinGW include/library/package paths out of the MSVC configuration; use a fresh build directory. Dependencies and hashes are recorded in the repository CPM manifests and packaged `dependency-sources.json`.

Package with the matching Qt 6.8.3 `windeployqt.exe --release --no-translations --no-compiler-runtime --no-opengl-sw --no-system-dxc-compiler --no-system-d3d-compiler`, the matching upstream Windows DX compiler/DXIL/software OpenGL files and redistributable, licenses, `qt.conf` and metadata recipe. Audit PE imports. Do not bundle personal user data or proprietary NVIDIA SDK runtime DLLs. See `LICENSE.txt`, `LICENSES/` and the corresponding source for licensing.

## Validation

The clang-cl build passed all 39 focused microphone/audio-input/sysmix tests (14,566 assertions). On the local RTX 3090 system the player confirmed the DLSS checkbox appears and the save loads. Logs confirmed Streamline 2.14.16, NVIDIA DLSS model 310.9.16 and real 2560x1440 to 3840x2160 DLSS evaluations/presentation, without a DLSS recording failure in the observed session. The input logs also reported nonzero captured microphone samples.

This does not reproduce or prove a fix for another user's after-shader crash. If it persists with revision 2, export a fresh log through Help > Export Log and attach it to the report. The prior stock-build log does not diagnose the corrected package.