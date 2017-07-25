ARM Trusted Firmware - version 1.4
==================================

ARM Trusted Firmware provides a reference implementation of secure world
software for `ARMv8-A`_, including a `Secure Monitor`_ executing at
Exception Level 3 (EL3). It implements various ARM interface standards, such as:

-  The `Power State Coordination Interface (PSCI)`_
-  Trusted Board Boot Requirements (TBBR, ARM DEN0006C-1)
-  `SMC Calling Convention`_
-  `System Control and Management Interface`_

As far as possible the code is designed for reuse or porting to other ARMv8-A
model and hardware platforms.

ARM will continue development in collaboration with interested parties to
provide a full reference implementation of Secure Monitor code and ARM standards
to the benefit of all developers working with ARMv8-A TrustZone technology.

License
-------

The software is provided under a BSD-3-Clause `license`_. Contributions to this
project are accepted under the same license with developer sign-off as
described in the `Contributing Guidelines`_.

This project contains code from other projects as listed below. The original
license text is included in those source files.

-  The stdlib source code is derived from FreeBSD code, which uses various
   BSD licenses, including BSD-3-Clause and BSD-2-Clause.

-  The libfdt source code is dual licensed. It is used by this project under
   the terms of the BSD-2-Clause license.

-  The LLVM compiler-rt source code is dual licensed. It is used by this
   project under the terms of the NCSA license (also known as the University of
   Illinois/NCSA Open Source License).

This Release
------------

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, in either the AArch32 or AArch64 execution
state.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from ARM Trusted Firmware.

Functionality
~~~~~~~~~~~~~

-  Initialization of the secure world, for example exception vectors, control
   registers and interrupts for the platform.

-  Library support for CPU specific reset and power down sequences. This
   includes support for errata workarounds and the latest ARM DynamIQ CPUs.

-  Drivers to enable standard initialization of ARM System IP, for example
   Generic Interrupt Controller (GIC), Cache Coherent Interconnect (CCI),
   Cache Coherent Network (CCN), Network Interconnect (NIC) and TrustZone
   Controller (TZC).

-  A generic `SCMI`_ driver to interface with conforming power controllers, for
   example the ARM System Control Processor (SCP).

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

-  Pre-integration of TBB with the ARM TrustZone CryptoCell product, to take
   advantage of its hardware Root of Trust and crypto acceleration services.

-  Support for alternative boot flows, for example to support platforms where
   the EL3 Runtime Software is loaded using other firmware or a separate
   secure system processor.

-  Support for the GCC, LLVM and ARM Compiler 6 toolchains.

For a full description of functionality and implementation details, please
see the `Firmware Design`_ and supporting documentation. The `Change Log`_
provides details of changes made since the last release.

Platforms
~~~~~~~~~

Various AArch32 and AArch64 builds of this release has been tested on variants
r0, r1 and r2 of the `Juno ARM Development Platform`_.

Various AArch64 builds of this release have been tested on the following ARM
`FVP`_\ s (64-bit host machine only):

NOTE: Unless otherwise stated, the FVP Version is 11.0, Build 11.0.34.

-  ``Foundation_Platform``
-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 8.5, Build 0.8.8502)
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_CSS_SGM-775`` (Version 11.0, Build 11.0.36)

Various AArch32 builds of this release has been tested on the following ARM
`FVP`_\ s (64-bit host machine only):

-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 8.5, Build 0.8.8502)
-  ``FVP_Base_Cortex-A32x4``

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from ARM. See the `ARM FVP website`_.

All the above platforms have been tested with `Linaro Release 17.04`_.

This release also contains the following platform support:

-  HiKey and HiKey960 boards
-  MediaTek MT6795 and MT8173 SoCs
-  NVidia T132, T186 and T210 SoCs
-  QEMU emulator
-  RockChip RK3328, RK3368 and RK3399 SoCs
-  Socionext UniPhier SoC family
-  Xilinx Zynq UltraScale + MPSoC

Still to Come
~~~~~~~~~~~~~

-  More platform support.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing support for new `PSCI`_, `SCMI`_ and TBBR features.

-  Ongoing security hardening, optimization and quality improvements.

For a full list of detailed issues in the current code, please see the `Change
Log`_ and the `GitHub issue tracker`_.

Getting Started
---------------

Get the Trusted Firmware source code from `GitHub`_.

See the `User Guide`_ for instructions on how to install, build and use
the Trusted Firmware with the ARM `FVP`_\ s.

See the `Firmware Design`_ for information on how the Trusted Firmware works.

See the `Porting Guide`_ as well for information about how to use this
software on another ARMv8-A platform.

See the `Contributing Guidelines`_ for information on how to contribute to this
project and the `Acknowledgments`_ file for a list of contributors to the
project.

Feedback and support
~~~~~~~~~~~~~~~~~~~~

ARM welcomes any feedback on Trusted Firmware. If you think you have found a
security vulnerability, please report this using the process defined in the
Trusted Firmware `Security Centre`_. For all other feedback, please use the
`GitHub issue tracker`_.

ARM licensees may contact ARM directly via their partner managers.

--------------

*Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.*

.. _ARMv8-A: http://www.arm.com/products/processors/armv8-architecture.php
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): PSCI_
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf
.. _System Control and Management Interface: SCMI_
.. _SCMI: http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf
.. _Juno ARM Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _ARM FVP website: FVP_
.. _FVP: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 17.04: https://community.arm.com/dev-platforms/b/documents/posts/linaro-release-notes-deprecated#LinaroRelease17.04
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
