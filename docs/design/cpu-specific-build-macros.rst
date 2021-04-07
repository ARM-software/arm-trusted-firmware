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

Refer to :ref:`firmware_design_cpu_errata_reporting` for information on how to
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
   r0p4 and onwards, this errata is enabled by default in hardware.

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

For Cortex-A77, the following errata build flags are defined :

-  ``ERRATA_A77_1508412``: This applies errata 1508412 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p0 of the CPU.

-  ``ERRATA_A77_1925769``: This applies errata 1925769 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

-  ``ERRATA_A77_1946167``: This applies errata 1946167 workaround to Cortex-A77
   CPU. This needs to be enabled only for revision <= r1p1 of the CPU.

For Cortex-A78, the following errata build flags are defined :

-  ``ERRATA_A78_1688305``: This applies errata 1688305 workaround to Cortex-A78
   CPU. This needs to be enabled only for revision r0p0 - r1p0 of the CPU.

-  ``ERRATA_A78_1941498``: This applies errata 1941498 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r0p0, r1p0, and r1p1 of the CPU.

-  ``ERRATA_A78_1951500``: This applies errata 1951500 workaround to Cortex-A78
   CPU. This needs to be enabled for revisions r1p0 and r1p1, r0p0 has the same
   issue but there is no workaround for that revision.

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

--------------

*Copyright (c) 2014-2021, Arm Limited and Contributors. All rights reserved.*

.. _CVE-2017-5715: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5715
.. _CVE-2018-3639: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-3639
.. _Cortex-A53 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm048406/index.html
.. _Cortex-A57 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm049219/index.html
.. _Cortex-A72 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm012079/index.html
.. _Cortex-A57 Software Optimization Guide: http://infocenter.arm.com/help/topic/com.arm.doc.uan0015b/Cortex_A57_Software_Optimization_Guide_external.pdf
.. _Arm DSU Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm138168/index.html
