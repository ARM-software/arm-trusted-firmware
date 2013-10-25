ARM Trusted Firmware - version 0.2
==================================

ARM Trusted Firmware provides a reference implementation of secure world
software for [ARMv8], including Exception Level 3 (EL3) software. This first
release focuses on support for ARM's [Fixed Virtual Platforms (FVPs)] [FVP].

The intent is to provide a reference implementation of various ARM interface
standards, such as the Power State Coordination Interface ([PSCI]), Trusted
Board Boot Requirements (TBBR) and [Secure Monitor] [TEE-SMC] code. As far as
possible the code is designed for reuse or porting to other ARMv8 model and
hardware platforms.

This release is the first one as source code: an initial prototype
release was available in binary form in the [Linaro AArch64 OpenEmbedded
Engineering Build] [AArch64 LEB] to support the new FVP Base platform
models from ARM.

ARM will continue development in collaboration with interested parties to
provide a full reference implementation of PSCI, TBBR and Secure Monitor code
to the benefit of all developers working with ARMv8 TrustZone software.


License
-------

The software is provided under a BSD 3-Clause [license]. Certain source files
are derived from FreeBSD code: the original license is included in these
source files.


This Release
------------

This software is an early implementation of the Trusted Firmware. Only
limited functionality is provided at present and it has not been optimized or
subjected to extended robustness or stress testing.

### Functionality

*   Initial implementation of a subset of the Trusted Board Boot Requirements
    Platform Design Document (PDD).

*   Initializes the secure world (for example, exception vectors, control
    registers, GIC and interrupts for the platform), before transitioning into
    the normal world.

*   Supports both GICv2 and GICv3 initialization for use by normal world
    software.

*   Starts the normal world at the highest available Exception Level: EL2
    if available, otherwise EL1.

*   Handles SMCs (Secure Monitor Calls) conforming to the [SMC Calling
    Convention PDD] [SMCCC].

*   Handles SMCs relating to the [Power State Coordination Interface PDD] [PSCI]
    for the Secondary CPU Boot and CPU hotplug use-cases.

For a full list of updated functionality and implementation details, please
see the [User Guide]. The [Change Log] provides details of changes made
since the last release.

### Platforms

This release of the Trusted Firmware has been tested on the following ARM
[FVP]s (64-bit versions only):

*   `FVP_Base_AEMv8A-AEMv8A` (Version 5.1 build 8).
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 5.1 build 8).
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 5.1 build 8).

These models can be licensed from ARM: see [www.arm.com/fvp] [FVP]

### Still to Come

*   Complete implementation of the [PSCI] specification.

*   Secure memory, Secure monitor, Test Secure OS & Secure interrupts.

*   Booting the firmware from a block device.

*   Completing the currently experimental GICv3 support.

For a full list of detailed issues in the current code, please see the [Change
Log].


Getting Started
---------------

Get the Trusted Firmware source code from
[GitHub](https://www.github.com/ARM-software/arm-trusted-firmware).

See the [User Guide] for instructions on how to install, build and use
the Trusted Firmware with the ARM [FVP]s.

See the [Porting Guide] as well for information about how to use this
software on another ARMv8 platform.

### Feedback and support

ARM welcomes any feedback on the Trusted Firmware. Please send feedback using
the [GitHub issue tracker](
https://github.com/ARM-software/arm-trusted-firmware/issues).

ARM licensees may contact ARM directly via their partner managers.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013 ARM Ltd. All rights reserved._


[License]:       license.md "BSD license for ARM Trusted Firmware"
[Change Log]:    ./docs/change-log.md
[User Guide]:    ./docs/user-guide.md
[Porting Guide]: ./docs/porting-guide.md

[ARMv8]:         http://www.arm.com/products/processors/armv8-architecture.php "ARMv8 Architecture"
[FVP]:           http://www.arm.com/fvp "ARM's Fixed Virtual Platforms"
[PSCI]:          http://infocenter.arm.com/help/topic/com.arm.doc.den0022b/index.html "Power State Coordination Interface PDD (ARM DEN 0022B.b)"
[SMCCC]:         http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html "SMC Calling Convention PDD (ARM DEN 0028A)"
[TEE-SMC]:       http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php "Secure Monitor and TEEs"
[AArch64 LEB]:   http://releases.linaro.org/13.09/openembedded/aarch64 "Linaro AArch64 OpenEmbedded ARM Fast Model 13.09 Release"
