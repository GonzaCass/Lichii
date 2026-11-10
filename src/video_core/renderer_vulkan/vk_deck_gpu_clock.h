// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace Vulkan {

class Device;

/// Steam Deck (AMD Van Gogh, RADV): keeps the GPU's minimum clock up while the renderer lives.
///
/// The Deck's power governor leaves the GPU at 200 MHz for stretches of a frame even though its
/// busy counter reads 100%: the work arrives in bursts, and a burst that starts at 200 MHz runs
/// slowly until the governor reacts. Raising the lowest clock the GPU may drop to (the same
/// "manual" range the Steam menu's GPU clock slider sets) costs a little power and gives about
/// 10% more frames in Ocarina of Time. Fixing the clock at its maximum gives only 2% more, and
/// the CPU and GPU share one power budget, so the minimum is what is raised.
///
/// Nothing happens unless the device is a Van Gogh GPU on RADV, the GPU is in automatic mode
/// (a clock set in the Steam menu is left alone), and EDEN_NX2_GPU_CLOCK is not 0.
/// EDEN_NX2_GPU_MIN_MHZ sets the minimum (default 1100, between 200 and the table's maximum).
/// The writes go through SteamOS's privileged helper (steamos-priv-write, no password), from
/// inside a container through distrobox-host-exec. The settings are put back (automatic mode)
/// when the object is destroyed, and by a watcher process when this process ends without
/// destroying it.
class DeckGpuClock {
public:
    explicit DeckGpuClock(const Device& device);
    ~DeckGpuClock();

    DeckGpuClock(const DeckGpuClock&) = delete;
    DeckGpuClock& operator=(const DeckGpuClock&) = delete;

    [[nodiscard]] bool Active() const {
        return active;
    }

private:
    bool active{};
    std::string level_path;
    std::string table_path;
};

} // namespace Vulkan
