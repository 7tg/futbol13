// Prospero BlackBox - The user's clean copy of a game: found, checked, imported.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "library/base_game.hpp"
#include "core/i18n.hpp"
#include "library/tree.hpp"
#include "third_party/miniz/miniz.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <tuple>
#include <dirent.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

namespace store::library
{
namespace
{
// ---- the games ----------------------------------------------------------------

// Pro Evolution Soccer 2013 for PC: players mod it heavily (squad updates, commentary), so
// the copy is taken as it is. Only the exe and the img folder must be there.
constexpr const char *kPesFolders[] = {"img"};
constexpr const char *kPesFiles[] = {"pes2013.exe"};
constexpr Manifest kPes = []
{
    Manifest m{"PES2013", kPesFolders, kPesFiles, "", {}, {}};
    m.unversioned = true;
    return m;
}();

constexpr const Manifest *kManifests[] = {&kPes};

// ---- files ----------------------------------------------------------------------

bool is_directory(const std::string &path)
{
    struct stat info{};
    return lstat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool is_file(const std::string &path, std::uint64_t *size = nullptr)
{
    struct stat info{};
    if (lstat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
        return false;
    if (size)
        *size = static_cast<std::uint64_t>(info.st_size);
    return true;
}

// One name in a folder, matched without regard to case ("" when none).
std::string find_name(const std::string &folder, const std::string &name)
{
    DIR *directory = opendir(folder.c_str());
    if (!directory)
        return {};
    std::string found;
    while (const dirent *entry = readdir(directory))
        if (strcasecmp(entry->d_name, name.c_str()) == 0)
        {
            found = entry->d_name;
            if (found == name)
                break; // an exact match wins over one that differs only in case
        }
    closedir(directory);
    return found;
}

bool crc_of(const std::string &path, std::uint32_t &crc)
{
    const int file = open(path.c_str(), O_RDONLY | O_NOFOLLOW);
    if (file < 0)
        return false;
    std::vector<unsigned char> buffer(1u << 20);
    mz_ulong value = mz_crc32(0, nullptr, 0);
    ssize_t count = 0;
    while ((count = read(file, buffer.data(), buffer.size())) > 0)
        value = mz_crc32(value, buffer.data(), static_cast<size_t>(count));
    close(file);
    crc = static_cast<std::uint32_t>(value);
    return count == 0;
}

// The CRC of the version file is read once per (path, size, mtime): the service checks the
// base games every pass, and the file is half a gigabyte. inspect runs on the service thread
// and on install jobs, so the cache is guarded.
struct CrcKey
{
    std::string path;
    std::uint64_t size;
    std::int64_t mtime_sec;
    std::int64_t mtime_nsec;
    bool operator==(const CrcKey &o) const
    {
        return std::tie(path, size, mtime_sec, mtime_nsec) == std::tie(o.path, o.size, o.mtime_sec, o.mtime_nsec);
    }
};
std::mutex crc_guard;
std::map<std::string, std::pair<CrcKey, std::uint32_t>> crc_cache;
std::atomic<unsigned> crc_reads{0};

bool cached_crc_of(const std::string &path, std::uint32_t &crc)
{
    struct stat info{};
    if (lstat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
        return false;
    const CrcKey key{path, static_cast<std::uint64_t>(info.st_size),
                     static_cast<std::int64_t>(info.st_mtim.tv_sec),
                     static_cast<std::int64_t>(info.st_mtim.tv_nsec)};
    {
        std::lock_guard lock(crc_guard);
        if (const auto it = crc_cache.find(path); it != crc_cache.end() && it->second.first == key)
        {
            crc = it->second.second;
            return true;
        }
    }
    ++crc_reads;
    if (!crc_of(path, crc))
        return false;
    std::lock_guard lock(crc_guard);
    crc_cache[path] = {key, crc};
    return true;
}

std::string join(const std::vector<std::string> &items)
{
    std::string text;
    for (std::size_t i = 0; i < items.size(); ++i)
        text += (i == 0 ? "" : i + 1 == items.size() ? " and " : ", ") + items[i];
    return text;
}

// The bytes in a folder's regular files, links not followed, unreadable parts skipped.
std::uint64_t folder_bytes(const std::string &folder)
{
    DIR *directory = opendir(folder.c_str());
    if (!directory)
        return 0;
    std::vector<std::string> names;
    while (const dirent *entry = readdir(directory))
        if (std::strcmp(entry->d_name, ".") != 0 && std::strcmp(entry->d_name, "..") != 0)
            names.emplace_back(entry->d_name);
    closedir(directory);
    std::uint64_t total = 0;
    for (const auto &name : names)
    {
        const auto path = folder + "/" + name;
        struct stat info{};
        if (lstat(path.c_str(), &info) != 0)
            continue;
        if (S_ISREG(info.st_mode))
            total += static_cast<std::uint64_t>(info.st_size);
        else if (S_ISDIR(info.st_mode))
            total += folder_bytes(path);
    }
    return total;
}

// "5.7 GB" from what rounds to a gigabyte up, "930 MB" below.
std::string size_text(std::uint64_t bytes)
{
    char text[32];
    if (bytes >= 999'500'000) // 999.5 MB and up would print "1000 MB"
        std::snprintf(text, sizeof text, "%.1f GB", static_cast<double>(bytes) / 1e9);
    else
        std::snprintf(text, sizeof text, "%.0f MB", static_cast<double>(bytes) / 1e6);
    return text;
}

} // namespace

const Manifest *manifest_for(const std::string &folder)
{
    for (const auto *manifest : kManifests)
        if (folder == manifest->folder)
            return manifest;
    return nullptr;
}

unsigned crc_reads_for_test()
{
    return crc_reads;
}

std::string find_entry(const std::string &folder, const std::string &relative)
{
    std::string path = folder;
    std::size_t at = 0;
    while (at <= relative.size())
    {
        const auto slash = std::min(relative.find('/', at), relative.size());
        const auto name = find_name(path, relative.substr(at, slash - at));
        if (name.empty())
            return {};
        path += "/" + name;
        at = slash + 1;
    }
    return path;
}

BaseGame inspect(const std::string &folder, const Manifest &manifest)
{
    BaseGame result;
    result.path = folder;
    if (!is_directory(folder))
        return result;
    std::vector<std::string> missing;
    for (const auto *name : manifest.folders)
        if (const auto path = find_entry(folder, name); path.empty() || !is_directory(path))
            missing.emplace_back(name);
    for (const auto *name : manifest.files)
        if (const auto path = find_entry(folder, name); path.empty() || !is_file(path))
            missing.emplace_back(name);
    if (!missing.empty())
    {
        result.state = State::incomplete;
        result.detail = i18n::trf("Missing {0}. Copy the whole game folder.", {join(missing)});
        return result;
    }
    std::vector<std::string> mods;
    for (const auto *name : manifest.mod_files)
        if (!find_entry(folder, name).empty())
            mods.emplace_back(name);
    std::string left; // what the page adds about mods that are left out
    if (!mods.empty() && manifest.mods_left_out)
        left = " " + i18n::trf("Mods found ({0}) are left out: the install adds its own.", {join(mods)});
    else if (!mods.empty())
    {
        result.state = State::modified;
        result.detail = i18n::trf("Mods are installed in it ({0}). Copy a clean install: the mods are "
                                  "added for you.",
                                  {join(mods)});
        return result;
    }
    if (manifest.unversioned)
    {
        result.state = State::ready;
        result.detail = i18n::trf("Copied as it is, mods and updates included ({0}).",
                                  {size_text(folder_bytes(folder))});
        return result;
    }
    const auto version_path = find_entry(folder, manifest.version_file);
    std::uint64_t size = 0;
    std::uint32_t crc = 0;
    if (!is_file(version_path, &size) || !cached_crc_of(version_path, crc))
    {
        result.state = State::incomplete;
        result.detail = i18n::trf("Can't read {0}.", {manifest.version_file});
        return result;
    }
    for (const auto &release : manifest.releases)
        if (release.size == size && release.crc == crc)
        {
            result.state = release.needs_patch ? State::needs_patch : State::ready;
            result.version = release.version;
            result.detail = release.needs_patch
                                ? i18n::trf("Version {0}: upgraded to {1} as it installs.",
                                            {result.version, manifest.releases.front().version})
                                : i18n::trf("Version {0}.", {result.version});
            result.detail += left;
            return result;
        }
    if (manifest.accept_unknown)
    {
        result.state = State::ready;
        result.version = manifest.releases.front().version;
        result.warning =
            i18n::trf("{0} is not the one we know: installing anyway.", {manifest.version_file});
        result.detail = i18n::trf("Version {0} (unchecked).", {result.version}) + left;
        return result;
    }
    result.state = State::unknown;
    result.detail = i18n::trf("{0} matches no release we know: the copy is modified, or it is a "
                              "release we haven't seen. Copy a clean install.",
                              {manifest.version_file});
    return result;
}

BaseGame locate(const std::string &data_root, std::span<const std::string> roots,
                const Manifest &manifest)
{
    const auto base = data_root + "/base-games/" + manifest.folder;
    // base-games/PES2013, pes2013 or Pes2013: whatever case the copy was made in.
    if (const auto existing = find_entry(data_root + "/base-games", manifest.folder);
        !existing.empty() && is_directory(existing))
        return inspect(existing, manifest);
    for (const auto &root : roots)
        if (const auto path = find_entry(root, manifest.folder); !path.empty() && is_directory(path))
        {
            auto found = inspect(path, manifest);
            found.on_usb = true;
            return found;
        }
    BaseGame absent;
    absent.path = base;
    absent.detail = i18n::trf("Put the game folder in {0}/base-games/{1} over FTP, or on a USB "
                              "drive in a folder named {1} (any case).",
                              {data_root, manifest.folder});
    return absent;
}

std::vector<std::string> usb_roots()
{
    std::vector<std::string> roots;
    for (int i = 0; i < 8; ++i)
        if (const auto path = "/mnt/usb" + std::to_string(i); is_directory(path))
            roots.push_back(path);
    for (int i = 0; i < 2; ++i)
        if (const auto path = "/mnt/ext" + std::to_string(i); is_directory(path))
            roots.push_back(path);
    return roots;
}

bool import_base(const std::string &from, const std::string &data_root, const Manifest &manifest,
                 const std::function<void(const ImportProgress &)> &progress,
                 const std::atomic<bool> &cancel, std::string &error)
{
    const auto games = data_root + "/base-games";
    const auto target = games + "/" + manifest.folder, work = target + ".importing";
    if (const auto existing = find_entry(games, manifest.folder);
        !existing.empty() && is_directory(existing))
    {
        error = existing + " already exists";
        return false;
    }
    if (mkdir(games.c_str(), 0777) != 0 && errno != EEXIST)
    {
        error = "Can't make " + games + ": " + std::strerror(errno);
        return false;
    }
    // A copy left by an earlier attempt is started again.
    if (!copy_tree(from, work, progress, cancel, error))
        return false;
    // Checked as a whole before it takes the place of the base copy.
    const auto copy = inspect(work, manifest);
    if (copy.state != State::ready && copy.state != State::needs_patch)
        error = "The copy didn't check out: " + copy.detail;
    else if (rename(work.c_str(), target.c_str()) != 0)
        error = "Can't move the copy into place: " + std::string(std::strerror(errno));
    else
        return true;
    remove_all(work);
    return false;
}
} // namespace store::library
