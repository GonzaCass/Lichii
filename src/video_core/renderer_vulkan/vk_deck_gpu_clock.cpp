// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "video_core/renderer_vulkan/vk_deck_gpu_clock.h"

#ifdef __linux__
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <system_error>
#include <vector>

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common/logging.h"
#include "video_core/vulkan_common/vulkan_device.h"

extern char** environ;
#endif

namespace Vulkan {

#ifdef __linux__
namespace {

namespace fs = std::filesystem;

constexpr const char* PrivWriteHelper = "/usr/bin/steamos-polkit-helpers/steamos-priv-write";
constexpr const char* HostExec = "distrobox-host-exec";
constexpr u32 AmdVendorId = 0x1002;
constexpr u32 VanGoghLcdDeviceId = 0x1435;
constexpr u32 VanGoghOledDeviceId = 0x163F;
constexpr long DefaultMinMhz = 1100;
constexpr long LowestMhz = 200;
constexpr long DefaultMaxMhz = 1600;

std::string Trim(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
        text.pop_back();
    }
    return text;
}

std::string ReadText(const fs::path& path) {
    std::ifstream file{path};
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

bool Run(const std::vector<std::string>& args) {
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (const std::string& argument : args) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);
    pid_t child{};
    if (posix_spawnp(&child, argv[0], nullptr, nullptr, argv.data(), environ) != 0) {
        return false;
    }
    int status{};
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) {
            return false;
        }
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/// Writes a sysfs file. SteamOS's helper leaves the file group-writable after its first write,
/// so later writes can go straight in. From inside a container the helper is run on the host.
bool PrivWrite(const std::string& path, const std::string& value) {
    {
        std::ofstream file{path};
        if (file && (file << value).flush()) {
            return true;
        }
    }
    if (fs::exists(PrivWriteHelper)) {
        return Run({PrivWriteHelper, path, value});
    }
    return Run({HostExec, PrivWriteHelper, path, value});
}

/// The table's highest clock: the line "1: <n>Mhz" of its OD_SCLK section.
long MaxClockOf(const std::string& table) {
    std::istringstream lines{table};
    std::string line;
    bool in_sclk = false;
    while (std::getline(lines, line)) {
        if (line.rfind("OD_SCLK", 0) == 0) {
            in_sclk = true;
        } else if (line.rfind("OD_", 0) == 0) {
            in_sclk = false;
        } else if (in_sclk && line.rfind("1:", 0) == 0) {
            const long value = std::strtol(line.c_str() + 2, nullptr, 10);
            return value > 0 ? value : DefaultMaxMhz;
        }
    }
    return DefaultMaxMhz;
}

/// The directory of the DRM card of this GPU ("/sys/class/drm/cardN/device"), or empty.
std::string FindCard(u32 vendor_id, u32 device_id) {
    std::error_code error;
    for (const auto& entry : fs::directory_iterator("/sys/class/drm", error)) {
        const std::string name = entry.path().filename().string();
        if (name.rfind("card", 0) != 0 || name.find('-') != std::string::npos) {
            continue;
        }
        const fs::path device_dir = entry.path() / "device";
        const auto vendor = std::strtoul(ReadText(device_dir / "vendor").c_str(), nullptr, 16);
        const auto device = std::strtoul(ReadText(device_dir / "device").c_str(), nullptr, 16);
        if (vendor == vendor_id && device == device_id) {
            return device_dir.string();
        }
    }
    return {};
}

/// A process that outlives this one and puts the clock back when this one ends, however it ends.
void StartWatcher(const std::string& level_path, const std::string& table_path) {
    const std::string helper = PrivWriteHelper;
    // A process that was killed stays a zombie until its parent collects it: that counts as ended.
    const std::string script = "p=/proc/" + std::to_string(getpid()) +
                               "/status; while [ -r $p ] && ! grep -q \"^State:.Z\" $p; do "
                               "sleep 1; done; " +
                               helper + " " + table_path + " r; " + helper + " " + table_path +
                               " c; " + helper + " " + level_path + " auto";
    const std::string host = fs::exists(PrivWriteHelper) ? std::string{} : std::string{HostExec} + " ";
    Run({"sh", "-c", "setsid " + host + "sh -c '" + script + "' >/dev/null 2>&1 &"});
}

} // Anonymous namespace
#endif

DeckGpuClock::DeckGpuClock([[maybe_unused]] const Device& device) {
#ifdef __linux__
    if (const char* option = std::getenv("EDEN_NX2_GPU_CLOCK");
        option != nullptr && std::string_view{option} == "0") {
        return;
    }
    const bool van_gogh = device.GetVendorID() == AmdVendorId &&
                          (device.GetDeviceID() == VanGoghLcdDeviceId ||
                           device.GetDeviceID() == VanGoghOledDeviceId) &&
                          device.GetDriverID() == VK_DRIVER_ID_MESA_RADV;
    if (!van_gogh) {
        return;
    }
    const std::string card = FindCard(device.GetVendorID(), device.GetDeviceID());
    if (card.empty()) {
        LOG_INFO(Render_Vulkan, "Deck GPU clock: no DRM card found for this GPU");
        return;
    }
    level_path = card + "/power_dpm_force_performance_level";
    table_path = card + "/pp_od_clk_voltage";
    if (const std::string level = Trim(ReadText(level_path)); level != "auto") {
        LOG_INFO(Render_Vulkan, "Deck GPU clock: left alone, the GPU is in '{}' mode", level);
        return;
    }
    const long max_mhz = MaxClockOf(ReadText(table_path));
    long min_mhz = DefaultMinMhz;
    if (const char* option = std::getenv("EDEN_NX2_GPU_MIN_MHZ"); option != nullptr) {
        min_mhz = std::strtol(option, nullptr, 10);
    }
    min_mhz = std::clamp(min_mhz, LowestMhz, max_mhz);
    const bool written = PrivWrite(level_path, "manual") &&
                         PrivWrite(table_path, "s 0 " + std::to_string(min_mhz)) &&
                         PrivWrite(table_path, "s 1 " + std::to_string(max_mhz)) &&
                         PrivWrite(table_path, "c");
    if (!written) {
        LOG_WARNING(Render_Vulkan, "Deck GPU clock: the clock range could not be set");
        PrivWrite(table_path, "r");
        PrivWrite(table_path, "c");
        PrivWrite(level_path, "auto");
        return;
    }
    active = true;
    StartWatcher(level_path, table_path);
    LOG_INFO(Render_Vulkan, "Deck GPU clock: {} to {} MHz while the renderer runs "
                            "(EDEN_NX2_GPU_CLOCK=0 turns it off)",
             min_mhz, max_mhz);
#endif
}

DeckGpuClock::~DeckGpuClock() {
#ifdef __linux__
    if (active) {
        PrivWrite(table_path, "r");
        PrivWrite(table_path, "c");
        PrivWrite(level_path, "auto");
        LOG_INFO(Render_Vulkan, "Deck GPU clock: back to automatic");
    }
#endif
}

} // namespace Vulkan
