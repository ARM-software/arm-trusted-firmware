ARM Trusted Firmware - version 0.4
==================================

ARM Trusted Firmware provides a reference implementation of secure world
software for [ARMv8-A], including Exception Level 3 (EL3) software. This
release focuses on support for ARM's [Fixed Virtual Platforms (FVPs)] [FVP].

The intent is to provide a reference implementation of various ARM interface
standards, such as the Power State Coordination Interface ([PSCI]), Trusted
Board Boot Requirements (TBBR) and [Secure Monitor] [TEE-SMC] code. As far as
possible the code is designed for reuse or porting to other ARMv8-A model and
hardware platforms.

This release builds on previous source code releases, supporting the Base and
Foundation FVP platform models from ARM.

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

This release is a limited functionality implementation of the Trusted Firmware.
It provides a suitable starting point for productization. Future versions will
contain new features, optimizations and quality improvements.

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
    functionality such as world switching, EL1 context management and interrupt
    routing. This also demonstrates Secure-EL1 interaction with PSCI. Some of
    this functionality is provided in library form for re-use by other
    Secure-EL1 Payload Dispatchers.

*   Support for alternative Trusted Boot Firmware. Some platforms have their own
    Trusted Boot implementation and only require the Secure Monitor
    functionality provided by ARM Trusted Firmware.

*   Isolation of memory accessible by the secure world from the normal world
    through programming of a TrustZone controller.

For a full description of functionality and implementation details, please
see the [Firmware Design] and supporting documentation. The [Change Log]
provides details of changes made since the last release.

### Platforms

This release of the Trusted Firmware has been tested on the following ARM
[FVP]s (64-bit versions only):

*   `Foundation_v8` (Version 2.0, Build 0.8.5206)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 5.6, Build 0.8.5602)

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from ARM: see [www.arm.com/fvp] [FVP].

### Still to Come

*   Support for ARMv8-A development board as a reference platform.

*   Complete Trusted Boot implementation.

*   Complete implementation of the [PSCI] specification.

*   Support for alternative types of Secure-EL1 Payloads.

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
project and the [Acknowledgments] file for a list of contributors to the
project.

### Feedback and support

ARM welcomes any feedback on the Trusted Firmware. Please send feedback using
the [GitHub issue tracker].

ARM licensees may contact ARM directly via their partner managers.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[License]:                  ./license.md "BSD license for ARM Trusted Firmware"
[Contributing Guidelines]:  ./contributing.md "Guidelines for contributors"
[Acknowledgments]:          ./acknowledgements.md "Contributor acknowledgments"
[Change Log]:               ./docs/change-log.md
[User Guide]:               ./docs/user-guide.md
[Firmware Design]:          ./docs/firmware-design.md
[Porting Guide]:            ./docs/porting-guide.md

[ARMv8-A]:               http://www.arm.com/products/processors/armv8-architecture.php "ARMv8-A Architecture"
[FVP]:                   http://www.arm.com/fvp "ARM's Fixed Virtual Platforms"
[PSCI]:                  http://infocenter.arm.com/help/topic/com.arm.doc.den0022b/index.html "Power State Coordination Interface PDD (ARM DEN 0022B.b)"
[SMCCC]:                 http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html "SMC Calling Convention PDD (ARM DEN 0028A)"
[TEE-SMC]:               http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php "Secure Monitor and TEEs"
[GitHub issue tracker]:  https://github.com/ARM-software/tf-issues/issues
