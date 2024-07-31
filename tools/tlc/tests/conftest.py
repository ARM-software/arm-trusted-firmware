#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

""" Common configurations and fixtures for test environment."""

import pytest
from click.testing import CliRunner

from tlc.cli import cli


@pytest.fixture
def tmptlstr(tmpdir):
    return tmpdir.join("tl.bin").strpath


@pytest.fixture
def tmpfdt(tmpdir):
    fdt = tmpdir.join("fdt.dtb")
    fdt.write_binary(b"\x00" * 100)
    return fdt


@pytest.fixture
def tlcrunner(tmptlstr):
    runner = CliRunner()
    with runner.isolated_filesystem():
        runner.invoke(cli, ["create", tmptlstr])
    return runner


@pytest.fixture
def tlc_entries(tmpfdt):
    return [(0, "/dev/null"), (1, tmpfdt.strpath), (0x102, tmpfdt.strpath)]
