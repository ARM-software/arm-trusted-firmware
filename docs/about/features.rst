Feature Overview
================

This page provides an overview of the current |TF-A| feature set. For a full
description of these features and their implementation details, please see
the documents that are part of the *Components* and *System Design* chapters.

The :ref:`Change Log & Release Notes` provides details of changes made since the
last release.

Current features
----------------

-  Initialization of the secure world, for example exception vectors, control
   registers and interrupts for the platform.

-  Library support for CPU specific reset and power down sequences. This
   includes support for errata workarounds and the latest Arm DynamIQ CPUs.

-  Drivers to enable standard initialization of Arm System IP, for example
   Generic Interrupt Controller (GIC), Cache Coherent Interconnect (CCI),
   Cache Coherent Network (CCN), Network Interconnect (NIC) and TrustZone
   Controller (TZC).

-  A generic |SCMI| driver to interface with conforming power controllers, for
   example the Arm System Control Processor (SCP).

-  SMC (Secure Monitor Call) handling, conforming to the `SMC Calling
   Convention`_ using an EL3 runtime services framework.

-  |PSCI| library support for CPU, cluster and system power management
   use-cases.
   This library is pre-integrated with the AArch64 EL3 Runtime Software, and
   is also suitable for integration with other AArch32 EL3 Runtime Software,
   for example an AArch32 Secure OS.

-  A minimal AArch32 Secure Payload (*SP_MIN*) to demonstrate |PSCI| library
   integration with AArch32 EL3 Runtime Software.

-  Secure Monitor library code such as world switching, EL1 context management
   and interrupt routing.
   When a Secure-EL1 Payload (SP) is present, for example a Secure OS, the
   AArch64 EL3 Runtime Software must be integrated with a Secure Payload
   Dispatcher (SPD) component to customize the interaction with the SP.

-  A Test SP and SPD to demonstrate AArch64 Secure Monitor functionality and SP
   interaction with PSCI.

-  SPDs for the `OP-TEE Secure OS`_, `NVIDIA Trusted Little Kernel`_
   and `Trusty Secure OS`_.

-  A Trusted Board Boot implementation, conforming to all mandatory TBBR
   requirements. This includes image authentication, Firmware Update (or
   recovery mode), and packaging of the various firmware images into a
   Firmware Image Package (FIP).

-  Pre-integration of TBB with the Arm CryptoCell product, to take advantage of
   its hardware Root of Trust and crypto acceleration services.

-  Reliability, Availability, and Serviceability (RAS) functionality, including

   -  A Secure Partition Manager (SPM) to manage Secure Partitions in
      Secure-EL0, which can be used to implement simple management and
      security services.

   -  An |SDEI| dispatcher to route interrupt-based |SDEI| events.

   -  An Exception Handling Framework (EHF) that allows dispatching of EL3
      interrupts to their registered handlers, to facilitate firmware-first
      error handling.

-  A dynamic configuration framework that enables each of the firmware images
   to be configured at runtime if required by the platform. It also enables
   loading of a hardware configuration (for example, a kernel device tree)
   as part of the FIP, to be passed through the firmware stages.

-  Support for alternative boot flows, for example to support platforms where
   the EL3 Runtime Software is loaded using other firmware or a separate
   secure system processor, or where a non-TF-A ROM expects BL2 to be loaded
   at EL3.

-  Support for the GCC, LLVM and Arm Compiler 6 toolchains.

-  Support for combining several libraries into a "romlib" image that may be
   shared across images to reduce memory footprint. The romlib image is stored
   in ROM but is accessed through a jump-table that may be stored
   in read-write memory, allowing for the library code to be patched.

-  A prototype implementation of a Secure Partition Manager (SPM) that is based
   on the SPCI Alpha 1 and SPRT draft specifications.

-  Support for ARMv8.3 pointer authentication in the normal and secure worlds.
   The use of pointer authentication in the normal world is enabled whenever
   architectural support is available, without the need for additional build
   flags. Use of pointer authentication in the secure world remains an
   experimental configuration at this time and requires the
   ``BRANCH_PROTECTION`` option to be set to non-zero.

-  Position-Independent Executable (PIE) support. Initially for BL31 only, with
   further support to be added in a future release.

Still to come
-------------

-  Support for additional platforms.

-  Refinements to Position Independent Executable (PIE) support.

-  Continued support for the draft SPCI specification, to enable the use of
   secure partition management in the secure world.

-  Documentation enhancements.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing support for new Arm system architecture specifications.

-  Ongoing security hardening, optimization and quality improvements.

.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf
.. _OP-TEE Secure OS: https://github.com/OP-TEE/optee_os
.. _NVIDIA Trusted Little Kernel: http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
.. _Trusty Secure OS: https://source.android.com/security/trusty

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
