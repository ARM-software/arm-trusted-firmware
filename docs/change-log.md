
ARM Trusted Firmware - version 1.3
==================================

New features
------------

*   Added support for running Trusted Firmware in AArch32 execution state.

    The PSCI library has been refactored to allow integration with **EL3 Runtime
    Software**. This is software that is executing at the highest secure
    privilege which is EL3 in AArch64 or Secure SVC/Monitor mode in AArch32. See
    [PSCI Integration Guide].

    Included is a minimal AArch32 Secure Payload, **SP-MIN**, that illustrates
    the usage and integration of the PSCI library with EL3 Runtime Software
    running in AArch32 state.

    Booting to the BL1/BL2 images as well as booting straight to the Secure
    Payload is supported.

*   Improvements to the initialization framework for the PSCI service and ARM
    Standard Services in general.

    The PSCI service is now initialized as part of ARM Standard Service
    initialization. This consolidates the initializations of any ARM Standard
    Service that may be added in the future.

    A new function `get_arm_std_svc_args()` is introduced to get arguments
    corresponding to each standard service and must be implemented by the EL3
    Runtime Software.

    For PSCI, a new versioned structure `psci_lib_args_t` is introduced to
    initialize the PSCI Library. **Note** this is a compatibility break due to
    the change in the prototype of `psci_setup()`.

*   To support AArch32 builds of BL1 and BL2, implemented a new, alternative
    firmware image loading mechanism that adds flexibility.

    The current mechanism has a hard-coded set of images and execution order
    (BL31, BL32, etc). The new mechanism is data-driven by a list of image
    descriptors provided by the platform code.

    ARM platforms have been updated to support the new loading mechanism.

    The new mechanism is enabled by a build flag (`LOAD_IMAGE_V2`) which is
    currently off by default for the AArch64 build.

    **Note** `TRUSTED_BOARD_BOOT` is currently not supported when
    `LOAD_IMAGE_V2` is enabled.

*   Updated requirements for making contributions to ARM TF.

    Commits now must have a 'Signed-off-by:' field to certify that the
    contribution has been made under the terms of the
    [Developer Certificate of Origin].

    A signed CLA is no longer required.

    The [Contribution Guide] has been updated to reflect this change.

*   Introduced Performance Measurement Framework (PMF) which provides support
    for capturing, storing, dumping and retrieving time-stamps to measure the
    execution time of critical paths in the firmware. This relies on defining
    fixed sample points at key places in the code.

*   To support the QEMU platform port, imported libfdt v1.4.1 from
    https://git.kernel.org/cgit/utils/dtc/dtc.git

*   Updated PSCI support:

    *   Added support for PSCI NODE_HW_STATE API for ARM platforms.

    *   New optional platform hook, `pwr_domain_pwr_down_wfi()`, in
        `plat_psci_ops` to enable platforms to perform platform-specific actions
        needed to enter powerdown, including the 'wfi' invocation.

    *   PSCI STAT residency and count functions have been added on ARM platforms
        by using PMF.

*   Enhancements to the translation table library:

    *   Limited memory mapping support for region overlaps to only allow regions
        to overlap that are identity mapped or have the same virtual to physical
        address offset, and overlap completely but must not cover the same area.

        This limitation will enable future enhancements without having to
        support complex edge cases that may not be necessary.

    *   The initial translation lookup level is now inferred from the virtual
        address space size. Previously, it was hard-coded.

    *   Added support for mapping Normal, Inner Non-cacheable, Outer
        Non-cacheable memory in the translation table library.

        This can be useful to map a non-cacheable memory region, such as a DMA
        buffer.

    *   Introduced the MT_EXECUTE/MT_EXECUTE_NEVER memory mapping attributes to
        specify the access permissions for instruction execution of a memory
        region.

*   Enabled support to isolate code and read-only data on separate memory pages,
    allowing independent access control to be applied to each.

*   Enabled SCR_EL3.SIF (Secure Instruction Fetch) bit in BL1 and BL31 common
    architectural setup code, preventing fetching instructions from non-secure
    memory when in secure state.

*   Enhancements to FIP support:

    *   Replaced `fip_create` with `fiptool` which provides a more consistent
        and intuitive interface as well as additional support to remove an image
        from a FIP file.

    *   Enabled printing the SHA256 digest with info command, allowing quick
        verification of an image within a FIP without having to extract the
        image and running sha256sum on it.

    *   Added support for unpacking the contents of an existing FIP file into
        the working directory.

    *   Aligned command line options for specifying images to use same naming
        convention as specified by TBBR and already used in cert_create tool.

*   Refactored the TZC-400 driver to also support memory controllers that
    integrate TZC functionality, for example ARM CoreLink DMC-500. Also added
    DMC-500 specific support.

*   Implemented generic delay timer based on the system generic counter and
    migrated all platforms to use it.

*   Enhanced support for ARM platforms:

    *   Updated image loading support to make SCP images (SCP_BL2 and SCP_BL2U)
        optional.

    *   Enhanced topology description support to allow multi-cluster topology
        definitions.

    *   Added interconnect abstraction layer to help platform ports select the
        right interconnect driver, CCI or CCN, for the platform.

    *   Added support to allow loading BL31 in the TZC-secured DRAM instead of
        the default secure SRAM.

    *   Added support to use a System Security Control (SSC) Registers Unit
        enabling ARM TF to be compiled to support multiple ARM platforms and
        then select one at runtime.

    *   Restricted mapping of Trusted ROM in BL1 to what is actually needed by
        BL1 rather than entire Trusted ROM region.

    *   Flash is now mapped as execute-never by default. This increases security
        by restricting the executable region to what is strictly needed.

*   Applied following erratum workarounds for Cortex-A57: 833471, 826977,
    829520, 828024 and 826974.

*   Added support for Mediatek MT6795 platform.

*   Added support for QEMU virtualization ARMv8-A target.

*   Added support for Rockchip RK3368 and RK3399 platforms.

*   Added support for Xilinx Zynq UltraScale+ MPSoC platform.

*   Added support for ARM Cortex-A73 MPCore Processor.

*   Added support for ARM Cortex-A72 processor.

*   Added support for ARM Cortex-A35 processor.

*   Added support for ARM Cortex-A32 MPCore Processor.

*   Enabled preloaded BL33 alternative boot flow, in which BL2 does not load
    BL33 from non-volatile storage and BL31 hands execution over to a preloaded
    BL33. The User Guide has been updated with an example of how to use this
    option with a bootwrapped kernel.

*   Added support to build ARM TF on a Windows-based host machine.

*   Updated Trusted Board Boot prototype implementation:

    *   Enabled the ability for a production ROM with TBBR enabled to boot test
        software before a real ROTPK is deployed (e.g. manufacturing mode).
        Added support to use ROTPK in certificate without verifying against the
        platform value when `ROTPK_NOT_DEPLOYED` bit is set.

    *   Added support for non-volatile counter authentication to the
        Authentication Module to protect against roll-back.

*   Updated GICv3 support:

    *   Enabled processor power-down and automatic power-on using GICv3.

    *   Enabled G1S or G0 interrupts to be configured independently.

    *   Changed FVP default interrupt driver to be the GICv3-only driver.
        **Note** the default build of Trusted Firmware will not be able to boot
        Linux kernel with GICv2 FDT blob.

    *   Enabled wake-up from CPU_SUSPEND to stand-by by temporarily re-routing
        interrupts and then restoring after resume.

Issues resolved since last release
----------------------------------

Known issues
------------

*   The version of the AEMv8 Base FVP used in this release resets the model
    instead of terminating its execution in response to a shutdown request using
    the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of
    the model.

*   Building TF with compiler optimisations disabled (`-O0`) fails.


*   ARM TF cannot be built with mbed TLS version v2.3.0 due to build warnings
    that the ARM TF build system interprets as errors.

*   TBBR is not currently supported when running Trusted Firmware in AArch32
    state.


ARM Trusted Firmware - version 1.2
==================================

New features
------------

*   The Trusted Board Boot implementation on ARM platforms now conforms to the
    mandatory requirements of the TBBR specification.

    In particular, the boot process is now guarded by a Trusted Watchdog, which
    will reset the system in case of an authentication or loading error. On ARM
    platforms, a secure instance of ARM SP805 is used as the Trusted Watchdog.

    Also, a firmware update process has been implemented. It enables
    authenticated firmware to update firmware images from external interfaces to
    SoC Non-Volatile memories. This feature functions even when the current
    firmware in the system is corrupt or missing; it therefore may be used as
    a recovery mode.

*   Improvements have been made to the Certificate Generation Tool
    (`cert_create`) as follows.

    *   Added support for the Firmware Update process by extending the Chain
        of Trust definition in the tool to include the Firmware Update
        certificate and the required extensions.

    *   Introduced a new API that allows one to specify command line options in
        the Chain of Trust description. This makes the declaration of the tool's
        arguments more flexible and easier to extend.

    *   The tool has been reworked to follow a data driven approach, which
        makes it easier to maintain and extend.

*   Extended the FIP tool (`fip_create`) to support the new set of images
    involved in the Firmware Update process.

*   Various memory footprint improvements. In particular:

    *   The bakery lock structure for coherent memory has been optimised.

    *   The mbed TLS SHA1 functions are not needed, as SHA256 is used to
        generate the certificate signature. Therefore, they have been compiled
        out, reducing the memory footprint of BL1 and BL2 by approximately
        6 KB.

    *   On ARM development platforms, each BL stage now individually defines
        the number of regions that it needs to map in the MMU.

*   Added the following new design documents:

    *   [Authentication framework]
    *   [Firmware Update]
    *   [TF Reset Design]
    *   [Power Domain Topology Design]

*   Applied the new image terminology to the code base and documentation, as
    described on the [TF wiki on GitHub][TF Image Terminology].

*   The build system has been reworked to improve readability and facilitate
    adding future extensions.

*   On ARM standard platforms, BL31 uses the boot console during cold boot
    but switches to the runtime console for any later logs at runtime. The TSP
    uses the runtime console for all output.

*   Implemented a basic NOR flash driver for ARM platforms. It programs the
    device using CFI (Common Flash Interface) standard commands.

*   Implemented support for booting EL3 payloads on ARM platforms, which
    reduces the complexity of developing EL3 baremetal code by doing essential
    baremetal initialization.

*   Provided separate drivers for GICv3 and GICv2. These expect the entire
    software stack to use either GICv2 or GICv3; hybrid GIC software systems
    are no longer supported and the legacy ARM GIC driver has been deprecated.

*   Added support for Juno r1 and r2. A single set of Juno TF binaries can run
    on Juno r0, r1 and r2 boards. Note that this TF version depends on a Linaro
    release that does *not* contain Juno r2 support.

*   Added support for MediaTek mt8173 platform.

*   Implemented a generic driver for ARM CCN IP.

*   Major rework of the PSCI implementation.

    *   Added framework to handle composite power states.

    *   Decoupled the notions of affinity instances (which describes the
        hierarchical arrangement of cores) and of power domain topology, instead
        of assuming a one-to-one mapping.

    *   Better alignment with version 1.0 of the PSCI specification.

*   Added support for the SYSTEM_SUSPEND PSCI API on ARM platforms. When invoked
    on the last running core on a supported platform, this puts the system
    into a low power mode with memory retention.

*   Unified the reset handling code as much as possible across BL stages.
    Also introduced some build options to enable optimization of the reset path
    on platforms that support it.

*   Added a simple delay timer API, as well as an SP804 timer driver, which is
    enabled on FVP.

*   Added support for NVidia Tegra T210 and T132 SoCs.

*   Reorganised ARM platforms ports to greatly improve code shareability and
    facilitate the reuse of some of this code by other platforms.

*   Added support for ARM Cortex-A72 processor in the CPU specific framework.

*   Provided better error handling. Platform ports can now define their own
    error handling, for example to perform platform specific bookkeeping or
    post-error actions.

*   Implemented a unified driver for ARM Cache Coherent Interconnects used for
    both CCI-400 & CCI-500 IPs. ARM platforms ports have been migrated to this
    common driver. The standalone CCI-400 driver has been deprecated.


Issues resolved since last release
----------------------------------

*   The Trusted Board Boot implementation has been redesigned to provide greater
    modularity and scalability. See the [Authentication Framework] document.
    All missing mandatory features are now implemented.

*   The FVP and Juno ports may now use the hash of the ROTPK stored in the
    Trusted Key Storage registers to verify the ROTPK. Alternatively, a
    development public key hash embedded in the BL1 and BL2 binaries might be
    used instead. The location of the ROTPK is chosen at build-time using the
    `ARM_ROTPK_LOCATION` build option.

*   GICv3 is now fully supported and stable.


Known issues
------------

*   The version of the AEMv8 Base FVP used in this release resets the model
    instead of terminating its execution in response to a shutdown request using
    the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of
    the model.

*   While this version has low on-chip RAM requirements, there are further
    RAM usage enhancements that could be made.

*   The upstream documentation could be improved for structural consistency,
    clarity and completeness. In particular, the design documentation is
    incomplete for PSCI, the TSP(D) and the Juno platform.

*   Building TF with compiler optimisations disabled (`-O0`) fails.


ARM Trusted Firmware - version 1.1
==================================

New features
------------

*   A prototype implementation of Trusted Board Boot has been added. Boot
    loader images are verified by BL1 and BL2 during the cold boot path. BL1 and
    BL2 use the PolarSSL SSL library to verify certificates and images. The
    OpenSSL library is used to create the X.509 certificates. Support has been
    added to `fip_create` tool to package the certificates in a FIP.

*   Support for calling CPU and platform specific reset handlers upon entry into
    BL3-1 during the cold and warm boot paths has been added. This happens after
    another Boot ROM `reset_handler()` has already run. This enables a developer
    to perform additional actions or undo actions already performed during the
    first call of the reset handlers e.g. apply additional errata workarounds.

*   Support has been added to demonstrate routing of IRQs to EL3 instead of
    S-EL1 when execution is in secure world.

*   The PSCI implementation now conforms to version 1.0 of the PSCI
    specification. All the mandatory APIs and selected optional APIs are
    supported. In particular, support for the `PSCI_FEATURES` API has been
    added. A capability variable is constructed during initialization by
    examining the `plat_pm_ops` and `spd_pm_ops` exported by the platform and
    the Secure Payload Dispatcher.  This is used by the PSCI FEATURES function
    to determine which PSCI APIs are supported by the platform.

*   Improvements have been made to the PSCI code as follows.

    *   The code has been refactored to remove redundant parameters from
        internal functions.

    *   Changes have been made to the code for PSCI `CPU_SUSPEND`, `CPU_ON` and
        `CPU_OFF` calls to facilitate an early return to the caller in case a
        failure condition is detected. For example, a PSCI `CPU_SUSPEND` call
        returns `SUCCESS` to the caller if a pending interrupt is detected early
        in the code path.

    *   Optional platform APIs have been added to validate the `power_state` and
        `entrypoint` parameters early in PSCI `CPU_ON` and `CPU_SUSPEND` code
        paths.

    *   PSCI migrate APIs have been reworked to invoke the SPD hook to determine
        the type of Trusted OS and the CPU it is resident on (if
        applicable). Also, during a PSCI `MIGRATE` call, the SPD hook to migrate
        the Trusted OS is invoked.

*   It is now possible to build Trusted Firmware without marking at least an
    extra page of memory as coherent. The build flag `USE_COHERENT_MEM` can be
    used to choose between the two implementations. This has been made possible
    through these changes.

    *   An implementation of Bakery locks, where the locks are not allocated in
        coherent memory has been added.

    *   Memory which was previously marked as coherent is now kept coherent
        through the use of software cache maintenance operations.

    Approximately, 4K worth of memory is saved for each boot loader stage when
    `USE_COHERENT_MEM=0`. Enabling this option increases the latencies
    associated with acquire and release of locks. It also requires changes to
    the platform ports.

*   It is now possible to specify the name of the FIP at build time by defining
    the `FIP_NAME` variable.

*   Issues with depedencies on the 'fiptool' makefile target have been
    rectified. The `fip_create` tool is now rebuilt whenever its source files
    change.

*   The BL3-1 runtime console is now also used as the crash console. The crash
    console is changed to SoC UART0 (UART2) from the previous FPGA UART0 (UART0)
    on Juno. In FVP, it is changed from UART0 to UART1.

*   CPU errata workarounds are applied only when the revision and part number
    match. This behaviour has been made consistent across the debug and release
    builds. The debug build additionally prints a warning if a mismatch is
    detected.

*   It is now possible to issue cache maintenance operations by set/way for a
    particular level of data cache. Levels 1-3 are currently supported.

*   The following improvements have been made to the FVP port.

    *   The build option `FVP_SHARED_DATA_LOCATION` which allowed relocation of
        shared data into the Trusted DRAM has been deprecated. Shared data is
        now always located at the base of Trusted SRAM.

    *   BL2 Translation tables have been updated to map only the region of
        DRAM which is accessible to normal world. This is the region of the 2GB
        DDR-DRAM memory at 0x80000000 excluding the top 16MB. The top 16MB is
        accessible to only the secure world.

    *   BL3-2 can now reside in the top 16MB of DRAM which is accessible only to
        the secure world. This can be done by setting the build flag
        `FVP_TSP_RAM_LOCATION` to the value `dram`.

*   Separate transation tables are created for each boot loader image. The
    `IMAGE_BLx` build options are used to do this.  This allows each stage to
    create mappings only for areas in the memory map that it needs.

*   A Secure Payload Dispatcher (OPTEED) for the OP-TEE Trusted OS has been
    added.  Details of using it with ARM Trusted Firmware can be found in
    [OP-TEE Dispatcher]



Issues resolved since last release
----------------------------------

*   The Juno port has been aligned with the FVP port as follows.

    *   Support for reclaiming all BL1 RW memory and BL2 memory by overlaying
        the BL3-1/BL3-2 NOBITS sections on top of them has been added to the
        Juno port.

    *   The top 16MB of the 2GB DDR-DRAM memory at 0x80000000 is configured
        using the TZC-400 controller to be accessible only to the secure world.

    *   The ARM GIC driver is used to configure the GIC-400 instead of using a
        GIC driver private to the Juno port.

    *   PSCI `CPU_SUSPEND` calls that target a standby state are now supported.

    *   The TZC-400 driver is used to configure the controller instead of direct
        accesses to the registers.

*   The Linux kernel version referred to in the user guide has DVFS and HMP
    support enabled.

*   DS-5 v5.19 did not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in
    CADI server mode. This issue is not seen with DS-5 v5.20 and Version 6.2 of
    the Cortex-A57-A53 Base FVPs.


Known issues
------------

*   The Trusted Board Boot implementation is a prototype. There are issues with
    the modularity and scalability of the design. Support for a Trusted
    Watchdog, firmware update mechanism, recovery images and Trusted debug is
    absent. These issues will be addressed in future releases.

*   The FVP and Juno ports do not use the hash of the ROTPK stored in the
    Trusted Key Storage registers to verify the ROTPK in the
    `plat_match_rotpk()` function. This prevents the correct establishment of
    the Chain of Trust at the first step in the Trusted Board Boot process.

*   The version of the AEMv8 Base FVP used in this release resets the model
    instead of terminating its execution in response to a shutdown request using
    the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of
    the model.

*   GICv3 support is experimental. There are known issues with GICv3
    initialization in the ARM Trusted Firmware.

*   While this version greatly reduces the on-chip RAM requirements, there are
    further RAM usage enhancements that could be made.

*   The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
    its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

*   The Juno-specific firmware design documentation is incomplete.


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

_Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved._

[OP-TEE Dispatcher]:                  optee-dispatcher.md
[Power Domain Topology Design]:       psci-pd-tree.md
[TF Image Terminology]:               https://github.com/ARM-software/arm-trusted-firmware/wiki/Trusted-Firmware-Image-Terminology
[Authentication Framework]:           auth-framework.md
[Firmware Update]:                    firmware-update.md
[TF Reset Design]:                    reset-design.md
[PSCI Integration Guide]:             psci-lib-integration-guide.md
[Firmware Design]:                    firmware-design.md
[CPU Specific Build Macros]:          cpu-specific-build-macros.md
[User Guide]:                         user-guide.md
[Porting Guide]:                      porting-guide.md
[Developer Certificate of Origin]:    ../dco.txt
[Contribution Guide]:                 ../contributing.md
