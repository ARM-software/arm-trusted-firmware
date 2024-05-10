#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Contains unit tests for the types TransferEntry and TransferList."""

import math

import pytest

from tlc.te import TransferEntry
from tlc.tl import TransferList

large_data = 0xDEADBEEF.to_bytes(4, "big")
small_data = 0x1234.to_bytes(3, "big")
test_entries = [
    (0, b""),
    (1, small_data),
    (1, large_data),
    (0xFFFFFF, small_data),
    (0xFFFFFF, large_data),
]


@pytest.mark.parametrize(
    "size,csum",
    [
        (-1, None),
        (0x18, 0x9E),
        (0x1000, 0xA6),
        (0x2000, 0x96),
        (0x4000, 0x76),
    ],
)
def test_make_transfer_list(size, csum):
    if size < 8:
        with pytest.raises(AssertionError):
            tl = TransferList(size)
    else:
        tl = TransferList(size)

        assert tl.signature == 0x4A0FB10B
        assert not tl.entries
        assert tl.sum_of_bytes() == 0
        assert tl.checksum == csum


@pytest.mark.parametrize(("tag_id", "data"), test_entries)
def test_add_transfer_entry(tag_id, data):
    tl = TransferList(0x1000)
    te = TransferEntry(tag_id, len(data), data)

    tl.add_transfer_entry(tag_id, data)

    assert te in tl.entries
    assert tl.size == TransferList.hdr_size + te.size


@pytest.mark.parametrize(
    ("tag_id", "data"),
    [
        (-1, None),  # tag out of range
        (1, None),  # no data provided
        (1, bytes(8000)),  # very large data > total size
        (0x100000, b"0dd0edfe"),  # tag out of range
    ],
)
def test_add_out_of_range_transfer_entry(tag_id, data):
    tl = TransferList()

    with pytest.raises(Exception):
        tl.add_transfer_entry(tag_id, data)


@pytest.mark.parametrize(("tag_id", "data"), test_entries)
def test_calculate_te_sum_of_bytes(tag_id, data):
    te = TransferEntry(tag_id, len(data), data)
    csum = (
        sum(data)
        + sum(len(data).to_bytes(4, "big"))
        + te.hdr_size
        + sum(tag_id.to_bytes(4, "big"))
    ) % 256
    assert te.sum_of_bytes == csum


@pytest.mark.parametrize(("tag_id", "data"), test_entries)
def test_calculate_tl_checksum(tag_id, data):
    tl = TransferList(0x1000)

    tl.add_transfer_entry(tag_id, data)
    assert tl.sum_of_bytes() == 0


def test_empty_transfer_list_blob(tmpdir):
    """Check that we can correctly create a transfer list header."""
    test_file = tmpdir.join("test_tl_blob.bin")
    tl = TransferList()
    tl.write_to_file(test_file)

    with open(test_file, "rb") as f:
        assert f.read(tl.hdr_size) == tl.header_to_bytes()


@pytest.mark.parametrize(("tag_id", "data"), test_entries)
def test_single_te_transfer_list(tag_id, data, tmpdir):
    """Check that we can create a complete TL with a single TE."""
    test_file = tmpdir.join("test_tl_blob.bin")
    tl = TransferList(0x1000)

    tl.add_transfer_entry(tag_id, data)
    tl.write_to_file(test_file)

    te = tl.entries[0]

    with open(test_file, "rb") as f:
        assert f.read(tl.hdr_size) == tl.header_to_bytes()
        assert int.from_bytes(f.read(3), "little") == te.id
        assert int.from_bytes(f.read(1), "little") == te.hdr_size
        assert int.from_bytes(f.read(4), "little") == te.data_size
        assert f.read(te.data_size) == te.data


def test_multiple_te_transfer_list(tmpdir):
    """Check that we can create a TL with multiple TE's."""
    test_file = tmpdir.join("test_tl_blob.bin")
    tl = TransferList(0x1000)

    for tag_id, data in test_entries:
        tl.add_transfer_entry(tag_id, data)

    tl.write_to_file(test_file)

    with open(test_file, "rb") as f:
        assert f.read(tl.hdr_size) == tl.header_to_bytes()
        # Ensure that TE's have the correct alignment
        for tag_id, data in test_entries:
            f.seek(int(math.ceil(f.tell() / 2**tl.alignment) * 2**tl.alignment))
            print(f.tell())
            assert int.from_bytes(f.read(3), "little") == tag_id
            assert int.from_bytes(f.read(1), "little") == TransferEntry.hdr_size
            # Make sure the data in the TE matches the data in the original case
            data_size = int.from_bytes(f.read(4), "little")
            assert f.read(data_size) == data


def test_read_empty_transfer_list_from_file(tmpdir):
    test_file = tmpdir.join("test_tl_blob.bin")
    original_tl = TransferList(0x1000)
    original_tl.write_to_file(test_file)

    # Read the contents of the file we just wrote
    tl = TransferList.fromfile(test_file)
    assert tl.header_to_bytes() == original_tl.header_to_bytes()
    assert tl.sum_of_bytes() == 0


def test_read_single_transfer_list_from_file(tmpdir):
    test_file = tmpdir.join("test_tl_blob.bin")
    original_tl = TransferList(0x1000)

    original_tl.add_transfer_entry(test_entries[0][0], test_entries[0][1])
    original_tl.write_to_file(test_file)

    # Read the contents of the file we just wrote
    tl = TransferList.fromfile(test_file)
    assert tl.entries

    te = tl.entries[0]
    assert te.id == test_entries[0][0]
    assert te.data == test_entries[0][1]
    assert tl.sum_of_bytes() == 0


def test_read_multiple_transfer_list_from_file(tmpdir):
    test_file = tmpdir.join("test_tl_blob.bin")
    original_tl = TransferList(0x1000)

    for tag_id, data in test_entries:
        original_tl.add_transfer_entry(tag_id, data)

    original_tl.write_to_file(test_file)

    # Read the contents of the file we just wrote
    tl = TransferList.fromfile(test_file)

    # The TE we derive from the file might have a an associated offset, compare
    # the TE's based on the header in bytes, which doesn't account for this.
    for te0, te1 in zip(tl.entries, original_tl.entries):
        assert te0.header_to_bytes() == te1.header_to_bytes()

    assert tl.sum_of_bytes() == 0


@pytest.mark.parametrize("tag", [tag for tag, _ in test_entries])
def test_remove_tag_from_file(tag):
    tl = TransferList(0x1000)

    for tag_id, data in test_entries:
        tl.add_transfer_entry(tag_id, data)

    removed_entries = list(filter(lambda te: te.id == tag, tl.entries))
    original_size = tl.size
    tl.remove_tag(tag)

    assert not any(tag == te.id for te in tl.entries)
    assert tl.size == original_size - sum(map(lambda te: te.size, removed_entries))


def test_get_fdt_offset(tmpdir):
    tl = TransferList(0x1000)
    tl.add_transfer_entry(1, 0xEDFE0DD0.to_bytes(4, "big"))
    f = tmpdir.join("blob.bin")

    tl.write_to_file(f)

    blob_tl = TransferList.fromfile(f)

    assert blob_tl.hdr_size + TransferEntry.hdr_size == blob_tl.get_entry_data_offset(1)


def test_get_missing_fdt_offset(tmpdir):
    tl = TransferList(0x1000)
    f = tmpdir.join("blob.bin")

    tl.write_to_file(f)
    blob_tl = TransferList.fromfile(f)

    with pytest.raises(ValueError):
        blob_tl.get_entry_data_offset(1)
