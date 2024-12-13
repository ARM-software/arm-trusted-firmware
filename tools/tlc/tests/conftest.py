#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

""" Common configurations and fixtures for test environment."""

from random import randint

import pytest
import yaml
from click.testing import CliRunner

from tlc.cli import cli


def generate_random_bytes(n):
    return bytes([randint(0, 255) for _ in range(n)])


@pytest.fixture
def tmptlstr(tmpdir):
    return tmpdir.join("tl.bin").strpath


@pytest.fixture
def tmpyamlconfig(tmpdir):
    return tmpdir.join("config.yaml").strpath


@pytest.fixture
def tmpfdt(tmpdir):
    fdt = tmpdir.join("fdt.dtb")
    fdt.write_binary(b"\x00" * 100)
    return fdt


@pytest.fixture(params=[1, 2, 3, 4, 5, 0x100, 0x101, 0x102, 0x104])
def non_empty_tag_id(request):
    return request.param


@pytest.fixture
def tmpyamlconfig_blob_file(tmpdir, tmpfdt, non_empty_tag_id):
    config_path = tmpdir.join("config.yaml")

    config = {
        "has_checksum": True,
        "max_size": 0x1000,
        "entries": [
            {
                "tag_id": non_empty_tag_id,
                "blob_file_path": tmpfdt.strpath,
            },
        ],
    }

    with open(config_path, "w") as f:
        yaml.safe_dump(config, f)

    return config_path


@pytest.fixture
def tlcrunner(tmptlstr):
    runner = CliRunner()
    with runner.isolated_filesystem():
        runner.invoke(cli, ["create", "--size", 0x1F000, tmptlstr])
    return runner


@pytest.fixture
def tlc_entries(tmpfdt):
    return [(0, "/dev/null"), (1, tmpfdt.strpath), (0x102, tmpfdt.strpath)]


@pytest.fixture
def random_entry():
    def _random_entry(max_size):
        return randint(0, 0xFFFFFF), generate_random_bytes(randint(0, max_size))

    return _random_entry


@pytest.fixture
def random_entries(random_entry):
    def _random_entries(n=5, max_size=0x100):
        for _ in range(n):
            yield random_entry(max_size)

    return _random_entries
