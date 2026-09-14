# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import json
import logging
from pathlib import Path

from openai import APIError, AsyncOpenAI
from openai.types.responses import Response
from src.models.erratum_classification import ErratumClassification

_prompt_fetch_errata = """
**Follow this process to the letter**:
- Select IDs only from the Errata summary table or equivalent errata matrix.
- Before every category comparison below, normalise the table's category:
  - join layout whitespace and compare labels without regard to case;
  - recognise "CatB" and a "Cat" column containing "B" as "Category B";
  - if that row has a rare marker, use "Category B (Rare)" for matching.
- Select only the normalised categories "Category B" and "Category B (Rare)".
  Preserve the original category text in <category_string>.
- If the table has no matching rows, return an empty result.
-Hard constraints:
  - Do NOT use any errata listed outside the table (ignore other tables and all other sections for selecting IDs).
  - Do NOT include Category A, Category C, or any other categories.
  - Do NOT "guess" missing errata IDs from elsewhere. If it's not in the table, it does not exist for this task.
  - Output must be a single JSON array only (no markdown, no commentary).

Extraction steps:
1. Extract ALL errata listed under the table that match the constraints.
2. Identify entries whose category is exactly:
  - "Category B"
  - "Category B (Rare)"
3. Ignore Category A, Category C, and all other categories. If both Category B and Category B (Rare) exist, include both.
4. Preserve the erratum id exactly as written in <erratum_id>.
5. Fetch the Fault status. Convert the revision in which the erratum is present to a list and preserve it to <revisions>.
   Preserve the status as <status>. If the status is Fixed, preserve the revision in <fixed_revision>.
   The 'Fixed in versions' should not be present in `Found in versions`.
6. Copy the complete summary cell for the selected erratum into <summary>.
   Preserve its wording and punctuation, including apparent errors or stray
   words. Do not replace it with a body heading or edit it into a cleaner title.
7. Fetch the errata complete 'Workaround' section for each errata as <workaround>.
8. Fetch the issue or version number of the document as <document_version>. Extract only the numbers, eg 24, 24.0

Return ONLY a JSON array in this format. Return [] when no rows match:
[
  {
    "document_version": "<document_version>",
    "id": "<erratum_id>",
    "category" : "<category_string>",
    "status": "<open|fixed>",
    "present_revisions": ["<revisions>"],
    "fixed_revision": "<fixed_revision>",
    "summary": "<summary>",
    "workaround": "<workaround>"
  }
]
"""

_prompt_classify_errata = """
You are an expert in Arm A-profile CPU errata and Trusted Firmware-A (TF-A).
Classify the supplied erratum by where and when its workaround must run.

Base the result only on the supplied text. Do not invent register names,
bit numbers, values, instruction encodings, affected revisions, exception
levels or ordering requirements. Use the field's documented fallback when
information is missing; use `UNCLASSIFIED` when safe placement is unknown.

For register metadata, use `register_type="Unknown"` and `register_encoding=null`
unless the supplied text explicitly gives the corresponding type or encoding.
A familiar register name is not evidence for either value.

CLASSIFICATION RULES

Classify by when and where the workaround ACTION must execute, not where the
failure is observed.

Allowed classes:
- CPU_RESET: A workaround action required during CPU cold or warm reset,
  or persistent per-CPU configuration established at reset and remaining
  effective until reset or power loss. Examples include setting a control
  bit, disabling a feature, or programming a persistent instruction patch.
  EL3 execution alone is not sufficient. An action required on every EL3
  entry or return belongs to RUNTIME_CALLSITE; an action repeated or ordered
  within every power-down transition belongs to POWERDOWN.

- POWERDOWN: An action that must execute for every CPU or cluster power-down,
  with ordering relative to PWRDN_EN, WFI, ISB, cache/coherency shutdown, or
  another power transition step.

- RUNTIME_CALLSITE: An action that must execute before or after a recurring
  instruction or operation such as TLBI, AT, cache maintenance, RAS, trace, or
  a system-register access.
  Or An action required on exception entry to EL3, before
  returning from EL3, or in an SMC/ERET path.
  Or An action required when switching security world, VM, vCPU,
  process, thread, or translation regime.

- IGNORED: The supplied text does not establish a safe implementation location.
  Or There is know workaround avilable,
  Or EL1 or EL2 owns the triggering event and EL3 cannot perform a
  complete global mitigation.

- UNCLASSIFIED:** Use this category only when none of the categories above apply.

Use exactly one class field for workaround_class.
- If there are workaround details as follow fetch them as <workaround_details>:
    - chicken_bit: The workaround is persistent configuration of one or more named
      CPU implementation or control registers by setting, clearing, toggling, or
      writing bits/fields. List every register explicitly named.
      DO NOT create chicken-bit with missing register_name.
      if the bits cannot be extracted mark as 'n/a'
      if the value cannot be extracted mark as 'n/a'
      if the register_field_name cannot be extracted mark as 'n/a'
      Valid register mappings examples:
      - CPUECTLR_EL1[53] to 1 -> regname: CPUECTLR_EL1, register_field_name: n/a, bits: 53, value: 1
      - Setting PMBLIMITR_EL1.E = 0 -> regname: PMBLIMITR_EL1,  register_field_name: E, bits: n/a, value: 0
      - Setting HCR.TIDCP -> regname: HCR,  register_field_name: TIDCP, bits: n/a, value: 1
      - Set CPUACTLR_EL1[11] to one -> regname: CPUACTLR_EL1, register_field_name: n/a, bits: 11, value: 1
      - Set CPUACTLR_EL1[11] -> regname: CPUACTLR_EL1, register_field_name: n/a, bits: 11, value: 1
      - Clear CPUACTLR_EL1[11] -> regname: CPUACTLR_EL1, register_field_name: n/a, bits: 11, value: 0
      - set SCTLR_EL3.IESB -> regname: SCTLR_EL3 register_field_name: IESB, bits: n/a, value: 1
      - clear SCTLR_EL3.IESB -> regname: SCTLR_EL3, register_field_name: IESB, bits: n/a, value: 0
      - CPUACTLR5_EL1[56:55] to 2'b01 -> regname: CPUACTLR5_EL1 bits: 56:55 value: 2'b01
      - set CNTKCTL_EL1.{EL0PTEN, EL0VTEN, EL0VCTEN, EL0PCTEN} to 0
        -> regname: CNTKCTL_EL1, register_field_name: EL0PTEN, bits: n/a value: 0
        -> regname: CNTKCTL_EL1, register_field_name: EL0VTEN, bits: n/a value: 0
        -> regname: CNTKCTL_EL1, register_field_name: EL0VCTEN, bits: n/a value: 0
        -> regname: CNTKCTL_EL1, register_field_name: EL0PCTEN, bits: n/a value: 0
      - HCR_EL2.{E2H,TGE}={1,1}
        -> regname: HCR_EL2, register_field_name: E2H, bits: n/a, value: 1
        -> regname: HCR_EL2, register_field_name: TGE, bits: n/a, value: 1
      - TRBLIMITR_EL1.TM = 0b00 -> regname: TRBLIMITR_EL1 register_field_name: TM, bits: n/a value 0b00
     chicken_bit cannot contain assembly instructions. If the workaorund provides an
     explicit assembly block that writes registers, classify that block as a
     sequence instead.

    - sequence: The workaround contains one or more explicit, consecutive assembly
      instruction lines. Copy only those instruction lines and preserve their
      exact order. A prose description of operations is never a sequence, even
      when the prose names instructions or describes ordered steps.

  For chicken_bit, report all named registers. Never convert an encoding such as
  S3_6_C15_C1_0 into a guessed architectural register name. Never guess missing
  bits or values. Never include encoded registers such as S3_6_C15_C1_0 in chicken_bits.

  For sequence, copy only assembly instruction lines explicitly given by the
  workaround. Do not translate prose into assembly, do not convert prose actions into
  sequence entries, and do not add implied instructions such as save, restore,
  RET, ISB, or register writes. Exclude prose, headings, explanations,
  page headers, and document metadata. If no explicit assembly instruction block
  is present, sequence must be [].

  If both chicken_bits and sequence are present, extract and include both fields.
  Never omit either field when both are present.
  If a field does not apply, return an empty array ([]) for that field.

  Important distinctions:
1. A failure during power-down is CPU_RESET if the workaround is only a
   persistent register setting. Use POWERDOWN only when the action must be
   repeated or ordered within each power-down transition.
2. A failure while EL1 runs is not automatically LOWER_EL. If EL3 can program
   one persistent control that protects all ELs, use CPU_RESET.
3. Use RUNTIME_CALLSITE or POWERDOWN only when the workaround must run at
   that specific runtime event or during every power-down sequence.
4. Determine ownership separately: per-PE actions belong in the CPU driver;
   shared cluster/DSU actions belong to their shared owner; interconnect/SoC
   actions belong in platform code.
"""


class OpenAIClient:
    _logger: logging.Logger

    _client: AsyncOpenAI
    _openai_api_key: str
    _model: str
    _embedding_model: str
    _temperature: float # Controls randomness. Lower values make classification more deterministic.

    def __init__(
            self,
            model: str,
            embedding_model: str,
            temperature: float,
            openai_api_key: str,
            openai_base_url: str | None
        ):

        self._logger = logging.getLogger(self.__class__.__name__)

        self._openai_api_key = openai_api_key
        self._model = model
        self._embedding_model = embedding_model
        self._temperature = temperature

        self._client = AsyncOpenAI(
            api_key=self._openai_api_key,
            base_url=openai_base_url,
        )

    async def parse_sden(self, sden_file_path: Path):
        """
        - prompt_cache_key:
            A stable identifier used to improve prompt-cache routing for requests
            that share the same prompt prefix.
            it only helps with caching reusable input tokens.

        - input:
            - role:
                Defines the purpose of the message.

                "system":
                    Provides instructions that control how the model should behave.
                    In this case, it contains the errata classification rules.

                "user":
                    Contains the actual data to be processed by the model.
                    In this case, it contains the erratum summary and workaround.

            - content:
                The text associated with the message.
        The fix part of the input is sructured first, and then the changeing part.
        This helps the caching.

        - purpose: `user_data` marks the file as general application-provided input.
                    This is the default for response API.
        """

        with sden_file_path.open("rb") as pdf:
            uploaded_file = await self._client.files.create(
                file=pdf,
                purpose="user_data",
                expires_after={
                    "anchor": "created_at",
                    "seconds": 3600
                }
            )

        try:
            response = await self._client.responses.create(
                model=self._model,
                temperature=self._temperature,
                prompt_cache_key="errata-extraction-v1",
                input=[
                    {
                        "role": "system",
                        "content": _prompt_fetch_errata,
                    },
                    {
                        "role": "user",
                        "content": [
                            {
                                "type": "input_file",
                                "file_id": uploaded_file.id,
                            }
                        ],
                    },
                ]
            )

            self.log_token_usage(response)

            return json.loads(response.output_text)

        finally:
            try:
                await self._client.files.delete(uploaded_file.id)

            except APIError:
                self._logger.exception(
                    "Failed to delete OpenAI file %s. "
                    "The file will expire automatically.",
                    uploaded_file.id,
                )

    async def classify_erratum(self, summary: str, workaround: str):
        """
        - prompt_cache_key:
            A stable identifier used to improve prompt-cache routing for requests
            that share the same prompt prefix.
            it only helps with caching reusable input tokens.

        - input:
            - role:
                Defines the purpose of the message.

                "system":
                    Provides instructions that control how the model should behave.
                    In this case, it contains the errata classification rules.

                "user":
                    Contains the actual data to be processed by the model.
                    In this case, it contains the erratum summary and workaround.

            - content:
                The text associated with the message.
        The fix part of the input is sructured first, and then the changeing part.
        This helps the caching.
        """
        response = await self._client.responses.parse(
            model=self._model,
            temperature=self._temperature,
            prompt_cache_key="errata-classifier-v1",
            input=[
                {
                    "role": "system",
                    "content": _prompt_classify_errata,
                },
                {
                    "role": "user",
                    "content": (
                        f"Summary: {summary}\n"
                        f"Workaround: {workaround}"
                    ),
                },
            ],
            text_format=ErratumClassification,
        )

        self.log_token_usage(response)

        return response.output_parsed

    async def get_embedding(self, text: str):
        response = await self._client.embeddings.create(model=self._embedding_model, input=text)

        return response.data[0].embedding

    def log_token_usage(self, response: Response):
        if not self._logger.isEnabledFor(logging.DEBUG):
            return

        usage = response.usage

        if usage is None:
            return

        input_tokens = usage.input_tokens or 0
        output_tokens = usage.output_tokens or 0
        total_tokens = usage.total_tokens or 0

        cached_tokens = 0
        if usage.input_tokens_details:
            cached_tokens = usage.input_tokens_details.cached_tokens or 0

        uncached_tokens = input_tokens - cached_tokens

        cache_hit_pct = (
            cached_tokens / input_tokens * 100
            if input_tokens > 0
            else 0
        )

        self._logger.debug(
            "Token usage:\n"
            "\tinput_tokens=%d\n"
            "\tcached_tokens=%d\n"
            "\tuncached_tokens=%d\n"
            "\toutput_tokens=%d\n"
            "\ttotal_tokens=%d\n"
            "\tcache_hit=%.2f%%",
            input_tokens,
            cached_tokens,
            uncached_tokens,
            output_tokens,
            total_tokens,
            cache_hit_pct,
        )
