ARM CPU Specific Build Macros
=============================


.. section-numbering::
    :suffix: .

.. contents::

This document describes the various build options present in the CPU specific
operations framework to enable errata workarounds and to enable optimizations
for a specific CPU on a platform.

Security Vulnerability Workarounds
----------------------------------

ARM Trusted Firmware exports a series of build flags which control which
security vulnerability workarounds should be applied at runtime.

-  ``WORKAROUND_CVE_2017_5715``: Enables the security workaround for
   `CVE-2017-5715`_. Defaults to 1.

CPU Errata Workarounds
----------------------

ARM Trusted Firmware exports a series of build flags which control the
errata workarounds that are applied to each CPU by the reset handler. The
errata details can be found in the CPU specific errata documents published
by ARM:

-  `Cortex-A53 MPCore Software Developers Errata Notice`_
-  `Cortex-A57 MPCore Software Developers Errata Notice`_
-  `Cortex-A72 MPCore Software Developers Errata Notice`_

The errata workarounds are implemented for a particular revision or a set of
processor revisions. This is checked by the reset handler at runtime. Each
errata workaround is identified by its ``ID`` as specified in the processor's
errata notice document. The format of the define used to enable/disable the
errata workaround is ``ERRATA_<Processor name>_<ID>``, where the ``Processor name``
is for example ``A57`` for the ``Cortex_A57`` CPU.

Refer to the section *CPU errata status reporting* in
`Firmware Design guide`_ for information on how to write errata workaround
functions.

All workarounds are disabled by default. The platform is responsible for
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

-  ``ERRATA_A53_826319``: This applies errata 826319 workaround to Cortex-A53
   CPU. This needs to be enabled only for revision <= r0p2 of the CPU.

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
   a chicken bit in CPUACTLR\_EL1 to enable a hardware workaround.
   Earlier revisions of the CPU have other errata which require the same
   workaround in software, so they should be covered anyway.

For Cortex-A57, following errata build flags are defined :

-  ``ERRATA_A57_806969``: This applies errata 806969 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_813419``: This applies errata 813419 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

-  ``ERRATA_A57_813420``: This applies errata 813420 workaround to Cortex-A57
   CPU. This needs to be enabled only for revision r0p0 of the CPU.

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


For Cortex-A72, following errata build flags are defined :

-  ``ERRATA_A72_859971``: This applies errata 859971 workaround to Cortex-A72
   CPU. This needs to be enabled only for revision <= r0p3 of the CPU.

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
   significant speed degradation to any code that employs them. The ARMv8-A
   architecture (see ARM DDI 0487A.h, section D3.4.3) allows cores to ignore
   the non-temporal hint and treat LDNP/STNP as LDP/STP instead. Enabling this
   flag enforces this behaviour. This needs to be enabled only for revisions
   <= r0p3 of the CPU and is enabled by default.

-  ``A57_DISABLE_NON_TEMPORAL_HINT``: This flag has the same behaviour as
   ``A53_DISABLE_NON_TEMPORAL_HINT`` but for Cortex-A57. This needs to be
   enabled only for revisions <= r1p2 of the CPU and is enabled by default,
   as recommended in section "4.7 Non-Temporal Loads/Stores" of the
   `Cortex-A57 Software Optimization Guide`_.

--------------

*Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.*

.. _CVE-2017-5715: http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=2017-5715
.. _Cortex-A53 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm048406/Cortex_A53_MPCore_Software_Developers_Errata_Notice.pdf
.. _Cortex-A57 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm049219/cortex_a57_mpcore_software_developers_errata_notice.pdf
.. _Cortex-A72 MPCore Software Developers Errata Notice: http://infocenter.arm.com/help/topic/com.arm.doc.epm012079/index.html
.. _Firmware Design guide: firmware-design.rst
.. _Cortex-A57 Software Optimization Guide: http://infocenter.arm.com/help/topic/com.arm.doc.uan0015b/Cortex_A57_Software_Optimization_Guide_external.pdf
