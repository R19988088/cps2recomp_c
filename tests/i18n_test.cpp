// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// The words a port draws in the player's language (src/i18n.h).
#include "i18n.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace {

const std::vector<recomp::i18n::Translation> kTable = {
    {"Features", "功能"},
    {"Search features", "搜索功能"},
    {"", ""},
};

} // namespace

int main() {
    using recomp::i18n::Language;
    // English before any table: every text is itself, and English is the only language
    assert(recomp::i18n::language() == Language::English);
    assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);
    assert(recomp::i18n::languages().size() == 1);

    recomp::i18n::set_translations(kTable);
    // A table means there is Chinese to offer, and English is still what is shown
    assert(recomp::i18n::languages().size() == 2);
    assert(recomp::i18n::languages()[0] == Language::English);
    assert(recomp::i18n::languages()[1] == Language::Chinese);
    assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);

    recomp::i18n::set_language("zh");
    assert(recomp::i18n::language() == Language::Chinese);
    assert(std::strcmp(recomp::i18n::tr("Features"), "功能") == 0);
    assert(std::strcmp(recomp::i18n::tr("Search features"), "搜索功能") == 0);
    // A text the table does not have is itself, and so is no text at all
    assert(std::strcmp(recomp::i18n::tr("Controllers"), "Controllers") == 0);
    assert(recomp::i18n::tr(nullptr) == nullptr);
    // (an empty translation means the text itself, not an empty line)
    assert(std::strcmp(recomp::i18n::tr(""), "") == 0);

    // A region's name is its language's
    recomp::i18n::set_language("en");
    assert(recomp::i18n::language() == Language::English);
    recomp::i18n::set_language("zh-CN");
    assert(recomp::i18n::language() == Language::Chinese);
    recomp::i18n::set_language("ZH_tw");
    assert(recomp::i18n::language() == Language::Chinese);
    // A language it does not know is English
    recomp::i18n::set_language("fr");
    assert(recomp::i18n::language() == Language::English);
    assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);

    // The names, as a file and as the player reads them
    assert(std::strcmp(recomp::i18n::code(Language::English), "en") == 0);
    assert(std::strcmp(recomp::i18n::code(Language::Chinese), "zh") == 0);
    assert(std::strcmp(recomp::i18n::name(Language::Chinese), "中文") == 0);

    // A title with no translation at all: the same
    // (drawn where the font has no other language's letters: the text itself, even in Chinese)
    {
        recomp::i18n::Untranslated untranslated;
        assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);
        {
            recomp::i18n::Untranslated also;
            assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);
        }
        assert(std::strcmp(recomp::i18n::tr("Features"), "Features") == 0);
    }
    recomp::i18n::set_language("zh");
    assert(std::strcmp(recomp::i18n::tr("Features"), "功能") == 0);

    std::puts("i18n_test: all as it should be");
    return 0;
}
