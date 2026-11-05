# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import argparse
import asyncio
import logging
import os
import re
from collections import defaultdict
from datetime import datetime
from pathlib import Path

from colorlog import ColoredFormatter
from src.errata_handler import ErrataHandler

logger = logging.getLogger(__name__)

tfa_base_path = Path(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))

date_format = "%Y-%m-%d %H:%M:%S"
no_colored_format = "%(asctime)s.%(msecs)03d %(name)s[%(levelname)s]: %(message)s"
colored_format = f"%(log_color)s{no_colored_format}%(reset)s"

def init_logging(working_dir_path: str | Path):
    handler = logging.StreamHandler()
    handler.setFormatter(
        ColoredFormatter(
            colored_format,
            datefmt=date_format,
            log_colors={
                "DEBUG": "cyan",
                "INFO": "green",
                "WARNING": "yellow",
                "ERROR": "red",
                "CRITICAL": "bold_red",
            },
        )
    )

    log_dir = Path(working_dir_path) / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().astimezone().strftime("%d%m%y_%H%M%S")
    log_path = log_dir / f"log_{timestamp}.log"

    file_handler = logging.FileHandler(
        log_path,
        mode="a",
        encoding="utf-8",
    )

    file_handler.setFormatter(
        logging.Formatter(no_colored_format, datefmt=date_format)
    )

    logging.basicConfig(
        level=logging.INFO,
        force=True,
        handlers=[
            handler,
            file_handler
        ]
    )


class HelpFormatter(argparse.RawTextHelpFormatter):
    def _format_action(self, action: argparse.Action) -> str:
        # Add an extra blank line after each argument in the help output.
        return super()._format_action(action) + "\n"

def parse_args(tfa_base_path: Path):
    parser = argparse.ArgumentParser(
        formatter_class=HelpFormatter,
        epilog=(
            "Environment variables:\n"
            "\n"
            "  OPENAI_API_KEY\n"
            "      Required. Authentication token for the OpenAI API.\n"
            "      The API key must have permission to access the OpenAI API directly.\n"
            "      It can generally be generated at https://platform.openai.com/api-keys\n"
            "      or through your organization's IT infrastructure.\n"
            "\n"
            "  OPENAI_BASE_URL\n"
            "      Base URL for the OpenAI API. Can be overridden to use a custom\n"
            "      or OpenAI-compatible API endpoint.\n"
            "      Default: https://api.openai.com/v1\n"
        ),
    )

    parser.add_argument(
        "--cpus",
        default=".*",
        metavar="REGEX_PATTERN",
        help=(
            "Regex pattern used to select CPUs to load.\n"
            "Examples:\n"
            "  'cortex.*'                 matches all Cortex CPUs\n"
            "  '(c1_pro|neoverse_n3)'     matches only those two CPUs\n"
            "Default: '%(default)s' (loads all CPUs)"
        ),
    )

    parser.add_argument(
        "--debug-mode",
        action="store_true",
        help=(
            "Enable debug mode. When enabled, the errata cache is saved\n"
            "for debugging purposes.\n"
            "Default: %(default)s"
        ),
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help=(
            "Loads the CPUs and checks for new errata, but skips implementing "
            "the workarounds."
        ),
    )

    parser.add_argument(
        "--list-cpus",
        action="store_true",
        help="List available CPUs and exit.",
    )

    parser.add_argument(
        "--openai-model",
        type=str,
        default="gpt-5.4",
        help=(
            "OpenAI model to use for requests.\n"
            "See all available models: https://platform.openai.com/docs/models\n"
            "Default: %(default)s"
        ),
    )

    parser.add_argument(
        "--openai-embeddingmodel",
        type=str,
        choices=[
            "text-embedding-3-small",
            "text-embedding-3-large",
        ],
        default="text-embedding-3-small",
        help=(
            "OpenAI embedding model to use for generating embeddings.\n"
            "Options: %(choices)s\n"
            "Default: %(default)s"
        ),
    )

    parser.add_argument(
        "--openai-temperature",
        type=float,
        default=0.3,
        help=(
            "Sampling temperature used for OpenAI model responses.\n"
            "Higher values make responses more varied; lower values "
            "make them more deterministic.\n"
            "Range: 0.0 to 2.0\n"
            "Default: %(default)s"
        ),
    )

    parser.add_argument(
        "--similarity-cutoff",
        type=float,
        default=0.8,
        help=(
            "Minimum similarity score required for a match.\n"
            "Range: 0.0 to 1.0\n"
            "Default: %(default)s"
        ),
    )

    parser.add_argument(
        "--working-dir-path",
        type=Path,
        default=tfa_base_path / "build" / "errata_automation",
        metavar="PATH",
        help=(
            "Working directory used by the tool. Defaults to the current directory.\n"
            "The 'docs' and 'logs' directories will be created inside it.\n"
            "Default: %(default)s"
        )
    )

    return parser.parse_args()

def print_available_cpus(cpu_names: list[str]) -> None:
    groups = defaultdict(list)

    for cpu in cpu_names:
        match = re.match(r"^(?P<family>[^_]+)_[^_]+$", cpu, re.IGNORECASE)
        group = match.group("family").title() if match else "Other"

        groups[group].append(cpu)

    for group, cpus in list(groups.items()):
        if group != "Other" and len(cpus) == 1:
            groups["Other"].extend(cpus)
            del groups[group]

    print("Available CPUs:")

    for group, cpus in sorted(groups.items()):
        print(f"    {group}:")

        cpus.sort()

        for i in range(0, len(cpus), 5):
            print(f"        {', '.join(cpus[i:i + 5])}")

        print()


async def main(args):
    openai_api_key = os.environ.get("OPENAI_API_KEY")
    if not openai_api_key:
        logger.error("OPENAI_API_KEY environment variable is not set.")
        raise SystemExit(1)

    openai_base_url = os.environ.get("OPENAI_BASE_URL")
    if not openai_base_url:
        logger.warning("OPENAI_BASE_URL is not set using default url, https://api.openai.com/v1.")

    errata_handler = ErrataHandler(
        tfa_base_path=tfa_base_path,
        working_dir_path=args.working_dir_path,
        openai_model=args.openai_model,
        openai_base_url=openai_base_url,
        openai_embedding_model=args.openai_embeddingmodel,
        openai_temperature=args.openai_temperature,
        openai_api_key=openai_api_key,
        classification_similarity_cutoff=args.similarity_cutoff,
        cpus_to_load=args.cpus,
    )

    if args.list_cpus:
        print_available_cpus(errata_handler.get_cpu_names())
        return

    await errata_handler.load_cpus_and_fetch_errata()

    await errata_handler.build_corpus()

    await errata_handler.classify_missing_errata_and_extract_workaround_details()

    if not args.dry_run:
        errata_handler.implement_workarounds()

    if args.debug_mode:
        errata_handler.save_corpus()


if __name__ == "__main__":
    args = parse_args(tfa_base_path)

    init_logging(args.working_dir_path)

    logger.info("ErrataHandler started")

    try:
        asyncio.run(main(args))
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
    finally:
        logger.info("ErrataHandler exited")
