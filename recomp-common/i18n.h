// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
//
// The words a port draws, in the player's language. A port installs a table of English text and its
// translation (recomp::i18n::set_translations); the drawing code asks for a text with tr(), which gives
// the translation when there is one and the text itself when there is not. Nothing here knows of a game
// or a system.
//
// The language is the player's, not a feature of the game: it is kept with the launcher's own settings
// and changed on its Settings page. English is the text the port was written in, so a table is only
// needed for the others.
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace recomp::i18n {

// A port's table: the English text, and what it is in the language.
using Translation = std::pair<const char *, const char *>;

enum class Language { English, Chinese };

// The language the port draws in (English until set, and after set_language with a name it does not know)
Language language();
// A name as a file keeps it: "en", "zh", "zh-CN" (or "zh_CN"), case-insensitive. Unknown: English.
void set_language(const std::string &name);
void set_language(Language language);

// The names, for a file ("en", "zh") and for the player ("English", "中文")
const char *code(Language language);
const char *name(Language language);

// The languages there is a table for, English first; the launcher offers these. English alone until a
// table is installed.
const std::vector<Language> &languages();

// The port's table, installed once at the start. Copied: the port may hand a temporary.
void set_translations(const std::vector<Translation> &table);

// Every text of the table's translations, joined: the letters a font must have to draw them (a launcher
// asks for their glyphs before it draws, so that a text drawn straight to the draw list has them).
std::string translation_text();

// The code points of every translation, sorted and without repeats: the glyph ranges a font is given, so
// that all of them are in the atlas from the first frame.
std::vector<unsigned> translation_codepoints();

// The translation of `text`, or `text` itself when there is none (and for nullptr). The pointer is kept
// until the table is replaced: safe to hand to a drawing call.
const char *tr(const char *text);

// While this is set, tr() gives the text itself: for what is drawn in a font that has no other language's
// letters (the in-game menu's 5x7 dot matrix, which would draw "?" for them). The drawing code that can show
// them (a launcher's own font) leaves it off.
class Untranslated {
public:
    Untranslated();
    ~Untranslated();
    Untranslated(const Untranslated &) = delete;
    Untranslated &operator=(const Untranslated &) = delete;
};

} // namespace recomp::i18n
