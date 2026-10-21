import importlib.util, pathlib, re, struct
ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("settings_dat", ROOT / "profile/pes2013/settings_dat.py")
sd = importlib.util.module_from_spec(spec); spec.loader.exec_module(sd)

def test_template_is_hex_of_the_file_size():
    for hexed in (sd.TEMPLATE_HEX, sd.DEFAULT_HEX):
        assert re.fullmatch(r"[0-9a-f]+", hexed) and len(hexed) == 2 * sd.SIZE
    assert sd.SIZE == 852 and bytes.fromhex(sd.TEMPLATE_HEX)[:4] == b"WECF"

def test_fields_fit_and_name_the_picture_size():
    kinds = [f["u32"] for f in sd.FIELDS]
    assert "{width}" in kinds and "{height}" in kinds
    for f in sd.FIELDS:
        assert f["at"] % 4 == 0 and f["at"] + 4 <= sd.SIZE
        assert isinstance(f["u32"], int) or f["u32"] in ("{width}", "{height}")
    assert len({f["at"] for f in sd.FIELDS}) == len(sd.FIELDS)
    assert sd.CHECKSUM == {"kind": "crc16-xmodem-not", "at": 12}

def test_checksum_matches_what_settings_exe_wrote():
    for hexed, stored in ((sd.DEFAULT_HEX, 0x3D6E), (sd.TEMPLATE_HEX, 0x3B3C)):
        data = bytearray(bytes.fromhex(hexed))
        assert struct.unpack_from("<H", data, 12)[0] == stored
        struct.pack_into("<H", data, 12, 0)
        assert sd.crc16_not(data) == stored

def test_render_matches_the_capture():
    out = sd.render(1920, 1080)
    assert out == bytes.fromhex(sd.TEMPLATE_HEX)            # settings.exe's own 1080p file
    assert struct.unpack_from("<I", out, sd.WIDTH_AT)[0] == 1920
    assert struct.unpack_from("<I", out, sd.HEIGHT_AT)[0] == 1080
    assert struct.unpack_from("<H", out, 0x0E)[0] == 0x00CB   # XInput bit set
    big = sd.render(3840, 2160)
    assert struct.unpack_from("<I", big, sd.WIDTH_AT)[0] == 3840 and big[0x0C:0x0E] != out[0x0C:0x0E]
    assert big[:0x0C] + big[0x0E:0x10] + big[0x18:] == out[:0x0C] + out[0x0E:0x10] + out[0x18:]  # only size and checksum differ
