#!/usr/bin/env bash
# FUTBOL13 - host run: the PES 2013 tools/host-install.sh installed, started under the host's
# Wine with the DXVK the build made.
# SPDX-License-Identifier: GPL-3.0-or-later
#
#   tools/host-run.sh [seconds]     (seconds: stop it after that long; default: run until it exits)
#
# The installed prefix (HOST_DIR/win/prefixes/futbol13) is prospero-win's: its 32-bit side is set
# up for the wowprospero CPU, and the host's Wine can't open a window from a 32-bit program in
# it. So the run uses a host prefix of its own (HOST_DIR/run/prefix, made once with wineboot):
# the installed game folder is copied into it on every run, with the installed prefix's DXVK
# DLLs (the build's, x32 into syswow64 and x64 into system32) and its Documents/KONAMI folder
# (settings.dat and the save) into the run prefix's user.
#
# HOST_DIR  as for tools/host-install.sh (default: ./host-check)
# PES_CODE  your PES 2013 CD key, written where the game's installer puts it (the game won't
#           start without it): "code" under [Software\\Wow6432Node\\KONAMI\\PES2013] in
#           your own Wine prefix's system.reg
# WINE      the host's wine (default: wine)
# HEADLESS  WxH: run inside a headless gamescope of that size instead of on the desktop (no
#           window, no focus taken); screenshots with
#           GAMESCOPE_WAYLAND_DISPLAY=gamescope-0 gamescopectl screenshot FILE.png, keys through
#           XTEST on its DISPLAY (printed, e.g. python-xlib's xtest.fake_input). PES reads its
#           picture size from settings.dat, which the install wrote.
set -euo pipefail
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
HOST_DIR=${HOST_DIR:-$here/host-check}
WINE=${WINE:-wine}
seconds=${1:-0}
if [[ -n ${HEADLESS:-} ]]; then
    w=${HEADLESS%x*} h=${HEADLESS#*x}
    exec env -u HEADLESS -u DISPLAY -u WAYLAND_DISPLAY HOST_DIR="$HOST_DIR" \
        gamescope --backend headless -w "$w" -h "$h" -W "$w" -H "$h" -- \
        bash -c 'echo "== gamescope DISPLAY=$DISPLAY"; exec "$0" "$@"' "$0" "$@"
fi
installed=$HOST_DIR/win/prefixes/futbol13
[[ -f $installed/drive_c/Games/PES2013/pes2013.exe ]] || { echo "no install in $installed: run tools/host-install.sh" >&2; exit 1; }
[[ -n ${DISPLAY:-}${WAYLAND_DISPLAY:-} ]] || { echo "no display (DISPLAY / WAYLAND_DISPLAY)" >&2; exit 1; }

export WINEPREFIX=$HOST_DIR/run/prefix WINEARCH=win64
if [[ ! -f $WINEPREFIX/system.reg ]]; then
    mkdir -p "$HOST_DIR/run"
    WINEDEBUG=-all wineboot -i
    wineserver -w
fi
for dll in d3d8 d3d9 d3d10core d3d11 dxgi; do
    cp "$installed/drive_c/windows/syswow64/$dll.dll" "$WINEPREFIX/drive_c/windows/syswow64/"
    cp "$installed/drive_c/windows/system32/$dll.dll" "$WINEPREFIX/drive_c/windows/system32/"
done
game=$WINEPREFIX/drive_c/Games/PES2013
rm -rf "$game"
mkdir -p "${game%/*}"
cp -a --reflink=auto "$installed/drive_c/Games/PES2013" "$game"

# Documents/KONAMI from the installed prefix's user (the bare prefix's, not Public) into the run
# prefix's user. Host Wine may have made Documents a link to the real home's: it is replaced by
# a directory of the prefix's own, so nothing is written through it.
from=
for user in "$installed"/drive_c/users/*; do
    if [[ ${user##*/} != Public && -d $user/Documents/KONAMI ]]; then
        from=$user/Documents/KONAMI
        break
    fi
done
[[ -n $from ]] || { echo "no Documents/KONAMI in $installed/drive_c/users" >&2; exit 1; }
documents=$WINEPREFIX/drive_c/users/$USER/Documents
if [[ -L $documents ]]; then
    rm "$documents"
fi
mkdir -p "$documents"
rm -rf "$documents/KONAMI"
cp -a "$from" "$documents/KONAMI"

# The game's registry key (KONAMI\PES2013: installdir, version), as the install wrote it into
# the installed prefix, and PES_CODE as its "code": the CD key the game's own installer stores
# and pes2013.exe asks for at start, which no install of ours can write.
key='Software\Wow6432Node\KONAMI\PES2013'
SECTION="[${key//\\/\\\\}]" awk 'index($0, ENVIRON["SECTION"]) == 1 { on = 1; next }
    on && /^$/ { exit }
    on && /^"[^"]*"="/ { print }' "$installed/system.reg" |
    sed 's/^"\([^"]*\)"="\(.*\)"$/\1\t\2/; s/\\\\/\\/g' |
    while IFS=$'\t' read -r name value; do
        WINEDEBUG=-all "$WINE" reg add "HKLM\\$key" /v "$name" /t REG_SZ /d "$value" /f >/dev/null
    done
if [[ -n ${PES_CODE:-} ]]; then
    WINEDEBUG=-all "$WINE" reg add "HKLM\\$key" /v code /t REG_SZ /d "$PES_CODE" /f >/dev/null
fi
wineserver -w

# The profile's overrides (PES2013.json: dll_overrides): DXVK's d3d9/dxgi, dinput8 native first.
export WINEDLLOVERRIDES=${WINEDLLOVERRIDES:-"d3d9,dxgi=n;dinput8=n,b"}
export DXVK_HUD=${DXVK_HUD:-fps}
export WINEDEBUG=${WINEDEBUG:-+loaddll,err}
log=$HOST_DIR/run.log
touch "$HOST_DIR/run.log.start"
echo "== $WINEPREFIX: pes2013.exe, log in $log"
cd "$game"
if (( seconds > 0 )); then
    timeout "$seconds" "$WINE" pes2013.exe > "$log" 2>&1 || true
    wineserver -k || true
else
    "$WINE" pes2013.exe > "$log" 2>&1
fi
