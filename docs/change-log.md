ARM Trusted Firmware
====================

Detailed changes since last release
-----------------------------------

*   Support for Foundation FVP Version 2.0 added.
    The documented UEFI configuration disables some devices that are unavailable
    in the Foundation FVP, including MMC and CLCD. The resultant UEFI binary can
    be used on the AEMv8 and Cortex-A57-A53 FVPs, as well as the Foundation FVP.
    NOTE: The software will not work on Version 1.0 of the Foundation FVP.

*   Regression-checked against latest Base FVPs (Version 5.2).

*   The supplied FDTs expose the Interrupt Translation Service (ITS) available
    in GICv3.

*   Fixed various GCC compiler warnings.

*   Unmask SError and Debug exceptions in the trusted firmware.
    Also route external abort and SError interrupts to EL3.

*   The amount of physical RAM available to Linux as specified in the FDTs for
    Base FVPs have been increased from 2GB to 4GB. This resolves the issue of
    failing to start user-space when using a RAM-disk file-system.

*   Build products are now created in a separate build directory tree.

*   Analyze at link-time whether bootloader images will fit in memory and won't
    overlap each other at run time. If it is not the case then image linking
    will now fail.

*   Reduce the size of the bootloader images by cutting some sections out of
    their disk images and allocating them at load time, whenever possible.

*   Properly initialise the C runtime environment. C code can now safely assume
    that global variables are initialised to 0 and that initialised data holds
    the correct value.

*   General changes on the memory layout: some sections have been moved, some of
    them have been merged together, and some alignment constraints on sections
    have changed.

ARM Trusted Firmware - version 0.2
==================================

New features
------------

*   First source release.

*   Code for the PSCI suspend feature is supplied, although this is not enabled
    by default since there are known issues (see below).


Issues resolved since last release
----------------------------------

*   The "psci" nodes in the FDTs provided in this release now fully comply
    with the recommendations made in the PSCI specification.


Known issues
------------

The following is a list of issues which are expected to be fixed in the future
releases of the ARM Trusted Firmware.

*   The TrustZone Address Space Controller (TZC-400) is not being programmed
    yet. Use of model parameter `-C bp.secure_memory=1` is not supported.

*   No support yet for secure world interrupt handling or for switching context
    between secure and normal worlds in EL3.

*   GICv3 support is experimental. The Linux kernel patches to support this are
    not widely available. There are known issues with GICv3 initialization in
    the ARM Trusted Firmware.

*   Dynamic image loading is not available yet. The current image loader
    implementation (used to load BL2 and all subsequent images) has some
    limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
    to loading errors, even if the images should theoretically fit in memory.

*   Although support for PSCI `CPU_SUSPEND` is present, it is not yet stable
    and ready for use.

*   PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` are implemented but have not
    been tested.

*   The ARM Trusted Firmware make files result in all build artifacts being
    placed in the root of the project. These should be placed in appropriate
    sub-directories.

*   The compilation of ARM Trusted Firmware is not free from compilation
    warnings. Some of these warnings have not been investigated yet so they
    could mask real bugs.

*   The ARM Trusted Firmware currently uses toolchain/system include files like
    stdio.h. It should provide versions of these within the project to maintain
    compatibility between toolchains/systems.

*   The PSCI code takes some locks in an incorrect sequence. This may cause
    problems with suspend and hotplug in certain conditions.

*   The Linux kernel used in this release is based on version 3.12-rc4. Using
    this kernel with the ARM Trusted Firmware fails to start the file-system as
    a RAM-disk. It fails to execute user-space `init` from the RAM-disk. As an
    alternative, the VirtioBlock mechanism can be used to provide a file-system
    to the kernel.


Detailed changes since last release
-----------------------------------

First source release – not applicable.

- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013 ARM Ltd. All rights reserved._
