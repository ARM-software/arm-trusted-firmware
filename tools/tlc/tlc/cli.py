#!/usr/bin/env python3
# type: ignore[attr-defined]

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Module defining the Transfer List Compiler (TLC) command line interface."""

from pathlib import Path

import click
import jinja2
import yaml

from tlc.tl import *


@click.group()
@click.version_option()
def cli():
    pass


@cli.command()
@click.argument("filename", type=click.Path(dir_okay=False))
@click.option(
    "-s", "--size", default=0x1000, type=int, help="Maximum size of the Transfer List"
)
@click.option(
    "--fdt",
    type=click.Path(exists=True),
    help="Path to flattened device tree (FDT).",
)
@click.option(
    "--entry",
    type=(int, click.Path(exists=True)),
    multiple=True,
    help="A tag ID and the corresponding path to a binary blob in the form <id> <path-to-blob>.",
)
@click.option(
    "--flags",
    default=TRANSFER_LIST_ENABLE_CHECKSUM,
    show_default=True,
    help="Settings for the TL's properties.",
)
@click.option(
    "--from-yaml",
    type=click.Path(exists=True),
    help="Create the transfer list from a YAML config file.",
)
def create(filename, size, fdt, entry, flags, from_yaml):
    """Create a new Transfer List."""
    try:
        if from_yaml:
            with open(from_yaml, "r") as f:
                config = yaml.safe_load(f)

            tl = TransferList.from_dict(config)
        else:
            tl = TransferList(size)

            entry = (*entry, (1, fdt)) if fdt else entry

            for id, path in entry:
                tl.add_transfer_entry_from_file(id, path)
    except MemoryError as mem_excp:
        raise MemoryError(
            "TL max size exceeded, consider increasing with the option -s"
        ) from mem_excp

    tl.write_to_file(filename)


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
@click.option(
    "--fdt-offset",
    is_flag=True,
    help="Returns the offset of FDT in the TL if it is present.",
)
@click.option(
    "--header",
    is_flag=True,
    help="Print the Transfer List header.",
)
@click.option(
    "--entries",
    is_flag=True,
    help="Print the Transfer List entries.",
)
def info(filename, fdt_offset, header, entries):
    """Print the contents of an existing Transfer List.

    This command allows you to extract the data stored in a binary blob
    representing a transfer list (TL). The transfer list must comply with the
    version of the firmware handoff specification supported by this tool.
    """
    tl = TransferList.fromfile(filename)

    if fdt_offset:
        return print(tl.get_entry_data_offset(1))

    if header and entries or not (header or entries):
        print(tl, sep="")
        if tl.entries:
            print("----", tl.get_transfer_entries_str(), sep="\n")
    elif entries:
        print(tl.get_transfer_entries_str())
    elif header:
        print(tl)


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
@click.option(
    "--tags",
    type=int,
    multiple=True,
    help="Tags to be removed from TL.",
)
def remove(filename, tags):
    """Remove Transfer Entries with given tags.

    Remove Transfer Entries with given tags from a Transfer List."""
    tl = TransferList.fromfile(filename)

    for tag in tags:
        tl.remove_tag(tag)
    tl.write_to_file(filename)


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
@click.option(
    "--entry",
    type=(int, click.Path(exists=True)),
    multiple=True,
    help="A tag ID and the corresponding path to a binary blob in the form <id> <path-to-blob>.",
)
def add(filename, entry):
    """Update an existing Transfer List with given images."""
    tl = TransferList.fromfile(filename)

    for id, path in entry:
        tl.add_transfer_entry_from_file(id, path)

    tl.write_to_file(filename)


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
@click.option(
    "-C", type=click.Path(exists=True), help="Output directory for extracted images."
)
def unpack(filename, c):
    """Unpack images from a Transfer List."""
    tl = TransferList.fromfile(filename)
    pwd = Path(".") if not c else Path(c)

    for i, te in enumerate(tl.entries):
        with open(pwd / f"te_{i}_{te.id}.bin", "wb") as f:
            f.write(te.data)


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
@click.option(
    "--output",
    "-O",
    type=click.Path(exists=False),
    help="Output filename for the header",
    default=Path("header.h"),
)
def gen_header(filename, output):
    """Generate a header with common definitions."""
    tl = TransferList.fromfile(filename)
    tmp_keys = tl.__dict__
    tmp_keys["header_guard"] = Path(output).name.replace(".", "_").upper()

    dtb_te = tl.get_entry(1)

    if dtb_te:
        tmp_keys["dtb_offset"] = dtb_te.offset + dtb_te.hdr_size

    env = jinja2.Environment(
        loader=jinja2.PackageLoader("tlc", "templates"),
    )
    template = env.get_template("header.h.j2")
    with open(output, "w") as f:
        f.write(template.render(tmp_keys))


@cli.command()
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
def validate(filename):
    """Validate the contents of an existing Transfer List."""
    TransferList.fromfile(filename)
    print("Valid TL!")
