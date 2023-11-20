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

-  Secure Monitor library code such as world switching, EL2/EL1 context
   management and interrupt routing.

-  SMC (Secure Monitor Call) handling, conforming to the `SMC Calling
   Convention`_ using an EL3 runtime services framework.

-  |PSCI| library support for CPU, cluster and system power management
   use-cases.
   This library is pre-integrated with the AArch64 EL3 Runtime Software, and
   is also suitable for integration with other AArch32 EL3 Runtime Software,
   for example an AArch32 Secure OS.

-  A generic |SCMI| driver to interface with conforming power controllers, for
   example the Arm System Control Processor (SCP).

-  A minimal AArch32 Secure Payload (*SP_MIN*) to demonstrate |PSCI| library
   integration with AArch32 EL3 Runtime Software.

-  Secure partition manager dispatcher(SPMD) with following two configurations:

   -  S-EL2 SPMC implementation, widely compliant with FF-A v1.1 EAC0 and initial
      support of FF-A v1.2.

   -  EL3 SPMC implementation, compliant with a subset of FF-A v1.1 EAC0.

-  Support for Arm CCA based on FEAT_RME which supports authenticated boot and
   execution of RMM with the necessary routing of RMI commands as specified in
   RMM Beta 0 Specification.

-  A Test SP and SPD to demonstrate AArch64 Secure Monitor functionality and SP
   interaction with PSCI.

-  SPDs for the `OP-TEE Secure OS`_, `NVIDIA Trusted Little Kernel`_,
   `Trusty Secure OS`_ and `ProvenCore Secure OS`_.

-  A Trusted Board Boot implementation, conforming to all mandatory TBBR
   requirements. This includes image authentication, Firmware recovery,
   Firmware encryption and packaging of the various firmware images into a
   Firmware Image Package (FIP).

-  Measured boot support with PoC to showcase its interaction with firmware TPM
   (fTPM) service implemneted on top of OP-TEE.

-  Support for Dynamic Root of Trust for Measurement (DRTM).

-  Following firmware update mechanisms available:

   -  PSA Firmware Update (PSA FWU)

   -  TBBR Firmware Update (TBBR FWU)

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
   This feature is now incorporated inside the firmware configuration framework
   (fconf).

-  Support for alternative boot flows, for example to support platforms where
   the EL3 Runtime Software is loaded using other firmware or a separate
   secure system processor, or where a non-TF-A ROM expects BL2 to be loaded
   at EL3.

-  Support for Errata management firmware interface.

-  Support for the GCC, LLVM and Arm Compiler 6 toolchains.

-  Support for combining several libraries into a "romlib" image that may be
   shared across images to reduce memory footprint. The romlib image is stored
   in ROM but is accessed through a jump-table that may be stored
   in read-write memory, allowing for the library code to be patched.

-  Position-Independent Executable (PIE) support.

Experimental features
---------------------

A feature is considered experimental when still in development or isn't known
to the TF-A team as widely deployed or proven on end products. It is generally
advised such options aren't pulled into real deployments, or done with the
appropriate level of supplementary integration testing.

A feature is no longer considered experimental when it is generally agreed
the said feature has reached a level of maturity and quality comparable to
other features that have been integrated into products.

Experimental build options are found in following section
:ref:`build_options_experimental`. Their use through the build emits a warning
message.

Additionally the following libraries are marked experimental when included
in a platform:

-  MPU translation library ``lib/xlat_mpu``
-  RSS comms driver ``drivers/arm/rss``

Still to come
-------------

-  Support for additional platforms.

-  Documentation enhancements.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing support for new Arm system architecture specifications.

-  Ongoing security hardening, optimization and quality improvements.

.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
.. _OP-TEE Secure OS: https://github.com/OP-TEE/optee_os
.. _NVIDIA Trusted Little Kernel: http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
.. _Trusty Secure OS: https://source.android.com/security/trusty
.. _ProvenCore Secure OS: https://provenrun.com/products/provencore/

--------------

*Copyright (c) 2019-2023, Arm Limited. All rights reserved.*
