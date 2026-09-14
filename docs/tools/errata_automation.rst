Errata Automation
=================

``tools/errata_automation`` finds missing Category-B and Category-B(rare)
CPU errata and can generates TF-A CPU-reset workarounds.

Core concept
------------

The tool reads each CPU's TF-A assembly implementation and its SDEN/TRM links.
It downloads the SDEN, extracts applicable Category-B errata, and compares them
with workarounds already in TF-A.

Existing reset-time workarounds form a trusted reference corpus;
new errata are compared against this corpus using embedding similarity to reuse
trusted classifications, while unmatched errata are classified by an LLM.


The tool resolves register fields
and encodings from architectural data or the TRM, then updates the assembly,
header, makefile, and build-macro documentation.

All classifications and proposed changes require human review.

Currently, automatic implementation supports only reset-time workarounds.
Support for power-down and runtime workarounds is planned.

Workflow
--------
1. Install python depenedncies through Poetry, run ``poetry install --no-root``

2. Configure the OpenAI API environment variables:
   ``OPENAI_API_KEY`` (required): API authentication token with direct
   OpenAI API access.

   ``OPENAI_BASE_URL`` (optional): OpenAI's API endpoint can be overriden.
   The script uses https://api.openai.com/v1 by default.

3. Select the CPU sources to process. Each assembly file identifies the CPU
   and provides its SDEN and TRM links.

   Run ``poetry run python tools/errata_automation/errata.py --list-cpus`` to
   list the cpu names.

4. Run ``tools/errata_automation/errata.py`` through Poetry to identify
   potentially missing Category-B and Category-B(rare) errata and,
   where supported, generate proposed TF-A changes.

   eg.: ``poetry run python tools/errata_automation/errata.py --cpus c1_pro``

5. Review each reported erratum and verify that:

   * the erratum applies to the selected CPU and revision
   * the proposed workaround classification is correct
   * any referenced registers, fields, and encodings
     match the architectural documentation or TRM
   * the generated TF-A changes correctly implement the documented workaround.

6. Accept and integrate validated reset-time workarounds. Errata that cannot
   be implemented automatically, or for which the classification or register
   information is uncertain, must be handled manually.

7. Build and test the resulting TF-A changes using the normal platform
   validation process before submitting them for review.


Data validation
---------------

The script using `Pydantic <https://docs.pydantic.dev/>`_ to validate
untrusted LLM model output before it reaches code generation.
Invalid responses therefore fail validation and are flagged for review
instead of being used to edit TF-A sources.
