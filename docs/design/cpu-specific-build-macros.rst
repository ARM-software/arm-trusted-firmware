Arm CPU Specific Build Macros
=============================

This document describes the various build options present in the CPU specific
operations framework to enable errata workarounds and to enable optimizations
for a specific CPU on a platform.

Security Vulnerability Workarounds
----------------------------------

TF-A exports a series of build flags which control which security
vulnerability workarounds should be applied at runtime.

-  ``WORKAROUND_CVE_2017_5715``: Enables the security workaround for
   `CVE-2017-5715`_. This flag can be set to 0 by the platform if none
   of the PEs in the system need the workaround. Setting this flag to 0 provides
   no performance benefit for non-affected platforms, it just helps to comply
   with the recommendation in the spec regarding workaround discovery.
   Defaults to 1.

-  ``WORKAROUND_CVE_2018_3639``: Enables the security workaround for
   `CVE-2018-3639`_. Defaults to 1. The TF-A project recommends to keep
   the default value of 1 even on platforms that are unaffected by
   CVE-2018-3639, in order to comply with the recommendation in the spec
   regarding workaround discovery.

-  ``DYNAMIC_WORKAROUND_CVE_2018_3639``: Enables dynamic mitigation for
   `CVE-2018-3639`_. This build option should be set to 1 if the target
   platform contains at least 1 CPU that requires dynamic mitigation.
   Defaults to 0.

-  ``WORKAROUND_CVE_2022_23960``: Enables mitigation for `CVE-2022-23960`_.
   This build option should be set to 1 if the target platform contains at
   least 1 CPU that requires this mitigation. Defaults to 1.

-  ``WORKAROUND_CVE_2024_5660``: Enables mitigation for `CVE-2024-5660`.
   The fix is to disable hardware page aggregation by setting CPUECTLR_EL1[46]
   in EL3 FW. This build option should be set to 1 if the target platform contains
   at least 1 CPU that requires this mitigation. Defaults to 1.

-  ``WORKAROUND_CVE_2024_7881``: Enables mitigation for `CVE-2024-7881`.
   This build option should be set to 1 if the target platform contains at
   least 1 CPU that requires this mitigation. Defaults to 1.

.. _arm_cpu_macros_errata_workarounds:

CPU Errata Workarounds
----------------------

TF-A exports a series of build flags which control the errata workarounds that
are applied to each CPU by the reset handler. The errata details can be found
in the CPU specific errata documents published by Arm:

-  `Cortex-A53 MPCore Software Developers Errata Notice`_
-  `Cortex-A57 MPCore Software Developers Errata Notice`_
-  `Cortex-A72 MPCore Software Developers Errata Notice`_

The errata workarounds are implemented for a particular revision or a set of
processor revisions. This is checked by the reset handler at runtime. Each
errata workaround is identified by its ``ID`` as specified in the processor's
errata notice document. The format of the define used to enable/disable the
errata workaround is ``ERRATA_<Processor name>_<ID>``, where the ``Processor name``
is for example ``A57`` for the ``Cortex_A57`` CPU.

Refer to :ref:`firmware_design_cpu_errata_implementation` for information on how to
write errata workaround functions.

All workarounds are disabled by default. The platform is responsible for
enabling these workarounds according to its requirement by defining the
errata workaround build flags in the platform specific makefile. In case
these workarounds are enabled for the wrong CPU revision then the errata
workaround is not applied. In the DEBUG build, this is indicated by
printing a warning to the crash console.

In the current implementation, a platform which has more than 1 variant
with different revisions of a processor has no runtime mechanism available
for it to specify which errata workarounds should be enabled or not.

The value of the build flags is 0 by default, that is, disabled. A value of 1
will enable it.

For Cortex-A9, the following errata build flags are defined :

-  ``ERRATA_A9_794073``: This applies errata 794073 workaround to Cortex-A9
   CPU. This needs to be enabled for all revisions of the CPU.

For Cortex-A15, the following errata build flags are defined :

-  ``ERRATA_A15_816470``: This applies errata 816470 workaround to Cortex-A15
   CPU. This needs to be enabled only for revision >= r3p0 of the CPU.

-  ``ERRATA_A15_827671``: This applies errata 827671 workaround to Cortex-A15
   CPU. This needs to be enabled only for revision >= r3p0 of the CPU.

For Cortex-A17, the following errata build flags are defined :

-  ``ERRATA_A17_852421``: This applies errata 852421 workaround to Cortex-A17
   CPU. This needs to be enabled only for revision <= r1p2 of the CPU.

-  ``ERRATA_A17_852423``: This applies errata 852423 workaround to Cortex-A17
   CPU. This needs to be enabled only for revision <= r1p2 of the CPU.

For Cortex-A35, the following errata build flags are defined :

-  ``ERRATA_A35_855472``: This applies errata 855472 workaround to Cortex-A35
   CPUs. This needs to be enabled only for revision r0p0 of Cortex-A35.

For Cortex-A53, the following errata build flags are defined :

-  ``ERRATA_A53_819472``: This applies errata 819472 workaround to all
   CPUs. This needs to be enabled only for revision <= r0p1 of Cortex-A53.

-  ``ERRATA_A53_824069``: This applies errata 824069 workaround to all
   CPUs. This needs to be enabled only for revision <= r0p2 of Cortex-A53.

-  ``ERRATA_A53_826319``: This applies errata 826319 workaround to Cortex-A53
   CPU. This needs to be enabled only for revision <= r0p2 of the CPU.

-  ``ERRATA_A53_827319``: This applies errata 827319 workaround to all
   CPUs. This needs to be enabled only for revision <= r0p2 of Cortex-A53.

-  ``ERRATA_A53_835769``: This applies erratum 835769 workaround at compile and
   link time to Cortex-A53 CPU. This needs to be enabled for some variants of
   revision <= r0p4. This workaround can lead the linker to create ``*.stub``
   sections.

-  ``ERRATA_A53_836870``: This applies errata 836870 workaround to Cortex-A53
   CPU. This needs to be enabled only for revision <= r0p3 of the CPU. From
   r0p4 and onwards, this errata is enabled by default in hardware. Identical to
   ``A53_DISABLE_NON_TEMPORAL_HINT``.

-  ``ERRATA_A53_843419``: This applies erratum 843419 workaround at link time
   to Cortex-A53 CPU.  This needs to be enabled for some variants of revision
   <= r0p4. This workaround can lead the linker to emit ``*.stub`` sections
   which are 4kB aligned.

-  ``ERRATA_A53_855873``: This applies errata 855873 workaround to Cortex-A53
   CPUs. Though the erratum is present in every revision of the CPU,
   this workaround is only applied to CPUs from r0p3 onwards, which feature
   a chicken bit in CPUACTLR_EL1 to enable a hardware workaround.
   Earlier revisions of the CPU have other errata which require the same
   workaround in software, so they should be covered anyway.

-  ``ERRATA_A53_1530924``: This applies errata 1530924 workaround to all
   revisions of Cortex-A53 CPU.

For Cortex-A55, the following errata build flags are defined :

-  ``ERRATA_A55_768277``: This applies errata 768277 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A55_778703``: This applies errata 778703 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A55_798797``: This applies errata 798797 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A55_846532``: This applies errata 846532 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision <= r0p1 of the CPU.

-  ``ERRATA_A55_903758``: This applies errata 903758 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision <= r0p1 of the CPU.

-  ``ERRATA_A55_1221012``: This applies errata 1221012 workaround to Cortex-A55
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

-  ``ERRATA_A55_1530923``: This applies errata 1530923 workaround to all
   revisions of Cortex-A55 CPU.

For Cortex-A57, the following errata build flags are defined :

-  ``ERRATA_A57_806969``: This applies errata 806969 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_813419``: This applies errata 813419 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_813420``: This applies errata 813420 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_814670``: This applies errata 814670 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_817169``: This applies errata 817169 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r0p1 of the CPU.

-  ``ERRATA_A57_826974``: This applies errata 826974 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A57_826977``: This applies errata 826977 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A57_828024``: This applies errata 828024 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A57_829520``: This applies errata 829520 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p2 of the CPU.

-  ``ERRATA_A57_833471``: This applies errata 833471 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p2 of the CPU.

-  ``ERRATA_A57_859972``: This applies errata 859972 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision <= r1p3 of the CPU.

-  ``ERRATA_A57_1319537``: This applies errata 1319537 workaround to all
   revisions of Cortex-A57 CPU.

For Cortex-A72, the following errata build flags are defined :

-  ``ERRATA_A72_859971``: This applies errata 859971 workaround to Cortex-A72
   CPU. This needs to be enabled only for revision <= r0p3 of the CPU.

-  ``ERRATA_A72_1319367``: This applies errata 1319367 workaround to all
   revisions of Cortex-A72 CPU.

For Cortex-A73, the following errata build flags are defined :

-  ``ERRATA_A73_852427``: This applies errata 852427 workaround to Cortex-A73
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A73_855423``: This applies errata 855423 workaround to Cortex-A73
   CPU. This needs to be enabled only for revision <= r0p1 of the CPU.

For Cortex-A75, the following errata build flags are defined :

-  ``ERRATA_A75_764081``: This applies errata 764081 workaround to Cortex-A75
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A75_790748``: This applies errata 790748 workaround to Cortex-A75
    CPU. This needs to be enabled only for revision r0p0 of the CPU.

For Cortex-A76, the following errata build flags are defined :

-  ``ERRATA_A76_1073348``: This applies errata 1073348 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

-  ``ERRATA_A76_1130799``: This applies errata 1130799 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_A76_1220197``: This applies errata 1220197 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_A76_1257314``: This applies errata 1257314 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_A76_1262606``: This applies errata 1262606 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_A76_1262888``: This applies errata 1262888 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_A76_1275112``: This applies errata 1275112 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_A76_1791580``: This applies errata 1791580 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r4p0 of the CPU.

-  ``ERRATA_A76_1165522``: This applies errata 1165522 workaround to all
   revisions of Cortex-A76 CPU. This errata is fixed in r3p0 but due to
   limitation of errata framework this errata is applied to all revisions
   of Cortex-A76 CPU.

-  ``ERRATA_A76_1868343``: This applies errata 1868343 workaround to Cortex-A76
   CPU. This needs to be enabled only for revision <= r4p0 of the CPU.

-  ``ERRATA_A76_1946160``: This applies errata 1946160 workaround to Cortex-A76
   CPU. This needs to be enabled only for revisions r3p0 - r4p1 of the CPU.

-  ``ERRATA_A76_2743102``: This applies errata 2743102 workaround to Cortex-A76
   CPU. This needs to be enabled for all revisions <= r4p1 of the CPU and is
   still open.

For Cortex-A77, the following errata build flags are defined :

-  ``ERRATA_A77_1508412``: This applies errata 1508412 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

-  ``ERRATA_A77_1925769``: This applies errata 1925769 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A77_1946167``: This applies errata 1946167 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A77_1791578``: This applies errata 1791578 workaround to Cortex-A77
   CPU. This needs to be enabled for r0p0, r1p0, and r1p1, it is still open.

-  ``ERRATA_A77_2356587``: This applies errata 2356587 workaround to Cortex-A77
   CPU. This needs to be enabled for r0p0, r1p0, and r1p1, it is still open.

 -  ``ERRATA_A77_1800714``: This applies errata 1800714 workaround to Cortex-A77
    CPU. This needs to be enabled for revisions <= r1p1 of the CPU.

 -  ``ERRATA_A77_2743100``: This applies errata 2743100 workaround to Cortex-A77
    CPU. This needs to be enabled for r0p0, r1p0, and r1p1, it is still open.

For Cortex-A78, the following errata build flags are defined :

-  ``ERRATA_A78_1688305``: This applies errata 1688305 workaround to Cortex-A78
   CPU. This needs to be enabled only for revision r0p0 - r1p0 of the CPU.

-  ``ERRATA_A78_1941498``: This applies errata 1941498 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r1p1 of the CPU.

-  ``ERRATA_A78_1951500``: This applies errata 1951500 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r1p0 and r1p1, r0p0 has the same
   issue but there is no workaround for that revision.

-  ``ERRATA_A78_1821534``: This applies errata 1821534 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0 and r1p0.

-  ``ERRATA_A78_1952683``: This applies errata 1952683 workaround to Cortex-A78
   CPU. This needs to be enabled for revision r0p0, it is fixed in r1p0.

-  ``ERRATA_A78_2132060``: This applies errata 2132060 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, and r1p2. It
   is still open.

-  ``ERRATA_A78_2242635``: This applies errata 2242635 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r1p0, r1p1, and r1p2. The issue
   is present in r0p0 but there is no workaround. It is still open.

-  ``ERRATA_A78_2376745``: This applies errata 2376745 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, and r1p2, and
   it is still open.

-  ``ERRATA_A78_2395406``: This applies errata 2395406 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, and r1p2, and
   it is still open.

- ``ERRATA_A78_2712571``: This applies erratum 2712571 workaround to Cortex-A78
   CPU, this erratum affects system configurations that do not use an ARM
   interconnect IP. This needs to be enabled for revisions r0p0, r1p0, r1p1
   and r1p2 and it is still open.

-  ``ERRATA_A78_2742426``: This applies erratum 2742426 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1 and r1p2 and
   it is still open.

-  ``ERRATA_A78_2772019``: This applies errata 2772019 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, and r1p2, and
   it is still open.

-  ``ERRATA_A78_2779479``: This applies erratum 2779479 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1 and r1p2 and
   it is still open.

For Cortex-A78AE, the following errata build flags are defined :

- ``ERRATA_A78_AE_1941500`` : This applies errata 1941500 workaround to
   Cortex-A78AE CPU. This needs to be enabled for revisions r0p0 and r0p1.
   This erratum is still open.

- ``ERRATA_A78_AE_1951502`` : This applies errata 1951502 workaround to
  Cortex-A78AE CPU. This needs to be enabled for revisions r0p0 and r0p1. This
  erratum is still open.

- ``ERRATA_A78_AE_2376748`` : This applies errata 2376748 workaround to
  Cortex-A78AE CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2.
  This erratum is still open.

- ``ERRATA_A78_AE_2395408`` : This applies errata 2395408 workaround to
  Cortex-A78AE CPU. This needs to be enabled for revisions r0p0 and r0p1. This
  erratum is still open.

- ``ERRATA_A78_AE_2712574`` : This applies erratum 2712574 workaround to
  Cortex-A78AE CPU. This erratum affects system configurations that do not use
  an ARM interconnect IP. This needs to be enabled for revisions r0p0, r0p1 and
  r0p2. This erratum is still open.

For Cortex-A78C, the following errata build flags are defined :

- ``ERRATA_A78C_1827430`` : This applies errata 1827430 workaround to
  Cortex-A78C CPU. This needs to be enabled for revision r0p0. The erratum is
  fixed in r0p1.

- ``ERRATA_A78C_1827440`` : This applies errata 1827440 workaround to
  Cortex-A78C CPU. This needs to be enabled for revision r0p0. The erratum is
  fixed in r0p1.

- ``ERRATA_A78C_2132064`` : This applies errata 2132064 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1, r0p2 and
  it is still open.

- ``ERRATA_A78C_2242638`` : This applies errata 2242638 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1, r0p2 and
  it is still open.

- ``ERRATA_A78C_2376749`` : This applies errata 2376749 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1 and r0p2. This
  erratum is still open.

- ``ERRATA_A78C_2395411`` : This applies errata 2395411 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1 and r0p2. This
  erratum is still open.

- ``ERRATA_A78C_2683027`` : This applies errata 2683027 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1 and r0p2. This
  erratum is still open.

- ``ERRATA_A78C_2712575`` : This applies erratum 2712575 workaround to
  Cortex-A78C CPU, this erratum affects system configurations that do not use
  an ARM interconnect IP. This needs to be enabled for revisions r0p1 and r0p2
  and is still open.

- ``ERRATA_A78C_2743232`` : This applies erratum 2743232 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1 and r0p2.
  This erratum is still open.

- ``ERRATA_A78C_2772121`` : This applies errata 2772121 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2.
  This erratum is still open.

- ``ERRATA_A78C_2779484`` : This applies errata 2779484 workaround to
  Cortex-A78C CPU. This needs to be enabled for revisions r0p1 and r0p2.
  This erratum is still open.

For Cortex-X1 CPU, the following errata build flags are defined:

- ``ERRATA_X1_1821534`` : This applies errata 1821534 workaround to Cortex-X1
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

- ``ERRATA_X1_1688305`` : This applies errata 1688305 workaround to Cortex-X1
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

- ``ERRATA_X1_1827429`` : This applies errata 1827429 workaround to Cortex-X1
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

For Neoverse N1, the following errata build flags are defined :

-  ``ERRATA_N1_1073348``: This applies errata 1073348 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision r0p0 and r1p0 of the CPU.

-  ``ERRATA_N1_1130799``: This applies errata 1130799 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_N1_1165347``: This applies errata 1165347 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_N1_1207823``: This applies errata 1207823 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_N1_1220197``: This applies errata 1220197 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r2p0 of the CPU.

-  ``ERRATA_N1_1257314``: This applies errata 1257314 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_N1_1262606``: This applies errata 1262606 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_N1_1262888``: This applies errata 1262888 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_N1_1275112``: This applies errata 1275112 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_N1_1315703``: This applies errata 1315703 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r3p0 of the CPU.

-  ``ERRATA_N1_1542419``: This applies errata 1542419 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revisions r3p0 - r4p0 of the CPU.

-  ``ERRATA_N1_1868343``: This applies errata 1868343 workaround to Neoverse-N1
   CPU. This needs to be enabled only for revision <= r4p0 of the CPU.

-  ``ERRATA_N1_1946160``: This applies errata 1946160 workaround to Neoverse-N1
   CPU. This needs to be enabled for revisions r3p0, r3p1, r4p0, and r4p1, for
   revisions r0p0, r1p0, and r2p0 there is no workaround.

-  ``ERRATA_N1_2743102``: This applies errata 2743102 workaround to Neoverse-N1
   CPU. This needs to be enabled for all revisions <= r4p1 of the CPU and is
   still open.

For Neoverse V1, the following errata build flags are defined :

-  ``ERRATA_V1_1618635``: This applies errata 1618635 workaround to Neoverse-V1
   CPU. This needs to be enabled for revision r0p0 of the CPU, it is fixed in
   r1p0.

-  ``ERRATA_V1_1774420``: This applies errata 1774420 workaround to Neoverse-V1
   CPU. This needs to be enabled only for revisions r0p0 and r1p0, it is fixed
   in r1p1.

-  ``ERRATA_V1_1791573``: This applies errata 1791573 workaround to Neoverse-V1
   CPU. This needs to be enabled only for revisions r0p0 and r1p0, it is fixed
   in r1p1.

-  ``ERRATA_V1_1852267``: This applies errata 1852267 workaround to Neoverse-V1
   CPU. This needs to be enabled only for revisions r0p0 and r1p0, it is fixed
   in r1p1.

-  ``ERRATA_V1_1925756``: This applies errata 1925756 workaround to Neoverse-V1
   CPU. This needs to be enabled for r0p0, r1p0, and r1p1, it is still open.

-  ``ERRATA_V1_1940577``: This applies errata 1940577 workaround to Neoverse-V1
   CPU. This needs to be enabled only for revision r1p0 and r1p1 of the
   CPU.

-  ``ERRATA_V1_1966096``: This applies errata 1966096 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r1p0 and r1p1 of the CPU, the
   issue is present in r0p0 as well but there is no workaround for that
   revision.  It is still open.

-  ``ERRATA_V1_2139242``: This applies errata 2139242 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r1p1 of the
   CPU.  It is still open.

-  ``ERRATA_V1_2108267``: This applies errata 2108267 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r1p1 of the CPU.
   It is still open.

-  ``ERRATA_V1_2216392``: This applies errata 2216392 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r1p0 and r1p1 of the CPU, the
   issue is present in r0p0 as well but there is no workaround for that
   revision.  It is still open.

-  ``ERRATA_V1_2294912``: This applies errata 2294912 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r1p1 and r1p2 of
   the CPU.

-  ``ERRATA_V1_2348377``: This applies errata 2348377 workaroud to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0 and r1p1 of the CPU.
   It has been fixed in r1p2.

-  ``ERRATA_V1_2372203``: This applies errata 2372203 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0 and r1p1 of the CPU.
   It is still open.

- ``ERRATA_V1_2701953``: This applies erratum 2701953 workaround to Neoverse-V1
   CPU, this erratum affects system configurations that do not use an ARM
   interconnect IP. This needs to be enabled for revisions r0p0, r1p0 and r1p1.
   It has been fixed in r1p2.

-  ``ERRATA_V1_2743093``: This applies errata 2743093 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1 and r1p2 of the
   CPU. It is still open.

-  ``ERRATA_V1_2743233``: This applies erratum 2743233 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, and r1p2 of the
   CPU. It is still open.

-  ``ERRATA_V1_2779461``: This applies erratum 2779461 workaround to Neoverse-V1
   CPU. This needs to be enabled for revisions r0p0, r1p0, r1p1, r1p2 of the
   CPU. It is still open.

For Neoverse V2, the following errata build flags are defined :

-  ``ERRATA_V2_2331132``: This applies errata 2331132 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2. It is still
   open.

-  ``ERRATA_V2_2618597``: This applies errata 2618597 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
   r0p2.

-  ``ERRATA_V2_2662553``: This applies errata 2662553 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
   r0p2.

-  ``ERRATA_V2_2719103``: This applies errata 2719103 workaround to Neoverse-V2
   CPU, this affects system configurations that do not use and ARM interconnect
   IP. This needs to be enabled for revisions r0p0 and r0p1. It has been fixed
   in r0p2.

-  ``ERRATA_V2_2719105``: This applies errata 2719105 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
   r0p2.

-  ``ERRATA_V2_2743011``: This applies errata 2743011 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
   r0p2.

-  ``ERRATA_V2_2779510``: This applies errata 2779510 workaround to Neoverse-V2
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
   r0p2.

-  ``ERRATA_V2_2801372``: This applies errata 2801372 workaround to Neoverse-V2
   CPU, this affects all configurations. This needs to be enabled for revisions
   r0p0 and r0p1. It has been fixed in r0p2.

For Neoverse V3, the following errata build flags are defined :

- ``ERRATA_V3_3701767``: This applies errata 3701767 workaround to Neoverse-V3
  CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2 of the CPU and
  is still open.

For Cortex-A710, the following errata build flags are defined :

-  ``ERRATA_A710_1987031``: This applies errata 1987031 workaround to
   Cortex-A710 CPU. This needs to be enabled only for revisions r0p0, r1p0 and
   r2p0 of the CPU. It is still open.

-  ``ERRATA_A710_2081180``: This applies errata 2081180 workaround to
   Cortex-A710 CPU. This needs to be enabled only for revisions r0p0, r1p0 and
   r2p0 of the CPU. It is still open.

-  ``ERRATA_A710_2055002``: This applies errata 2055002 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r1p0, r2p0 of the CPU
   and is still open.

-  ``ERRATA_A710_2017096``: This applies errata 2017096 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is still open.

-  ``ERRATA_A710_2083908``: This applies errata 2083908 workaround to
   Cortex-A710 CPU. This needs to be enabled for revision r2p0 of the CPU and
   is still open.

-  ``ERRATA_A710_2058056``: This applies errata 2058056 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   and r2p1 of the CPU and is still open.

-  ``ERRATA_A710_2267065``: This applies errata 2267065 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2136059``: This applies errata 2136059 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2147715``: This applies errata 2147715 workaround to
   Cortex-A710 CPU. This needs to be enabled for revision r2p0 of the CPU
   and is fixed in r2p1.

-  ``ERRATA_A710_2216384``: This applies errata 2216384 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2282622``: This applies errata 2282622 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and
   r2p1 of the CPU and is still open.

- ``ERRATA_A710_2291219``: This applies errata 2291219 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2008768``: This applies errata 2008768 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2371105``: This applies errata 2371105 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0
   of the CPU and is fixed in r2p1.

-  ``ERRATA_A710_2701952``: This applies erratum 2701952 workaround to Cortex-A710
   CPU, and applies to system configurations that do not use and ARM
   interconnect IP. This needs to be enabled for r0p0, r1p0, r2p0 and r2p1 and
   is still open.

-  ``ERRATA_A710_2742423``: This applies errata 2742423 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and
   r2p1 of the CPU and is still open.

-  ``ERRATA_A710_2768515``: This applies errata 2768515 workaround to
   Cortex-A710 CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and
   r2p1 of the CPU and is still open.

-  ``ERRATA_A710_2778471``: This applies errata 2778471 workaround to Cortex-A710
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and is still open.

- ``ERRATA_A710_3701772``: This applies errata 3701772 workaround to Cortex-A710
  CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0, r2p1 of the
  CPU and is still open.

For Neoverse N2, the following errata build flags are defined :

-  ``ERRATA_N2_2002655``: This applies errata 2002655 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2009478``: This applies errata 2009478 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2067956``: This applies errata 2067956 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2025414``: This applies errata 2025414 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2189731``: This applies errata 2189731 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2138956``: This applies errata 2138956 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2138953``: This applies errata 2138953 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2, r0p3 and is still open.

-  ``ERRATA_N2_2242415``: This applies errata 2242415 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2138958``: This applies errata 2138958 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2242400``: This applies errata 2242400 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2280757``: This applies errata 2280757 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU and is fixed in r0p1.

-  ``ERRATA_N2_2326639``: This applies errata 2326639 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU, it is fixed in
   r0p1.

-  ``ERRATA_N2_2340933``: This applies errata 2340933 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU, it is fixed in
   r0p1.

-  ``ERRATA_N2_2346952``: This applies errata 2346952 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2 of the CPU,
   it is fixed in r0p3.

-  ``ERRATA_N2_2376738``: This applies errata 2376738 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0, r0p1, r0p2, r0p3 and is still open.

-  ``ERRATA_N2_2388450``: This applies errata 2388450 workaround to Neoverse-N2
   CPU. This needs to be enabled for revision r0p0 of the CPU, it is fixed in
   r0p1.

-  ``ERRATA_N2_2743014``: This applies errata 2743014 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2. It is fixed
   in r0p3.

-  ``ERRATA_N2_2743089``: This applies errata 2743089 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2. It is fixed
   in r0p3.

- ``ERRATA_N2_2728475``: This applies erratum 2728475 workaround to Neoverse-N2
   CPU, this erratum affects system configurations that do not use and ARM
   interconnect IP. This needs to be enabled for revisions r0p0, r0p1 and r0p2.
   It is fixed in r0p3.

-  ``ERRATA_N2_2779511``: This applies errata 2779511 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1 and r0p2. It is fixed
   in r0p3.

-  ``ERRATA_N2_3701773``: This applies errata 3701773 workaround to Neoverse-N2
   CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2, r0p3 and is
   still open.

For Neoverse N3, the following errata build flags are defined :

-  ``ERRATA_N3_3699563``: This applies errata 3699563 workaround to Neoverse-N3
   CPU. This needs to be enabled for revisions r0p0 and is still open.

For Cortex-X2, the following errata build flags are defined :

-  ``ERRATA_X2_2002765``: This applies errata 2002765 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r2p0 of the CPU,
   it is still open.

-  ``ERRATA_X2_2058056``: This applies errata 2058056 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the CPU,
   it is still open.

-  ``ERRATA_X2_2083908``: This applies errata 2083908 workaround to Cortex-X2
   CPU. This needs to be enabled for revision r2p0 of the CPU, it is still open.

-  ``ERRATA_X2_2017096``: This applies errata 2017096 workaround to Cortex-X2
   CPU. This needs to be enabled only for revisions r0p0, r1p0 and r2p0 of the
   CPU, it is fixed in r2p1.

-  ``ERRATA_X2_2081180``: This applies errata 2081180 workaround to Cortex-X2
   CPU. This needs to be enabled only for revisions r0p0, r1p0 and r2p0 of the
   CPU, it is fixed in r2p1.

-  ``ERRATA_X2_2216384``: This applies errata 2216384 workaround to Cortex-X2
   CPU. This needs to be enabled only for revisions r0p0, r1p0 and r2p0 of the
   CPU, it is fixed in r2p1.

-  ``ERRATA_X2_2147715``: This applies errata 2147715 workaround to Cortex-X2
   CPU. This needs to be enabled only for revision r2p0 of the CPU, it is fixed
   in r2p1.

-  ``ERRATA_X2_2282622``: This applies errata 2282622 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and is still open.

-  ``ERRATA_X2_2371105``: This applies errata 2371105 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0 and r2p0 of the CPU
   and is fixed in r2p1.

- ``ERRATA_X2_2701952``: This applies erratum 2701952 workaround to Cortex-X2
   CPU and affects system configurations that do not use an ARM interconnect IP.
   This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 and is
   still open.

-  ``ERRATA_X2_2742423``: This applies errata 2742423 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and is still open.

-  ``ERRATA_X2_2768515``: This applies errata 2768515 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and is still open.

-  ``ERRATA_X2_2778471``: This applies errata 2778471 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and it is still open.

-  ``ERRATA_X2_3701772``: This applies errata 3701772 workaround to Cortex-X2
   CPU. This needs to be enabled for revisions r0p0, r1p0, r2p0 and r2p1 of the
   CPU and it is still open.

For Cortex-X3, the following errata build flags are defined :

- ``ERRATA_X3_2070301``: This applies errata 2070301 workaround to the Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0, r1p1 and r1p2 of
  the CPU and is still open.

- ``ERRATA_X3_2266875``: This applies errata 2266875 workaround to the Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0 and r1p0 of the CPU, it
  is fixed in r1p1.

- ``ERRATA_X3_2302506``: This applies errata 2302506 workaround to the Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0 and r1p1, it is
  fixed in r1p2.

- ``ERRATA_X3_2313909``: This applies errata 2313909 workaround to
  Cortex-X3 CPU. This needs to be enabled only for revisions r0p0 and r1p0
  of the CPU, it is fixed in r1p1.

- ``ERRATA_X3_2372204``: This applies errata 2372204 workaround to
  Cortex-X3 CPU. This needs to be enabled only for revisions r0p0 and r1p0
  of the CPU, it is fixed in r1p1.

- ``ERRATA_X3_2615812``: This applies errata 2615812 workaround to Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0 and r1p1 of the
  CPU, it is fixed in r1p2.

- ``ERRATA_X3_2641945``: This applies errata 2641945 workaround to Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0 and r1p0 of the CPU.
  It is fixed in r1p1.

- ``ERRATA_X3_2701951``: This applies erratum 2701951 workaround to Cortex-X3
  CPU and affects system configurations that do not use an ARM interconnect
  IP. This needs to be applied to revisions r0p0, r1p0 and r1p1. It is fixed
  in r1p2.

- ``ERRATA_X3_2742421``: This applies errata 2742421 workaround to
  Cortex-X3 CPU. This needs to be enabled only for revisions r0p0, r1p0 and
  r1p1. It is fixed in r1p2.

- ``ERRATA_X3_2743088``: This applies errata 2743088 workaround to Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0 and r1p1. It is
  fixed in r1p2.

- ``ERRATA_X3_2779509``: This applies errata 2779509 workaround to Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0 and r1p1 of the
  CPU. It is fixed in r1p2.

- ``ERRATA_X3_3701769``: This applies errata 3701769 workaround to Cortex-X3
  CPU. This needs to be enabled only for revisions r0p0, r1p0, r1p1 and r1p2
  of the CPU and it is still open.

For Cortex-X4, the following errata build flags are defined :

- ``ERRATA_X4_2701112``: This applies erratum 2701112 workaround to Cortex-X4
  CPU and affects system configurations that do not use an Arm interconnect IP.
  This needs to be enabled for revisions r0p0 and is fixed in r0p1.
  The workaround for this erratum is not implemented in EL3, but the flag can
  be enabled/disabled at the platform level. The flag is used when the errata ABI
  feature is enabled and can assist the Kernel in the process of
  mitigation of the erratum.

- ``ERRATA_X4_2726228``: This applies erratum 2726228 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in
  r0p2.

-  ``ERRATA_X4_2740089``: This applies errata 2740089 workaround to Cortex-X4
   CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed
   in r0p2.

- ``ERRATA_X4_2763018``: This applies errata 2763018 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in r0p2.

- ``ERRATA_X4_2816013``: This applies errata 2816013 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in r0p2.

- ``ERRATA_X4_2897503``: This applies errata 2897503 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in r0p2.

- ``ERRATA_X4_2923985``: This applies errata 2923985 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in r0p2.

- ``ERRATA_X4_3076789``: This applies errata 3076789 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is fixed in r0p2.

- ``ERRATA_X4_3701758``: This applies errata 3701758 workaround to Cortex-X4
  CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2 and r0p3.
  It is still open.

For Cortex-X925, the following errata build flags are defined :

- ``ERRATA_X925_3701747``: This applies errata 3701747 workaround to Cortex-X925
  CPU. This needs to be enabled for revisions r0p0 and r0p1. It is still open.

For Cortex-A510, the following errata build flags are defined :

-  ``ERRATA_A510_1922240``: This applies errata 1922240 workaround to
   Cortex-A510 CPU. This needs to be enabled only for revision r0p0, it is
   fixed in r0p1.

-  ``ERRATA_A510_2288014``: This applies errata 2288014 workaround to
   Cortex-A510 CPU. This needs to be enabled only for revisions r0p0, r0p1,
   r0p2, r0p3 and r1p0, it is fixed in r1p1.

-  ``ERRATA_A510_2042739``: This applies errata 2042739 workaround to
   Cortex-A510 CPU. This needs to be enabled only for revisions r0p0, r0p1 and
   r0p2, it is fixed in r0p3.

-  ``ERRATA_A510_2041909``: This applies errata 2041909 workaround to
   Cortex-A510 CPU. This needs to be enabled only for revision r0p2 and is fixed
   in r0p3. The issue is also present in r0p0 and r0p1 but there is no
   workaround for those revisions.

-  ``ERRATA_A510_2080326``: This applies errata 2080326 workaround to
   Cortex-A510 CPU. This needs to be enabled only for revision r0p2 and is
   fixed in r0p3. This issue is also present in r0p0 and r0p1 but there is no
   workaround for those revisions.

-  ``ERRATA_A510_2250311``: This applies errata 2250311 workaround to
   Cortex-A510 CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2,
   r0p3 and r1p0, it is fixed in r1p1. This workaround disables MPMM even if
   ENABLE_MPMM=1.

-  ``ERRATA_A510_2218950``: This applies errata 2218950 workaround to
   Cortex-A510 CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2,
   r0p3 and r1p0, it is fixed in r1p1.

-  ``ERRATA_A510_2172148``: This applies errata 2172148 workaround to
   Cortex-A510 CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2,
   r0p3 and r1p0, it is fixed in r1p1.

-  ``ERRATA_A510_2347730``: This applies errata 2347730 workaround to
   Cortex-A510 CPU. This needs to be enabled for revisions r0p0, r0p1, r0p2,
   r0p3, r1p0 and r1p1. It is fixed in r1p2.

-  ``ERRATA_A510_2371937``: This applies errata 2371937 workaround to
   Cortex-A510 CPU. This needs to applied for revisions r0p0, r0p1, r0p2,
   r0p3, r1p0, r1p1, and is fixed in r1p2.

-  ``ERRATA_A510_2666669``: This applies errata 2666669 workaround to
   Cortex-A510 CPU. This needs to applied for revisions r0p0, r0p1, r0p2,
   r0p3, r1p0, r1p1. It is fixed in r1p2.

-  ``ERRATA_A510_2684597``: This applies erratum 2684597 workaround to
   Cortex-A510 CPU. This needs to be applied to revision r0p0, r0p1, r0p2,
   r0p3, r1p0, r1p1 and r1p2. It is fixed in r1p3.

For Cortex-A520, the following errata build flags are defined :

-  ``ERRATA_A520_2630792``: This applies errata 2630792 workaround to
   Cortex-A520 CPU. This needs to applied for revisions r0p0, r0p1 of the
   CPU and is still open.

-  ``ERRATA_A520_2858100``: This applies errata 2858100 workaround to
   Cortex-A520 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is still open.

-  ``ERRATA_A520_2938996``: This applies errata 2938996 workaround to
   Cortex-A520 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

For Cortex-A715, the following errata build flags are defined :

-  ``ERRATA_A715_2331818``: This applies errata 2331818 workaround to
   Cortex-A715 CPU. This needs to be enabled for revisions r0p0 and r1p0.
   It is fixed in r1p1.

- ``ERRATA_A715_2344187``: This applies errata 2344187 workaround to
   Cortex-A715 CPU. This needs to be enabled for revisions r0p0 and r1p0. It is
   fixed in r1p1.

-  ``ERRATA_A715_2413290``: This applies errata 2413290 workaround to
   Cortex-A715 CPU. This needs to be enabled only for revision r1p0 and
   when SPE(Statistical profiling extension)=True. The errata is fixed
   in r1p1.

-  ``ERRATA_A715_2420947``: This applies errata 2420947 workaround to
   Cortex-A715 CPU. This needs to be enabled only for revision r1p0.
   It is fixed in r1p1.

-  ``ERRATA_A715_2429384``: This applies errata 2429384 workaround to
   Cortex-A715 CPU. This needs to be enabled for revision r1p0. There is no
   workaround for revision r0p0. It is fixed in r1p1.

-  ``ERRATA_A715_2561034``: This applies errata 2561034 workaround to
   Cortex-A715 CPU. This needs to be enabled only for revision r1p0.
   It is fixed in r1p1.

-  ``ERRATA_A715_2728106``: This applies errata 2728106 workaround to
   Cortex-A715 CPU. This needs to be enabled for revisions r0p0, r1p0
   and r1p1. It is fixed in r1p2.

-  ``ERRATA_A715_3699560``: This applies errata 3699560 workaround to
   Cortex-A715 CPU. This needs to be enabled for revisions r0p0, r1p0,
   r1p2, r1p3. It is still open.

For Cortex-A720, the following errata build flags are defined :

-  ``ERRATA_A720_2792132``: This applies errata 2792132 workaround to
   Cortex-A720 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

-  ``ERRATA_A720_2844092``: This applies errata 2844092 workaround to
   Cortex-A720 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

-  ``ERRATA_A720_2926083``: This applies errata 2926083 workaround to
   Cortex-A720 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

-  ``ERRATA_A720_2940794``: This applies errata 2940794 workaround to
   Cortex-A720 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

-  ``ERRATA_A720_3699561``: This applies errata 3699561 workaround to
   Cortex-A720 CPU. This needs to be enabled for revisions r0p0, r0p1
   and r0p2. It is still open.

For Cortex-A725, the following errata build flags are defined :

-  ``ERRATA_A725_3699564``: This applies errata 3699564 workaround to
   Cortex-A725 CPU. This needs to be enabled for revisions r0p0 and r0p1.
   It is fixed in r0p2.

DSU Errata Workarounds
----------------------

Similar to CPU errata, TF-A also implements workarounds for DSU (DynamIQ
Shared Unit) errata. The DSU errata details can be found in the respective Arm
documentation:

- `Arm DSU Software Developers Errata Notice`_.

Each erratum is identified by an ``ID``, as defined in the DSU errata notice
document. Thus, the build flags which enable/disable the errata workarounds
have the format ``ERRATA_DSU_<ID>``. The implementation and application logic
of DSU errata workarounds are similar to `CPU errata workarounds`_.

For DSU errata, the following build flags are defined:

-  ``ERRATA_DSU_798953``: This applies errata 798953 workaround for the
   affected DSU configurations. This errata applies only for those DSUs that
   revision is r0p0 (on r0p1 it is fixed). However, please note that this
   workaround results in increased DSU power consumption on idle.

-  ``ERRATA_DSU_936184``: This applies errata 936184 workaround for the
   affected DSU configurations. This errata applies only for those DSUs that
   contain the ACP interface **and** the DSU revision is older than r2p0 (on
   r2p0 it is fixed). However, please note that this workaround results in
   increased DSU power consumption on idle.

-  ``ERRATA_DSU_2313941``: This applies errata 2313941 workaround for the
   affected DSU configurations. This errata applies for those DSUs with
   revisions r0p0, r1p0, r2p0, r2p1, r3p0, r3p1 and is still open. However,
   please note that this workaround results in increased DSU power consumption
   on idle.

CPU Specific optimizations
--------------------------

This section describes some of the optimizations allowed by the CPU micro
architecture that can be enabled by the platform as desired.

-  ``SKIP_A57_L1_FLUSH_PWR_DWN``: This flag enables an optimization in the
   Cortex-A57 cluster power down sequence by not flushing the Level 1 data
   cache. The L1 data cache and the L2 unified cache are inclusive. A flush
   of the L2 by set/way flushes any dirty lines from the L1 as well. This
   is a known safe deviation from the Cortex-A57 TRM defined power down
   sequence. Each Cortex-A57 based platform must make its own decision on
   whether to use the optimization.

-  ``A53_DISABLE_NON_TEMPORAL_HINT``: This flag disables the cache non-temporal
   hint. The LDNP/STNP instructions as implemented on Cortex-A53 do not behave
   in a way most programmers expect, and will most probably result in a
   significant speed degradation to any code that employs them. The Armv8-A
   architecture (see Arm DDI 0487A.h, section D3.4.3) allows cores to ignore
   the non-temporal hint and treat LDNP/STNP as LDP/STP instead. Enabling this
   flag enforces this behaviour. This needs to be enabled only for revisions
   <= r0p3 of the CPU and is enabled by default.

-  ``A57_DISABLE_NON_TEMPORAL_HINT``: This flag has the same behaviour as
   ``A53_DISABLE_NON_TEMPORAL_HINT`` but for Cortex-A57. This needs to be
   enabled only for revisions <= r1p2 of the CPU and is enabled by default,
   as recommended in section "4.7 Non-Temporal Loads/Stores" of the
   `Cortex-A57 Software Optimization Guide`_.

- ''A57_ENABLE_NON_CACHEABLE_LOAD_FWD'': This flag enables non-cacheable
   streaming enhancement feature for Cortex-A57 CPUs. Platforms can set
   this bit only if their memory system meets the requirement that cache
   line fill requests from the Cortex-A57 processor are atomic. Each
   Cortex-A57 based platform must make its own decision on whether to use
   the optimization. This flag is disabled by default.

-  ``NEOVERSE_Nx_EXTERNAL_LLC``: This flag indicates that an external last
   level cache(LLC) is present in the system, and that the DataSource field
   on the master CHI interface indicates when data is returned from the LLC.
   This is used to control how the LL_CACHE* PMU events count.
   Default value is 0 (Disabled).

GIC Errata Workarounds
----------------------
-  ``GIC600_ERRATA_WA_2384374``: This flag applies part 2 of errata 2384374
   workaround for the affected GIC600 and GIC600-AE implementations. It applies
   to implementations of GIC600 and GIC600-AE with revisions less than or equal
   to r1p6 and r0p2 respectively. If the platform sets GICV3_SUPPORT_GIC600,
   then this flag is enabled; otherwise, it is 0 (Disabled).

--------------

*Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.*

.. _CVE-2017-5715: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5715
.. _CVE-2018-3639: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-3639
.. _CVE-2022-23960: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2022-23960
.. _Cortex-A53 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm048406/index.html
.. _Cortex-A57 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm049219/index.html
.. _Cortex-A72 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm012079/index.html
.. _Cortex-A57 Software Optimization Guide: http://infocenter.arm.com/help/topic/com.arm.doc.uan0015b/Cortex_A57_Software_Optimization_Guide_external.pdf
.. _Arm DSU Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm138168/index.html
