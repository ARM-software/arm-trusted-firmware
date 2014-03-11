ARM Trusted Firmware - version 0.3
==================================

TEST CHANGE 2

ARM Trusted Firmware provides a reference implementation of secure world
software for [ARMv8-A], including Exception Level 3 (EL3) software. This
release focuses on support for ARM's [Fixed Virtual Platforms (FVPs)] [FVP].

The intent is to provide a reference implementation of various ARM interface
standards, such as the Power State Coordination Interface ([PSCI]), Trusted
Board Boot Requirements (TBBR) and [Secure Monitor] [TEE-SMC] code. As far as
possible the code is designed for reuse or porting to other ARMv8-A model and
hardware platforms.

This release builds on the previous source code release, which has been
available in source and binary form since the [Linaro AArch64 OpenEmbedded 13.11
Engineering Build] [AArch64 LEB]. These support the Base FVP platform
models from ARM.

ARM will continue development in collaboration with interested parties to
provide a full reference implementation of PSCI, TBBR and Secure Monitor code
to the benefit of all developers working with ARMv8-A TrustZone software.


License
-------

The software is provided under a BSD 3-Clause [license]. Certain source files
are derived from FreeBSD code: the original license is included in these
source files.


This Release
------------

This release is an incomplete implementation of the Trusted Firmware. Only
limited functionality is provided at present and it has not been optimized or
subjected to extended robustness or stress testing.

### Functionality

*   Initial implementation of a subset of the Trusted Board Boot Requirements
    Platform Design Document (PDD). This includes packaging the various firmware
    images into a Firmware Image Package (FIP) to be loaded from non-volatile
    storage.

*   Initializes the secure world (for example, exception vectors, control
    registers, GIC and interrupts for the platform), before transitioning into
    the normal world.

*   Supports both GICv2 and GICv3 initialization for use by normal world
    software.

*   Starts the normal world at the highest available Exception Level: EL2
    if available, otherwise EL1.

*   Handles SMCs (Secure Monitor Calls) conforming to the [SMC Calling
    Convention PDD] [SMCCC] using an EL3 runtime services framework.

*   Handles SMCs relating to the [Power State Coordination Interface PDD] [PSCI]
    for the Secondary CPU Boot, CPU hotplug and CPU idle use-cases.

*   A Test Secure-EL1 Payload and Dispatcher to demonstrate Secure Monitor
    functionality such as world switching and EL1 context management. This
    also demonstrates Secure-EL1 interaction with PSCI. Some of this
    functionality is provided in library form for re-use by other Secure-EL1
    Payload Dispatchers.

For a full list of updated functionality and implementation details, please
see the [User Guide]. The [Change Log] provides details of changes made
since the last release.

### Platforms

This release of the Trusted Firmware has been tested on the following ARM
[FVP]s (64-bit versions only):

*   `Foundation_v8` (Version 2.0, Build 0.8.5206)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 5.4, Build 0.8.5405)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 5.4, Build 0.8.5405)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 5.4, Build 0.8.5405)

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from ARM: see [www.arm.com/fvp] [FVP].

### Still to Come

*   Complete implementation of the [PSCI] specification.

*   Secure memory, Secure interrupts and support for other types of Secure-EL1
    Payloads.

*   Booting the firmware from a Virtio block device.

*   Completing the currently experimental GICv3 support.

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
project and the [Acknowledgements] file for a list of contributors to the
project.

### Feedback and support

ARM welcomes any feedback on the Trusted Firmware. Please send feedback using
the [GitHub issue tracker].

ARM licensees may contact ARM directly via their partner managers.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[License]:                  ./license.md "BSD license for ARM Trusted Firmware"
[Contributing Guidelines]:  ./contributing.md "Guidelines for contributors"
[Acknowledgements]:         ./acknowledgements.md "Contributor acknowledgements"
[Change Log]:               ./docs/change-log.md
[User Guide]:               ./docs/user-guide.md
[Firmware Design]:          ./docs/firmware-design.md
[Porting Guide]:            ./docs/porting-guide.md

[ARMv8-A]:               http://www.arm.com/products/processors/armv8-architecture.php "ARMv8-A Architecture"
[FVP]:                   http://www.arm.com/fvp "ARM's Fixed Virtual Platforms"
[PSCI]:                  http://infocenter.arm.com/help/topic/com.arm.doc.den0022b/index.html "Power State Coordination Interface PDD (ARM DEN 0022B.b)"
[SMCCC]:                 http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html "SMC Calling Convention PDD (ARM DEN 0028A)"
[TEE-SMC]:               http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php "Secure Monitor and TEEs"
[AArch64 LEB]:           http://releases.linaro.org/13.11/openembedded/aarch64 "Linaro AArch64 OpenEmbedded ARM Fast Model 13.11 Release"
[GitHub issue tracker]:  https://github.com/ARM-software/tf-issues/issues
