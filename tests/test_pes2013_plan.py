import json, subprocess, sys, pathlib, re
ROOT = pathlib.Path(__file__).resolve().parents[1]
GEN = ROOT / "profile/pes2013/make-pes2013-plan.py"
SAVE = "Documents/KONAMI/Pro Evolution Soccer 2013"

def generate(tmp_path):
    out = tmp_path / "PES2013.json"
    subprocess.run([sys.executable, str(GEN), str(out)], check=True)
    return out

def plan(tmp_path):
    return json.loads(generate(tmp_path).read_text())

def test_identity(tmp_path):
    p = plan(tmp_path)
    assert (p["schema"], p["game"], p["title"], p["prefix"]) == (1, "PES2013", "PES 2013", "futbol13")
    assert p["profile"]["id"] == "pb-futbol13" and p["profile"]["name"] == "FUTBOL13"
    assert p["profile"]["executable"] == "C:\\Games\\PES2013\\pes2013.exe"
    assert p["profile"]["working_directory"] == "C:\\Games\\PES2013"
    assert p["profile"]["dll_overrides"] == "d3d9,dxgi=n;dinput8=n,b"
    assert (p["profile"]["width"], p["profile"]["height"]) == (3840, 2160)
    assert p["profile"]["after"] == "futbol13.profile"

def test_nothing_downloaded_nothing_added_nothing_stripped(tmp_path):
    p = plan(tmp_path)
    assert p["sources"] == {} and p["game_files"] == [] and p["system_files"] == []
    assert p["strip"] == [] and p["options"] == [] and p["pending"] == [] and p["upgrades"] == []
    assert p["patches"] == [] and p["ini"] == [] and p["expect"] == {}
    assert p["registry"]["user"] == [] and p["user_files"] == []

def test_registry_is_what_the_games_installer_leaves(tmp_path):
    # installdir and version as the PC installer writes them; the product code from the
    # player's code.txt in the game folder, never a code of ours.
    key = "Software\\Wow6432Node\\KONAMI\\PES2013"
    machine = plan(tmp_path)["registry"]["machine"]
    assert [(v["key"], v["name"]) for v in machine[:3]] == [(key, "installdir"), (key, "version"), (key, "code")]
    assert machine[0]["string"] == "C:\\Games\\PES2013\\" and machine[1]["string"] == "1.00.0000"
    code = machine[2]
    assert code["from_file"] == "code.txt" and "string" not in code and "dword" not in code

def test_processor_keys_for_the_system_check(tmp_path):
    # wineboot never runs on the console: the plan writes the processor keys PES reads.
    cpu = "Hardware\\Description\\System\\CentralProcessor\\0"
    machine = plan(tmp_path)["registry"]["machine"][3:]
    assert machine == [
        {"key": cpu, "name": "~MHz", "dword": 3500},
        {"key": cpu, "name": "Identifier", "string": "AMD64 Family 23 Model 71 Stepping 0"},
        {"key": cpu, "name": "VendorIdentifier", "string": "AuthenticAMD"},
        {"key": cpu, "name": "ProcessorNameString", "string": "AMD 4700S 8-Core Processor"},
        {"key": cpu, "name": "FeatureSet", "dword": 0x1FFF},
    ]

def test_our_dxvk_conf_is_the_only_text(tmp_path):
    p = plan(tmp_path)
    assert [t["to"] for t in p["texts"]] == ["dxvk.conf"]
    assert p["texts"][0]["body"] == (ROOT / "profile/pes2013/dxvk.conf").read_text()

def test_save_folder_seed(tmp_path):
    assert plan(tmp_path)["user_seeds"] == [{"from": "save", "file": SAVE + "/save"}]

def test_settings_dat_binary(tmp_path):
    [b] = plan(tmp_path)["user_binaries"]
    assert b["file"] == SAVE + "/settings.dat"
    assert re.fullmatch(r"[0-9a-f]+", b["bytes"]) and len(b["bytes"]) % 2 == 0
    size = len(b["bytes"]) // 2
    kinds = [f["u32"] for f in b["fields"]]
    assert "{width}" in kinds and "{height}" in kinds
    for f in b["fields"]:
        assert 0 <= f["at"] and f["at"] + 4 <= size
    assert b["checksum"] == {"kind": "crc16-xmodem-not", "at": 12}

def test_no_other_game_in_the_text(tmp_path):
    text = json.dumps(plan(tmp_path)).lower()
    for word in ("blur", "amax", "sahin", "asi", "securom"):
        assert word not in text, word

def test_the_committed_plan_is_what_the_generator_writes(tmp_path):
    # The launcher's parser accepting it is checked by the C++ tests (store_game_install_test);
    # here only that the committed file is in step with the generator.
    committed = ROOT / "launcher/assets/plans/PES2013.json"
    assert committed.read_text() == generate(tmp_path).read_text()
