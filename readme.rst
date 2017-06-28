ARM Trusted Firmware - version 1.3
==================================

ARM Trusted Firmware provides a reference implementation of secure world
software for `ARMv8-A`_, including a `Secure Monitor`_ executing at
Exception Level 3 (EL3). It implements various ARM interface standards, such as
the Power State Coordination Interface (`PSCI`_), Trusted Board Boot Requirements
(TBBR, ARM DEN0006C-1) and `SMC Calling Convention`_. As far as possible
the code is designed for reuse or porting to other ARMv8-A model and hardware
platforms.

ARM will continue development in collaboration with interested parties to
provide a full reference implementation of PSCI, TBBR and Secure Monitor code
to the benefit of all developers working with ARMv8-A TrustZone technology.

License
-------

The software is provided under a BSD-3-Clause `license`_. Contributions to this
project are accepted under the same license with developer sign-off as
described in the `Contributing Guidelines`_.

This project contains code from other projects as listed below. The original
license text is included in those source files.

-  The stdlib source code is derived from FreeBSD code.

-  The libfdt source code is dual licensed. It is used by this project under
   the terms of the BSD-2-Clause license.

This Release
------------

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, executing in either the AArch32 or AArch64
execution state.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from ARM Trusted Firmware.

Functionality
~~~~~~~~~~~~~

-  Initialization of the secure world (for example, exception vectors, control
   registers, interrupt controller and interrupts for the platform), before
   transitioning into the normal world at the Exception Level and Register
   Width specified by the platform.

-  Library support for CPU specific reset and power down sequences. This
   includes support for errata workarounds.

-  Drivers for both versions 2.0 and 3.0 of the ARM Generic Interrupt
   Controller specifications (GICv2 and GICv3). The latter also enables GICv3
   hardware systems that do not contain legacy GICv2 support.

-  Drivers to enable standard initialization of ARM System IP, for example
   Cache Coherent Interconnect (CCI), Cache Coherent Network (CCN), Network
   Interconnect (NIC) and TrustZone Controller (TZC).

-  SMC (Secure Monitor Call) handling, conforming to the
   `SMC Calling Convention`_ using an EL3 runtime services framework.

-  `PSCI`_ library support for the Secondary CPU Boot, CPU Hotplug, CPU Idle
   and System Shutdown/Reset/Suspend use-cases.
   This library is pre-integrated with the provided AArch64 EL3 Runtime
   Software, and is also suitable for integration into other EL3 Runtime
   Software.

-  A minimal AArch32 Secure Payload to demonstrate `PSCI`_ library integration
   on platforms with AArch32 EL3 Runtime Software.

-  Secure Monitor library code such as world switching, EL1 context management
   and interrupt routing.
   When using the provided AArch64 EL3 Runtime Software, this must be
   integrated with a Secure-EL1 Payload Dispatcher (SPD) component to
   customize the interaction with a Secure-EL1 Payload (SP), for example a
   Secure OS.

-  A Test Secure-EL1 Payload and Dispatcher to demonstrate AArch64 Secure
   Monitor functionality and Secure-EL1 interaction with PSCI.

-  AArch64 SPDs for the `OP-TEE Secure OS`_ and `NVidia Trusted Little Kernel`_.

-  A Trusted Board Boot implementation, conforming to all mandatory TBBR
   requirements. This includes image authentication using certificates, a
   Firmware Update (or recovery mode) boot flow, and packaging of the various
   firmware images into a Firmware Image Package (FIP) to be loaded from
   non-volatile storage.
   The TBBR implementation is currently only supported in the AArch64 build.

-  Support for alternative boot flows. Some platforms have their own boot
   firmware and only require the AArch64 EL3 Runtime Software provided by this
   project. Other platforms require minimal initialization before booting
   into an arbitrary EL3 payload.

For a full description of functionality and implementation details, please
see the `Firmware Design`_ and supporting documentation. The `Change Log`_
provides details of changes made since the last release.

Platforms
~~~~~~~~~

The AArch64 build of this release has been tested on variants r0, r1 and r2
of the `Juno ARM Development Platform`_ with `Linaro Release 16.06`_.

The AArch64 build of this release has been tested on the following ARM
`FVP`_\ s (64-bit host machine only, with `Linaro Release 16.06`_):

-  ``Foundation_Platform`` (Version 10.1, Build 10.1.32)
-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 7.7, Build 0.8.7701)
-  ``FVP_Base_Cortex-A57x4-A53x4`` (Version 7.7, Build 0.8.7701)
-  ``FVP_Base_Cortex-A57x1-A53x1`` (Version 7.7, Build 0.8.7701)
-  ``FVP_Base_Cortex-A57x2-A53x4`` (Version 7.7, Build 0.8.7701)

The AArch32 build of this release has been tested on the following ARM
`FVP`_\ s (64-bit host machine only, with `Linaro Release 16.06`_):

-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 7.7, Build 0.8.7701)
-  ``FVP_Base_Cortex-A32x4`` (Version 10.1, Build 10.1.32)

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from ARM: see `www.arm.com/fvp`_.

This release also contains the following platform support:

-  MediaTek MT6795 and MT8173 SoCs
-  NVidia T210 and T132 SoCs
-  QEMU emulator
-  RockChip RK3368 and RK3399 SoCs
-  Xilinx Zynq UltraScale + MPSoC

Still to Come
~~~~~~~~~~~~~

-  AArch32 TBBR support and ongoing TBBR alignment.

-  More platform support.

-  Ongoing support for new architectural features, CPUs and System IP.

-  Ongoing `PSCI`_ alignment and feature support.

-  Ongoing security hardening, optimization and quality improvements.

For a full list of detailed issues in the current code, please see the
`Change Log`_ and the `GitHub issue tracker`_.

Getting Started
---------------

Get the Trusted Firmware source code from
`GitHub`_.

See the `User Guide`_ for instructions on how to install, build and use
the Trusted Firmware with the ARM `FVP`_\ s.

See the `Firmware Design`_ for information on how the ARM Trusted Firmware works.

See the `Porting Guide`_ as well for information about how to use this
software on another ARMv8-A platform.

See the `Contributing Guidelines`_ for information on how to contribute to this
project and the `Acknowledgments`_ file for a list of contributors to the
project.

Feedback and support
~~~~~~~~~~~~~~~~~~~~

ARM welcomes any feedback on the Trusted Firmware. Please send feedback using
the `GitHub issue tracker`_.

ARM licensees may contact ARM directly via their partner managers.

--------------

*Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.*

.. _ARMv8-A: http://www.arm.com/products/processors/armv8-architecture.php
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html
.. _license: ./license.rst
.. _Contributing Guidelines: ./contributing.rst
.. _OP-TEE Secure OS: https://github.com/OP-TEE/optee_os
.. _NVidia Trusted Little Kernel: http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
.. _Firmware Design: ./docs/firmware-design.rst
.. _Change Log: ./docs/change-log.rst
.. _Juno ARM Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _Linaro Release 16.06: https://community.arm.com/docs/DOC-10952#jive_content_id_Linaro_Release_1606
.. _FVP: http://www.arm.com/fvp
.. _www.arm.com/fvp: http://www.arm.com/fvp
.. _GitHub issue tracker: https://github.com/ARM-software/tf-issues/issues
.. _GitHub: https://www.github.com/ARM-software/arm-trusted-firmware
.. _User Guide: ./docs/user-guide.rst
.. _Porting Guide: ./docs/porting-guide.rst
.. _Acknowledgments: ./acknowledgements.rst
