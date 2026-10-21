#!/usr/bin/env bash
# FUTBOL13 - host install check: the launcher's installer run against a copy of your PES 2013
# folder.
# SPDX-License-Identifier: GPL-3.0-or-later
#
#   tools/host-install.sh [--size WxH] [option...]      (the PES 2013 plan has no options)
#
# PES       your PES 2013 folder, the one with pes2013.exe and img (read-only: it is copied,
#           never changed or linked to)
# HOST_DIR  where the copy, the prefix and the install go (default: ./host-check, untracked);
#           on the same btrfs/xfs as PES the copy is a reflink and costs no space; a save folder
#           put in HOST_DIR/data/saves/PES2013/save/ beforehand is seeded into the prefix
# BARE      the bare prefix zip (default: release/bare-prefix-futbol13.zip)
set -euo pipefail
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
L=$here/launcher
PES=${PES:-"$HOME/Games/pes2013/drive_c/Program Files (x86)/KONAMI/Pro Evolution Soccer 2013"}
HOST_DIR=${HOST_DIR:-$here/host-check}
BARE=${BARE:-"$here/release/bare-prefix-futbol13.zip"}
[[ -f "$PES/pes2013.exe" ]] || { echo "no pes2013.exe in $PES" >&2; exit 1; }
[[ -f $BARE ]] || { echo "no bare prefix at $BARE (tools/build.sh prefix)" >&2; exit 1; }

# The installer, compiled as store-test.sh compiles store_game_install_test.
build=$L/build/host-install
mkdir -p "$build"
objects=()
for name in yyjson/yyjson miniz/miniz miniz/miniz_tinfl miniz/miniz_tdef miniz/miniz_zip; do
    object=$build/${name//\//_}.o
    [[ $object -nt $L/src/third_party/$name.c ]] || clang -std=c11 -O1 -w -c "$L/src/third_party/$name.c" -o "$object"
    objects+=("$object")
done
clang++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -I"$L/src" \
    "$L/host/install_main.cpp" "$L/src/library/game_sources.cpp" "$L/src/library/prefix.cpp" \
    "$L/src/library/game_install.cpp" "$L/src/library/upgrade.cpp" "$L/src/library/rtpatch.cpp" \
    "$L/src/library/install_plan.cpp" "$L/src/library/game_profile.cpp" \
    "$L/src/library/wine_registry.cpp" "$L/src/library/base_game.cpp" "$L/src/library/tree.cpp" \
    "$L/src/core/i18n.cpp" "$L/src/core/i18n_strings.cpp" "$L/src/core/save_file.cpp" \
    "${objects[@]}" -pthread -o "$build/host-install"

# base-games/PES2013: a real copy of the user's folder (reflinked where the file system can).
data=$HOST_DIR/data
win=$HOST_DIR/win
base=$data/base-games/PES2013
if [[ ! -f "$base/pes2013.exe" ]]; then
    mkdir -p "$data/base-games"
    rm -rf "$base.part"
    cp -a --reflink=auto "$PES" "$base.part"
    if [[ -n $(find "$base.part" -type l -print -quit) ]]; then
        echo "the copy holds symlinks; refusing" >&2
        exit 1
    fi
    mv "$base.part" "$base"
fi
mkdir -p "$win/prefixes" "$win/profiles"  # both are there on the console

"$build/host-install" "$L/assets/plans/PES2013.json" "$data" "$win" --bare-prefix "$BARE" "$@"

game=$win/prefixes/futbol13/drive_c/Games/PES2013
echo "== $game"
ls -la "$game" 2>/dev/null || true
for documents in "$win"/prefixes/futbol13/drive_c/users/*/Documents/KONAMI/*; do
    echo "== $documents"
    ls -laR "$documents"
done
