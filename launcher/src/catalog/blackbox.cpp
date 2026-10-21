// Prospero BlackBox - The built-in catalog: PES 2013.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "catalog/blackbox.hpp"

namespace store::catalog
{
namespace
{
constexpr Game kGames[] = {
    {"PPSA99405", "PES2013", "PES 2013", "2012-09-20", "futbol13",
     "Football from your own copy of PES 2013, with the squad updates and commentary you "
     "installed on the PC. 4K picture, one pad per signed-in player.",
     true, {0x06240f, 0x0f5a2a, 0xf2f6f0, 0x0a2a14}}, // pitch green, white
};
} // namespace

std::span<const Game> blackbox_games()
{
    return kGames;
}

const Palette &blackbox_palette(std::string_view id)
{
    for (const auto &game : kGames)
        if (id == game.id)
            return game.palette;
    return kManagerPalette;
}

Snapshot blackbox_snapshot(const std::string &app_folder)
{
    Snapshot snapshot;
    for (const auto &game : kGames)
    {
        Entry entry;
        entry.id = game.id;
        entry.name = game.name;
        entry.author = "FUTBOL13";
        entry.kind = "game";
        entry.status = game.available ? "available" : "coming_soon";
        entry.released = game.released;
        entry.description = game.description;
        entry.icon = app_folder + "/assets/games/" + game.art + ".png";
        snapshot.entries.push_back(std::move(entry));
    }
    // Built into the app: nothing to verify or download.
    snapshot.verified = true;
    snapshot.accepted = true;
    snapshot.online = false;
    return snapshot;
}
} // namespace store::catalog
