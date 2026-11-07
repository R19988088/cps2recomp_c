// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// Downloading a game's file from where the port says it is (launcher.h, Description::download): the system's curl
// (macOS, Windows 10 and later and Linux have one), or Android's own HTTP (run_program.h).

#include <filesystem>
#include <string>

namespace launcher {

// Downloads `url` to `to`, through `to` + ".part" so that a download cut short is never taken for the file (it is
// removed). Other threads may watch that file's size. Blocks: for another thread. 0 when it worked.
int fetch_to_file(const std::string& url, const std::filesystem::path& to);

// What fetch_to_file's code means, for the player
std::string fetch_problem(int code);

// The same file name with ".part" after it: where a download of `to` is written while it comes
std::filesystem::path part_of(const std::filesystem::path& to);

}  // namespace launcher
