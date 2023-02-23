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
    "-s",
    "--symbols",
    is_flag=True,
    show_default=True,
    default=True,
    help="Generate a map of important TF symbols.",
)
@click.option("-w", "--width", type=int, envvar="COLUMNS")
@click.option(
    "-d",
    is_flag=True,
    default=False,
    help="Display numbers in decimal base.",
)
def main(
    root: Path,
    platform: str,
    build_type: str,
    symbols: bool,
    width: int,
    d: bool,
):
    build_path = root if root else Path("build/", platform, build_type)
    click.echo(f"build-path: {build_path.resolve()}")

    parser = TfaBuildParser(build_path)
    printer = TfaPrettyPrinter(columns=width, as_decimal=d)

    if symbols:
        expr = (
            r"(.*)(TEXT|BSS|RODATA|STACKS|_OPS|PMF|XLAT|GOT|FCONF"
            r"|R.M)(.*)(START|END)__$"
        )
        printer.print_symbol_table(
            parser.filter_symbols(parser.symbols, expr), parser.module_names
        )


if __name__ == "__main__":
    main()
