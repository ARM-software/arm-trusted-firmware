Trusted Firmware-A Documentation
================================

.. toctree::
   :maxdepth: 1
   :hidden:

   Home<self>
   getting_started/index
   process/index
   components/index
   design/index
   plat/index
   perf/index
   security_advisories/index
   change-log
   acknowledgements
   glossary
   maintainers
   license

.. contents:: On This Page
    :depth: 3

Trusted Firmware-A (TF-A) provides a reference implementation of secure world
software for `Armv7-A and Armv8-A`_, including a `Secure Monitor`_ executing
at Exception Level 3 (EL3). It implements various Arm interface standards,
such as:

-  The `Power State Coordination Interface (PSCI)`_
-  `Trusted Board Boot Requirements CLIENT (TBBR-CLIENT)`_
-  `SMC Calling Convention`_
-  `System Control and Management Interface (SCMI)`_
-  `Software Delegated Exception Interface (SDEI)`_

Where possible, the code is designed for reuse or porting to other Armv7-A and
Armv8-A model and hardware platforms.

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, in either the AArch32 or AArch64 execution
states.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from TF-A.

Arm will continue development in collaboration with interested parties to
provide a full reference implementation of Secure Monitor code and Arm standards
to the benefit of all developers working with Armv7-A and Armv8-A TrustZone
technology.

Functionality
-------------

-  Initialization of the secure world, for example exception vectors, control
   registers and interrupts for the platform.

-  Library support for CPU specific reset and power down sequences. This
   includes support for errata workarounds and the latest Arm DynamIQ CPUs.

-  Drivers to enable standard initialization of Arm System IP, for example
   Generic Interrupt Controller (GIC), Cache Coherent Interconnect (CCI),
   Cache Coherent Network (CCN), Network Interconnect (NIC) and TrustZone
   Controller (TZC).

-  A generic `SCMI`_ driver to interface with conforming power controllers, for
   example the Arm System Control Processor (SCP).

-  SMC (Secure Monitor Call) handling, conforming to the `SMC Calling
   Convention`_ using an EL3 runtime services framework.

-  `PSCI`_ library support for CPU, cluster and system power management
   use-cases.
   This library is pre-integrated with the AArch64 EL3 Runtime Software, and
   is also suitable for integration with other AArch32 EL3 Runtime Software,
   for example an AArch32 Secure OS.

-  A minimal AArch32 Secure Payload (SP\_MIN) to demonstrate `PSCI`_ library
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
   experimental configuration at this time and requires the ``ENABLE_PAUTH``
   build flag to be set.

-  Position-Independent Executable (PIE) support. Initially for BL31 only, with
   further support to be added in a future release.

For a full description of functionality and implementation details, please
see the `Firmware Design`_ and supporting documentation. The `Change Log`_
provides details of changes made since the last release.

Platforms
---------

Various AArch32 and AArch64 builds of this release have been tested on r0, r1
and r2 variants of the `Juno Arm Development Platform`_.

The latest version of the AArch64 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

.. note::
   The FVP models used are Version 11.5 Build 33, unless otherwise stated.

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_AEMv8A-AEMv8A-AEMv8A-AEMv8A-CCN502``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55x4+Cortex-A75x4``
-  ``FVP_Base_Cortex-A55x4``
-  ``FVP_Base_Cortex-A57x1-A53x1``
-  ``FVP_Base_Cortex-A57x2-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A75x4``
-  ``FVP_Base_Cortex-A76x4``
-  ``FVP_Base_Cortex-A76AEx4`` (Tested with internal model)
-  ``FVP_Base_Cortex-A76AEx8`` (Tested with internal model)
-  ``FVP_Base_Neoverse-N1x4`` (Tested with internal model)
-  ``FVP_Base_Deimos``
-  ``FVP_CSS_SGI-575`` (Version 11.3 build 42)
-  ``FVP_CSS_SGM-775`` (Version 11.3 build 42)
-  ``FVP_RD_E1Edge`` (Version 11.3 build 42)
-  ``FVP_RD_N1Edge`` (Version 11.3 build 42)
-  ``Foundation_Platform``

The latest version of the AArch32 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A32x4``

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` FVP only supports shifted affinities.

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from Arm. See the `Arm FVP website`_.

All the above platforms have been tested with `Linaro Release 18.04`_.

This release also contains the following platform support:

-  Allwinner sun50i_a64 and sun50i_h6
-  Amlogic Meson S905 (GXBB)
-  Arm Juno Software Development Platform
-  Arm Neoverse N1 System Development Platform (N1SDP)
-  Arm Neoverse Reference Design N1 Edge (RD-N1-Edge) FVP
-  Arm Neoverse Reference Design E1 Edge (RD-E1-Edge) FVP
-  Arm SGI-575 and SGM-775
-  Arm Versatile Express FVP
-  HiKey, HiKey960 and Poplar boards
-  Intel Stratix 10 SoC FPGA
-  Marvell Armada 3700 and 8K
-  MediaTek MT6795 and MT8173 SoCs
-  NVIDIA T132, T186 and T210 SoCs
-  NXP QorIQ LS1043A, i.MX8MM, i.MX8MQ, i.MX8QX, i.MX8QM and i.MX7Solo WaRP7
-  QEMU
-  Raspberry Pi 3
-  Renesas R-Car Generation 3
-  RockChip RK3328, RK3368 and RK3399 SoCs
-  Socionext UniPhier SoC family and SynQuacer SC2A11 SoCs
-  STMicroelectronics STM32MP1
-  Texas Instruments K3 SoCs
-  Xilinx Versal and Zynq UltraScale + MPSoC

Still to come
-------------

-  Support for additional platforms.

-  Refinements to Position Independent Executable (PIE) support.

-  Refinements to the SPCI-based SPM implementation as the draft SPCI and SPRT
   specifications continue to evolve.

-  Documentation enhancements.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing support for new Arm system architecture specifications.

-  Ongoing security hardening, optimization and quality improvements.

For a full list of detailed issues in the current code, please see the `Change
Log`_ and the `issue tracker`_.

Getting started
---------------

See the `User Guide`_ for instructions on how to download, install, build and
use TF-A with the Arm `FVP`_\ s.

See the `Firmware Design`_ for information on how TF-A works.

See the `Porting Guide`_ as well for information about how to use this
software on another Armv7-A or Armv8-A platform.

See the `Contributing Guidelines`_ for information on how to contribute to this
project and the `Acknowledgments`_ file for a list of contributors to the
project.

Contact us
~~~~~~~~~~

We welcome any feedback on TF-A. If you think you have found a security
vulnerability, please report this using the process defined in the TF-A
`Security Center`_. For all other feedback, you can use either the
`issue tracker`_ or our `mailing list`_.

Arm licensees may contact Arm directly via their partner managers.

--------------

*Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.*

.. _Armv7-A and Armv8-A: https://developer.arm.com/products/architecture/a-profile
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): PSCI_
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _Trusted Board Boot Requirements CLIENT (TBBR-CLIENT): https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf
.. _System Control and Management Interface (SCMI): SCMI_
.. _SCMI: http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf
.. _Software Delegated Exception Interface (SDEI): SDEI_
.. _SDEI: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _Juno Arm Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _Arm FVP website: FVP_
.. _FVP: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 18.04: https://community.arm.com/dev-platforms/b/documents/posts/linaro-release-notes-deprecated#LinaroRelease18.04
.. _OP-TEE Secure OS: https://github.com/OP-TEE/optee_os
.. _NVIDIA Trusted Little Kernel: http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
.. _Trusty Secure OS: https://source.android.com/security/trusty
.. _trustedfirmware.org: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
.. _issue tracker: https://issues.trustedfirmware.org
.. _mailing list: https://lists.trustedfirmware.org/mailman/listinfo/tf-a
.. _Security Center: ./process/security.rst
.. _license: ./license.rst
.. _Contributing Guidelines: ./process/contributing.rst
.. _Acknowledgments: ./acknowledgements.rst
.. _Firmware Design: ./design/firmware-design.rst
.. _Change Log: ./change-log.rst
.. _User Guide: ./getting_started/user-guide.rst
.. _Porting Guide: ./getting_started/porting-guide.rst
