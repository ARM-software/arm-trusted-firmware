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

*   Enable third party contributions. Add a new contributing.md containing
    instructions for how to contribute and update copyright text in all files to
    acknowledge contributors.

*   The wake up enable bit in the FVP power controller is cleared when a cpu is
    physically powered up to prevent a spurious wake up from a subsequent cpu
    off state.

*   Definitions of some constants related to the PSCI api calls AFFINITY_INFO
    and CPU_SUSPEND have been corrected.

*   A bug which triggered an error condition in the code executed after a cpu
    is powered on, if a non zero context id parameter was passed in the PSCI
    CPU_ON and CPU_SUSPEND api calls has been corrected.

*   A restriction in the FVP code which did not allow the non-secure entrypoint
    to lie outside the DRAM has been removed.

*   The PSCI CPU_SUSPEND api has been stabilised to an extent where it can be
    used for entry into power down states with the following restrictions:
    -   Entry into standby states is not supported.
    -   The api is only supported on the AEMv8 Base FVP.

*   The PSCI AFFINITY_INFO api has undergone limited testing on the AEMv8 Base
    FVP to allow experimental use.

*   Locks corresponding to each affinity level are acquired and released in
    the correct sequence in the PSCI implementation. Invocation of the PSCI
    CPU_SUSPEND and CPU_OFF apis simultaneously across cpus & clusters should
    not result in unexpected behaviour.

*   The API to return the memory layout structures for each bootloader stage has
    undergone change. A pointer to these structures is returned instead of their
    copy.

*   Required C library and runtime header files are now included locally in ARM
    Trusted Firmware instead of depending on the toolchain standard include
    paths. The local implementation has been cleaned up and reduced in scope.
    Implementations for `putchar()` and `strchr()` were added to the local C
    library.

*   GCC compiler built-in function support has been disabled in order to improve
    compiler independence.

*   The references to GitHub issues in the documentation now to point to a
    separate issue tracking repository
    https://github.com/ARM-software/tf-issues.

*   Cleared bits in the architectural trap feature register (CPTR_EL3) during
    early boot to prevent traps when accessing certain registers, including
    floating point registers. Also added `-mgeneral-regs-only` flag to GCC
    settings to prevent generation of code using floating point registers.

*   The GICv3 distributor can have more ports than CPUs are available in the
    system. The GICv3 re-distributors are probed to work out which
    re-distributor should be used with which CPU.

*   Add multi-platform support to the build system. The user may now specify
    which platform to build using PLAT=<platform> as part of the make command
    line.  Default behaviour is to make all platforms.  New platforms are
    automatically detected by the make file when they are added to the plat
    directory.

*   An issue in the PSCI implementation has been fixed which could result in the
    power down of an affinity instance at level X even though at least one
    affinity instance at level X - 1 does not allow this.


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

First source release - not applicable.

- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._
