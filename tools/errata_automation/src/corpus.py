# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging

import numpy as np
from numpy.typing import NDArray
from src.models.erratum import Erratum
from src.models.erratum_classification import ErratumClassification
from src.models.workaround_details import WorkaroundClass
from src.openai_client import OpenAIClient


class Corpus:
    _logger: logging.Logger
    _openai_client: OpenAIClient

    _errata_list: list[Erratum]
    _errata_embeddings: NDArray[np.float32] | None

    def __init__(
            self,
            openai_client: OpenAIClient,
            similarity_cutoff:float
        ):
        super().__init__()

        self._logger = logging.getLogger(self.__class__.__name__)

        self._openai_client = openai_client
        self._similarity_cutoff = similarity_cutoff

        self._errata_list = []
        self._errata_embeddings = None

    async def add_erratum(self, erratum: Erratum):
        embedding = await self.get_embedding(erratum.summary, erratum.workaround)

        if self.contains_erratum(erratum):
            raise ValueError(
                f"Erratum {erratum.id} already exists for CPU "
                f"{erratum.cpu_name.human_readable_name}."
            )

        if erratum.workaround_class == WorkaroundClass.UNCLASSIFIED:
            raise ValueError(
                f"Erratum {erratum.id} cannot be added to the Corpus. "
                "The Corpus cannot contain unclassified errata."
            )

        self._errata_list.append(erratum)
        self._append_embedding(embedding)

        self._logger.info(
            "Erratum %s added to the corpus for CPU %s",
            erratum.id,
            erratum.cpu_name.human_readable_name
        )

    def contains_erratum(self, erratum: Erratum) -> bool:
        return any(
            item.cpu_name == erratum.cpu_name
            and item.id == erratum.id
            for item in self._errata_list
        )

    async def classify(self, erratum: Erratum) -> ErratumClassification:
        self._logger.info(
            "Classifying erratum: %s - %s",
            erratum.cpu_name.human_readable_name,
            erratum.id
        )

        erratum_classification = await self._openai_client.classify_erratum(erratum.summary, erratum.workaround)

        if self._errata_embeddings is not None:
            """
            For normalized vectors, the dot product is equivalent to cosine similarity.
            self._embeddings @ new_embedding is equivalent to np.dot(a, b).
            Cosine similarity measures how similar two vectors are by comparing
            the angle between them rather than their absolute size.
            A value close to 1.0 means the vectors point in very similar
            directions and therefore represent semantically similar text.
            We use cosine similarity to compare the embedding of a new erratum
            with embeddings of previously classified errata.
            If the similarity is above a chosen threshold, the previous classification
            can be reused because the two errata are likely describing similar behavior or workarounds.
            """
            new_embedding = await self.get_embedding(erratum.summary, erratum.workaround)
            similarities = self._errata_embeddings @ new_embedding
            best_position = similarities.argmax()
            similarity = similarities[best_position]

            if similarity >= self._similarity_cutoff:
                similar_erratum = self._errata_list[best_position]

                erratum_classification.workaround_class = similar_erratum.workaround_class

        self._logger.info(
            "Erratum %s for %s classified as %s",
            erratum.id,
            erratum.cpu_name.human_readable_name,
            erratum_classification,
        )

        return erratum_classification

    async def get_embedding(self, summary: str, workaround: str) -> NDArray[np.float32]:
        #OpenAI’s Embeddings API returns L2-normalized vectors by default
        return np.array(
            await self._openai_client.get_embedding(f"{summary}: {workaround}"),
            dtype=np.float32
        )

    def _append_embedding(self, embedding: NDArray[np.float32]) -> None:
        if self._errata_embeddings is None:
            self._errata_embeddings = np.atleast_2d(embedding)
            return

        assert embedding.shape[0] == self._errata_embeddings.shape[1], (
            f"new embedding has {embedding.shape[0]} dimensions, "
            f"but existing embeddings have {self._errata_embeddings.shape[1]}"
        )

        self._errata_embeddings = np.vstack(
            (self._errata_embeddings, embedding)
        )
