#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Contains unit tests for the CLI functionality."""

from pathlib import Path
from unittest import mock

import pytest
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
