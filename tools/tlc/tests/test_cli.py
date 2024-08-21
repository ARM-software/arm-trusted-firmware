#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Contains unit tests for the CLI functionality."""

from math import ceil, log2
from pathlib import Path
from re import findall, search
from unittest import mock

import pytest
import yaml
from click.testing import CliRunner

from tlc.cli import cli
from tlc.te import TransferEntry
from tlc.tl import TransferList


def test_create_empty_tl(tmpdir):
    runner = CliRunner()
    test_file = tmpdir.join("tl.bin")

    result = runner.invoke(cli, ["create", test_file.strpath])
    assert result.exit_code == 0
    assert TransferList.fromfile(test_file) is not None


def test_create_with_fdt(tmpdir):
    runner = CliRunner()
    fdt = tmpdir.join("fdt.dtb")
    fdt.write_binary(b"\x00" * 100)

    result = runner.invoke(
        cli,
        [
            "create",
            "--fdt",
            fdt.strpath,
            "--size",
            "1000",
            tmpdir.join("tl.bin").strpath,
        ],
    )
    assert result.exit_code == 0


def test_add_single_entry(tlcrunner, tmptlstr):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == 0


def test_add_multiple_entries(tlcrunner, tlc_entries, tmptlstr):
    for id, path in tlc_entries:
        tlcrunner.invoke(cli, ["add", "--entry", id, path, tmptlstr])

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == len(tlc_entries)


def test_info(tlcrunner, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])
    tlcrunner.invoke(cli, ["add", "--fdt", tmpfdt.strpath, tmptlstr])

    result = tlcrunner.invoke(cli, ["info", tmptlstr])
    assert result.exit_code == 0
    assert "signature" in result.stdout
    assert "id" in result.stdout

    result = tlcrunner.invoke(cli, ["info", "--header", tmptlstr])
    assert result.exit_code == 0
    assert "signature" in result.stdout
    assert "id" not in result.stdout

    result = tlcrunner.invoke(cli, ["info", "--entries", tmptlstr])
    assert result.exit_code == 0
    assert "signature" not in result.stdout
    assert "id" in result.stdout


def test_raises_max_size_error(tmptlstr, tmpfdt):
    tmpfdt.write_binary(bytes(6000))

    runner = CliRunner()
    result = runner.invoke(cli, ["create", "--fdt", tmpfdt, tmptlstr])

    assert result.exception
    assert isinstance(result.exception, MemoryError)
    assert "TL max size exceeded, consider increasing with the option -s" in str(
        result.exception
    )
    assert "TL size has exceeded the maximum allocation" in str(
        result.exception.__cause__
    )


def test_info_get_fdt_offset(tmptlstr, tmpfdt):
    runner = CliRunner()
    with runner.isolated_filesystem():
        runner.invoke(cli, ["create", "--size", "1000", tmptlstr])
        runner.invoke(cli, ["add", "--entry", "1", tmpfdt.strpath, tmptlstr])
        result = runner.invoke(cli, ["info", "--fdt-offset", tmptlstr])

    assert result.exit_code == 0
    assert result.output.strip("\n").isdigit()


def test_remove_tag(tlcrunner, tmptlstr):
    tlcrunner.invoke(cli, ["add", "--entry", "0", "/dev/null", tmptlstr])
    result = tlcrunner.invoke(cli, ["info", tmptlstr])

    assert result.exit_code == 0
    assert "signature" in result.stdout

    tlcrunner.invoke(cli, ["remove", "--tags", "0", tmptlstr])
    tl = TransferList.fromfile(tmptlstr)

    assert result.exit_code == 0
    assert len(tl.entries) == 0


def test_unpack_tl(tlcrunner, tmptlstr, tmpfdt, tmpdir):
    with tlcrunner.isolated_filesystem(temp_dir=tmpdir):
        tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
        tlcrunner.invoke(cli, ["unpack", tmptlstr])
        assert Path("te_0_1.bin").exists()


def test_unpack_multiple_tes(tlcrunner, tlc_entries, tmptlstr, tmpdir):
    with tlcrunner.isolated_filesystem(temp_dir=tmpdir):
        for id, path in tlc_entries:
            tlcrunner.invoke(cli, ["add", "--entry", id, path, tmptlstr])

    assert all(
        filter(
            lambda te: (Path(tmpdir.strpath) / f"te_{te[0]}.bin").exists(), tlc_entries
        )
    )


def test_unpack_into_dir(tlcrunner, tmpdir, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
    tlcrunner.invoke(cli, ["unpack", "-C", tmpdir.strpath, tmptlstr])

    assert (Path(tmpdir.strpath) / "te_0_1.bin").exists()


def test_unpack_into_dir_with_conflicting_tags(tlcrunner, tmpdir, tmptlstr, tmpfdt):
    tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
    tlcrunner.invoke(cli, ["add", "--entry", 1, tmpfdt.strpath, tmptlstr])
    tlcrunner.invoke(cli, ["unpack", "-C", tmpdir.strpath, tmptlstr])

    assert (Path(tmpdir.strpath) / "te_0_1.bin").exists()
    assert (Path(tmpdir.strpath) / "te_1_1.bin").exists()


def test_validate_invalid_signature(tmptlstr, tlcrunner, monkeypatch):
    tl = TransferList()
    tl.signature = 0xDEADBEEF

    mock_open = lambda tmptlstr, mode: mock.mock_open(read_data=tl.header_to_bytes())()
    monkeypatch.setattr("builtins.open", mock_open)

    result = tlcrunner.invoke(cli, ["validate", tmptlstr])
    assert result.exit_code != 0


def test_validate_misaligned_entries(tmptlstr, tlcrunner, monkeypatch):
    """Base address of a TE must be 8-byte aligned."""
    mock_open = lambda tmptlstr, mode: mock.mock_open(
        read_data=TransferList().header_to_bytes()
        + bytes(5)
        + TransferEntry(0, 0, bytes(0)).header_to_bytes
    )()
    monkeypatch.setattr("builtins.open", mock_open)

    result = tlcrunner.invoke(cli, ["validate", tmptlstr])

    assert result.exit_code == 1


@pytest.mark.parametrize(
    "version", [0, TransferList.version, TransferList.version + 1, 1 << 8]
)
def test_validate_unsupported_version(version, tmptlstr, tlcrunner, monkeypatch):
    tl = TransferList()
    tl.version = version

    mock_open = lambda tmptlstr, mode: mock.mock_open(read_data=tl.header_to_bytes())()
    monkeypatch.setattr("builtins.open", mock_open)

    result = tlcrunner.invoke(cli, ["validate", tmptlstr])

    if version >= TransferList.version and version <= 0xFF:
        assert result.exit_code == 0
    else:
        assert result.exit_code == 1


def test_create_entry_from_yaml_and_blob_file(
    tlcrunner, tmpyamlconfig_blob_file, tmptlstr, non_empty_tag_id
):
    tlcrunner.invoke(
        cli,
        [
            "create",
            "--from-yaml",
            tmpyamlconfig_blob_file.strpath,
            tmptlstr,
        ],
    )

    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1
    assert tl.entries[0].id == non_empty_tag_id


@pytest.mark.parametrize(
    "entry",
    [
        {"tag_id": 0},
        {
            "tag_id": 0x104,
            "addr": 0x0400100000000010,
            "size": 0x0003300000000000,
        },
        {
            "tag_id": 0x100,
            "pp_addr": 100,
        },
        {
            "tag_id": "optee_pageable_part",
            "pp_addr": 100,
        },
    ],
)
def test_create_from_yaml_check_sum_bytes(tlcrunner, tmpyamlconfig, tmptlstr, entry):
    """Test creating a TL from a yaml file, but only check that the sum of the
    data in the yaml file matches the sum of the data in the TL. This means
    you don't have to type the exact sequence of expected bytes. All the data
    in the yaml file must be integers (except for the tag IDs, which can be
    strings).
    """
    # create yaml config file
    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [entry],
    }
    with open(tmpyamlconfig, "w") as f:
        yaml.safe_dump(config, f)

    # invoke TLC
    tlcrunner.invoke(
        cli,
        [
            "create",
            "--from-yaml",
            tmpyamlconfig,
            tmptlstr,
        ],
    )

    # open created TL, and check
    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1

    # Check that the sum of all the data in the transfer entry in the yaml file
    # is the same as the sum of all the data in the transfer list. Don't count
    # the tag id or the TE headers.

    # every item in the entry dict must be an integer
    yaml_total = 0
    for key, data in iter_nested_dict(entry):
        if key != "tag_id":
            num_bytes = ceil(log2(data + 1) / 8)
            yaml_total += sum(data.to_bytes(num_bytes, "little"))

    tl_total = sum(tl.entries[0].data)

    assert tl_total == yaml_total


@pytest.mark.parametrize(
    "entry,expected",
    [
        (
            {
                "tag_id": 0x102,
                "ep_info": {
                    "h": {
                        "type": 0x01,
                        "version": 0x02,
                        "attr": 8,
                    },
                    "pc": 67239936,
                    "spsr": 965,
                    "args": [67112976, 67112960, 0, 0, 0, 0, 0, 0],
                },
            },
            (
                "0x00580201 0x00000008 0x04020000 0x00000000 "
                "0x000003C5 0x00000000 0x04001010 0x00000000 "
                "0x04001000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000"
            ),
        ),
        (
            {
                "tag_id": 0x102,
                "ep_info": {
                    "h": {
                        "type": 0x01,
                        "version": 0x02,
                        "attr": "EP_NON_SECURE | EP_ST_ENABLE",
                    },
                    "pc": 67239936,
                    "spsr": 965,
                    "args": [67112976, 67112960, 0, 0, 0, 0, 0, 0],
                },
            },
            (
                "0x00580201 0x00000005 0x04020000 0x00000000 "
                "0x000003C5 0x00000000 0x04001010 0x00000000 "
                "0x04001000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000 0x00000000 0x00000000 "
                "0x00000000 0x00000000"
            ),
        ),
    ],
)
def test_create_from_yaml_check_exact_data(
    tlcrunner, tmpyamlconfig, tmptlstr, entry, expected
):
    """Test creating a TL from a yaml file, checking the exact sequence of
    bytes. This is useful for checking that the alignment is correct. You can
    get the expected sequence of bytes by copying it from the ArmDS debugger.
    """
    # create yaml config file
    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [entry],
    }
    with open(tmpyamlconfig, "w") as f:
        yaml.safe_dump(config, f)

    # invoke TLC
    tlcrunner.invoke(
        cli,
        [
            "create",
            "--from-yaml",
            tmpyamlconfig,
            tmptlstr,
        ],
    )

    # open TL and check
    tl = TransferList.fromfile(tmptlstr)
    assert tl is not None
    assert len(tl.entries) == 1

    # check expected and actual data
    actual = tl.entries[0].data
    actual = bytes_to_hex(actual)

    assert actual == expected


@pytest.mark.parametrize("option", ["-O", "--output"])
def test_gen_tl_header_with_output_name(tlcrunner, tmptlstr, option, filename="test.h"):
    with tlcrunner.isolated_filesystem():
        result = tlcrunner.invoke(
            cli,
            [
                "gen-header",
                option,
                filename,
                tmptlstr,
            ],
        )

        assert result.exit_code == 0
        assert Path(filename).exists()


def test_gen_tl_with_fdt_header(tmptlstr, tmpfdt):
    tlcrunner = CliRunner()

    with tlcrunner.isolated_filesystem():
        tlcrunner.invoke(cli, ["create", "--size", 1000, "--fdt", tmpfdt, tmptlstr])

        result = tlcrunner.invoke(
            cli,
            [
                "gen-header",
                tmptlstr,
            ],
        )

        assert result.exit_code == 0
        assert Path("header.h").exists()

        with open("header.h", "r") as f:
            dtb_match = search(r"DTB_OFFSET\s+(\d+)", "".join(f.readlines()))
            assert dtb_match and dtb_match[1].isnumeric()


def test_gen_empty_tl_c_header(tlcrunner, tmptlstr):
    with tlcrunner.isolated_filesystem():
        result = tlcrunner.invoke(
            cli,
            [
                "gen-header",
                tmptlstr,
            ],
        )

        assert result.exit_code == 0
        assert Path("header.h").exists()

        with open("header.h", "r") as f:
            lines = "".join(f.readlines())

            assert TransferList.hdr_size == int(
                findall(r"SIZE\s+(0x[0-9a-fA-F]+|\d+)", lines)[0], 16
            )
            assert TransferList.version == int(
                findall(r"VERSION.+(0x[0-9a-fA-F]+|\d+)", lines)[0]
            )


def bytes_to_hex(data: bytes) -> str:
    """Convert bytes to a hex string in the same format as the debugger in
    ArmDS

    You can copy data from the debugger in Arm Development Studio and put it
    into a unit test. You can then run this function on the output from tlc,
    and compare it to the data you copied.

    The format is groups of 4 bytes with 0x prefixes separated by spaces.
    Little endian is used.
    """
    words_hex = []
    for i in range(0, len(data), 4):
        word = data[i : i + 4]
        word_int = int.from_bytes(word, "little")
        word_hex = "0x" + f"{word_int:0>8x}".upper()
        words_hex.append(word_hex)

    return " ".join(words_hex)


def iter_nested_dict(dictionary: dict):
    for key, value in dictionary.items():
        if isinstance(value, dict):
            yield from iter_nested_dict(value)
        else:
            yield key, value
