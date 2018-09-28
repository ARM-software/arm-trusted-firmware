Trusted Firmware-A - version 2.0
================================

Trusted Firmware-A (TF-A) provides a reference implementation of secure world
software for `Armv7-A and Armv8-A`_, including a `Secure Monitor`_ executing
at Exception Level 3 (EL3). It implements various Arm interface standards,
such as:

-  The `Power State Coordination Interface (PSCI)`_
-  Trusted Board Boot Requirements (TBBR, Arm DEN0006C-1)
-  `SMC Calling Convention`_
-  `System Control and Management Interface`_
-  `Software Delegated Exception Interface (SDEI)`_

Where possible, the code is designed for reuse or porting to other Armv7-A and
Armv8-A model and hardware platforms.

Arm will continue development in collaboration with interested parties to
provide a full reference implementation of Secure Monitor code and Arm standards
to the benefit of all developers working with Armv7-A and Armv8-A TrustZone
technology.

License
-------

The software is provided under a BSD-3-Clause `license`_. Contributions to this
project are accepted under the same license with developer sign-off as
described in the `Contributing Guidelines`_.

This project contains code from other projects as listed below. The original
license text is included in those source files.

-  The libc source code is derived from `FreeBSD`_ and `SCC`_. FreeBSD uses
   various BSD licenses, including BSD-3-Clause and BSD-2-Clause. The SCC code
   is used under the BSD-3-Clause license with the author's permission.

-  The libfdt source code is disjunctively dual licensed
   (GPL-2.0+ OR BSD-2-Clause). It is used by this project under the terms of
   the BSD-2-Clause license. Any contributions to this code must be made under
   the terms of both licenses.

-  The LLVM compiler-rt source code is disjunctively dual licensed
   (NCSA OR MIT). It is used by this project under the terms of the NCSA
   license (also known as the University of Illinois/NCSA Open Source License),
   which is a permissive license compatible with BSD-3-Clause. Any
   contributions to this code must be made under the terms of both licenses.

-  The zlib source code is licensed under the Zlib license, which is a
   permissive license compatible with BSD-3-Clause.

-  Some STMicroelectronics platform source code is disjunctively dual licensed
   (GPL-2.0+ OR BSD-3-Clause). It is used by this project under the terms of the
   BSD-3-Clause license. Any contributions to this code must be made under the
   terms of both licenses.

This release
------------

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, in either the AArch32 or AArch64 execution
state.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from TF-A.

Functionality
~~~~~~~~~~~~~

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
   AArch64 EL3 Runtime Software must be integrated with a dispatcher component
   (SPD) to customize the interaction with the SP.

-  A Test SP/SPD to demonstrate AArch64 Secure Monitor functionality and SP
   interaction with PSCI.

-  SPDs for the `OP-TEE Secure OS`_, `NVidia Trusted Little Kernel`_
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

   -  An SDEI dispatcher to route interrupt-based SDEI events.

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

-  Support combining several libraries into a self-called "romlib" image, that
   may be shared across images to reduce memory footprint. The romlib image
   is stored in ROM but is accessed through a jump-table that may be stored
   in read-write memory, allowing for the library code to be patched.

For a full description of functionality and implementation details, please
see the `Firmware Design`_ and supporting documentation. The `Change Log`_
provides details of changes made since the last release.

Platforms
~~~~~~~~~

Various AArch32 and AArch64 builds of this release has been tested on variants
r0, r1 and r2 of the `Juno Arm Development Platform`_.

Various AArch64 builds of this release have been tested on the following Arm
Fixed Virtual Platforms (`FVP`_) without shifted affinities that do not
support threaded CPU cores (64-bit host machine only):

NOTE: Unless otherwise stated, the model version is Version 11.4 Build 37.

-  ``FVP_Base_Aresx4``
-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_AEMv8A-AEMv8A-AEMv8A-AEMv8A-CCN502``
-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55x4+Cortex-A75x4``
-  ``FVP_Base_Cortex-A55x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A75x4``
-  ``FVP_Base_Cortex-A76x4``
-  ``FVP_CSS_SGI-575`` (Version 11.3 build 40)
-  ``Foundation_Platform``

The latest version of the AArch32 build of TF-A has been tested on the following
Arm FVPs without shifted affinities that do not support threaded CPU cores
(64-bit host machine only).

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A32x4``

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from Arm. See the `Arm FVP website`_.

All the above platforms have been tested with `Linaro Release 18.04`_.

This release also contains the following platform support:

-  Allwinner sun50i_64 and sun50i_h6
-  ARM SGI-575 and SGM-775
-  HiKey, HiKey960 and Poplar boards
-  Marvell Armada 8K
-  MediaTek MT6795 and MT8173 SoCs
-  NVidia T132, T186 and T210 SoCs
-  NXP QorIQ LS1043A, i.MX8QX, i.MX8QM and i.MX7Solo WaRP7
-  QEMU emulator
-  Raspberry Pi 3 board
-  RockChip RK3328, RK3368 and RK3399 SoCs
-  Socionext UniPhier SoC family and SynQuacer SC2A11 SoCs
-  STMicroelectronics STM32MP1
-  Texas Instruments K3 SoCs
-  Xilinx Zynq UltraScale + MPSoC

Still to come
~~~~~~~~~~~~~

-  More platform support.

-  Position independent executable (PIE) support.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing support for new Arm system architecture specifications.

-  Ongoing security hardening, optimization and quality improvements.

For a full list of detailed issues in the current code, please see the `Change
Log`_ and the `GitHub issue tracker`_.

Getting started
---------------

Get the TF-A source code from `GitHub`_.

See the `User Guide`_ for instructions on how to install, build and use
the TF-A with the Arm `FVP`_\ s.

See the `Firmware Design`_ for information on how the TF-A works.

See the `Porting Guide`_ as well for information about how to use this
software on another Armv7-A or Armv8-A platform.

See the `Contributing Guidelines`_ for information on how to contribute to this
project and the `Acknowledgments`_ file for a list of contributors to the
project.

IRC channel
~~~~~~~~~~~

Development discussion takes place on the #trusted-firmware-a channel
on the Freenode IRC network. This is not an official support channel.
If you have an issue to raise, please use the `GitHub issue tracker`_.

Feedback and support
~~~~~~~~~~~~~~~~~~~~

Arm welcomes any feedback on TF-A. If you think you have found a security
vulnerability, please report this using the process defined in the TF-A
`Security Centre`_. For all other feedback, please use the
`GitHub issue tracker`_.

Arm licensees may contact Arm directly via their partner managers.

--------------

*Copyright (c) 2013-2018, Arm Limited and Contributors. All rights reserved.*

.. _Armv7-A and Armv8-A: https://developer.arm.com/products/architecture/a-profile
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): PSCI_
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf
.. _System Control and Management Interface: SCMI_
.. _SCMI: http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf
.. _Software Delegated Exception Interface (SDEI): SDEI_
.. _SDEI: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _Juno Arm Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _Arm FVP website: FVP_
.. _FVP: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 18.04: https://community.arm.com/dev-platforms/b/documents/posts/linaro-release-notes-deprecated#LinaroRelease18.04
.. _OP-TEE Secure OS: https://github.com/OP-TEE/optee_os
.. _NVidia Trusted Little Kernel: http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
.. _Trusty Secure OS: https://source.android.com/security/trusty
.. _GitHub: https://www.github.com/ARM-software/arm-trusted-firmware
.. _GitHub issue tracker: https://github.com/ARM-software/tf-issues/issues
.. _Security Centre: https://github.com/ARM-software/arm-trusted-firmware/wiki/ARM-Trusted-Firmware-Security-Centre
.. _license: ./license.rst
.. _Contributing Guidelines: ./contributing.rst
.. _Acknowledgments: ./acknowledgements.rst
.. _Firmware Design: ./docs/firmware-design.rst
.. _Change Log: ./docs/change-log.rst
.. _User Guide: ./docs/user-guide.rst
.. _Porting Guide: ./docs/porting-guide.rst
.. _FreeBSD: http://www.freebsd.org
.. _SCC: http://www.simple-cc.org/
