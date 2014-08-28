ARM Trusted Firmware - version 1.0
==================================

New features
------------

*   It is now possible to map higher physical addresses using non-flat virtual
    to physical address mappings in the MMU setup.

*   Wider use is now made of the per-CPU data cache in BL3-1 to store:

    *   Pointers to the non-secure and secure security state contexts.

    *   A pointer to the CPU-specific operations.

    *   A pointer to PSCI specific information (for example the current power
        state).

    *   A crash reporting buffer.

*   The following RAM usage improvements result in a BL3-1 RAM usage reduction
    from 96KB to 56KB (for FVP with TSPD), and a total RAM usage reduction
    across all images from 208KB to 88KB, compared to the previous release.

    *   Removed the separate `early_exception` vectors from BL3-1 (2KB code size
        saving).

    *   Removed NSRAM from the FVP memory map, allowing the removal of one
        (4KB) translation table.

    *   Eliminated the internal `psci_suspend_context` array, saving 2KB.

    *   Correctly dimensioned the PSCI `aff_map_node` array, saving 1.5KB in the
        FVP port.

    *   Removed calling CPU mpidr from the bakery lock API, saving 160 bytes.

    *   Removed current CPU mpidr from PSCI common code, saving 160 bytes.

    *   Inlined the mmio accessor functions, saving 360 bytes.

    *   Fully reclaimed all BL1 RW memory and BL2 memory on the FVP port by
        overlaying the BL3-1/BL3-2 NOBITS sections on top of these at runtime.

    *   Made storing the FP register context optional, saving 0.5KB per context
        (8KB on the FVP port, with TSPD enabled and running on 8 CPUs).

    *   Implemented a leaner `tf_printf()` function, allowing the stack to be
        greatly reduced.

    *   Removed coherent stacks from the codebase. Stacks allocated in normal
        memory are now used before and after the MMU is enabled. This saves 768
        bytes per CPU in BL3-1.

    *   Reworked the crash reporting in BL3-1 to use less stack.

    *   Optimized the EL3 register state stored in the `cpu_context` structure
        so that registers that do not change during normal execution are
        re-initialized each time during cold/warm boot, rather than restored
        from memory. This saves about 1.2KB.

    *   As a result of some of the above, reduced the runtime stack size in all
        BL images. For BL3-1, this saves 1KB per CPU.

*   PSCI SMC handler improvements to correctly handle calls from secure states
    and from AArch32.

*   CPU contexts are now initialized from the `entry_point_info`. BL3-1 fully
    determines the exception level to use for the non-trusted firmware (BL3-3)
    based on the SPSR value provided by the BL2 platform code (or otherwise
    provided to BL3-1). This allows platform code to directly run non-trusted
    firmware payloads at either EL2 or EL1 without requiring an EL2 stub or OS
    loader.

*   Code refactoring improvements:

    *   Refactored `fvp_config` into a common platform header.

    *   Refactored the fvp gic code to be a generic driver that no longer has an
        explicit dependency on platform code.

    *   Refactored the CCI-400 driver to not have dependency on platform code.

    *   Simplified the IO driver so it's no longer necessary to call `io_init()`
        and moved all the IO storage framework code to one place.

    *   Simplified the interface the the TZC-400 driver.

    *   Clarified the platform porting interface to the TSP.

    *   Reworked the TSPD setup code to support the alternate BL3-2
        intialization flow where BL3-1 generic code hands control to BL3-2,
        rather than expecting the TSPD to hand control directly to BL3-2.

    *   Considerable rework to PSCI generic code to support CPU specific
        operations.

*   Improved console log output, by:

    *   Adding the concept of debug log levels.

    *   Rationalizing the existing debug messages and adding new ones.

    *   Printing out the version of each BL stage at runtime.

    *   Adding support for printing console output from assembler code,
        including when a crash occurs before the C runtime is initialized.

*   Moved up to the latest versions of the FVPs, toolchain, EDK2, kernel, Linaro
    file system and DS-5.

*   On the FVP port, made the use of the Trusted DRAM region optional at build
    time (off by default). Normal platforms will not have such a "ready-to-use"
    DRAM area so it is not a good example to use it.

*   Added support for PSCI `SYSTEM_OFF` and `SYSTEM_RESET` APIs.

*   Added support for CPU specific reset sequences, power down sequences and
    register dumping during crash reporting. The CPU specific reset sequences
    include support for errata workarounds.

*   Merged the Juno port into the master branch. Added support for CPU hotplug
    and CPU idle. Updated the user guide to describe how to build and run on the
    Juno platform.


Issues resolved since last release
----------------------------------

*   Removed the concept of top/bottom image loading. The image loader now
    automatically detects the position of the image inside the current memory
    layout and updates the layout to minimize fragementation. This resolves the
    image loader limitations of previously releases. There are currently no
    plans to support dynamic image loading.

*   CPU idle now works on the publicized version of the Foundation FVP.

*   All known issues relating to the compiler version used have now been
    resolved. This TF version uses Linaro toolchain 14.07 (based on GCC 4.9).


Known issues
------------

*   GICv3 support is experimental. The Linux kernel patches to support this are
    not widely available. There are known issues with GICv3 initialization in
    the ARM Trusted Firmware.

*   While this version greatly reduces the on-chip RAM requirements, there are
    further RAM usage enhancements that could be made.

*   The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
    its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

*   The Juno-specific firmware design documentation is incomplete.

*   Some recent enhancements to the FVP port have not yet been translated into
    the Juno port. These will be tracked via the tf-issues project.

*   The Linux kernel version referred to in the user guide has DVFS and HMP
    support disabled due to some known instabilities at the time of this
    release. A future kernel version will re-enable these features.

*   DS-5 v5.19 does not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in
    CADI server mode. This is because the `<SimName>` reported by the FVP in
    this version has changed. For example, for the Cortex-A57x4-A53x4 Base FVP,
    the `<SimName>` reported by the FVP is `FVP_Base_Cortex_A57x4_A53x4`, while
    DS-5 expects it to be `FVP_Base_A57x4_A53x4`.

    The temporary fix to this problem is to change the name of the FVP in
    `sw/debugger/configdb/Boards/ARM FVP/Base_A57x4_A53x4/cadi_config.xml`.
    Change the following line:

        <SimName>System Generator:FVP_Base_A57x4_A53x4</SimName>
    to
        <SimName>System Generator:FVP_Base_Cortex-A57x4_A53x4</SimName>

    A similar change can be made to the other Cortex-A57-A53 Base FVP variants.


ARM Trusted Firmware - version 0.4
==================================

New features
------------

*   Makefile improvements:

    *   Improved dependency checking when building.

    *   Removed `dump` target (build now always produces dump files).

    *   Enabled platform ports to optionally make use of parts of the Trusted
        Firmware (e.g. BL3-1 only), rather than being forced to use all parts.
        Also made the `fip` target optional.

    *   Specified the full path to source files and removed use of the `vpath`
        keyword.

*   Provided translation table library code for potential re-use by platforms
    other than the FVPs.

*   Moved architectural timer setup to platform-specific code.

*   Added standby state support to PSCI cpu_suspend implementation.

*   SRAM usage improvements:

    *   Started using the `-ffunction-sections`, `-fdata-sections` and
        `--gc-sections` compiler/linker options to remove unused code and data
        from the images. Previously, all common functions were being built into
        all binary images, whether or not they were actually used.

    *   Placed all assembler functions in their own section to allow more unused
        functions to be removed from images.

    *   Updated BL1 and BL2 to use a single coherent stack each, rather than one
        per CPU.

    *   Changed variables that were unnecessarily declared and initialized as
        non-const (i.e. in the .data section) so they are either uninitialized
        (zero init) or const.

*   Moved the Test Secure-EL1 Payload (BL3-2) to execute in Trusted SRAM by
    default. The option for it to run in Trusted DRAM remains.

*   Implemented a TrustZone Address Space Controller (TZC-400) driver. A
    default configuration is provided for the Base FVPs. This means the model
    parameter `-C bp.secure_memory=1` is now supported.

*   Started saving the PSCI cpu_suspend 'power_state' parameter prior to
    suspending a CPU. This allows platforms that implement multiple power-down
    states at the same affinity level to identify a specific state.

*   Refactored the entire codebase to reduce the amount of nesting in header
    files and to make the use of system/user includes more consistent. Also
    split platform.h to separate out the platform porting declarations from the
    required platform porting definitions and the definitions/declarations
    specific to the platform port.

*   Optimized the data cache clean/invalidate operations.

*   Improved the BL3-1 unhandled exception handling and reporting. Unhandled
    exceptions now result in a dump of registers to the console.

*   Major rework to the handover interface between BL stages, in particular the
    interface to BL3-1. The interface now conforms to a specification and is
    more future proof.

*   Added support for optionally making the BL3-1 entrypoint a reset handler
    (instead of BL1). This allows platforms with an alternative image loading
    architecture to re-use BL3-1 with fewer modifications to generic code.

*   Reserved some DDR DRAM for secure use on FVP platforms to avoid future
    compatibility problems with non-secure software.

*   Added support for secure interrupts targeting the Secure-EL1 Payload (SP)
    (using GICv2 routing only). Demonstrated this working by adding an interrupt
    target and supporting test code to the TSP. Also demonstrated non-secure
    interrupt handling during TSP processing.


Issues resolved since last release
----------------------------------

*   Now support use of the model parameter `-C bp.secure_memory=1` in the Base
    FVPs (see **New features**).

*   Support for secure world interrupt handling now available (see **New
    features**).

*   Made enough SRAM savings (see **New features**) to enable the Test Secure-EL1
    Payload (BL3-2) to execute in Trusted SRAM by default.

*   The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
    14.04) now correctly reports progress in the console.

*   Improved the Makefile structure to make it easier to separate out parts of
    the Trusted Firmware for re-use in platform ports. Also, improved target
    dependency checking.


Known issues
------------

*   GICv3 support is experimental. The Linux kernel patches to support this are
    not widely available. There are known issues with GICv3 initialization in
    the ARM Trusted Firmware.

*   Dynamic image loading is not available yet. The current image loader
    implementation (used to load BL2 and all subsequent images) has some
    limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
    to loading errors, even if the images should theoretically fit in memory.

*   The ARM Trusted Firmware still uses too much on-chip Trusted SRAM. A number
    of RAM usage enhancements have been identified to rectify this situation.

*   CPU idle does not work on the advertised version of the Foundation FVP.
    Some FVP fixes are required that are not available externally at the time
    of writing. This can be worked around by disabling CPU idle in the Linux
    kernel.

*   Various bugs in ARM Trusted Firmware, UEFI and the Linux kernel have been
    observed when using Linaro toolchain versions later than 13.11. Although
    most of these have been fixed, some remain at the time of writing. These
    mainly seem to relate to a subtle change in the way the compiler converts
    between 64-bit and 32-bit values (e.g. during casting operations), which
    reveals previously hidden bugs in client code.

*   The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
    its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.


ARM Trusted Firmware - version 0.3
==================================

New features
------------

*   Support for Foundation FVP Version 2.0 added.
    The documented UEFI configuration disables some devices that are unavailable
    in the Foundation FVP, including MMC and CLCD. The resultant UEFI binary can
    be used on the AEMv8 and Cortex-A57-A53 Base FVPs, as well as the Foundation
    FVP.

    NOTE: The software will not work on Version 1.0 of the Foundation FVP.

*   Enabled third party contributions. Added a new contributing.md containing
    instructions for how to contribute and updated copyright text in all files
    to acknowledge contributors.

*   The PSCI CPU_SUSPEND API has been stabilised to the extent where it can be
    used for entry into power down states with the following restrictions:
    -   Entry into standby states is not supported.
    -   The API is only supported on the AEMv8 and Cortex-A57-A53 Base FVPs.

*   The PSCI AFFINITY_INFO api has undergone limited testing on the Base FVPs to
    allow experimental use.

*   Required C library and runtime header files are now included locally in ARM
    Trusted Firmware instead of depending on the toolchain standard include
    paths. The local implementation has been cleaned up and reduced in scope.

*   Added I/O abstraction framework, primarily to allow generic code to load
    images in a platform-independent way. The existing image loading code has
    been reworked to use the new framework. Semi-hosting and NOR flash I/O
    drivers are provided.

*   Introduced Firmware Image Package (FIP) handling code and tools. A FIP
    combines multiple firmware images with a Table of Contents (ToC) into a
    single binary image. The new FIP driver is another type of I/O driver. The
    Makefile builds a FIP by default and the FVP platform code expect to load a
    FIP from NOR flash, although some support for image loading using semi-
    hosting is retained.

    NOTE: Building a FIP by default is a non-backwards-compatible change.

    NOTE: Generic BL2 code now loads a BL3-3 (non-trusted firmware) image into
    DRAM instead of expecting this to be pre-loaded at known location. This is
    also a non-backwards-compatible change.

    NOTE: Some non-trusted firmware (e.g. UEFI) will need to be rebuilt so that
    it knows the new location to execute from and no longer needs to copy
    particular code modules to DRAM itself.

*   Reworked BL2 to BL3-1 handover interface. A new composite structure
    (bl31_args) holds the superset of information that needs to be passed from
    BL2 to BL3-1, including information on how handover execution control to
    BL3-2 (if present) and BL3-3 (non-trusted firmware).

*   Added library support for CPU context management, allowing the saving and
    restoring of
    -   Shared system registers between Secure-EL1 and EL1.
    -   VFP registers.
    -   Essential EL3 system registers.

*   Added a framework for implementing EL3 runtime services. Reworked the PSCI
    implementation to be one such runtime service.

*   Reworked the exception handling logic, making use of both SP_EL0 and SP_EL3
    stack pointers for determining the type of exception, managing general
    purpose and system register context on exception entry/exit, and handling
    SMCs. SMCs are directed to the correct EL3 runtime service.

*   Added support for a Test Secure-EL1 Payload (TSP) and a corresponding
    Dispatcher (TSPD), which is loaded as an EL3 runtime service. The TSPD
    implements Secure Monitor functionality such as world switching and
    EL1 context management, and is responsible for communication with the TSP.
    NOTE: The TSPD does not yet contain support for secure world interrupts.
    NOTE: The TSP/TSPD is not built by default.


Issues resolved since last release
----------------------------------

*   Support has been added for switching context between secure and normal
    worlds in EL3.

*   PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` have now been tested (to
    a limited extent).

*   The ARM Trusted Firmware build artifacts are now placed in the `./build`
    directory and sub-directories instead of being placed in the root of the
    project.

*   The ARM Trusted Firmware is now free from build warnings. Build warnings
    are now treated as errors.

*   The ARM Trusted Firmware now provides C library support locally within the
    project to maintain compatibility between toolchains/systems.

*   The PSCI locking code has been reworked so it no longer takes locks in an
    incorrect sequence.

*   The RAM-disk method of loading a Linux file-system has been confirmed to
    work with the ARM Trusted Firmware and Linux kernel version (based on
    version 3.13) used in this release, for both Foundation and Base FVPs.


Known issues
------------

The following is a list of issues which are expected to be fixed in the future
releases of the ARM Trusted Firmware.

*   The TrustZone Address Space Controller (TZC-400) is not being programmed
    yet. Use of model parameter `-C bp.secure_memory=1` is not supported.

*   No support yet for secure world interrupt handling.

*   GICv3 support is experimental. The Linux kernel patches to support this are
    not widely available. There are known issues with GICv3 initialization in
    the ARM Trusted Firmware.

*   Dynamic image loading is not available yet. The current image loader
    implementation (used to load BL2 and all subsequent images) has some
    limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
    to loading errors, even if the images should theoretically fit in memory.

*   The ARM Trusted Firmware uses too much on-chip Trusted SRAM. Currently the
    Test Secure-EL1 Payload (BL3-2) executes in Trusted DRAM since there is not
    enough SRAM. A number of RAM usage enhancements have been identified to
    rectify this situation.

*   CPU idle does not work on the advertised version of the Foundation FVP.
    Some FVP fixes are required that are not available externally at the time
    of writing.

*   Various bugs in ARM Trusted Firmware, UEFI and the Linux kernel have been
    observed when using Linaro toolchain versions later than 13.11. Although
    most of these have been fixed, some remain at the time of writing. These
    mainly seem to relate to a subtle change in the way the compiler converts
    between 64-bit and 32-bit values (e.g. during casting operations), which
    reveals previously hidden bugs in client code.

*   The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
    14.01) does not report progress correctly in the console. It only seems to
    produce error output, not standard output. It otherwise appears to function
    correctly. Other filesystem versions on the same software stack do not
    exhibit the problem.

*   The Makefile structure doesn't make it easy to separate out parts of the
    Trusted Firmware for re-use in platform ports, for example if only BL3-1 is
    required in a platform port. Also, dependency checking in the Makefile is
    flawed.

*   The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
    its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.


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


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._
