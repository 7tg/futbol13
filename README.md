# FUTBOL13

FUTBOL13 runs your own copy of the PC game *Pro Evolution Soccer 2013* (Konami, 2012) on a
jailbroken PS5, as a homebrew app with its own Home-screen tile (`PPSA99405`). It runs the PC
version through [prospero-win](https://github.com/mpereiraesaa/prospero-win): Wine, DXVK and
the PS5's Vulkan driver. It is made from [Sahin](https://github.com/7tg/sahin), which does the
same for Blur.

Türkçe kılavuz: [docs/KILAVUZ.md](docs/KILAVUZ.md).

**You need your own copy of the game. Nothing here contains it.** The repo and the app
hold only our own code, patches, configuration and art, plus open-source projects built from
source. The app downloads nothing. FUTBOL13 is not affiliated with Konami or Sony.

## What the app does

- Installs PES 2013 from your folder into its Wine prefix, as it is. Your copy is never
  changed.
- Takes your own copy's folder as you set it up on the PC, with the squad updates or
  commentary in your language you added there; it is installed as it is. Only `pes2013.exe`
  and `img/` must be there, and the only file added is our `dxvk.conf`.
- Carries your saves and squads over from the PC, if you copy them (see below).
- Writes the product code from your `code.txt` into the game's settings in the prefix (the
  registry), where the PC installer kept it.
- Writes the game's `settings.dat` itself, since PES's `settings.exe` has no place on the
  console: full screen, the highest quality, the pad as an XInput controller, and the
  resolution chosen on the page.
- Starts the game straight from its tile. **Hold □ while the tile starts** to open
  FUTBOL13's page instead. The page has the resolution (3840×2160, 2560×1440 or 1920×1080),
  a performance overlay (DXVK's HUD), Repair (△) and Play.
- In the game, the touchpad moves the mouse pointer and pressing it clicks. To quit, hold
  Options and the touchpad together for about a second.
- The DualSense rumbles when the game asks for it.
- Each signed-in PS5 user gets a pad, for local matches.
- FUTBOL13's own page is in Turkish on a console set to Turkish, and in English on any
  other. The game's own language is chosen in its menus.

## What you need

- A PS5 that runs homebrew: an FTP server and an ELF loader with elfldr on local port 9021,
  and [ShadowMount+](https://github.com/drakmor/ShadowMountPlus) to install apps from
  `/data/homebrew`. Developed for firmware 13.60 with kstuff-lite, ShadowMount+ and Payload
  Manager.
- **Your PES 2013 folder**, the one with `pes2013.exe` and `img/`, set up on the PC as you
  want it. A copy that doesn't start under Wine on a PC won't start here either: if the game
  closes as soon as it starts, the page tells you so on the next start. FUTBOL13 does not
  remove or get around copy protection and does not point to anything that does.

## On the console

1. Copy `release/PPSA99405/` to `/data/homebrew/PPSA99405/` over FTP. ShadowMount+ adds the
   tile; reboot if it doesn't appear.
2. Put your product code, as you typed it when installing PES 2013 on the PC, on one line in
   a file named `code.txt` inside the game folder (next to `pes2013.exe`) before you copy the
   folder: a USB copy is imported as it is, so `code.txt` must be in its folder on the USB
   drive, and a folder already in `/data/futbol13/base-games/PES2013/` must get it over FTP
   before you press Install. Without it the game says it is not installed, and the page says
   so too.
3. Copy your whole PES 2013 folder to `/data/futbol13/base-games/PES2013` over FTP, or put it
   on a USB drive in a folder named `PES2013`. A USB copy is copied to the console as part of
   the install.
4. If you want your saves and squads: on the PC, PES keeps its options, edited squads, master
   league and the `EDIT` file a squad update gives in
   `Documents\KONAMI\Pro Evolution Soccer 2013\save\`. Copy that `save` folder to
   `/data/futbol13/saves/PES2013/save/` **before you install**. A squad update applied on the
   PC often lives there, not in the game folder, so this step is what carries it over.
   The install only adds files the game doesn't have yet; it never changes the ones it has.
   The game's own files are in
   `/data/prospero-win/prefixes/futbol13/drive_c/users/<user>/Documents/KONAMI/Pro Evolution Soccer 2013/save/`;
   to bring in newer squads later, remove the old files there over FTP, then repair.
5. Open FUTBOL13 and press Install. It copies your game into the prefix, writes `dxvk.conf`
   and `settings.dat`, writes your product code and the system values PES's start-up check reads into the prefix's registry, and places your save files where the game keeps its save, for each
   Windows user of the prefix, wherever the game has none. Nothing is downloaded.

Once the game has save files of its own, install and repair never replace them.

After that, the tile starts the game.

## Building

Everything builds with one script inside a pinned container (Ubuntu 24.04 by digest, apt from
a dated snapshot), from the committed sources. The same commit gives the same bytes.

```sh
git clone --recurse-submodules https://github.com/7tg/futbol13 futbol13
cd futbol13 && tools/build.sh
```

You need x86-64 Linux with Docker. The app and its zip land in `release/PPSA99405/` and
`release/PPSA99405.zip`, with `release/SHA256SUMS`. Steps can be run on their own:
`tools/build.sh game-app audit`. The `audit` step fails the build if the repo or the app holds
game data or a Windows executable not built from source.

Releases: <https://github.com/7tg/futbol13/releases>.

Checks on the build machine, without a console:

- `python3 -m pytest tests`: the plan generator, the `settings.dat` layout, the art, the
  audit, the app zip and the notices.
- `cd launcher && bash tools/store-test.sh`: the launcher's tests (clang, ASan/UBSan).
- `tools/host-install.sh`: the launcher's own installer run against your PES 2013 folder
  (`PES=…`). Your folder is read only, never changed or linked to.
- `tools/host-run.sh 120`: that install started under the host's Wine with the DXVK the build
  made. `HEADLESS=1920x1080` runs it inside a headless gamescope.

## What is where

| Path | What |
| --- | --- |
| `tools/` | the build (`build.sh`), prospero-win, DXVK, the ps5-opengl SDK, the bare prefix, the bundle, the app's `SOURCES.txt` and `THIRD_PARTY.md` (`make-notices.py`), the app zip, the audit, the host checks |
| `patches/` | changes to prospero-win, PS5_Mesa, PS5_Vulkan and DXVK, from nfs-blackbox-ps5 |
| `launcher/` | the app's launcher (`eboot.bin`): copy check, install, settings page. A fork of nfs-blackbox-launcher. |
| `profile/pes2013/` | PES 2013 as the install sees it: the plan generator (`make-pes2013-plan.py`), our `dxvk.conf`, the bare prefix (`bare-prefix.yml`), and `settings_dat.py`, the `settings.dat` layout and template as data |
| `art/` | the Home-screen art, drawn by `make-art.py` (nothing from the game) |
| `sce_sys/param.json` | a copy of the `param.json` the app ships (written by the build), so the title ID and content version can be read from the repo |
| submodules | prospero-win, PS5_Mesa, PS5_Vulkan, PS5_PayloadSDK, ps5-native-app-boilerplate and ps5-opengl, at pinned commits |

## Credits

- [Sahin](https://github.com/7tg/sahin) by 7tg. FUTBOL13 is made from it.
- [nfs-blackbox-ps5](https://github.com/lavavex/nfs-blackbox-ps5) and
  [nfs-blackbox-launcher](https://github.com/lavavex/nfs-blackbox-launcher) by lavavex, which
  Sahin is built from.
- [prospero-win](https://github.com/mpereiraesaa/prospero-win), PS5_Mesa, PS5_Vulkan and the
  [ps5-opengl](https://github.com/mpereiraesaa/ps5-opengl) fork by mpereiraesaa; ps5-opengl,
  the native app boilerplate and the launcher's UI originally by BlackBearReloaded; the PS5
  Payload SDK.
- Wine, Mesa, DXVK and LLVM: the stack underneath.
- *Pro Evolution Soccer 2013* is Konami's. FUTBOL13 is not affiliated with Konami or Sony.

## License

GPL-3.0 (`LICENSE`). The patches in `patches/` follow the licences of the projects they
change, and the submodules keep their own. The app folder's `THIRD_PARTY.md` says what each
part of it is and under which licence, with the texts in `LICENSES/`, and its `SOURCES.txt`
names the exact source of each part: this repository at the commit the app was built from,
the launcher, DXVK, and prospero-win's own records of Wine, Mesa and the rest.
