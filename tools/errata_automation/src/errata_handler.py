# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import asyncio
import json
import logging
import re
from collections.abc import Awaitable, Callable
from datetime import datetime
from functools import partial
from pathlib import Path
from typing import TypeVar

from pydantic import ValidationError
from src.corpus import Corpus
from src.cpu import CPU
from src.files.mk_file import MkFile
from src.files.rst_file import RstFile
from src.models.erratum import Erratum
from src.models.workaround_details import WorkaroundClass
from src.openai_client import OpenAIClient
from src.resolvers.resolver import RegisterResolver

R = TypeVar("R")


class AsyncTaskQueue:
    _queue: asyncio.Queue[dict]
    _worker_count: int
    _workers: list[asyncio.Task[None]]

    def __init__(self, worker_count: int = 5) -> None:
        self._queue = asyncio.Queue()
        self._worker_count = worker_count
        self._workers = []

        self._workers = [
            asyncio.create_task(self._worker())
            for _ in range(self._worker_count)
        ]

    def submit(
        self,
        func: Callable[[], Awaitable[R]],
    ) -> asyncio.Future[R]:
        future: asyncio.Future[R] = (
            asyncio.get_running_loop().create_future()
        )

        self._queue.put_nowait(
            {
                "func": func,
                "future": future
            }
        )

        return future

    async def _worker(self) -> None:
        while True:
            job = await self._queue.get()

            try:
                result = await job["func"]()
                job["future"].set_result(result)

            except Exception as error:
                job["future"].set_exception(error)


            self._queue.task_done()


class ErrataHandler:
    _doc_dir_path: Path
    _working_dir_path: Path

    _openai_client: OpenAIClient

    corpus: Corpus

    register_resolver: RegisterResolver

    _logger: logging.Logger

    _async_task_queue: AsyncTaskQueue

    cpus: list[CPU]
    cpu_ops_mk_file: MkFile
    cpu_rst_file: RstFile
    _cpu_asm_dir: Path
    _cpu_files_paths: list[Path]

    def __init__(
            self,
            tfa_base_path: Path,
            working_dir_path: Path,
            openai_api_key: str,
            openai_base_url: str | None,
            openai_model: str,
            openai_embedding_model: str,
            openai_temperature: float,
            classification_similarity_cutoff: float,
            cpus_to_load: str = ".*",
            max_concurent_tasks: int = 20
        ) -> None:

        self._logger = logging.getLogger(self.__class__.__name__)

        self._async_task_queue = AsyncTaskQueue(max_concurent_tasks)

        self.cpus = []
        self._cpu_asm_dir =  tfa_base_path / "lib" / "cpus" / "aarch64"
        self._cpu_files_paths = self._get_cpu_files_paths(cpus_to_load)

        self._working_dir_path = working_dir_path
        self._doc_dir_path = working_dir_path / "docs"

        self._openai_client = OpenAIClient(
            openai_model,
            openai_embedding_model,
            openai_temperature,
            openai_api_key,
            openai_base_url
        )

        self.corpus = Corpus(
            self._openai_client,
            classification_similarity_cutoff
        )

        self.cpu_ops_mk_file = MkFile(
            tfa_base_path / "lib" / "cpus" / "cpu-ops.mk"
        )

        self.cpu_rst_file = RstFile(
            tfa_base_path / "docs" / "design" / "cpu-specific-build-macros.rst"
        )

        self.register_resolver = RegisterResolver(self._doc_dir_path)

    def implement_workarounds(self):
        self._logger.info("Implementing workarounds...")

        for cpu in self.cpus:
            for missing_erratum in cpu.missing_errata_from_sden:
                if missing_erratum.workaround_class is WorkaroundClass.CPU_RESET:
                    self._logger.info(
                        "Implementing workaround: CPU: %s, erratum: %s",
                        cpu.name.human_readable_name,
                        missing_erratum.id
                    )

                    self.register_resolver.resolve_registers(cpu, missing_erratum)

                    missing_erratum.set_require_review()
                    if missing_erratum.requires_review:
                        self._logger.warning(
                            "Erratum %s for CPU %s requires review.",
                            missing_erratum.id,
                            missing_erratum.cpu_name.human_readable_name,
                        )

                    is_updated = cpu.assembly_file.create_and_add_workaround(cpu.name, missing_erratum)

                    if is_updated:
                        cpu.header_file.create_and_add_workaround(cpu.name, missing_erratum)
                        self.cpu_ops_mk_file.create_and_add_workaround(cpu.name, missing_erratum)
                        self.cpu_rst_file.create_and_add_workaround(cpu.name, missing_erratum)
                        missing_erratum.implemented = True

                        self._logger.info(
                            "Workaround implemented: CPU: %s, erratum: %s",
                            cpu.name.human_readable_name,
                            missing_erratum.id
                        )

                    else:
                        self._logger.warning(
                            "Workaround not implemented: CPU: %s, erratum: %s",
                            cpu.name.human_readable_name,
                            missing_erratum.id
                        )

        self._logger.info("Implementing workarounds...done")

    async def load_cpus_and_fetch_errata(self) -> None:
        self._logger.info("Loading CPUs...")

        tasks = []
        for cpu_file in self._cpu_files_paths:
            tasks.append(
                self._async_task_queue.submit(
                    partial(
                        self._load_cpu_and_fetch_errata,
                        cpu_file
                    )
                )
            )

        await asyncio.gather(*tasks)

        self._logger.info("Loading CPUs done")

    async def build_corpus(self) -> None:
        self._logger.info("Bulding corpus...")

        tasks = []
        for cpu in self.cpus:
            for impld_erratum in cpu.implemented_errata_from_sden:
                if impld_erratum.workaround_class == WorkaroundClass.CPU_RESET:
                    tasks.append(
                        self._async_task_queue.submit(
                            partial(
                                self.corpus.add_erratum,
                                impld_erratum,
                            )
                        )
                    )

        await asyncio.gather(*tasks)

        self._logger.info("Bulding corpus...done")

    async def classify_missing_errata_and_extract_workaround_details(self) -> None:
        self._logger.info("Classifying errata...")

        tasks = []
        for cpu in self.cpus:
            for missing_erratum in cpu.missing_errata_from_sden:
                if missing_erratum.workaround_class is WorkaroundClass.UNCLASSIFIED:
                    tasks.append(
                        self._async_task_queue.submit(
                            partial(
                                self._classify_and_extract_workaround_details,
                                cpu,
                                missing_erratum,
                            )
                        )
                    )

        await asyncio.gather(*tasks)

        self._logger.info("Classifying errata...done")

    def get_cpu_names(self):
        return sorted(cpu_file.stem for cpu_file in self._cpu_files_paths)

    def save_corpus(self) -> None:
        timestamp = datetime.now().astimezone().strftime("%d%m%y_%H%M%S")
        cache_path = self._working_dir_path / f"corpus_{timestamp}.json"

        data = {
            str(cpu.name): {
                "missing_errata": [
                    erratum.model_dump(mode="json")
                    for erratum in cpu.missing_errata_from_sden
                ],
                "implemented_errata": [
                    erratum.model_dump(mode="json")
                    for erratum in cpu.implemented_errata_from_sden
                ],
            }
            for cpu in self.cpus
        }

        data["corpus"] = [
            erratum.model_dump(mode="json")
            for erratum in self.corpus._errata_list
        ]

        cache_path.write_text(
            json.dumps(data, indent=2),
            encoding="utf-8",
        )

    async def _add_implemented_errata_to_corpus(self, cpu: CPU) -> None:
        tasks = []
        for impld_erratum in cpu.implemented_errata_from_sden:
            if impld_erratum.workaround_class == WorkaroundClass.CPU_RESET:
                tasks.append(
                    self._async_task_queue.submit(
                        partial(
                            self.corpus.add_erratum,
                            impld_erratum,
                        )
                    )
                )

        await asyncio.gather(*tasks)

    async def _fetch_errata(self, cpu: CPU) -> list[Erratum]:
        new_errata = []

        await cpu.sden_document.fetch()

        errata_json = await self._openai_client.parse_sden(cpu.sden_document.path)

        for erratum_json in errata_json:
            try:
                erratum_json["cpu_name"] = cpu.name
                new_erratum = Erratum.model_validate(erratum_json)
                new_errata.append(new_erratum)
            except ValidationError as error:
                self._logger.warning(
                    "Invalid erratum from openai.\n"
                    "\tcpu: %s\n"
                    "\terratum id:%s\n"
                    "\t%s",
                    cpu.name.human_readable_name,
                    erratum_json["id"],
                    error
                )

        return new_errata

    def _get_cpu_files_paths(self, pattern: str = r".*") -> list[Path]:
        regex = re.compile(pattern)

        paths =  [
            path
            for path in self._cpu_asm_dir.rglob("*.S")
            if regex.fullmatch(path.stem)
        ]

        if not paths:
            self._logger.error(
                "No CPU files matched pattern %r.",
                pattern,
            )

        return paths

    async def _load_cpu_and_fetch_errata(self, cpu_file_path: Path) -> None:
        self._logger.info("Loading CPU file %s...",  cpu_file_path.name)

        cpu = await CPU(cpu_file_path, self._doc_dir_path)

        self.cpus.append(cpu)

        self._logger.info("Fetching errata for %s...", cpu.name.human_readable_name)

        all_errata = await self._fetch_errata(cpu)

        self._logger.info(
            "%d errata fetched for CPU %s",
            len(all_errata),
            cpu.name.human_readable_name
        )

        cpu.update_missing_errata(all_errata)
        self._logger.info(
            "CPU %s has %s missing errata",
            cpu.name,
            len(cpu.missing_errata_from_sden)
        )

        cpu.update_implemented_errata(all_errata)
        self._logger.info(
            "CPU %s has %s implemented errata",
            cpu.name,
            len(cpu.implemented_errata_from_sden)
        )

        self._logger.info("CPU loaded: %s", cpu.name.human_readable_name)

    async def _classify_and_extract_workaround_details(self, cpu: CPU, erratum: Erratum):
        try:
            erratum_classification = await self.corpus.classify(erratum)
            erratum.workaround_class = erratum_classification.workaround_class
            erratum.workaround_details = erratum_classification.workaround_details

        except ValidationError as error:
            erratum.requires_review  = True

            self._logger.error(
                "Failed to classify erratum %s for CPU %s:\n%s",
                erratum.id,
                cpu.name.human_readable_name,
                error,
            )
