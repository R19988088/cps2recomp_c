#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# A macOS app built on this machine, from whatever has been built here, in dist/: the launcher, the game
# programs beside it, SDL3, and a Chinese font. No ROM set goes in (the launcher reads the player's own).
#
#   tools/make_local_app.sh          the collection if build/collection has it, else the stand-in game (example)
#   B=folder                         where the programs are (build/collection or build/example)
#   CJK_FONT=path/to/font.ttf        the Chinese font to carry (else the system's own is used)
#
# Built from the source tree, a bundle is not a release's download: it has no update source (see
# launcher/CMakeLists.txt), so it never replaces itself with another project's release.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

if [ -z "${B:-}" ]; then
    if [ -x build/collection/cps2recomp-launcher ]; then
        B="$ROOT/build/collection"
        what=--collection
    else
        B="$ROOT/example/build/example"
        what=example
    fi
fi
echo "app from $B"

# (a font the bundle can carry: the first the system has, unless CJK_FONT names one)
if [ -z "${CJK_FONT:-}" ]; then
    for font in /System/Library/Fonts/Supplemental/Songti.ttc /System/Library/Fonts/STHeiti\ Medium.ttc; do
        [ -f "$font" ] && { CJK_FONT="$font"; break; }
    done
fi
export CJK_FONT
export FORCE=1 B

# (make_app.sh reads games/<id>/game.cfg from the folder make runs in: the example's own tree for the stand-in)
if [ "$what" = example ]; then
    (cd example && sh "$ROOT/tools/make_app.sh" example)
else
    sh "$ROOT/tools/make_app.sh" --collection
fi

# (the app's name, from the plist make_app.sh wrote)
app=$(find "$B" -maxdepth 1 -name '*.app' -type d | head -1)
mkdir -p dist
rm -rf "dist/$(basename "$app")"
cp -R "$app" dist/
echo "dist/$(basename "$app")"
