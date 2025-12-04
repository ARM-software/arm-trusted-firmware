Architecture Features
=====================

The Arm architecture evolves through versioning, with new architectural features being introduced
almost every year listed in `Feature_description`_. While most of these features require no control
at EL3, some demand explicit configuration of EL3 control registers to enable their use at lower
Exception Levels. For features with EL3 controls, it is relatively straightforward to examine the
code and determine whether TF-A support them. However, for features that are transparent to EL3, it
is more challenging to confirm whether they have been analyzed and verified as not requiring changes
in TF-A.

This document serves as a central reference point to track and clarify the support status of
architectural features within TF-A.

This document organizes features according to the yearly updates, while the specific architecture
versions (8.X, 9.X) to which they apply can be found in `Feature_description`_

.. note::

   **Status legend** —

   * ``OK``: TF-A has explicit support;
   * ``NA``: Transparent from EL3 (no EL3 changes required);
   * ``NO``: Analyzed but decided not to implement;
   * ``WIP``: Implementation in progress;
   * *(empty)*: Not analyzed.

2025 Extensions
---------------

+-------------------------+--------+
| Feature                 | Status |
+=========================+========+
| FEAT_GCIE               |        |
+-------------------------+--------+
| FEAT_GCIE_LEGACY        |        |
+-------------------------+--------+
| FEAT_F16F32DOT          |        |
+-------------------------+--------+
| FEAT_F16F32MM           |        |
+-------------------------+--------+
| FEAT_F16MM              |        |
+-------------------------+--------+
| FEAT_SVE_B16MM          |        |
+-------------------------+--------+
| FEAT_SME2p3             |        |
+-------------------------+--------+
| FEAT_MTE_EIRG           |  NA    |
+-------------------------+--------+
| FEAT_MTETC              |        |
+-------------------------+--------+
| FEAT_SVE2p3             |        |
+-------------------------+--------+
| FEAT_SCR2               |        |
+-------------------------+--------+
| FEAT_LSCP               |        |
+-------------------------+--------+
| FEAT_MPAMv2_VID         |        |
+-------------------------+--------+
| FEAT_MPAMv2_MSC         |        |
+-------------------------+--------+
| FEAT_MPAMv2_MSC_MON_SEC |        |
+-------------------------+--------+
| FEAT_MPAMv2_ALT_ID      |        |
+-------------------------+--------+
| FEAT_MPAMv2             |        |
+-------------------------+--------+
| FEAT_FDIT               |        |
+-------------------------+--------+
| FEAT_CMH                |        |
+-------------------------+--------+
| FEAT_EAESR              |        |
+-------------------------+--------+
| FEAT_TLBID              |        |
+-------------------------+--------+
| FEAT_SRMASK2            |        |
+-------------------------+--------+
| FEAT_BBML3              |        |
+-------------------------+--------+
| FEAT_NV3                |        |
+-------------------------+--------+
| FEAT_PAuth_EnhCtl       |        |
+-------------------------+--------+
| FEAT_LORRL              |        |
+-------------------------+--------+

2024 Extensions
---------------

+-----------------------+--------+
| Feature               | Status |
+=======================+========+
| FEAT_PMUv3_EXTPMN     |        |
+-----------------------+--------+
| FEAT_SPEv1p5          |        |
+-----------------------+--------+
| FEAT_SPE_EXC          |        |
+-----------------------+--------+
| FEAT_SPE_nVM          |        |
+-----------------------+--------+
| FEAT_TRBEv1p1         |        |
+-----------------------+--------+
| FEAT_TRBE_EXC         |        |
+-----------------------+--------+
| FEAT_SME_MOP4         |        |
+-----------------------+--------+
| FEAT_SME_TMOP         |        |
+-----------------------+--------+
| FEAT_PoPS             |        |
+-----------------------+--------+
| FEAT_SME2p2           |        |
+-----------------------+--------+
| FEAT_CMPBR            |        |
+-----------------------+--------+
| FEAT_SSVE_AES         |        |
+-----------------------+--------+
| FEAT_RME_GPC3         |  WIP   |
+-----------------------+--------+
| FEAT_RME_GDI          |  OK    |
+-----------------------+--------+
| FEAT_SVE2p2           |        |
+-----------------------+--------+
| FEAT_SSVE_BitPerm     |        |
+-----------------------+--------+
| FEAT_SSVE_FEXPA       |        |
+-----------------------+--------+
| FEAT_SVE_AES2         |        |
+-----------------------+--------+
| FEAT_LSFE             |        |
+-----------------------+--------+
| FEAT_FPRCVT           |        |
+-----------------------+--------+
| FEAT_SVE_F16F32MM     |        |
+-----------------------+--------+
| FEAT_F8F16MM          |        |
+-----------------------+--------+
| FEAT_F8F32MM          |        |
+-----------------------+--------+
| FEAT_SVE_BFSCALE      |        |
+-----------------------+--------+
| FEAT_OCCMO            |        |
+-----------------------+--------+
| FEAT_LS64WB           |        |
+-----------------------+--------+
| FEAT_AMU_EXTACR       |        |
+-----------------------+--------+
| FEAT_MPAM_MSC_DOMAINS |        |
+-----------------------+--------+
| FEAT_MPAM_MSC_DCTRL   |        |
+-----------------------+--------+
| FEAT_MPAM_PE_BW_CTRL  |   OK   |
+-----------------------+--------+
| FEAT_PCDPHINT         |        |
+-----------------------+--------+
| FEAT_SRMASK           |        |
+-----------------------+--------+
| FEAT_IDTE3            |   OK   |
+-----------------------+--------+
| FEAT_UINJ             |   OK   |
+-----------------------+--------+
| FEAT_NV2p1            |        |
+-----------------------+--------+
| FEAT_LSUI             |        |
+-----------------------+--------+

2023 Extensions
---------------

+-------------------+--------+
| Feature           | Status |
+===================+========+
| FEAT_E2H0         |   NA   |
+-------------------+--------+
| FEAT_SME_LUTv2    |   NA   |
+-------------------+--------+
| FEAT_LUT          |   NA   |
+-------------------+--------+
| FEAT_FAMINMAX     |   NA   |
+-------------------+--------+
| FEAT_FPMR         |   OK   |
+-------------------+--------+
| FEAT_FP8          |   NA   |
+-------------------+--------+
| FEAT_FP8FMA       |   NA   |
+-------------------+--------+
| FEAT_SSVE_FP8FMA  |   NA   |
+-------------------+--------+
| FEAT_FP8DOT4      |   NA   |
+-------------------+--------+
| FEAT_SSVE_FP8DOT4 |   NA   |
+-------------------+--------+
| FEAT_FP8DOT2      |   NA   |
+-------------------+--------+
| FEAT_SSVE_FP8DOT2 |   NA   |
+-------------------+--------+
| FEAT_SME_F8F16    |   NA   |
+-------------------+--------+
| FEAT_SME_F8F32    |   NA   |
+-------------------+--------+
| FEAT_CPA          |   NA   |
+-------------------+--------+
| FEAT_CPA2         |   OK   |
+-------------------+--------+
| FEAT_STEP2        |        |
+-------------------+--------+
| FEAT_BWE2         |   NA   |
+-------------------+--------+
| FEAT_SPE_FPF      |   NA   |
+-------------------+--------+
| FEAT_SPE_EFT      |   NA   |
+-------------------+--------+
| FEAT_PMUv3_TH2    |   NA   |
+-------------------+--------+
| FEAT_SPMU2        |   NA   |
+-------------------+--------+
| FEAT_E3DSE        |        |
+-------------------+--------+
| FEAT_PMUv3_SME    |   NA   |
+-------------------+--------+
| FEAT_SPE_SME      |   NA   |
+-------------------+--------+
| FEAT_SPE_ALTCLK   |   NA   |
+-------------------+--------+
| FEAT_HDBSS        |        |
+-------------------+--------+
| FEAT_HACDBS       |        |
+-------------------+--------+
| FEAT_TLBIW        |   NA   |
+-------------------+--------+
| FEAT_ASID2        |   NA   |
+-------------------+--------+
| FEAT_RME_GPC2     |   OK   |
+-------------------+--------+
| FEAT_FGWTE3       |   OK   |
+-------------------+--------+
| FEAT_PAuth_LR     |   OK   |
+-------------------+--------+
| FEAT_ETS3         |   NA   |
+-------------------+--------+

2022 Extensions
---------------

+--------------------------+--------+
| Feature                  | Status |
+==========================+========+
| FEAT_BWE                 |        |
+--------------------------+--------+
| FEAT_ADERR               |        |
+--------------------------+--------+
| FEAT_ANERR               |        |
+--------------------------+--------+
| FEAT_DoubleFault2        |        |
+--------------------------+--------+
| FEAT_PFAR                |   OK   |
+--------------------------+--------+
| FEAT_RASv2               |        |
+--------------------------+--------+
| FEAT_CSSC                |        |
+--------------------------+--------+
| FEAT_RPRFM               |        |
+--------------------------+--------+
| FEAT_PRFMSLC             |        |
+--------------------------+--------+
| FEAT_SPECRES2            |        |
+--------------------------+--------+
| FEAT_CLRBHB              |   Ok   |
+--------------------------+--------+
| FEAT_ECBHB               |   NA   |
+--------------------------+--------+
| FEAT_SME2p1              |        |
+--------------------------+--------+
| FEAT_SME_F16F16          |        |
+--------------------------+--------+
| FEAT_SVE_B16B16          |        |
+--------------------------+--------+
| FEAT_SME_B16B16          |        |
+--------------------------+--------+
| FEAT_FGT2                |   OK   |
+--------------------------+--------+
| FEAT_MTE4                |   NA   |
+--------------------------+--------+
| FEAT_MTE_CANONICAL_TAGS  |   NA   |
+--------------------------+--------+
| FEAT_MTE_NO_ADDRESS_TAGS |   NA   |
+--------------------------+--------+
| FEAT_MTE_TAGGED_FAR      |        |
+--------------------------+--------+
| FEAT_MTE_PERM            |   NA   |
+--------------------------+--------+
| FEAT_MTE_STORE_ONLY      |   NA   |
+--------------------------+--------+
| FEAT_ITE                 |        |
+--------------------------+--------+
| FEAT_TRBE_EXT            |        |
+--------------------------+--------+
| FEAT_TRBE_MPAM           |        |
+--------------------------+--------+
| FEAT_ETEv1p3             |        |
+--------------------------+--------+
| FEAT_GCS                 |   OK   |
+--------------------------+--------+
| FEAT_CHK                 |        |
+--------------------------+--------+
| FEAT_SPE_DPFZS           |        |
+--------------------------+--------+
| FEAT_SPE_CRR             |        |
+--------------------------+--------+
| FEAT_EBEP                |   OK   |
+--------------------------+--------+
| FEAT_SEBEP               |   OK   |
+--------------------------+--------+
| FEAT_PMUv3_SS            |        |
+--------------------------+--------+
| FEAT_SPMU                |        |
+--------------------------+--------+
| FEAT_PMUv3_ICNTR         |        |
+--------------------------+--------+
| FEAT_PMUv3p9             |        |
+--------------------------+--------+
| FEAT_SVE2p1              |        |
+--------------------------+--------+
| FEAT_Debugv8p9           |   OK   |
+--------------------------+--------+
| FEAT_ABLE                |        |
+--------------------------+--------+
| FEAT_PCSRv8p9            |        |
+--------------------------+--------+
| FEAT_LRCPC3              |        |
+--------------------------+--------+
| FEAT_SPEv1p4             |        |
+--------------------------+--------+
| FEAT_SPE_FDS             |        |
+--------------------------+--------+
| FEAT_PMUv3_EDGE          |        |
+--------------------------+--------+
| FEAT_D128                |   OK   |
+--------------------------+--------+
| FEAT_AIE                 |   OK   |
+--------------------------+--------+
| FEAT_S1PIE               |   OK   |
+--------------------------+--------+
| FEAT_ATS1A               |        |
+--------------------------+--------+
| FEAT_S1POE               |   OK   |
+--------------------------+--------+
| FEAT_S2PIE               |   OK   |
+--------------------------+--------+
| FEAT_S2POE               |   OK   |
+--------------------------+--------+
| FEAT_SYSREG128           |   OK   |
+--------------------------+--------+
| FEAT_SYSINSTR128         |        |
+--------------------------+--------+
| FEAT_LSE128              |        |
+--------------------------+--------+
| FEAT_HAFT                |        |
+--------------------------+--------+
| FEAT_THE                 |   OK   |
+--------------------------+--------+
| FEAT_LVA3                |        |
+--------------------------+--------+
| FEAT_MTE_ASYNC           |        |
+--------------------------+--------+
| FEAT_EDHSR               |        |
+--------------------------+--------+
| FEAT_AMU_EXT64           |        |
+--------------------------+--------+


2021 Extensions
---------------

+------------------+--------+
| Feature          | Status |
+==================+========+
| FEAT_SME2        |   OK   |
+------------------+--------+
| FEAT_MEC         |   OK   |
+------------------+--------+
| FEAT_BRBEv1p1    |   WIP  |
+------------------+--------+
| FEAT_CMOW        |   NA   |
+------------------+--------+
| FEAT_Debugv8p8   |   NA   |
+------------------+--------+
| FEAT_HBC         |   NA   |
+------------------+--------+
| FEAT_MOPS        |   OK   |
+------------------+--------+
| FEAT_NMI         |   OK   |
+------------------+--------+
| FEAT_SPEv1p3     |   NA   |
+------------------+--------+
| FEAT_TIDCP1      |   NA   |
+------------------+--------+
| FEAT_PMUv3_TH    |   NA   |
+------------------+--------+
| FEAT_PMUv3p8     |   NA   |
+------------------+--------+
| FEAT_SCTLR2      |   OK   |
+------------------+--------+
| FEAT_TCR2        |   OK   |
+------------------+--------+
| FEAT_PMUv3_EXT64 |   NA   |
+------------------+--------+


2020 Extensions
---------------

+---------------------+--------+
| Feature             | Status |
+=====================+========+
| FEAT_EBF16          |   NA   |
+---------------------+--------+
| FEAT_ETEv1p1        |   NA   |
+---------------------+--------+
| FEAT_HCX            |   OK   |
+---------------------+--------+
| FEAT_PAN3           |   OK   |
+---------------------+--------+
| FEAT_WFxT           |   NA   |
+---------------------+--------+
| FEAT_XS             |   NO   |
+---------------------+--------+
| FEAT_AFP            |   NA   |
+---------------------+--------+
| FEAT_RPRES          |   NA   |
+---------------------+--------+
| FEAT_LPA2           |   NO   |
+---------------------+--------+
| FEAT_LS64           |   NA   |
+---------------------+--------+
| FEAT_LS64_V         |   NA   |
+---------------------+--------+
| FEAT_LS64_ACCDATA   |   OK   |
+---------------------+--------+
| FEAT_MTE3           |   NA   |
+---------------------+--------+
| FEAT_MTE_ASYM_FAULT |   NA   |
+---------------------+--------+
| FEAT_SPEv1p2        |   OK   |
+---------------------+--------+
| FEAT_SPE_FnE        |   NA   |
+---------------------+--------+
| FEAT_SPE_PBT        |   OK   |
+---------------------+--------+
| FEAT_PMUv3p7        |   OK   |
+---------------------+--------+

The ``ENABLE_FEAT`` mechanism
-----------------------------

The Arm architecture defines several architecture extension features, named
FEAT_xxx in the architecure manual. Some of those features require setup code in
higher exception levels, other features might be used by TF-A code itself.

Most of the feature flags defined in the TF-A build system are allowed to take
the values 0, 1 or 2, with the following meaning:

::

    ENABLE_FEAT_* = 0: Feature is unconditionally disabled at compile time.
    ENABLE_FEAT_* = 1: Feature is unconditionally enabled at compile time.
    ENABLE_FEAT_* = 2: Feature support is compiled in, but only enabled if the
                       feature present at runtime.

When setting the flag to 0, the feature is disabled during compilation, and the
compiler's optimization stage and the linker will try to remove as much of this
code as possible.

If it is defined to 1, the code will use the feature unconditionally, so the CPU
is expected to support that feature. The FEATURE_DETECTION debug feature, if
enabled, will verify this.

If the feature flag is set to 2, support for the feature will be compiled in,
but its existence will be checked at runtime, so it works on CPUs with or
without the feature. This is mostly useful for platforms which either support
multiple different CPUs, or where the CPU is configured at runtime, like in
emulators.

Adding support for a new feature
--------------------------------

Adding support for a feature is required when:

a) a feature is added to the architecture and it includes controls at EL3

b) a feature is to be used by EL3 itself

c) EL3 wants to hide a feature from lower ELs (eg. with ``FEAT_IDTE3``)

The following is a checklist with guidance on what to do to add a new feature,
in this case a fictional ``FEAT_ABC``. This is not an exhaustive list on how to
do this, so please consult with previous such patches and/or maintainers. Please
consult the Arm ARM on any specifics about the feature itself.

- Add a feature flag to the build system.

  - The flag's name should be in the form ``ENABLE_FEAT_ABC``.

  - Add it to the ``assert_numerics`` and ``add_defines`` lists in the
    ``Makefile``.

  - Add a default of ``0`` for it in ``make_helpers/arch_features.mk``. If the
    feature is listed as mandatory from a certain revision of the architecture,
    add it to the appropriate list at the top of the same file.

  - Add any constraints in ``make_helpers/constraints.mk``. This will usually be
    other features which this feature depends on or is incompatible with and can
    be found in the main description of the feature in the Arm ARM.

- Add the feature accessor functions in ``include/arch/aarch64/arch_features.h``.

  A pair of functions must be defined in the form of ``is_feat_abc_present()``
  and ``is_feat_abc_supported()``.  Please use the provided macros to define
  these.

  - The ``_supported`` variant is the one that should usually be used. If common
    code is to use it, a corresponding AArch32 version should be provided. It
    should return ``false`` if AArch32 is not to be supported.

  - In rare occasions, the feature must be checked in assembly. A
    ``is_feat_abc_present_asm`` macro must be added to
    ``include/lib/cpus/aarch64/cpu_macros.S``.

- Register the feature in ``common/feat_detect.c``.

- Add support for the feature. This will be very feature specific and exact code
  will depend on why support is being added. Usually, support will be a few
  lines in ``lib/el3_runtime/aarch64/context_mgmt.c``. Some common things to do
  include:

  - If support requires more extensive code sequences, consider adding them to a
    file in ``lib/extensions/abc/``. Functions in this file must be named
    similar to other features. Those are likely to be ``abc_enable()``,
    ``abc_enable_el3()``, or ``abc_disable()``. Naming depends on the call site.

 - If the feature introduces ``SCR_EL3``, ``MDCR_EL3``, or ``CPTR_EL3`` controls
   for lower ELs, access must be enabled for at least NS world, preferably all
   worlds by setting/clearing the bit in the context copy of this register.

 - If the feature introduces any control bits in ``SCR_EL3``, ``MDCR_EL3``,
   ``CPTR_EL3``, or ``MPAM3_EL3``, those must be added to the appropriate list
   in ``include/services/arm_arch_svc.h`` to register with
   ``ARCH_FEATURE_AVAILABILITY``. This must also be done in the
   ``test_smccc_arch_feature_availability`` test in TFTF, otherwise CI will
   fail.

 - If the feature comes with registers, those should be context switched if
   enabling the feature for multiple worlds.

 - Consider whether the feature introduces any potential side channels and how
   to close them.

- Add documentation in ``docs/getting_started/build-options.rst``. This should
  include a brief description of what the enablement does, what values the flag
  can take, the default, and that it conforms to the ``ENABLE_FEAT`` mechanism.
  Adding feature enablement that does not conform to this mechanism is only done
  in exceptional cases.

- Enable the feature for the FVP platform in ``plat/arm/board/fvp/platform.mk``,
  setting it to ``2`` (``FEAT_STATE_CHECKED``).

- If the feature is optional, enable it in CI in the "allconfig" fragments.
  Those are currently ``run_config/fvp-aemv8a.all.bmcov``,
  ``run_config/fvp-spm.all.bmcov``, ``run_config/fvp-spm.all``, and
  ``run_config/fvp-aemv8a.all``. Mandatory features are expected to be covered
  by the architecture revision passed to the model.

- Optionally, although recommended, is to add a test in TFTF. This should only
  be done if there are observable effects from lower ELs like register traps. It
  is not necessary to add a test for functionality that does not have EL3
  involvement (eg. whether a new instruction that cannot trap works).

.. _Feature_description: https://developer.arm.com/documentation/109697/latest/
