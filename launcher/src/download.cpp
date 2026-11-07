// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// Downloading a game's file (download.h), with the system's curl as the updates are (update.cpp).
#include "download.h"

#include "run_program.h"

#include <cctype>
#include <chrono>
#include <system_error>
#include <thread>
#include <vector>

namespace launcher {

namespace fs = std::filesystem;

namespace {

// A URL is only ever one the port's description gave: https, http, or a file:// one for tests
bool url_allowed(const std::string& url) {
    if (url.empty() || url.find_first_of("\"' \n\r\t") != std::string::npos) {
        return false;
    }
    return url.rfind("https://", 0) == 0 || url.rfind("http://", 0) == 0 || url.rfind("file://", 0) == 0;
}

}  // namespace

fs::path part_of(const fs::path& to) {
    fs::path part = to;
    part += ".part";
    return part;
}

int fetch_to_file(const std::string& url, const fs::path& to) {
    if (!url_allowed(url)) {
        return 22;
    }
    std::error_code error;
    fs::create_directories(to.parent_path(), error);
    const fs::path part = part_of(to);
    fs::remove(part, error);
    // (a transfer cut short — archive.org does that now and then — is a network error, which curl's own --retry does
    // not retry on the versions Windows 10 ships. So the tries are here, and each takes the file on from where it
    // got to: --speed-limit/time give up on a server that has stalled instead of waiting forever.)
    int code = 0;
    for (int attempt = 0; attempt < 4; attempt++) {
        std::vector<std::string> args = {"-fsSL", "--connect-timeout", "20", "--speed-limit", "1024", "--speed-time", "60", "-A",
                                         "recomp-launcher", "-o", part.u8string()};
        if (attempt > 0 && fs::exists(part, error)) {
            args.push_back("-C");
            args.push_back("-");  // (on from where it got to)
        }
        args.push_back(url);
        code = run_program(curl_program(), args, nullptr);
        if (code == 0) {
            fs::rename(part, to, error);
            if (error) {
                fs::remove(part, error);
                return 23;  // (curl's "write error": it could not be put where it belongs)
            }
            return 0;
        }
        if (code == 22 || code == 6) {
            break;  // (not there, or no connection: trying again would only wait)
        }
        std::this_thread::sleep_for(std::chrono::seconds(2 * (attempt + 1)));
    }
    fs::remove(part, error);
    return code;
}

std::string fetch_problem(int code) {
    switch (code) {
    case -1: return "the download could not be started (is curl there?)";
    case 6:
    case 7: return "no connection to the internet";
    case 22: return "the file was not found on the server";
    case 23: return "the file could not be written where it belongs";
    case 28: return "the connection timed out";
    default: return "the download failed (curl " + std::to_string(code) + ")";
    }
}

}  // namespace launcher
