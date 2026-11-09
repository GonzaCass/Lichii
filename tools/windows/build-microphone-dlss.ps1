param(
    [Parameter(Mandatory=$true)][string]$LlvmBin,
    [Parameter(Mandatory=$true)][string]$QtRoot,
    [Parameter(Mandatory=$true)][string]$CmakeBin,
    [Parameter(Mandatory=$true)][string]$NinjaBin,
    [string]$BuildDirectory,
    [int]$Jobs=8
)
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (-not $BuildDirectory) { $BuildDirectory=Join-Path $repoRoot 'build-msvc-dlss' }
. "$PSScriptRoot\load-msvc-env.ps1"
$env:Path="$llvmBin;$cmakeBin;$ninjaBin;"+$env:Path
$qt=$QtRoot -replace '\\','/'
$profile=($repoRoot -replace '\\','/')+'/eglantine.profdata'
$flags="/clang:-fprofile-use=$profile /clang:-Wno-backend-plugin /clang:-Wno-profile-instr-unprofiled /clang:-Wno-profile-instr-out-of-date"
cmake -S $repoRoot -B $BuildDirectory -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER="$llvmBin/lld-link.exe" -DCMAKE_AR="$llvmBin/llvm-lib.exe" -DCMAKE_C_COMPILER_AR="$llvmBin/llvm-lib.exe" -DCMAKE_CXX_COMPILER_AR="$llvmBin/llvm-lib.exe" "-DCMAKE_C_FLAGS=$flags" "-DCMAKE_CXX_FLAGS=$flags" -DCMAKE_EXE_LINKER_FLAGS=/machine:x64 "-DCMAKE_PREFIX_PATH=$qt" -DYUZU_USE_BUNDLED_QT=OFF -DYUZU_USE_BUNDLED_SDL3=ON -DYUZU_USE_BUNDLED_FFMPEG=ON -DYUZU_USE_BUNDLED_OPENSSL=ON -DENABLE_NVIDIA_DLSS=ON -DVulkanHeaders_FORCE_BUNDLED=ON -DVulkanUtilityLibraries_FORCE_BUNDLED=ON -DHTTPLIB_USE_BROTLI_IF_AVAILABLE=OFF -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON -DBUILD_TESTING=ON -DYUZU_TESTS=ON -DYUZU_CMD=OFF -DYUZU_ROOM=OFF -DYUZU_ROOM_STANDALONE=OFF -DENABLE_LTO=ON -DENABLE_WERROR=OFF -DUSE_DISCORD_PRESENCE=OFF -DENABLE_UPDATE_CHECKER=OFF -DENABLE_WEB_SERVICE=OFF *> "$BuildDirectory-configure.log"
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
cmake --build $BuildDirectory --target yuzu tests --parallel $Jobs *> "$BuildDirectory-build.log"
exit $LASTEXITCODE

# Focused verification after a successful build:
# & (Join-Path $BuildDirectory 'bin\tests.exe') '*Microphone*,*Audio input*,[input],[sysmix],[sysmix_sink]' --reporter compact
