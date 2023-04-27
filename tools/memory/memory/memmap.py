#!/usr/bin/env python3

#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from pathlib import Path

import click
from memory.buildparser import TfaBuildParser
from memory.printer import TfaPrettyPrinter


@click.command()
@click.option(
    "-r",
    "--root",
    type=Path,
    default=None,
    help="Root containing build output.",
)
@click.option(
    "-p",
    "--platform",
    show_default=True,
    default="fvp",
    help="The platform targeted for analysis.",
)
@click.option(
    "-b",
    "--build-type",
    default="release",
    help="The target build type.",
    type=click.Choice(["debug", "release"], case_sensitive=False),
)
@click.option(
    "-f",
    "--footprint",
    is_flag=True,
    show_default=True,
    help="Generate a high level view of memory usage by memory types.",
)
@click.option(
    "-t",
    "--tree",
    is_flag=True,
    help="Generate a hierarchical view of the modules, segments and sections.",
)
@click.option(
    "--depth",
    default=3,
    help="Generate a virtual address map of important TF symbols.",
)
@click.option(
    "-s",
    "--symbols",
    is_flag=True,
    help="Generate a map of important TF symbols.",
)
@click.option("-w", "--width", type=int, envvar="COLUMNS")
@click.option(
    "-d",
    is_flag=True,
    default=False,
    help="Display numbers in decimal base.",
)
@click.option(
    "--no-elf-images",
    is_flag=True,
    help="Analyse the build's map files instead of ELF images.",
)
def main(
    root: Path,
    platform: str,
    build_type: str,
    footprint: str,
    tree: bool,
    symbols: bool,
    depth: int,
    width: int,
    d: bool,
    no_elf_images: bool,
):
    build_path = root if root else Path("build/", platform, build_type)
    click.echo(f"build-path: {build_path.resolve()}")

    parser = TfaBuildParser(build_path, map_backend=no_elf_images)
    printer = TfaPrettyPrinter(columns=width, as_decimal=d)

    if footprint or not (tree or symbols):
        printer.print_footprint(parser.get_mem_usage_dict())

    if tree:
        printer.print_mem_tree(
            parser.get_mem_tree_as_dict(), parser.module_names, depth=depth
        )

    if symbols:
        expr = (
            r"(.*)(TEXT|BSS|RODATA|STACKS|_OPS|PMF|XLAT|GOT|FCONF"
            r"|R.M)(.*)(START|UNALIGNED|END)__$"
        )
        printer.print_symbol_table(
            parser.filter_symbols(parser.symbols, expr), parser.module_names
        )


if __name__ == "__main__":
    main()
