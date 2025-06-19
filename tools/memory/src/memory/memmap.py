#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional

import click

from memory.elfparser import TfaElfParser
from memory.image import Image
from memory.mapparser import TfaMapParser
from memory.printer import TfaPrettyPrinter
from memory.summary import MapParser


@dataclass
class Context:
    build_path: Optional[Path] = None
    printer: Optional[TfaPrettyPrinter] = None


@click.group()
@click.pass_obj
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
    "-w",
    "--width",
    type=int,
    default=shutil.get_terminal_size().columns,
    help="Column width for printing.",
)
@click.option(
    "-d",
    is_flag=True,
    default=False,
    help="Display numbers in decimal base.",
)
def cli(
    obj: Context,
    root: Optional[Path],
    platform: str,
    build_type: str,
    width: int,
    d: bool,
):
    obj.build_path = root if root is not None else Path("build", platform, build_type)
    click.echo(f"build-path: {obj.build_path.resolve()}")

    obj.printer = TfaPrettyPrinter(columns=width, as_decimal=d)


@cli.command()
@click.pass_obj
@click.option(
    "--no-elf-images",
    is_flag=True,
    help="Analyse the build's map files instead of ELF images.",
)
def footprint(obj: Context, no_elf_images: bool):
    """Generate a high level view of memory usage by memory types."""

    assert obj.build_path is not None
    assert obj.printer is not None

    elf_image_paths: List[Path] = (
        [] if no_elf_images else list(obj.build_path.glob("**/*.elf"))
    )

    map_file_paths: List[Path] = (
        [] if not no_elf_images else list(obj.build_path.glob("**/*.map"))
    )

    images: Dict[str, Image] = dict()

    for elf_image_path in elf_image_paths:
        with open(elf_image_path, "rb") as elf_image_io:
            images[elf_image_path.stem.upper()] = TfaElfParser(elf_image_io)

    for map_file_path in map_file_paths:
        with open(map_file_path, "r") as map_file_io:
            images[map_file_path.stem.upper()] = TfaMapParser(map_file_io)

    obj.printer.print_footprint({k: v.footprint for k, v in images.items()})


@cli.command()
@click.pass_obj
@click.option(
    "--depth",
    default=3,
    show_default=True,
    help="Generate a virtual address map of important TF symbols.",
)
def tree(obj: Context, depth: int):
    """Generate a hierarchical view of the modules, segments and sections."""

    assert obj.build_path is not None
    assert obj.printer is not None

    paths: List[Path] = list(obj.build_path.glob("**/*.elf"))
    images: Dict[str, TfaElfParser] = dict()

    for path in paths:
        with open(path, "rb") as io:
            images[path.stem] = TfaElfParser(io)

    mtree: Dict[str, Dict[str, Any]] = {
        k: {
            "name": k,
            **v.get_mod_mem_usage_dict(),
            **{"children": v.get_seg_map_as_dict()},
        }
        for k, v in images.items()
    }

    obj.printer.print_mem_tree(mtree, list(mtree.keys()), depth=depth)


@cli.command()
@click.pass_obj
@click.option(
    "--no-elf-images",
    is_flag=True,
    help="Analyse the build's map files instead of ELF images.",
)
def symbols(obj: Context, no_elf_images: bool):
    """Generate a map of important TF symbols."""

    assert obj.build_path is not None
    assert obj.printer is not None

    expr: str = (
        r"(.*)(TEXT|BSS|RO|RODATA|STACKS|_OPS|PMF|XLAT|GOT|FCONF|RELA"
        r"|R.M)(.*)(START|UNALIGNED|END)__$"
    )

    elf_image_paths: List[Path] = (
        [] if no_elf_images else list(obj.build_path.glob("**/*.elf"))
    )

    map_file_paths: List[Path] = (
        [] if not no_elf_images else list(obj.build_path.glob("**/*.map"))
    )

    images: Dict[str, Image] = dict()

    for elf_image_path in elf_image_paths:
        with open(elf_image_path, "rb") as elf_image_io:
            images[elf_image_path.stem] = TfaElfParser(elf_image_io)

    for map_file_path in map_file_paths:
        with open(map_file_path, "r") as map_file_io:
            images[map_file_path.stem] = TfaMapParser(map_file_io)

    symbols = {k: v.symbols for k, v in images.items()}
    symbols = {
        image: {
            symbol: symbol_value
            for symbol, symbol_value in symbols.items()
            if re.match(expr, symbol)
        }
        for image, symbols in symbols.items()
    }

    obj.printer.print_symbol_table(symbols, list(images.keys()))


@cli.command()
@click.option("-o", "--old", type=click.Path(exists=True))
@click.option("-d", "--depth", type=int, default=2)
@click.option("-e", "--exclude-fill")
@click.option(
    "-t",
    "--type",
    type=click.Choice(MapParser.export_formats, case_sensitive=False),
    default="table",
)
@click.argument("file", type=click.Path(exists=True))
def summary(file: Path, old: Optional[Path], depth: int, exclude_fill: bool, type: str):
    """Summarize the sizes of translation units within the resulting binary"""
    memap = MapParser()

    if not memap.parse(file, old, exclude_fill):
        exit(1)

    memap.generate_output(type, depth)


def main():
    cli(obj=Context())


if __name__ == "__main__":
    main()
