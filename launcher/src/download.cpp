// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// Downloading a game's file (download.h), with the system's curl as the updates are (update.cpp).
#include "download.h"

#include "run_program.h"

#include <cctype>
#include <system_error>

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
    const int code = run_program(curl_program(),
                                 {"-fsSL", "--connect-timeout", "20", "--speed-limit", "1024", "--speed-time", "60", "--retry", "2", "-A",
                                  "recomp-launcher", "-o", part.u8string(), url},
                                 nullptr);
    if (code != 0) {
        fs::remove(part, error);
        return code;
    }
    fs::rename(part, to, error);
    if (error) {
        fs::remove(part, error);
        return 23;  // (curl's "write error": it could not be put where it belongs)
    }
    return 0;
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
