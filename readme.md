ARM Trusted Firmware - version 1.2
==================================

ARM Trusted Firmware provides a reference implementation of secure world
software for [ARMv8-A], including a [Secure Monitor] [TEE-SMC] executing at
Exception Level 3 (EL3). It implements various ARM interface standards, such as
the Power State Coordination Interface ([PSCI]), Trusted Board Boot Requirements
(TBBR, ARM DEN0006C-1) and [SMC Calling Convention][SMCCC]. As far as possible
the code is designed for reuse or porting to other ARMv8-A model and hardware
platforms.

ARM will continue development in collaboration with interested parties to
provide a full reference implementation of PSCI, TBBR and Secure Monitor code
to the benefit of all developers working with ARMv8-A TrustZone technology.


License
-------

The software is provided under a BSD 3-Clause [license]. Certain source files
are derived from FreeBSD code: the original license is included in these
source files.


This Release
------------

This release provides a suitable starting point for productization of secure
world boot and runtime firmware. Future versions will contain new features,
optimizations and quality improvements.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from ARM Trusted Firmware.

### Functionality

*   Initialization of the secure world (for example, exception vectors, control
    registers, interrupt controller and interrupts for the platform), before
    transitioning into the normal world at the Exception Level and Register
    Width specified by the platform.

*   Library support for CPU specific reset and power down sequences. This
    includes support for errata workarounds.

*   Drivers for both the version 2.0 and version 3.0 ARM Generic Interrupt
    Controller specifications (GICv2 and GICv3). The latter also enables GICv3
    hardware systems that do not contain legacy GICv2 support.

*   Drivers to enable standard initialization of ARM System IP, for example
    Cache Coherent Interconnect (CCI), Cache Coherent Network (CCN), Network
    Interconnect (NIC) and TrustZone Controller (TZC).

*   SMC (Secure Monitor Call) handling, conforming to the [SMC Calling
    Convention][SMCCC] using an EL3 runtime services framework.

*   SMC handling relating to [PSCI] for the Secondary CPU Boot, CPU Hotplug,
    CPU Idle and System Shutdown/Reset/Suspend use-cases.

*   Secure Monitor library code such as world switching, EL1 context management
    and interrupt routing. This must be integrated with a Secure-EL1 Payload
    Dispatcher (SPD) component to customize the interaction with a Secure-EL1
    Payload (SP), for example a Secure OS.

*   A Test Secure-EL1 Payload and Dispatcher to demonstrate Secure Monitor
    functionality and Secure-EL1 interaction with PSCI.

*   SPDs for the [OP-TEE Secure OS] and [NVidia Trusted Little Kernel]
    [NVidia TLK].

*   A Trusted Board Boot implementation, conforming to all mandatory TBBR
    requirements. This includes image authentication using certificates, a
    Firmware Update (or recovery mode) boot flow, and packaging of the various
    firmware images into a Firmware Image Package (FIP) to be loaded from
    non-volatile storage.

*   Support for alternative boot flows. Some platforms have their own boot
    firmware and only require the ARM Trusted Firmware Secure Monitor
    functionality. Other platforms require minimal initialization before
    booting into an arbitrary EL3 payload.

For a full description of functionality and implementation details, please
see the [Firmware Design] and supporting documentation. The [Change Log]
provides details of changes made since the last release.

### Platforms

This release of the Trusted Firmware has been tested on variants r0 and r1 of
the [Juno ARM Development Platform] [Juno] with [Linaro Release 15.10]
[Linaro Release Notes].

The Trusted Firmware has also been tested on the 64-bit Linux versions of the
following ARM [FVP]s:

*   `Foundation_Platform` (Version 9.4, Build 9.4.59)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 7.0, Build 0.8.7004)

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from ARM: see [www.arm.com/fvp] [FVP].

This release also contains the following platform support:

*   NVidia T210 and T132 SoCs
*   MediaTek MT8173 SoC

### Still to Come

*   Complete implementation of the [PSCI] v1.0 specification.

*   Support for new CPUs and System IP.

*   More platform support.

*   Optimization and quality improvements.

For a full list of detailed issues in the current code, please see the [Change
Log] and the [GitHub issue tracker].


Getting Started
---------------

Get the Trusted Firmware source code from
[GitHub](https://www.github.com/ARM-software/arm-trusted-firmware).

See the [User Guide] for instructions on how to install, build and use
the Trusted Firmware with the ARM [FVP]s.

See the [Firmware Design] for information on how the ARM Trusted Firmware works.

See the [Porting Guide] as well for information about how to use this
software on another ARMv8-A platform.

See the [Contributing Guidelines] for information on how to contribute to this
project and the [Acknowledgments] file for a list of contributors to the
project.

### Feedback and support

ARM welcomes any feedback on the Trusted Firmware. Please send feedback using
the [GitHub issue tracker].

ARM licensees may contact ARM directly via their partner managers.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved._


[License]:                  ./license.md "BSD license for ARM Trusted Firmware"
[Contributing Guidelines]:  ./contributing.md "Guidelines for contributors"
[Acknowledgments]:          ./acknowledgements.md "Contributor acknowledgments"
[Change Log]:               ./docs/change-log.md
[User Guide]:               ./docs/user-guide.md
[Firmware Design]:          ./docs/firmware-design.md
[Porting Guide]:            ./docs/porting-guide.md

[ARMv8-A]:               http://www.arm.com/products/processors/armv8-architecture.php "ARMv8-A Architecture"
[FVP]:                   http://www.arm.com/fvp "ARM's Fixed Virtual Platforms"
[Juno]:                  http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php "Juno ARM Development Platform"
[PSCI]:                  http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf "Power State Coordination Interface PDD (ARM DEN 0022C)"
[SMCCC]:                 http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html "SMC Calling Convention PDD (ARM DEN 0028A)"
[TEE-SMC]:               http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php "Secure Monitor and TEEs"
[GitHub issue tracker]:  https://github.com/ARM-software/tf-issues/issues
[OP-TEE Secure OS]:      https://github.com/OP-TEE/optee_os
[NVidia TLK]:            http://nv-tegra.nvidia.com/gitweb/?p=3rdparty/ote_partner/tlk.git;a=summary
[Linaro Release Notes]:  https://community.arm.com/docs/DOC-10952#jive_content_id_Linaro_Release_1510
