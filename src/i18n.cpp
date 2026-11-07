// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// The words a port draws, in the player's language (i18n.h).
#include "i18n.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace recomp::i18n {

namespace {

Language g_language = Language::English;
// (a count, so that a nested Untranslated gives the text itself until both are gone)
int g_untranslated = 0;

// English text to its translation, for the language shown. Empty in English: the text is the translation.
std::unordered_map<std::string, std::string> g_shown;
// The port's table, kept so that the language can change without a new table
const std::vector<Translation> *g_table = nullptr;
std::vector<Language> g_languages{Language::English};

void build() {
    g_shown.clear();
    g_languages.clear();
    g_languages.push_back(Language::English);
    if (!g_table || g_table->empty()) {
        return;
    }
    g_languages.push_back(Language::Chinese);
    if (g_language == Language::English) {
        return;
    }
    for (const Translation &row : *g_table) {
        if (row.first && row.second && *row.second) {
            g_shown[row.first] = row.second;
        }
    }
}

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

} // namespace

Language language() {
    return g_language;
}

void set_language(Language language) {
    if (language != Language::Chinese) {
        language = Language::English;
    }
    if (language == g_language) {
        return;
    }
    g_language = language;
    build();
}

void set_language(const std::string &name) {
    const std::string lowered = lower(name);
    // (a region's name, "zh-CN" or "zh_CN": its language's)
    set_language(lowered == "zh" || lowered.rfind("zh-", 0) == 0 || lowered.rfind("zh_", 0) == 0 ? Language::Chinese : Language::English);
}

const char *code(Language language) {
    return language == Language::Chinese ? "zh" : "en";
}

const char *name(Language language) {
    return language == Language::Chinese ? "中文" : "English";
}

const std::vector<Language> &languages() {
    return g_languages;
}

void set_translations(const std::vector<Translation> &table) {
    g_table = &table;
    build();
}

std::string translation_text() {
    std::string text;
    if (!g_table) {
        return text;
    }
    // (each text once: the font atlas is asked for the letters, not for the lines)
    for (const Translation &row : *g_table) {
        if (row.second && *row.second) {
            text += row.second;
            text += '\n';
        }
    }
    return text;
}

std::vector<unsigned> translation_codepoints() {
    std::vector<unsigned> points;
    if (!g_table) {
        return points;
    }
    // (UTF-8 by hand: recomp-common has no dependency for it, and the table is the port's own)
    for (const Translation &row : *g_table) {
        for (const char *at = row.second; at && *at;) {
            const unsigned char lead = static_cast<unsigned char>(*at);
            unsigned point = 0;
            int bytes = 1;
            if (lead < 0x80) {
                point = lead;
            } else if ((lead & 0xE0) == 0xC0) {
                point = lead & 0x1Fu;
                bytes = 2;
            } else if ((lead & 0xF0) == 0xE0) {
                point = lead & 0x0Fu;
                bytes = 3;
            } else if ((lead & 0xF8) == 0xF0) {
                point = lead & 0x07u;
                bytes = 4;
            } else {
                at++;
                continue;  // (not a UTF-8 lead byte)
            }
            bool whole = true;
            for (int i = 1; i < bytes; i++) {
                const unsigned char next = static_cast<unsigned char>(at[i]);
                if ((next & 0xC0) != 0x80) {
                    whole = false;
                    break;
                }
                point = (point << 6) | (next & 0x3Fu);
            }
            at += whole ? bytes : 1;
            if (whole && point >= 0x20 && point != 0x7F) {
                points.push_back(point);
            }
        }
    }
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    return points;
}

const char *tr(const char *text) {
    if (!text || g_untranslated > 0 || g_shown.empty()) {
        return text;
    }
    const auto found = g_shown.find(text);
    return found == g_shown.end() ? text : found->second.c_str();
}

Untranslated::Untranslated() {
    g_untranslated++;
}

Untranslated::~Untranslated() {
    g_untranslated--;
}

} // namespace recomp::i18n
