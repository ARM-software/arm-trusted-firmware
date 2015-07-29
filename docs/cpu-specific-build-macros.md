ARM CPU Specific Build Macros
=============================

Contents
--------

1.  [Introduction](#1--introduction)
2.  [CPU Errata Workarounds](#2--cpu-errata-workarounds)
3.  [CPU Specific optimizations](#3--cpu-specific-optimizations)


1.  Introduction
----------------

This document describes the various build options present in the CPU specific
operations framework to enable errata workarounds and to enable optimizations
for a specific CPU on a platform.

2.  CPU Errata Workarounds
--------------------------

ARM Trusted Firmware exports a series of build flags which control the
errata workarounds that are applied to each CPU by the reset handler. The
errata details can be found in the CPU specific errata documents published
by ARM. The errata workarounds are implemented for a particular revision
or a set of processor revisions. This is checked by reset handler at runtime.
Each errata workaround is identified by its `ID` as specified in the processor's
errata notice document. The format of the define used to enable/disable the
errata is `ERRATA_<Processor name>_<ID>` where the `Processor name`
is either `A57` for the `Cortex_A57` CPU or `A53` for `Cortex_A53` CPU.

All workarounds are disabled by default. The platform is reponsible for
enabling these workarounds according to its requirement by defining the
errata workaround build flags in the platform specific makefile. In case
these workarounds are enabled for the wrong CPU revision then the errata
workaround is not applied. In the DEBUG build, this is indicated by
printing a warning to the crash console.

In the current implementation, a platform which has more than 1 variant
with different revisions of a processor has no runtime mechanism available
for it to specify which errata workarounds should be enabled or not.

The value of the build flags are 0 by default, that is, disabled. Any other
value will enable it.

For Cortex-A53, following errata build flags are defined :

*   `ERRATA_A53_826319`: This applies errata 826319 workaround to Cortex-A53
     CPU. This needs to be enabled only for revision <= r0p2 of the CPU.

*   `ERRATA_A53_836870`: This applies errata 836870 workaround to Cortex-A53
     CPU. This needs to be enabled only for revision <= r0p3 of the CPU. From
     r0p4 and onwards, this errata is enabled by default.

For Cortex-A57, following errata build flags are defined :

*   `ERRATA_A57_806969`: This applies errata 806969 workaround to Cortex-A57
     CPU. This needs to be enabled only for revision r0p0 of the CPU.

*   `ERRATA_A57_813420`: This applies errata 813420 workaround to Cortex-A57
     CPU. This needs to be enabled only for revision r0p0 of the CPU.

3.  CPU Specific optimizations
------------------------------

This section describes some of the optimizations allowed by the CPU micro
architecture that can be enabled by the platform as desired.

*    `SKIP_A57_L1_FLUSH_PWR_DWN`: This flag enables an optimization in the
     Cortex-A57 cluster power down sequence by not flushing the Level 1 data
     cache. The L1 data cache and the L2 unified cache are inclusive. A flush
     of the L2 by set/way flushes any dirty lines from the L1 as well. This
     is a known safe deviation from the Cortex-A57 TRM defined power down
     sequence. Each Cortex-A57 based platform must make its own decision on
     whether to use the optimization.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2014, ARM Limited and Contributors. All rights reserved._
