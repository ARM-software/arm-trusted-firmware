Change Log & Release Notes
==========================

This document contains a summary of the new features, changes, fixes and known
issues in each release of Trusted Firmware-A.

Version 2.2
-----------

New Features
^^^^^^^^^^^^

- Architecture
   - Enable Pointer Authentication (PAuth) support for Secure World
       - Adds support for ARMv8.3-PAuth in BL1 SMC calls and
         BL2U image for firmware updates.

   - Enable Memory Tagging Extension (MTE) support in both secure and non-secure
     worlds
       - Adds support for the new Memory Tagging Extension arriving in
         ARMv8.5. MTE support is now enabled by default on systems that
         support it at EL0.
       - To enable it at ELx for both the non-secure and the secure
         world, the compiler flag ``CTX_INCLUDE_MTE_REGS`` includes register
         saving and restoring when necessary in order to prevent information
         leakage between the worlds.

   - Add support for Branch Target Identification (BTI)

- Build System
   - Modify FVP makefile for CPUs that support both AArch64/32

   - AArch32: Allow compiling with soft-float toolchain

   - Makefile: Add default warning flags

   - Add Makefile check for PAuth and AArch64

   - Add compile-time errors for HW_ASSISTED_COHERENCY flag

   - Apply compile-time check for AArch64-only CPUs

   - build_macros: Add mechanism to prevent bin generation.

   - Add support for default stack-protector flag

   - spd: opteed: Enable NS_TIMER_SWITCH

   - plat/arm: Skip BL2U if RESET_TO_SP_MIN flag is set

   - Add new build option to let each platform select which implementation of spinlocks
     it wants to use

- CPU Support
   - DSU: Workaround for erratum 798953 and 936184

   - Neoverse N1: Force cacheable atomic to near atomic
   - Neoverse N1: Workaround for erratum 1073348, 1130799, 1165347, 1207823,
     1220197, 1257314, 1262606, 1262888, 1275112, 1315703, 1542419

   - Neoverse Zeus: Apply the MSR SSBS instruction

   - cortex-Hercules/HerculesAE: Support added for Cortex-Hercules and
     Cortex-HerculesAE CPUs
   - cortex-Hercules/HerculesAE: Enable AMU for Cortex-Hercules and Cortex-HerculesAE

   - cortex-a76AE: Support added for Cortex-A76AE CPU
   - cortex-a76: Workaround for erratum 1257314, 1262606, 1262888, 1275112,
     1286807

   - cortex-a65/a65AE: Support added for  Cortex-A65 and  Cortex-A65AE CPUs
   - cortex-a65: Enable AMU for  Cortex-A65

   - cortex-a55: Workaround for erratum 1221012

   - cortex-a35: Workaround for erratum 855472

   - cortex-a9: Workaround for erratum 794073

- Drivers
   - console: Allow the console to register multiple times

   - delay: Timeout detection support

   - gicv3: Enabled multi-socket GIC redistributor frame discovery and migrated
     ARM platforms to the new API
       - Adds ``gicv3_rdistif_probe`` function that delegates the responsibility
         of discovering the corresponding redistributor base frame to each CPU
         itself.

   - sbsa: Add SBSA watchdog driver

   - st/stm32_hash: Add HASH driver

   - ti/uart: Add an AArch32 variant

- Library at ROM (romlib)
   - Introduce BTI support in Library at ROM (romlib)

- New Platforms Support
   - amlogic: g12a: New platform support added for the S905X2 (G12A) platform
   - amlogic: meson/gxl: New platform support added for Amlogic Meson
     S905x (GXL)

   - arm/a5ds: New platform support added for A5 DesignStart

   - arm/corstone: New platform support added for Corstone-700

   - intel: New platform support added for Agilex

   - mediatek:  New platform support added for MediaTek mt8183

   - qemu/qemu_sbsa: New platform support added for QEMU SBSA platform

   - renesas/rcar_gen3: plat: New platform support added for D3

   - rockchip: New platform support added for px30
   - rockchip: New platform support added for rk3288

   - rpi: New platform support added for Raspberry Pi 4

- Platforms
   - arm/common: Introduce wrapper functions to setup secure watchdog

   - arm/fvp: Add Delay Timer driver to BL1 and BL31 and option for defining
     platform DRAM2 base
   - arm/fvp: Add Linux DTS files for 32 bit threaded FVPs

   - arm/n1sdp: Add code for DDR ECC enablement and BL33 copy to DDR, Initialise CNTFRQ
     in Non Secure CNTBaseN

   - arm/juno: Use shared mbedtls heap between BL1 and BL2 and add basic support for
     dynamic config

   - imx: Basic support for PicoPi iMX7D, rdc module init, caam module init,
     aipstz init, IMX_SIP_GET_SOC_INFO, IMX_SIP_BUILDINFO added

   - intel: Add ncore ccu driver

   - mediatek/mt81*: Use new bl31_params_parse() helper

   - nvidia: tegra: Add support for multi console interface

   - qemu/qemu_sbsa: Adding memory mapping for both FLASH0/FLASH1
   - qemu: Added gicv3 support, new console interface in AArch32, and sub-platforms

   - renesas/rcar_gen3: plat: Add R-Car V3M support, new board revision for H3ULCB, DBSC4
     setting before self-refresh mode

   - socionext/uniphier: Support console based on  multi-console

   - st: stm32mp1: Add OP-TEE, Avenger96, watchdog, LpDDR3, authentication support
     and general SYSCFG management

   - ti/k3: common: Add support for J721E, Use coherent memory for shared data, Trap all
     asynchronous bus errors to EL3

   - xilinx/zynqmp: Add support for multi console interface, Initialize IPI table from
     zynqmp_config_setup()

- PSCI
   - Adding new optional PSCI hook ``pwr_domain_on_finish_late``
      - This PSCI hook ``pwr_domain_on_finish_late`` is similar to
        ``pwr_domain_on_finish`` but is guaranteed to be invoked when the
        respective core and cluster are participating in coherency.

- Security
   - Speculative Store Bypass Safe (SSBS): Further enhance protection against Spectre
     variant 4 by disabling speculative loads/stores (SPSR.SSBS bit) by default.

   - UBSAN support and handlers
      - Adds support for the Undefined Behaviour sanitizer. There are two types of
        support offered - minimalistic trapping support which essentially immediately
        crashes on undefined behaviour and full support with full debug messages.

- Tools
   - cert_create: Add support for bigger RSA key sizes (3KB and 4KB),
     previously the maximum size was 2KB.

   - fiptool: Add support to build fiptool on Windows.


Changed
^^^^^^^

- Architecture
   - Refactor ARMv8.3 Pointer Authentication support code

   - backtrace: Strip PAC field when PAUTH is enabled

   - Prettify crash reporting output on AArch64.

   - Rework smc_unknown return code path in smc_handler
      - Leverage the existing ``el3_exit()`` return routine for smc_unknown return
        path rather than a custom set of instructions.

- BL-Specific
   - Invalidate dcache build option for BL2 entry at EL3

   - Add missing support for BL2_AT_EL3 in XIP memory

- Boot Flow
   - Add helper to parse BL31 parameters (both versions)

   - Factor out cross-BL API into export headers suitable for 3rd party code

   - Introduce lightweight BL platform parameter library

- Drivers
   - auth: Memory optimization for Chain of Trust (CoT) description

   - bsec: Move bsec_mode_is_closed_device() service to platform

   - cryptocell: Move Cryptocell specific API into driver

   - gicv3: Prevent pending G1S interrupt from becoming G0 interrupt

   - mbedtls: Remove weak heap implementation

   - mmc: Increase delay between ACMD41 retries
   - mmc: stm32_sdmmc2: Correctly manage block size
   - mmc: stm32_sdmmc2: Manage max-frequency property from DT

   - synopsys/emmc: Do not change FIFO TH as this breaks some platforms
   - synopsys: Update synopsys drivers to not rely on undefined overflow behaviour

   - ufs: Extend the delay after reset to wait for some slower chips

- Platforms
   - amlogic/meson/gxl: Remove BL2 dependency from BL31

   - arm/common: Shorten the Firmware Update (FWU) process

   - arm/fvp: Remove GIC initialisation from secondary core cold boot

   - arm/sgm: Temporarily disable shared Mbed TLS heap for SGM

   - hisilicon: Update hisilicon drivers to not rely on undefined overflow behaviour

   - imx: imx8: Replace PLAT_IMX8* with PLAT_imx8*, remove duplicated linker symbols and
     deprecated code include, keep only IRQ 32 unmasked, enable all power domain by default

   - marvell: Prevent SError accessing PCIe link, Switch to xlat_tables_v2, do not rely on
     argument passed via smc, make sure that comphy init will use correct address

   - mediatek: mt8173: Refactor RTC and PMIC drivers
   - mediatek: mt8173: Apply MULTI_CONSOLE framework

   - nvidia: Tegra: memctrl_v2: fix "overflow before widen" coverity issue

   - qemu: Simplify the image size calculation, Move and generalise FDT PSCI fixup, move
     gicv2 codes to separate file

   - renesas/rcar_gen3: Convert to multi-console API, update QoS setting, Update IPL and
     Secure Monitor Rev2.0.4, Change to restore timer counter value at resume, Update DDR
     setting rev.0.35, qos: change subslot cycle, Change periodic write DQ training option.

   - rockchip: Allow SOCs with undefined wfe check bits, Streamline and complete UARTn_BASE
     macros, drop rockchip-specific imported linker symbols for bl31, Disable binary generation
     for all SOCs, Allow console device to be set by DTB, Use new bl31_params_parse functions

   - rpi/rpi3: Move shared rpi3 files into common directory

   - socionext/uniphier: Set CONSOLE_FLAG_TRANSLATE_CRLF and clean up console driver
   - socionext/uniphier: Replace DIV_ROUND_UP() with div_round_up() from utils_def.h

   - st/stm32mp: Split stm32mp_io_setup function, move stm32_get_gpio_bank_clock() to private
     file, correctly handle Clock Spreading Generator, move oscillator functions to generic file,
     realign device tree files with internal devs, enable RTCAPB clock for dual-core chips, use a
     common function to check spinlock is available, move check_header() to common code

   - ti/k3: Enable SEPARATE_CODE_AND_RODATA by default, Remove shared RAM space,
     Drop _ADDRESS from K3_USART_BASE to match other defines, Remove MSMC port
     definitions, Allow USE_COHERENT_MEM for K3, Set L2 latency on A72 cores

- PSCI
   - PSCI: Lookup list of parent nodes to lock only once

- Secure Partition Manager (SPM): SPCI Prototype
   - Fix service UUID lookup

   - Adjust size of virtual address space per partition

   - Refactor xlat context creation

   - Move shim layer to TTBR1_EL1

   - Ignore empty regions in resource description

- Security
   - Refactor SPSR initialisation code

   - SMMUv3: Abort DMA transactions
      - For security DMA should be blocked at the SMMU by default unless explicitly
        enabled for a device. SMMU is disabled after reset with all streams bypassing
        the SMMU, and abortion of all incoming transactions implements a default deny
        policy on reset.
      - Moves ``bl1_platform_setup()`` function from arm_bl1_setup.c to FVP platforms'
        fvp_bl1_setup.c and fvp_ve_bl1_setup.c files.

- Tools
   - cert_create: Remove RSA PKCS#1 v1.5 support


Resolved Issues
^^^^^^^^^^^^^^^

- Architecture
   - Fix the CAS spinlock implementation by adding a missing DSB in ``spin_unlock()``

   - AArch64: Fix SCTLR bit definitions
      - Removes incorrect ``SCTLR_V_BIT`` definition and adds definitions for
        ARMv8.3-Pauth `EnIB`, `EnDA` and `EnDB` bits.

   - Fix restoration of PAuth context
      - Replace call to ``pauth_context_save()`` with ``pauth_context_restore()`` in
        case of unknown SMC call.

- BL-Specific Issues
   - Fix BL31 crash reporting on AArch64 only platforms

- Build System
   - Remove several warnings reported with W=2 and W=1

- Code Quality Issues
   - SCTLR and ACTLR are 32-bit for AArch32 and 64-bit for AArch64
   - Unify type of "cpu_idx" across PSCI module.
   - Assert if power level value greater then PSCI_INVALID_PWR_LVL
   - Unsigned long should not be used as per coding guidelines
   - Reduce the number of memory leaks in cert_create
   - Fix type of cot_desc_ptr
   - Use explicit-width data types in AAPCS parameter structs
   - Add python configuration for editorconfig
   - BL1: Fix type consistency

   - Enable -Wshift-overflow=2 to check for undefined shift behavior
   - Updated upstream platforms to not rely on undefined overflow behaviour

- Coverity Quality Issues
   - Remove GGC ignore -Warray-bounds
   - Fix Coverity #261967, Infinite loop
   - Fix Coverity #343017, Missing unlock
   - Fix Coverity #343008, Side affect in assertion
   - Fix Coverity #342970, Uninitialized scalar variable

- CPU Support
   - cortex-a12: Fix MIDR mask

- Drivers
   - console: Remove Arm console unregister on suspend

   - gicv3: Fix support for full SPI range

   - scmi: Fix wrong payload length

- Library Code
   - libc: Fix sparse warning for __assert()

   - libc: Fix memchr implementation

- Platforms
   - rpi: rpi3: Fix compilation error when stack protector is enabled

   - socionext/uniphier: Fix compilation fail for SPM support build config

   - st/stm32mp1: Fix TZC400 configuration against non-secure DDR

   - ti/k3: common: Fix RO data area size calculation

- Security
   - AArch32: Disable Secure Cycle Counter
      - Changes the implementation for disabling Secure Cycle Counter.
        For ARMv8.5 the counter gets disabled by setting ``SDCR.SCCD`` bit on
        CPU cold/warm boot. For the earlier architectures PMCR register is
        saved/restored on secure world entry/exit from/to Non-secure state,
        and cycle counting gets disabled by setting PMCR.DP bit.
   - AArch64: Disable Secure Cycle Counter
      - For ARMv8.5 the counter gets disabled by setting ``MDCR_El3.SCCD`` bit on
        CPU cold/warm boot. For the earlier architectures PMCR_EL0 register is
        saved/restored on secure world entry/exit from/to Non-secure state,
        and cycle counting gets disabled by setting PMCR_EL0.DP bit.

Deprecations
^^^^^^^^^^^^

- Common Code
   - Remove MULTI_CONSOLE_API flag and references to it

   - Remove deprecated `plat_crash_console_*`

   - Remove deprecated interfaces `get_afflvl_shift`, `mpidr_mask_lower_afflvls`, `eret`

   - AARCH32/AARCH64 macros are now deprecated in favor of ``__aarch64__``

   - ``__ASSEMBLY__`` macro is now deprecated in favor of ``__ASSEMBLER__``

- Drivers
   - console: Removed legacy console API
   - console: Remove deprecated finish_console_register

   - tzc: Remove deprecated types `tzc_action_t` and `tzc_region_attributes_t`

- Secure Partition Manager (SPM):
   - Prototype SPCI-based SPM (services/std_svc/spm) will be replaced with alternative
     methods of secure partitioning support.

Known Issues
^^^^^^^^^^^^

- Build System Issues
   - dtb: DTB creation not supported when building on a Windows host.

     This step in the build process is skipped when running on a Windows host. A
     known issue from the 1.6 release.

- Platform Issues
   - arm/juno: System suspend from Linux does not function as documented in the
     user guide

     Following the instructions provided in the user guide document does not
     result in the platform entering system suspend state as expected. A message
     relating to the hdlcd driver failing to suspend will be emitted on the
     Linux terminal.

   - mediatek/mt6795: This platform does not build in this release

Version 2.1
-----------

New Features
^^^^^^^^^^^^

- Architecture
   - Support for ARMv8.3 pointer authentication in the normal and secure worlds

     The use of pointer authentication in the normal world is enabled whenever
     architectural support is available, without the need for additional build
     flags.

     Use of pointer authentication in the secure world remains an
     experimental configuration at this time. Using both the ``ENABLE_PAUTH``
     and ``CTX_INCLUDE_PAUTH_REGS`` build flags, pointer authentication can be
     enabled in EL3 and S-EL1/0.

     See the :ref:`Firmware Design` document for additional details on the use
     of pointer authentication.

   - Enable Data Independent Timing (DIT) in EL3, where supported

- Build System
   - Support for BL-specific build flags

   - Support setting compiler target architecture based on ``ARM_ARCH_MINOR``
     build option.

   - New ``RECLAIM_INIT_CODE`` build flag:

     A significant amount of the code used for the initialization of BL31 is
     not needed again after boot time. In order to reduce the runtime memory
     footprint, the memory used for this code can be reclaimed after
     initialization.

     Certain boot-time functions were marked with the ``__init`` attribute to
     enable this reclamation.

- CPU Support
   - cortex-a76: Workaround for erratum 1073348
   - cortex-a76: Workaround for erratum 1220197
   - cortex-a76: Workaround for erratum 1130799

   - cortex-a75: Workaround for erratum 790748
   - cortex-a75: Workaround for erratum 764081

   - cortex-a73: Workaround for erratum 852427
   - cortex-a73: Workaround for erratum 855423

   - cortex-a57: Workaround for erratum 817169
   - cortex-a57: Workaround for erratum 814670

   - cortex-a55: Workaround for erratum 903758
   - cortex-a55: Workaround for erratum 846532
   - cortex-a55: Workaround for erratum 798797
   - cortex-a55: Workaround for erratum 778703
   - cortex-a55: Workaround for erratum 768277

   - cortex-a53: Workaround for erratum 819472
   - cortex-a53: Workaround for erratum 824069
   - cortex-a53: Workaround for erratum 827319

   - cortex-a17: Workaround for erratum 852423
   - cortex-a17: Workaround for erratum 852421

   - cortex-a15: Workaround for erratum 816470
   - cortex-a15: Workaround for erratum 827671

- Documentation
   - Exception Handling Framework documentation

   - Library at ROM (romlib) documentation

   - RAS framework documentation

   - Coding Guidelines document

- Drivers
   - ccn: Add API for setting and reading node registers
      - Adds ``ccn_read_node_reg`` function
      - Adds ``ccn_write_node_reg`` function

   - partition: Support MBR partition entries

   - scmi: Add ``plat_css_get_scmi_info`` function

     Adds a new API ``plat_css_get_scmi_info`` which lets the platform
     register a platform-specific instance of ``scmi_channel_plat_info_t`` and
     remove the default values

   - tzc380: Add TZC-380 TrustZone Controller driver

   - tzc-dmc620: Add driver to manage the TrustZone Controller within the
     DMC-620 Dynamic Memory Controller

- Library at ROM (romlib)
   - Add platform-specific jump table list

   - Allow patching of romlib functions

     This change allows patching of functions in the romlib. This can be done by
     adding "patch" at the end of the jump table entry for the function that
     needs to be patched in the file jmptbl.i.

- Library Code
   - Support non-LPAE-enabled MMU tables in AArch32

   - mmio: Add ``mmio_clrsetbits_16`` function
      - 16-bit variant of ``mmio_clrsetbits``

   - object_pool: Add Object Pool Allocator
      - Manages object allocation using a fixed-size static array
      - Adds ``pool_alloc`` and ``pool_alloc_n`` functions
      - Does not provide any functions to free allocated objects (by design)

   - libc: Added ``strlcpy`` function

   - libc: Import ``strrchr`` function from FreeBSD

   - xlat_tables: Add support for ARMv8.4-TTST

   - xlat_tables: Support mapping regions without an explicitly specified VA

- Math
   - Added softudiv macro to support software division

- Memory Partitioning And Monitoring (MPAM)
   - Enabled MPAM EL2 traps (``MPAMHCR_EL2`` and ``MPAM_EL2``)

- Platforms
   - amlogic: Add support for Meson S905 (GXBB)

   - arm/fvp_ve: Add support for FVP Versatile Express platform

   - arm/n1sdp: Add support for Neoverse N1 System Development platform

   - arm/rde1edge: Add support for Neoverse E1 platform

   - arm/rdn1edge: Add support for Neoverse N1 platform

   - arm: Add support for booting directly to Linux without an intermediate
     loader (AArch32)

   - arm/juno: Enable new CPU errata workarounds for A53 and A57

   - arm/juno: Add romlib support

     Building a combined BL1 and ROMLIB binary file with the correct page
     alignment is now supported on the Juno platform. When ``USE_ROMLIB`` is set
     for Juno, it generates the combined file ``bl1_romlib.bin`` which needs to
     be used instead of bl1.bin.

   - intel/stratix: Add support for Intel Stratix 10 SoC FPGA platform

   - marvell: Add support for Armada-37xx SoC platform

   - nxp: Add support for i.MX8M and i.MX7 Warp7 platforms

   - renesas: Add support for R-Car Gen3 platform

   - xilinx: Add support for Versal ACAP platforms

- Position-Independent Executable (PIE)

  PIE support has initially been added to BL31. The ``ENABLE_PIE`` build flag is
  used to enable or disable this functionality as required.

- Secure Partition Manager
   - New SPM implementation based on SPCI Alpha 1 draft specification

     A new version of SPM has been implemented, based on the SPCI (Secure
     Partition Client Interface) and SPRT (Secure Partition Runtime) draft
     specifications.

     The new implementation is a prototype that is expected to undergo intensive
     rework as the specifications change. It has basic support for multiple
     Secure Partitions and Resource Descriptions.

     The older version of SPM, based on MM (ARM Management Mode Interface
     Specification), is still present in the codebase. A new build flag,
     ``SPM_MM`` has been added to allow selection of the desired implementation.
     This flag defaults to 1, selecting the MM-based implementation.

- Security
   - Spectre Variant-1 mitigations (``CVE-2017-5753``)

   - Use Speculation Store Bypass Safe (SSBS) functionality where available

     Provides mitigation against ``CVE-2018-19440`` (Not saving x0 to x3
     registers can leak information from one Normal World SMC client to another)


Changed
^^^^^^^

- Build System
   - Warning levels are now selectable with ``W=<1,2,3>``

   - Removed unneeded include paths in PLAT_INCLUDES

   - "Warnings as errors" (Werror) can be disabled using ``E=0``

   - Support totally quiet output with ``-s`` flag

   - Support passing options to checkpatch using ``CHECKPATCH_OPTS=<opts>``

   - Invoke host compiler with ``HOSTCC / HOSTCCFLAGS`` instead of ``CC / CFLAGS``

   - Make device tree pre-processing similar to U-boot/Linux by:
      - Creating separate ``CPPFLAGS`` for DT preprocessing so that compiler
        options specific to it can be accommodated.
      - Replacing ``CPP`` with ``PP`` for DT pre-processing

- CPU Support
   - Errata report function definition is now mandatory for CPU support files

     CPU operation files must now define a ``<name>_errata_report`` function to
     print errata status. This is no longer a weak reference.

- Documentation
   - Migrated some content from GitHub wiki to ``docs/`` directory

   - Security advisories now have CVE links

   - Updated copyright guidelines

- Drivers
   - console: The ``MULTI_CONSOLE_API`` framework has been rewritten in C

   - console: Ported multi-console driver to AArch32

   - gic: Remove 'lowest priority' constants

     Removed ``GIC_LOWEST_SEC_PRIORITY`` and ``GIC_LOWEST_NS_PRIORITY``.
     Platforms should define these if required, or instead determine the correct
     priority values at runtime.

   - delay_timer: Check that the Generic Timer extension is present

   - mmc: Increase command reply timeout to 10 milliseconds

   - mmc: Poll eMMC device status to ensure ``EXT_CSD`` command completion

   - mmc: Correctly check return code from ``mmc_fill_device_info``

- External Libraries

   - libfdt: Upgraded from 1.4.2 to 1.4.6-9

   - mbed TLS: Upgraded from 2.12 to 2.16

     This change incorporates fixes for security issues that should be reviewed
     to determine if they are relevant for software implementations using
     Trusted Firmware-A. See the `mbed TLS releases`_ page for details on
     changes from the 2.12 to the 2.16 release.

- Library Code
   - compiler-rt: Updated ``lshrdi3.c`` and ``int_lib.h`` with changes from
     LLVM master branch (r345645)

   - cpu: Updated macro that checks need for ``CVE-2017-5715`` mitigation

   - libc: Made setjmp and longjmp C standard compliant

   - libc: Allowed overriding the default libc (use ``OVERRIDE_LIBC``)

   - libc: Moved setjmp and longjmp to the ``libc/`` directory

- Platforms
   - Removed Mbed TLS dependency from plat_bl_common.c

   - arm: Removed unused ``ARM_MAP_BL_ROMLIB`` macro

   - arm: Removed ``ARM_BOARD_OPTIMISE_MEM`` feature and build flag

   - arm: Moved several components into ``drivers/`` directory

     This affects the SDS, SCP, SCPI, MHU and SCMI components

   - arm/juno: Increased maximum BL2 image size to ``0xF000``

     This change was required to accommodate a larger ``libfdt`` library

- SCMI
   - Optimized bakery locks when hardware-assisted coherency is enabled using the
     ``HW_ASSISTED_COHERENCY`` build flag

- SDEI
   - Added support for unconditionally resuming secure world execution after
     |SDEI| event processing completes

     |SDEI| interrupts, although targeting EL3, occur on behalf of the non-secure
     world, and may have higher priority than secure world
     interrupts. Therefore they might preempt secure execution and yield
     execution to the non-secure |SDEI| handler. Upon completion of |SDEI| event
     handling, resume secure execution if it was preempted.

- Translation Tables (XLAT)
   - Dynamically detect need for ``Common not Private (TTBRn_ELx.CnP)`` bit

     Properly handle the case where ``ARMv8.2-TTCNP`` is implemented in a CPU
     that does not implement all mandatory v8.2 features (and so must claim to
     implement a lower architecture version).


Resolved Issues
^^^^^^^^^^^^^^^

- Architecture
   - Incorrect check for SSBS feature detection

   - Unintentional register clobber in AArch32 reset_handler function

- Build System
   - Dependency issue during DTB image build

   - Incorrect variable expansion in Arm platform makefiles

   - Building on Windows with verbose mode (``V=1``) enabled is broken

   - AArch32 compilation flags is missing ``$(march32-directive)``

- BL-Specific Issues
   - bl2: ``uintptr_t is not defined`` error when ``BL2_IN_XIP_MEM`` is defined

   - bl2: Missing prototype warning in ``bl2_arch_setup``

   - bl31: Omission of Global Offset Table (GOT) section

- Code Quality Issues
   - Multiple MISRA compliance issues

   - Potential NULL pointer dereference (Coverity-detected)

- Drivers
   - mmc: Local declaration of ``scr`` variable causes a cache issue when
     invalidating after the read DMA transfer completes

   - mmc: ``ACMD41`` does not send voltage information during initialization,
     resulting in the command being treated as a query. This prevents the
     command from initializing the controller.

   - mmc: When checking device state using ``mmc_device_state()`` there are no
     retries attempted in the event of an error

   - ccn: Incorrect Region ID calculation for RN-I nodes

   - console: ``Fix MULTI_CONSOLE_API`` when used as a crash console

   - partition: Improper NULL checking in gpt.c

   - partition: Compilation failure in ``VERBOSE`` mode (``V=1``)

- Library Code
   - common: Incorrect check for Address Authentication support

   - xlat: Fix XLAT_V1 / XLAT_V2 incompatibility

     The file ``arm_xlat_tables.h`` has been renamed to ``xlat_tables_compat.h``
     and has been moved to a common folder. This header can be used to guarantee
     compatibility, as it includes the correct header based on
     ``XLAT_TABLES_LIB_V2``.

   - xlat: armclang unused-function warning on ``xlat_clean_dcache_range``

   - xlat: Invalid ``mm_cursor`` checks in ``mmap_add`` and ``mmap_add_ctx``

   - sdei: Missing ``context.h`` header

- Platforms
   - common: Missing prototype warning for ``plat_log_get_prefix``

   - arm: Insufficient maximum BL33 image size

   - arm: Potential memory corruption during BL2-BL31 transition

     On Arm platforms, the BL2 memory can be overlaid by BL31/BL32. The memory
     descriptors describing the list of executable images are created in BL2
     R/W memory, which could be possibly corrupted later on by BL31/BL32 due
     to overlay. This patch creates a reserved location in SRAM for these
     descriptors and are copied over by BL2 before handing over to next BL
     image.

   - juno: Invalid behaviour when ``CSS_USE_SCMI_SDS_DRIVER`` is not set

     In ``juno_pm.c`` the ``css_scmi_override_pm_ops`` function was used
     regardless of whether the build flag was set. The original behaviour has
     been restored in the case where the build flag is not set.

- Tools
   - fiptool: Incorrect UUID parsing of blob parameters

   - doimage: Incorrect object rules in Makefile


Deprecations
^^^^^^^^^^^^

- Common Code
   - ``plat_crash_console_init`` function

   - ``plat_crash_console_putc`` function

   - ``plat_crash_console_flush`` function

   - ``finish_console_register`` macro

- AArch64-specific Code
   - helpers: ``get_afflvl_shift``

   - helpers: ``mpidr_mask_lower_afflvls``

   - helpers: ``eret``

- Secure Partition Manager (SPM)
   - Boot-info structure


Known Issues
^^^^^^^^^^^^

- Build System Issues
   - dtb: DTB creation not supported when building on a Windows host.

     This step in the build process is skipped when running on a Windows host. A
     known issue from the 1.6 release.

- Platform Issues
   - arm/juno: System suspend from Linux does not function as documented in the
     user guide

     Following the instructions provided in the user guide document does not
     result in the platform entering system suspend state as expected. A message
     relating to the hdlcd driver failing to suspend will be emitted on the
     Linux terminal.

   - arm/juno: The firmware update use-cases do not work with motherboard
     firmware version < v1.5.0 (the reset reason is not preserved). The Linaro
     18.04 release has MB v1.4.9. The MB v1.5.0 is available in Linaro 18.10
     release.

   - mediatek/mt6795: This platform does not build in this release

Version 2.0
-----------

New Features
^^^^^^^^^^^^

-  Removal of a number of deprecated APIs

   -  A new Platform Compatibility Policy document has been created which
      references a wiki page that maintains a listing of deprecated
      interfaces and the release after which they will be removed.

   -  All deprecated interfaces except the MULTI_CONSOLE_API have been removed
      from the code base.

   -  Various Arm and partner platforms have been updated to remove the use of
      removed APIs in this release.

   -  This release is otherwise unchanged from 1.6 release

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  No issues known at 1.6 release resolved in 2.0 release

Known Issues
^^^^^^^^^^^^

-  DTB creation not supported when building on a Windows host. This step in the
   build process is skipped when running on a Windows host. Known issue from
   1.6 version.

-  As a result of removal of deprecated interfaces the Nvidia Tegra, Marvell
   Armada 8K and MediaTek MT6795 platforms do not build in this release.
   Also MediaTek MT8173, NXP QorIQ LS1043A, NXP i.MX8QX, NXP i.MX8QMa,
   Rockchip RK3328, Rockchip RK3368 and Rockchip RK3399 platforms have not been
   confirmed to be working after the removal of the deprecated interfaces
   although they do build.

Version 1.6
-----------

New Features
^^^^^^^^^^^^

-  Addressing Speculation Security Vulnerabilities

   -  Implement static workaround for CVE-2018-3639 for AArch32 and AArch64

   -  Add support for dynamic mitigation for CVE-2018-3639

   -  Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76

   -  Ensure |SDEI| handler executes with CVE-2018-3639 mitigation enabled

-  Introduce RAS handling on AArch64

   -  Some RAS extensions are mandatory for Armv8.2 CPUs, with others
      mandatory for Armv8.4 CPUs however, all extensions are also optional
      extensions to the base Armv8.0 architecture.

   -  The Armv8 RAS Extensions introduced Standard Error Records which are a
      set of standard registers to configure RAS node policy and allow RAS
      Nodes to record and expose error information for error handling agents.

   -  Capabilities are provided to support RAS Node enumeration and iteration
      along with individual interrupt registrations and fault injections
      support.

   -  Introduce handlers for Uncontainable errors, Double Faults and EL3
      External Aborts

-  Enable Memory Partitioning And Monitoring (MPAM) for lower EL's

   -  Memory Partitioning And Monitoring is an Armv8.4 feature that enables
      various memory system components and resources to define partitions.
      Software running at various ELs can then assign themselves to the
      desired partition to control their performance aspects.

   -  When ENABLE_MPAM_FOR_LOWER_ELS is set to 1, EL3 allows
      lower ELs to access their own MPAM registers without trapping to EL3.
      This patch however, doesn't make use of partitioning in EL3; platform
      initialisation code should configure and use partitions in EL3 if
      required.

-  Introduce ROM Lib Feature

   -  Support combining several libraries into a self-called "romlib" image,
      that may be shared across images to reduce memory footprint. The romlib
      image is stored in ROM but is accessed through a jump-table that may be
      stored in read-write memory, allowing for the library code to be patched.

-  Introduce Backtrace Feature

   -  This function displays the backtrace, the current EL and security state
      to allow a post-processing tool to choose the right binary to interpret
      the dump.

   -  Print backtrace in assert() and panic() to the console.

-  Code hygiene changes and alignment with MISRA C-2012 guideline with fixes
   addressing issues complying to the following rules:

   -  MISRA rules 4.9, 5.1, 5.3, 5.7, 8.2-8.5, 8.8, 8.13, 9.3, 10.1,
      10.3-10.4, 10.8, 11.3, 11.6, 12.1, 14.4, 15.7, 16.1-16.7, 17.7-17.8,
      20.7, 20.10, 20.12, 21.1, 21.15, 22.7

   -  Clean up the usage of void pointers to access symbols

   -  Increase usage of static qualifier to locally used functions and data

   -  Migrated to use of u_register_t for register read/write to better
      match AArch32 and AArch64 type sizes

   -  Use int-ll64 for both AArch32 and AArch64 to assist in consistent
      format strings between architectures

   -  Clean up TF-A libc by removing non arm copyrighted implementations
      and replacing them with modified FreeBSD and SCC implementations

-  Various changes to support Clang linker and assembler

   -  The clang assembler/preprocessor is used when Clang is selected. However,
      the clang linker is not used because it is unable to link TF-A objects
      due to immaturity of clang linker functionality at this time.

-  Refactor support APIs into Libraries

   -  Evolve libfdt, mbed TLS library and standard C library sources as
      proper libraries that TF-A may be linked against.

-  CPU Enhancements

   -  Add CPU support for Cortex-Ares and Cortex-A76

   -  Add AMU support for Cortex-Ares

   -  Add initial CPU support for Cortex-Deimos

   -  Add initial CPU support for Cortex-Helios

   -  Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76

   -  Implement Cortex-Ares erratum 1043202 workaround

   -  Implement DSU erratum 936184 workaround

   -  Check presence of fix for errata 843419 in Cortex-A53

   -  Check presence of fix for errata 835769 in Cortex-A53

-  Translation Tables Enhancements

   -  The xlat v2 library has been refactored in order to be reused by
      different TF components at different EL's including the addition of EL2.
      Some refactoring to make the code more generic and less specific to TF,
      in order to reuse the library outside of this project.

-  SPM Enhancements

   -  General cleanups and refactoring to pave the way to multiple partitions
      support

-  SDEI Enhancements

   -  Allow platforms to define explicit events

   -  Determine client EL from NS context's SCR_EL3

   -  Make dispatches synchronous

   -  Introduce jump primitives for BL31

   -  Mask events after CPU wakeup in |SDEI| dispatcher to conform to the
      specification

-  Misc TF-A Core Common Code Enhancements

   -  Add support for eXecute In Place (XIP) memory in BL2

   -  Add support for the SMC Calling Convention 2.0

   -  Introduce External Abort handling on AArch64
      External Abort routed to EL3 was reported as an unhandled exception
      and caused a panic. This change enables Trusted Firmware-A to handle
      External Aborts routed to EL3.

   -  Save value of ACTLR_EL1 implementation-defined register in the CPU
      context structure rather than forcing it to 0.

   -  Introduce ARM_LINUX_KERNEL_AS_BL33 build option, which allows BL31 to
      directly jump to a Linux kernel. This makes for a quicker and simpler
      boot flow, which might be useful in some test environments.

   -  Add dynamic configurations for BL31, BL32 and BL33 enabling support for
      Chain of Trust (COT).

   -  Make TF UUID RFC 4122 compliant

-  New Platform Support

   -  Arm SGI-575

   -  Arm SGM-775

   -  Allwinner sun50i_64

   -  Allwinner sun50i_h6

   -  NXP QorIQ LS1043A

   -  NXP i.MX8QX

   -  NXP i.MX8QM

   -  NXP i.MX7Solo WaRP7

   -  TI K3

   -  Socionext Synquacer SC2A11

   -  Marvell Armada 8K

   -  STMicroelectronics STM32MP1

-  Misc Generic Platform Common Code Enhancements

   -  Add MMC framework that supports both eMMC and SD card devices

-  Misc Arm Platform Common Code Enhancements

   -  Demonstrate PSCI MEM_PROTECT from el3_runtime

   -  Provide RAS support

   -  Migrate AArch64 port to the multi console driver. The old API is
      deprecated and will eventually be removed.

   -  Move BL31 below BL2 to enable BL2 overlay resulting in changes in the
      layout of BL images in memory to enable more efficient use of available
      space.

   -  Add cpp build processing for dtb that allows processing device tree
      with external includes.

   -  Extend FIP io driver to support multiple FIP devices

   -  Add support for SCMI AP core configuration protocol v1.0

   -  Use SCMI AP core protocol to set the warm boot entrypoint

   -  Add support to Mbed TLS drivers for shared heap among different
      BL images to help optimise memory usage

   -  Enable non-secure access to UART1 through a build option to support
      a serial debug port for debugger connection

-  Enhancements for Arm Juno Platform

   -  Add support for TrustZone Media Protection 1 (TZMP1)

-  Enhancements for Arm FVP Platform

   -  Dynamic_config: remove the FVP dtb files

   -  Set DYNAMIC_WORKAROUND_CVE_2018_3639=1 on FVP by default

   -  Set the ability to dynamically disable Trusted Boot Board
      authentication to be off by default with DYN_DISABLE_AUTH

   -  Add librom enhancement support in FVP

   -  Support shared Mbed TLS heap between BL1 and BL2 that allow a
      reduction in BL2 size for FVP

-  Enhancements for Arm SGI/SGM Platform

   -  Enable ARM_PLAT_MT flag for SGI-575

   -  Add dts files to enable support for dynamic config

   -  Add RAS support

   -  Support shared Mbed TLS heap for SGI and SGM between BL1 and BL2

-  Enhancements for Non Arm Platforms

   -  Raspberry Pi Platform

   -  Hikey Platforms

   -  Xilinx Platforms

   -  QEMU Platform

   -  Rockchip rk3399 Platform

   -  TI Platforms

   -  Socionext Platforms

   -  Allwinner Platforms

   -  NXP Platforms

   -  NVIDIA Tegra Platform

   -  Marvell Platforms

   -  STMicroelectronics STM32MP1 Platform

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  No issues known at 1.5 release resolved in 1.6 release

Known Issues
^^^^^^^^^^^^

-  DTB creation not supported when building on a Windows host. This step in the
   build process is skipped when running on a Windows host. Known issue from
   1.5 version.

Version 1.5
-----------

New features
^^^^^^^^^^^^

-  Added new firmware support to enable RAS (Reliability, Availability, and
   Serviceability) functionality.

   -  Secure Partition Manager (SPM): A Secure Partition is a software execution
      environment instantiated in S-EL0 that can be used to implement simple
      management and security services. The SPM is the firmware component that
      is responsible for managing a Secure Partition.

   -  SDEI dispatcher: Support for interrupt-based |SDEI| events and all
      interfaces as defined by the |SDEI| specification v1.0, see
      `SDEI Specification`_

   -  Exception Handling Framework (EHF): Framework that allows dispatching of
      EL3 interrupts to their registered handlers which are registered based on
      their priorities. Facilitates firmware-first error handling policy where
      asynchronous exceptions may be routed to EL3.

      Integrated the TSPD with EHF.

-  Updated PSCI support:

   -  Implemented PSCI v1.1 optional features `MEM_PROTECT` and `SYSTEM_RESET2`.
      The supported PSCI version was updated to v1.1.

   -  Improved PSCI STAT timestamp collection, including moving accounting for
      retention states to be inside the locks and fixing handling of wrap-around
      when calculating residency in AArch32 execution state.

   -  Added optional handler for early suspend that executes when suspending to
      a power-down state and with data caches enabled.

      This may provide a performance improvement on platforms where it is safe
      to perform some or all of the platform actions from `pwr_domain_suspend`
      with the data caches enabled.

-  Enabled build option, BL2_AT_EL3, for BL2 to allow execution at EL3 without
   any dependency on TF BL1.

   This allows platforms which already have a non-TF Boot ROM to directly load
   and execute BL2 and subsequent BL stages without need for BL1. This was not
   previously possible because BL2 executes at S-EL1 and cannot jump straight to
   EL3.

-  Implemented support for SMCCC v1.1, including `SMCCC_VERSION` and
   `SMCCC_ARCH_FEATURES`.

   Additionally, added support for `SMCCC_VERSION` in PSCI features to enable
   discovery of the SMCCC version via PSCI feature call.

-  Added Dynamic Configuration framework which enables each of the boot loader
   stages to be dynamically configured at runtime if required by the platform.
   The boot loader stage may optionally specify a firmware configuration file
   and/or hardware configuration file that can then be shared with the next boot
   loader stage.

   Introduced a new BL handover interface that essentially allows passing of 4
   arguments between the different BL stages.

   Updated cert_create and fip_tool to support the dynamic configuration files.
   The COT also updated to support these new files.

-  Code hygiene changes and alignment with MISRA guideline:

   -  Fix use of undefined macros.

   -  Achieved compliance with Mandatory MISRA coding rules.

   -  Achieved compliance for following Required MISRA rules for the default
      build configurations on FVP and Juno platforms : 7.3, 8.3, 8.4, 8.5 and
      8.8.

-  Added support for Armv8.2-A architectural features:

   -  Updated translation table set-up to set the CnP (Common not Private) bit
      for secure page tables so that multiple PEs in the same Inner Shareable
      domain can use the same translation table entries for a given stage of
      translation in a particular translation regime.

   -  Extended the supported values of ID_AA64MMFR0_EL1.PARange to include the
      52-bit Physical Address range.

   -  Added support for the Scalable Vector Extension to allow Normal world
      software to access SVE functionality but disable access to SVE, SIMD and
      floating point functionality from the Secure world in order to prevent
      corruption of the Z-registers.

-  Added support for Armv8.4-A architectural feature Activity Monitor Unit (AMU)
    extensions.

   In addition to the v8.4 architectural extension, AMU support on Cortex-A75
   was implemented.

-  Enhanced OP-TEE support to enable use of pageable OP-TEE image. The Arm
   standard platforms are updated to load up to 3 images for OP-TEE; header,
   pager image and paged image.

   The chain of trust is extended to support the additional images.

-  Enhancements to the translation table library:

   -  Introduced APIs to get and set the memory attributes of a region.

   -  Added support to manage both privilege levels in translation regimes that
      describe translations for 2 Exception levels, specifically the EL1&0
      translation regime, and extended the memory map region attributes to
      include specifying Non-privileged access.

   -  Added support to specify the granularity of the mappings of each region,
      for instance a 2MB region can be specified to be mapped with 4KB page
      tables instead of a 2MB block.

   -  Disabled the higher VA range to avoid unpredictable behaviour if there is
      an attempt to access addresses in the higher VA range.

   -  Added helpers for Device and Normal memory MAIR encodings that align with
      the Arm Architecture Reference Manual for Armv8-A (Arm DDI0487B.b).

   -  Code hygiene including fixing type length and signedness of constants,
      refactoring of function to enable the MMU, removing all instances where
      the virtual address space is hardcoded and added comments that document
      alignment needed between memory attributes and attributes specified in
      TCR_ELx.

-  Updated GIC support:

   -  Introduce new APIs for GICv2 and GICv3 that provide the capability to
      specify interrupt properties rather than list of interrupt numbers alone.
      The Arm platforms and other upstream platforms are migrated to use
      interrupt properties.

   -  Added helpers to save / restore the GICv3 context, specifically the
      Distributor and Redistributor contexts and architectural parts of the ITS
      power management. The Distributor and Redistributor helpers also support
      the implementation-defined part of GIC-500 and GIC-600.

      Updated the Arm FVP platform to save / restore the GICv3 context on system
      suspend / resume as an example of how to use the helpers.

      Introduced a new TZC secured DDR carve-out for use by Arm platforms for
      storing EL3 runtime data such as the GICv3 register context.

-  Added support for Armv7-A architecture via build option ARM_ARCH_MAJOR=7.
   This includes following features:

   -  Updates GICv2 driver to manage GICv1 with security extensions.

   -  Software implementation for 32bit division.

   -  Enabled use of generic timer for platforms that do not set
      ARM_CORTEX_Ax=yes.

   -  Support for Armv7-A Virtualization extensions [DDI0406C_C].

   -  Support for both Armv7-A platforms that only have 32-bit addressing and
      Armv7-A platforms that support large page addressing.

   -  Included support for following Armv7 CPUs: Cortex-A12, Cortex-A17,
      Cortex-A7, Cortex-A5, Cortex-A9, Cortex-A15.

   -  Added support in QEMU for Armv7-A/Cortex-A15.

-  Enhancements to Firmware Update feature:

   -  Updated the FWU documentation to describe the additional images needed for
      Firmware update, and how they are used for both the Juno platform and the
      Arm FVP platforms.

-  Enhancements to Trusted Board Boot feature:

   -  Added support to cert_create tool for RSA PKCS1# v1.5 and SHA384, SHA512
      and SHA256.

   -  For Arm platforms added support to use ECDSA keys.

   -  Enhanced the mbed TLS wrapper layer to include support for both RSA and
      ECDSA to enable runtime selection between RSA and ECDSA keys.

-  Added support for secure interrupt handling in AArch32 sp_min, hardcoded to
   only handle FIQs.

-  Added support to allow a platform to load images from multiple boot sources,
   for example from a second flash drive.

-  Added a logging framework that allows platforms to reduce the logging level
   at runtime and additionally the prefix string can be defined by the platform.

-  Further improvements to register initialisation:

   -   Control register PMCR_EL0 / PMCR is set to prohibit cycle counting in the
       secure world. This register is added to the list of registers that are
       saved and restored during world switch.

   -   When EL3 is running in AArch32 execution state, the Non-secure version of
       SCTLR is explicitly initialised during the warmboot flow rather than
       relying on the hardware to set the correct reset values.

-  Enhanced support for Arm platforms:

   -  Introduced driver for Shared-Data-Structure (SDS) framework which is used
      for communication between SCP and the AP CPU, replacing Boot-Over_MHU
      (BOM) protocol.

      The Juno platform is migrated to use SDS with the SCMI support added in
      v1.3 and is set as default.

      The driver can be found in the plat/arm/css/drivers folder.

   -  Improved memory usage by only mapping TSP memory region when the TSPD has
      been included in the build. This reduces the memory footprint and avoids
      unnecessary memory being mapped.

   -  Updated support for multi-threading CPUs for FVP platforms - always check
      the MT field in MPDIR and access the bit fields accordingly.

   -  Support building for platforms that model DynamIQ configuration by
      implementing all CPUs in a single cluster.

   -  Improved nor flash driver, for instance clearing status registers before
      sending commands. Driver can be found plat/arm/board/common folder.

-  Enhancements to QEMU platform:

   -  Added support for TBB.

   -  Added support for using OP-TEE pageable image.

   -  Added support for LOAD_IMAGE_V2.

   -  Migrated to use translation table library v2 by default.

   -  Added support for SEPARATE_CODE_AND_RODATA.

-  Applied workarounds CVE-2017-5715 on Arm Cortex-A57, -A72, -A73 and -A75, and
   for Armv7-A CPUs Cortex-A9, -A15 and -A17.

-  Applied errata workaround for Arm Cortex-A57: 859972.

-  Applied errata workaround for Arm Cortex-A72: 859971.

-  Added support for Poplar 96Board platform.

-  Added support for Raspberry Pi 3 platform.

-  Added Call Frame Information (CFI) assembler directives to the vector entries
   which enables debuggers to display the backtrace of functions that triggered
   a synchronous abort.

-  Added ability to build dtb.

-  Added support for pre-tool (cert_create and fiptool) image processing
   enabling compression of the image files before processing by cert_create and
   fiptool.

   This can reduce fip size and may also speed up loading of images.  The image
   verification will also get faster because certificates are generated based on
   compressed images.

   Imported zlib 1.2.11 to implement gunzip() for data compression.

-  Enhancements to fiptool:

   -  Enabled the fiptool to be built using Visual Studio.

   -  Added padding bytes at the end of the last image in the fip to be
      facilitate transfer by DMA.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  TF-A can be built with optimisations disabled (-O0).

-  Memory layout updated to enable Trusted Board Boot on Juno platform when
   running TF-A in AArch32 execution mode (resolving `tf-issue#501`_).

Known Issues
^^^^^^^^^^^^

-  DTB creation not supported when building on a Windows host. This step in the
   build process is skipped when running on a Windows host.

Version 1.4
-----------

New features
^^^^^^^^^^^^

-  Enabled support for platforms with hardware assisted coherency.

   A new build option HW_ASSISTED_COHERENCY allows platforms to take advantage
   of the following optimisations:

   -  Skip performing cache maintenance during power-up and power-down.

   -  Use spin-locks instead of bakery locks.

   -  Enable data caches early on warm-booted CPUs.

-  Added support for Cortex-A75 and Cortex-A55 processors.

   Both Cortex-A75 and Cortex-A55 processors use the Arm DynamIQ Shared Unit
   (DSU). The power-down and power-up sequences are therefore mostly managed in
   hardware, reducing complexity of the software operations.

-  Introduced Arm GIC-600 driver.

   Arm GIC-600 IP complies with Arm GICv3 architecture. For FVP platforms, the
   GIC-600 driver is chosen when FVP_USE_GIC_DRIVER is set to FVP_GIC600.

-  Updated GICv3 support:

   -  Introduced power management APIs for GICv3 Redistributor. These APIs
      allow platforms to power down the Redistributor during CPU power on/off.
      Requires the GICv3 implementations to have power management operations.

      Implemented the power management APIs for FVP.

   -  GIC driver data is flushed by the primary CPU so that secondary CPU do
      not read stale GIC data.

-  Added support for Arm System Control and Management Interface v1.0 (SCMI).

   The SCMI driver implements the power domain management and system power
   management protocol of the SCMI specification (Arm DEN 0056ASCMI) for
   communicating with any compliant power controller.

   Support is added for the Juno platform. The driver can be found in the
   plat/arm/css/drivers folder.

-  Added support to enable pre-integration of TBB with the Arm TrustZone
   CryptoCell product, to take advantage of its hardware Root of Trust and
   crypto acceleration services.

-  Enabled Statistical Profiling Extensions for lower ELs.

   The firmware support is limited to the use of SPE in the Non-secure state
   and accesses to the SPE specific registers from S-EL1 will trap to EL3.

   The SPE are architecturally specified for AArch64 only.

-  Code hygiene changes aligned with MISRA guidelines:

   -  Fixed signed / unsigned comparison warnings in the translation table
      library.

   -  Added U(_x) macro and together with the existing ULL(_x) macro fixed
      some of the signed-ness defects flagged by the MISRA scanner.

-  Enhancements to Firmware Update feature:

   -  The FWU logic now checks for overlapping images to prevent execution of
      unauthenticated arbitrary code.

   -  Introduced new FWU_SMC_IMAGE_RESET SMC that changes the image loading
      state machine to go from COPYING, COPIED or AUTHENTICATED states to
      RESET state. Previously, this was only possible when the authentication
      of an image failed or when the execution of the image finished.

   -  Fixed integer overflow which addressed TFV-1: Malformed Firmware Update
      SMC can result in copy of unexpectedly large data into secure memory.

-  Introduced support for Arm Compiler 6 and LLVM (clang).

   TF-A can now also be built with the Arm Compiler 6 or the clang compilers.
   The assembler and linker must be provided by the GNU toolchain.

   Tested with Arm CC 6.7 and clang 3.9.x and 4.0.x.

-  Memory footprint improvements:

   -  Introduced `tf_snprintf`, a reduced version of `snprintf` which has
      support for a limited set of formats.

      The mbedtls driver is updated to optionally use `tf_snprintf` instead of
      `snprintf`.

   -  The `assert()` is updated to no longer print the function name, and
      additional logging options are supported via an optional platform define
      `PLAT_LOG_LEVEL_ASSERT`, which controls how verbose the assert output is.

-  Enhancements to TF-A support when running in AArch32 execution state:

   -  Support booting SP_MIN and BL33 in AArch32 execution mode on Juno. Due to
      hardware limitations, BL1 and BL2 boot in AArch64 state and there is
      additional trampoline code to warm reset into SP_MIN in AArch32 execution
      state.

   -  Added support for Arm Cortex-A53/57/72 MPCore processors including the
      errata workarounds that are already implemented for AArch64 execution
      state.

   -  For FVP platforms, added AArch32 Trusted Board Boot support, including the
      Firmware Update feature.

-  Introduced Arm SiP service for use by Arm standard platforms.

   -  Added new Arm SiP Service SMCs to enable the Non-secure  world to read PMF
      timestamps.

      Added PMF instrumentation points in TF-A in order to quantify the
      overall time spent in the PSCI software implementation.

   -  Added new Arm SiP service SMC to switch execution state.

      This allows the lower exception level to change its execution state from
      AArch64 to AArch32, or vice verse, via a request to EL3.

-  Migrated to use SPDX[0] license identifiers to make software license
   auditing simpler.

   .. note::
      Files that have been imported by FreeBSD have not been modified.

   [0]: https://spdx.org/

-  Enhancements to the translation table library:

   -  Added version 2 of translation table library that allows different
      translation tables to be modified by using different 'contexts'. Version 1
      of the translation table library only allows the current EL's translation
      tables to be modified.

      Version 2 of the translation table also added support for dynamic
      regions; regions that can be added and removed dynamically whilst the
      MMU is enabled. Static regions can only be added or removed before the
      MMU is enabled.

      The dynamic mapping functionality is enabled or disabled when compiling
      by setting the build option PLAT_XLAT_TABLES_DYNAMIC to 1 or 0. This can
      be done per-image.

   -  Added support for translation regimes with two virtual address spaces
      such as the one shared by EL1 and EL0.

      The library does not support initializing translation tables for EL0
      software.

   -  Added support to mark the translation tables as non-cacheable using an
      additional build option `XLAT_TABLE_NC`.

-  Added support for GCC stack protection. A new build option
   ENABLE_STACK_PROTECTOR was introduced that enables compilation of all BL
   images with one of the GCC -fstack-protector-* options.

   A new platform function plat_get_stack_protector_canary() was introduced
   that returns a value used to initialize the canary for stack corruption
   detection. For increased effectiveness of protection platforms must provide
   an implementation that returns a random value.

-  Enhanced support for Arm platforms:

   -  Added support for multi-threading CPUs, indicated by `MT` field in MPDIR.
      A new build flag `ARM_PLAT_MT` is added, and when enabled, the functions
      accessing MPIDR assume that the `MT` bit is set for the platform and
      access the bit fields accordingly.

      Also, a new API `plat_arm_get_cpu_pe_count` is added when `ARM_PLAT_MT` is
      enabled, returning the Processing Element count within the physical CPU
      corresponding to `mpidr`.

   -  The Arm platforms migrated to use version 2 of the translation tables.

   -  Introduced a new Arm platform layer API `plat_arm_psci_override_pm_ops`
      which allows Arm platforms to modify `plat_arm_psci_pm_ops` and therefore
      dynamically define PSCI capability.

   -  The Arm platforms migrated to use IMAGE_LOAD_V2 by default.

-  Enhanced reporting of errata workaround status with the following policy:

   -  If an errata workaround is enabled:

      -  If it applies (i.e. the CPU is affected by the errata), an INFO message
         is printed, confirming that the errata workaround has been applied.

      -  If it does not apply, a VERBOSE message is printed, confirming that the
         errata workaround has been skipped.

   -  If an errata workaround is not enabled, but would have applied had it
      been, a WARN message is printed, alerting that errata workaround is
      missing.

-  Added build options ARM_ARCH_MAJOR and ARM_ARM_MINOR to choose the
   architecture version to target TF-A.

-  Updated the spin lock implementation to use the more efficient CAS (Compare
   And Swap) instruction when available. This instruction was introduced in
   Armv8.1-A.

-  Applied errata workaround for Arm Cortex-A53: 855873.

-  Applied errata workaround for Arm-Cortex-A57: 813419.

-  Enabled all A53 and A57 errata workarounds for Juno, both in AArch64 and
   AArch32 execution states.

-  Added support for Socionext UniPhier SoC platform.

-  Added support for Hikey960 and Hikey platforms.

-  Added support for Rockchip RK3328 platform.

-  Added support for NVidia Tegra T186 platform.

-  Added support for Designware emmc driver.

-  Imported libfdt v1.4.2 that addresses buffer overflow in fdt_offset_ptr().

-  Enhanced the CPU operations framework to allow power handlers to be
   registered on per-level basis. This enables support for future CPUs that
   have multiple threads which might need powering down individually.

-  Updated register initialisation to prevent unexpected behaviour:

   -  Debug registers MDCR-EL3/SDCR and MDCR_EL2/HDCR are initialised to avoid
      unexpected traps into the higher exception levels and disable secure
      self-hosted debug. Additionally, secure privileged external debug on
      Juno is disabled by programming the appropriate Juno SoC registers.

   -  EL2 and EL3 configurable controls are initialised to avoid unexpected
      traps in the higher exception levels.

   -  Essential control registers are fully initialised on EL3 start-up, when
      initialising the non-secure and secure context structures and when
      preparing to leave EL3 for a lower EL. This gives better alignment with
      the Arm ARM which states that software must initialise RES0 and RES1
      fields with 0 / 1.

-  Enhanced PSCI support:

   -  Introduced new platform interfaces that decouple PSCI stat residency
      calculation from PMF, enabling platforms to use alternative methods of
      capturing timestamps.

   -  PSCI stat accounting performed for retention/standby states when
      requested at multiple power levels.

-  Simplified fiptool to have a single linked list of image descriptors.

-  For the TSP, resolved corruption of pre-empted secure context by aborting any
   pre-empted SMC during PSCI power management requests.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  TF-A can be built with the latest mbed TLS version (v2.4.2). The earlier
   version 2.3.0 cannot be used due to build warnings that the TF-A build
   system interprets as errors.

-  TBBR, including the Firmware Update feature  is now supported on FVP
   platforms when running TF-A in AArch32 state.

-  The version of the AEMv8 Base FVP used in this release has resolved the issue
   of the model executing a reset instead of terminating in response to a
   shutdown request using the PSCI SYSTEM_OFF API.

Known Issues
^^^^^^^^^^^^

-  Building TF-A with compiler optimisations disabled (-O0) fails.

-  Trusted Board Boot currently does not work on Juno when running Trusted
   Firmware in AArch32 execution state due to error when loading the sp_min to
   memory because of lack of free space available. See `tf-issue#501`_ for more
   details.

-  The errata workaround for A53 errata 843419 is only available from binutils
   2.26 and is not present in GCC4.9. If this errata is applicable to the
   platform, please use GCC compiler version of at least 5.0. See `PR#1002`_ for
   more details.

Version 1.3
-----------


New features
^^^^^^^^^^^^

-  Added support for running TF-A in AArch32 execution state.

   The PSCI library has been refactored to allow integration with **EL3 Runtime
   Software**. This is software that is executing at the highest secure
   privilege which is EL3 in AArch64 or Secure SVC/Monitor mode in AArch32. See
   :ref:`PSCI Library Integration guide for Armv8-A AArch32 systems`.

   Included is a minimal AArch32 Secure Payload, **SP-MIN**, that illustrates
   the usage and integration of the PSCI library with EL3 Runtime Software
   running in AArch32 state.

   Booting to the BL1/BL2 images as well as booting straight to the Secure
   Payload is supported.

-  Improvements to the initialization framework for the PSCI service and Arm
   Standard Services in general.

   The PSCI service is now initialized as part of Arm Standard Service
   initialization. This consolidates the initializations of any Arm Standard
   Service that may be added in the future.

   A new function ``get_arm_std_svc_args()`` is introduced to get arguments
   corresponding to each standard service and must be implemented by the EL3
   Runtime Software.

   For PSCI, a new versioned structure ``psci_lib_args_t`` is introduced to
   initialize the PSCI Library. **Note** this is a compatibility break due to
   the change in the prototype of ``psci_setup()``.

-  To support AArch32 builds of BL1 and BL2, implemented a new, alternative
   firmware image loading mechanism that adds flexibility.

   The current mechanism has a hard-coded set of images and execution order
   (BL31, BL32, etc). The new mechanism is data-driven by a list of image
   descriptors provided by the platform code.

   Arm platforms have been updated to support the new loading mechanism.

   The new mechanism is enabled by a build flag (``LOAD_IMAGE_V2``) which is
   currently off by default for the AArch64 build.

   **Note** ``TRUSTED_BOARD_BOOT`` is currently not supported when
   ``LOAD_IMAGE_V2`` is enabled.

-  Updated requirements for making contributions to TF-A.

   Commits now must have a 'Signed-off-by:' field to certify that the
   contribution has been made under the terms of the
   :download:`Developer Certificate of Origin <../dco.txt>`.

   A signed CLA is no longer required.

   The :ref:`Contributor's Guide` has been updated to reflect this change.

-  Introduced Performance Measurement Framework (PMF) which provides support
   for capturing, storing, dumping and retrieving time-stamps to measure the
   execution time of critical paths in the firmware. This relies on defining
   fixed sample points at key places in the code.

-  To support the QEMU platform port, imported libfdt v1.4.1 from
   https://git.kernel.org/pub/scm/utils/dtc/dtc.git

-  Updated PSCI support:

   -  Added support for PSCI NODE_HW_STATE API for Arm platforms.

   -  New optional platform hook, ``pwr_domain_pwr_down_wfi()``, in
      ``plat_psci_ops`` to enable platforms to perform platform-specific actions
      needed to enter powerdown, including the 'wfi' invocation.

   -  PSCI STAT residency and count functions have been added on Arm platforms
      by using PMF.

-  Enhancements to the translation table library:

   -  Limited memory mapping support for region overlaps to only allow regions
      to overlap that are identity mapped or have the same virtual to physical
      address offset, and overlap completely but must not cover the same area.

      This limitation will enable future enhancements without having to
      support complex edge cases that may not be necessary.

   -  The initial translation lookup level is now inferred from the virtual
      address space size. Previously, it was hard-coded.

   -  Added support for mapping Normal, Inner Non-cacheable, Outer
      Non-cacheable memory in the translation table library.

      This can be useful to map a non-cacheable memory region, such as a DMA
      buffer.

   -  Introduced the MT_EXECUTE/MT_EXECUTE_NEVER memory mapping attributes to
      specify the access permissions for instruction execution of a memory
      region.

-  Enabled support to isolate code and read-only data on separate memory pages,
   allowing independent access control to be applied to each.

-  Enabled SCR_EL3.SIF (Secure Instruction Fetch) bit in BL1 and BL31 common
   architectural setup code, preventing fetching instructions from non-secure
   memory when in secure state.

-  Enhancements to FIP support:

   -  Replaced ``fip_create`` with ``fiptool`` which provides a more consistent
      and intuitive interface as well as additional support to remove an image
      from a FIP file.

   -  Enabled printing the SHA256 digest with info command, allowing quick
      verification of an image within a FIP without having to extract the
      image and running sha256sum on it.

   -  Added support for unpacking the contents of an existing FIP file into
      the working directory.

   -  Aligned command line options for specifying images to use same naming
      convention as specified by TBBR and already used in cert_create tool.

-  Refactored the TZC-400 driver to also support memory controllers that
   integrate TZC functionality, for example Arm CoreLink DMC-500. Also added
   DMC-500 specific support.

-  Implemented generic delay timer based on the system generic counter and
   migrated all platforms to use it.

-  Enhanced support for Arm platforms:

   -  Updated image loading support to make SCP images (SCP_BL2 and SCP_BL2U)
      optional.

   -  Enhanced topology description support to allow multi-cluster topology
      definitions.

   -  Added interconnect abstraction layer to help platform ports select the
      right interconnect driver, CCI or CCN, for the platform.

   -  Added support to allow loading BL31 in the TZC-secured DRAM instead of
      the default secure SRAM.

   -  Added support to use a System Security Control (SSC) Registers Unit
      enabling TF-A to be compiled to support multiple Arm platforms and
      then select one at runtime.

   -  Restricted mapping of Trusted ROM in BL1 to what is actually needed by
      BL1 rather than entire Trusted ROM region.

   -  Flash is now mapped as execute-never by default. This increases security
      by restricting the executable region to what is strictly needed.

-  Applied following erratum workarounds for Cortex-A57: 833471, 826977,
   829520, 828024 and 826974.

-  Added support for Mediatek MT6795 platform.

-  Added support for QEMU virtualization Armv8-A target.

-  Added support for Rockchip RK3368 and RK3399 platforms.

-  Added support for Xilinx Zynq UltraScale+ MPSoC platform.

-  Added support for Arm Cortex-A73 MPCore Processor.

-  Added support for Arm Cortex-A72 processor.

-  Added support for Arm Cortex-A35 processor.

-  Added support for Arm Cortex-A32 MPCore Processor.

-  Enabled preloaded BL33 alternative boot flow, in which BL2 does not load
   BL33 from non-volatile storage and BL31 hands execution over to a preloaded
   BL33. The User Guide has been updated with an example of how to use this
   option with a bootwrapped kernel.

-  Added support to build TF-A on a Windows-based host machine.

-  Updated Trusted Board Boot prototype implementation:

   -  Enabled the ability for a production ROM with TBBR enabled to boot test
      software before a real ROTPK is deployed (e.g. manufacturing mode).
      Added support to use ROTPK in certificate without verifying against the
      platform value when ``ROTPK_NOT_DEPLOYED`` bit is set.

   -  Added support for non-volatile counter authentication to the
      Authentication Module to protect against roll-back.

-  Updated GICv3 support:

   -  Enabled processor power-down and automatic power-on using GICv3.

   -  Enabled G1S or G0 interrupts to be configured independently.

   -  Changed FVP default interrupt driver to be the GICv3-only driver.
      **Note** the default build of TF-A will not be able to boot
      Linux kernel with GICv2 FDT blob.

   -  Enabled wake-up from CPU_SUSPEND to stand-by by temporarily re-routing
      interrupts and then restoring after resume.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Known issues
^^^^^^^^^^^^

-  The version of the AEMv8 Base FVP used in this release resets the model
   instead of terminating its execution in response to a shutdown request using
   the PSCI ``SYSTEM_OFF`` API. This issue will be fixed in a future version of
   the model.

-  Building TF-A with compiler optimisations disabled (``-O0``) fails.

-  TF-A cannot be built with mbed TLS version v2.3.0 due to build warnings
   that the TF-A build system interprets as errors.

-  TBBR is not currently supported when running TF-A in AArch32 state.

Version 1.2
-----------

New features
^^^^^^^^^^^^

-  The Trusted Board Boot implementation on Arm platforms now conforms to the
   mandatory requirements of the TBBR specification.

   In particular, the boot process is now guarded by a Trusted Watchdog, which
   will reset the system in case of an authentication or loading error. On Arm
   platforms, a secure instance of Arm SP805 is used as the Trusted Watchdog.

   Also, a firmware update process has been implemented. It enables
   authenticated firmware to update firmware images from external interfaces to
   SoC Non-Volatile memories. This feature functions even when the current
   firmware in the system is corrupt or missing; it therefore may be used as
   a recovery mode.

-  Improvements have been made to the Certificate Generation Tool
   (``cert_create``) as follows.

   -  Added support for the Firmware Update process by extending the Chain
      of Trust definition in the tool to include the Firmware Update
      certificate and the required extensions.

   -  Introduced a new API that allows one to specify command line options in
      the Chain of Trust description. This makes the declaration of the tool's
      arguments more flexible and easier to extend.

   -  The tool has been reworked to follow a data driven approach, which
      makes it easier to maintain and extend.

-  Extended the FIP tool (``fip_create``) to support the new set of images
   involved in the Firmware Update process.

-  Various memory footprint improvements. In particular:

   -  The bakery lock structure for coherent memory has been optimised.

   -  The mbed TLS SHA1 functions are not needed, as SHA256 is used to
      generate the certificate signature. Therefore, they have been compiled
      out, reducing the memory footprint of BL1 and BL2 by approximately
      6 KB.

   -  On Arm development platforms, each BL stage now individually defines
      the number of regions that it needs to map in the MMU.

-  Added the following new design documents:

   -  :ref:`Authentication Framework & Chain of Trust`
   -  :ref:`Firmware Update (FWU)`
   -  :ref:`CPU Reset`
   -  :ref:`PSCI Power Domain Tree Structure`

-  Applied the new image terminology to the code base and documentation, as
   described in the :ref:`Image Terminology` document.

-  The build system has been reworked to improve readability and facilitate
   adding future extensions.

-  On Arm standard platforms, BL31 uses the boot console during cold boot
   but switches to the runtime console for any later logs at runtime. The TSP
   uses the runtime console for all output.

-  Implemented a basic NOR flash driver for Arm platforms. It programs the
   device using CFI (Common Flash Interface) standard commands.

-  Implemented support for booting EL3 payloads on Arm platforms, which
   reduces the complexity of developing EL3 baremetal code by doing essential
   baremetal initialization.

-  Provided separate drivers for GICv3 and GICv2. These expect the entire
   software stack to use either GICv2 or GICv3; hybrid GIC software systems
   are no longer supported and the legacy Arm GIC driver has been deprecated.

-  Added support for Juno r1 and r2. A single set of Juno TF-A binaries can run
   on Juno r0, r1 and r2 boards. Note that this TF-A version depends on a Linaro
   release that does *not* contain Juno r2 support.

-  Added support for MediaTek mt8173 platform.

-  Implemented a generic driver for Arm CCN IP.

-  Major rework of the PSCI implementation.

   -  Added framework to handle composite power states.

   -  Decoupled the notions of affinity instances (which describes the
      hierarchical arrangement of cores) and of power domain topology, instead
      of assuming a one-to-one mapping.

   -  Better alignment with version 1.0 of the PSCI specification.

-  Added support for the SYSTEM_SUSPEND PSCI API on Arm platforms. When invoked
   on the last running core on a supported platform, this puts the system
   into a low power mode with memory retention.

-  Unified the reset handling code as much as possible across BL stages.
   Also introduced some build options to enable optimization of the reset path
   on platforms that support it.

-  Added a simple delay timer API, as well as an SP804 timer driver, which is
   enabled on FVP.

-  Added support for NVidia Tegra T210 and T132 SoCs.

-  Reorganised Arm platforms ports to greatly improve code shareability and
   facilitate the reuse of some of this code by other platforms.

-  Added support for Arm Cortex-A72 processor in the CPU specific framework.

-  Provided better error handling. Platform ports can now define their own
   error handling, for example to perform platform specific bookkeeping or
   post-error actions.

-  Implemented a unified driver for Arm Cache Coherent Interconnects used for
   both CCI-400 & CCI-500 IPs. Arm platforms ports have been migrated to this
   common driver. The standalone CCI-400 driver has been deprecated.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  The Trusted Board Boot implementation has been redesigned to provide greater
   modularity and scalability. See the
   :ref:`Authentication Framework & Chain of Trust` document.
   All missing mandatory features are now implemented.

-  The FVP and Juno ports may now use the hash of the ROTPK stored in the
   Trusted Key Storage registers to verify the ROTPK. Alternatively, a
   development public key hash embedded in the BL1 and BL2 binaries might be
   used instead. The location of the ROTPK is chosen at build-time using the
   ``ARM_ROTPK_LOCATION`` build option.

-  GICv3 is now fully supported and stable.

Known issues
^^^^^^^^^^^^

-  The version of the AEMv8 Base FVP used in this release resets the model
   instead of terminating its execution in response to a shutdown request using
   the PSCI ``SYSTEM_OFF`` API. This issue will be fixed in a future version of
   the model.

-  While this version has low on-chip RAM requirements, there are further
   RAM usage enhancements that could be made.

-  The upstream documentation could be improved for structural consistency,
   clarity and completeness. In particular, the design documentation is
   incomplete for PSCI, the TSP(D) and the Juno platform.

-  Building TF-A with compiler optimisations disabled (``-O0``) fails.

Version 1.1
-----------

New features
^^^^^^^^^^^^

-  A prototype implementation of Trusted Board Boot has been added. Boot
   loader images are verified by BL1 and BL2 during the cold boot path. BL1 and
   BL2 use the PolarSSL SSL library to verify certificates and images. The
   OpenSSL library is used to create the X.509 certificates. Support has been
   added to ``fip_create`` tool to package the certificates in a FIP.

-  Support for calling CPU and platform specific reset handlers upon entry into
   BL3-1 during the cold and warm boot paths has been added. This happens after
   another Boot ROM ``reset_handler()`` has already run. This enables a developer
   to perform additional actions or undo actions already performed during the
   first call of the reset handlers e.g. apply additional errata workarounds.

-  Support has been added to demonstrate routing of IRQs to EL3 instead of
   S-EL1 when execution is in secure world.

-  The PSCI implementation now conforms to version 1.0 of the PSCI
   specification. All the mandatory APIs and selected optional APIs are
   supported. In particular, support for the ``PSCI_FEATURES`` API has been
   added. A capability variable is constructed during initialization by
   examining the ``plat_pm_ops`` and ``spd_pm_ops`` exported by the platform and
   the Secure Payload Dispatcher. This is used by the PSCI FEATURES function
   to determine which PSCI APIs are supported by the platform.

-  Improvements have been made to the PSCI code as follows.

   -  The code has been refactored to remove redundant parameters from
      internal functions.

   -  Changes have been made to the code for PSCI ``CPU_SUSPEND``, ``CPU_ON`` and
      ``CPU_OFF`` calls to facilitate an early return to the caller in case a
      failure condition is detected. For example, a PSCI ``CPU_SUSPEND`` call
      returns ``SUCCESS`` to the caller if a pending interrupt is detected early
      in the code path.

   -  Optional platform APIs have been added to validate the ``power_state`` and
      ``entrypoint`` parameters early in PSCI ``CPU_ON`` and ``CPU_SUSPEND`` code
      paths.

   -  PSCI migrate APIs have been reworked to invoke the SPD hook to determine
      the type of Trusted OS and the CPU it is resident on (if
      applicable). Also, during a PSCI ``MIGRATE`` call, the SPD hook to migrate
      the Trusted OS is invoked.

-  It is now possible to build TF-A without marking at least an extra page of
   memory as coherent. The build flag ``USE_COHERENT_MEM`` can be used to
   choose between the two implementations. This has been made possible through
   these changes.

   -  An implementation of Bakery locks, where the locks are not allocated in
      coherent memory has been added.

   -  Memory which was previously marked as coherent is now kept coherent
      through the use of software cache maintenance operations.

   Approximately, 4K worth of memory is saved for each boot loader stage when
   ``USE_COHERENT_MEM=0``. Enabling this option increases the latencies
   associated with acquire and release of locks. It also requires changes to
   the platform ports.

-  It is now possible to specify the name of the FIP at build time by defining
   the ``FIP_NAME`` variable.

-  Issues with dependencies on the 'fiptool' makefile target have been
   rectified. The ``fip_create`` tool is now rebuilt whenever its source files
   change.

-  The BL3-1 runtime console is now also used as the crash console. The crash
   console is changed to SoC UART0 (UART2) from the previous FPGA UART0 (UART0)
   on Juno. In FVP, it is changed from UART0 to UART1.

-  CPU errata workarounds are applied only when the revision and part number
   match. This behaviour has been made consistent across the debug and release
   builds. The debug build additionally prints a warning if a mismatch is
   detected.

-  It is now possible to issue cache maintenance operations by set/way for a
   particular level of data cache. Levels 1-3 are currently supported.

-  The following improvements have been made to the FVP port.

   -  The build option ``FVP_SHARED_DATA_LOCATION`` which allowed relocation of
      shared data into the Trusted DRAM has been deprecated. Shared data is
      now always located at the base of Trusted SRAM.

   -  BL2 Translation tables have been updated to map only the region of
      DRAM which is accessible to normal world. This is the region of the 2GB
      DDR-DRAM memory at 0x80000000 excluding the top 16MB. The top 16MB is
      accessible to only the secure world.

   -  BL3-2 can now reside in the top 16MB of DRAM which is accessible only to
      the secure world. This can be done by setting the build flag
      ``FVP_TSP_RAM_LOCATION`` to the value ``dram``.

-  Separate translation tables are created for each boot loader image. The
   ``IMAGE_BLx`` build options are used to do this. This allows each stage to
   create mappings only for areas in the memory map that it needs.

-  A Secure Payload Dispatcher (OPTEED) for the OP-TEE Trusted OS has been
   added. Details of using it with TF-A can be found in :ref:`OP-TEE Dispatcher`

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  The Juno port has been aligned with the FVP port as follows.

   -  Support for reclaiming all BL1 RW memory and BL2 memory by overlaying
      the BL3-1/BL3-2 NOBITS sections on top of them has been added to the
      Juno port.

   -  The top 16MB of the 2GB DDR-DRAM memory at 0x80000000 is configured
      using the TZC-400 controller to be accessible only to the secure world.

   -  The Arm GIC driver is used to configure the GIC-400 instead of using a
      GIC driver private to the Juno port.

   -  PSCI ``CPU_SUSPEND`` calls that target a standby state are now supported.

   -  The TZC-400 driver is used to configure the controller instead of direct
      accesses to the registers.

-  The Linux kernel version referred to in the user guide has DVFS and HMP
   support enabled.

-  DS-5 v5.19 did not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in
   CADI server mode. This issue is not seen with DS-5 v5.20 and Version 6.2 of
   the Cortex-A57-A53 Base FVPs.

Known issues
^^^^^^^^^^^^

-  The Trusted Board Boot implementation is a prototype. There are issues with
   the modularity and scalability of the design. Support for a Trusted
   Watchdog, firmware update mechanism, recovery images and Trusted debug is
   absent. These issues will be addressed in future releases.

-  The FVP and Juno ports do not use the hash of the ROTPK stored in the
   Trusted Key Storage registers to verify the ROTPK in the
   ``plat_match_rotpk()`` function. This prevents the correct establishment of
   the Chain of Trust at the first step in the Trusted Board Boot process.

-  The version of the AEMv8 Base FVP used in this release resets the model
   instead of terminating its execution in response to a shutdown request using
   the PSCI ``SYSTEM_OFF`` API. This issue will be fixed in a future version of
   the model.

-  GICv3 support is experimental. There are known issues with GICv3
   initialization in the TF-A.

-  While this version greatly reduces the on-chip RAM requirements, there are
   further RAM usage enhancements that could be made.

-  The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
   its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

-  The Juno-specific firmware design documentation is incomplete.

Version 1.0
-----------

New features
^^^^^^^^^^^^

-  It is now possible to map higher physical addresses using non-flat virtual
   to physical address mappings in the MMU setup.

-  Wider use is now made of the per-CPU data cache in BL3-1 to store:

   -  Pointers to the non-secure and secure security state contexts.

   -  A pointer to the CPU-specific operations.

   -  A pointer to PSCI specific information (for example the current power
      state).

   -  A crash reporting buffer.

-  The following RAM usage improvements result in a BL3-1 RAM usage reduction
   from 96KB to 56KB (for FVP with TSPD), and a total RAM usage reduction
   across all images from 208KB to 88KB, compared to the previous release.

   -  Removed the separate ``early_exception`` vectors from BL3-1 (2KB code size
      saving).

   -  Removed NSRAM from the FVP memory map, allowing the removal of one
      (4KB) translation table.

   -  Eliminated the internal ``psci_suspend_context`` array, saving 2KB.

   -  Correctly dimensioned the PSCI ``aff_map_node`` array, saving 1.5KB in the
      FVP port.

   -  Removed calling CPU mpidr from the bakery lock API, saving 160 bytes.

   -  Removed current CPU mpidr from PSCI common code, saving 160 bytes.

   -  Inlined the mmio accessor functions, saving 360 bytes.

   -  Fully reclaimed all BL1 RW memory and BL2 memory on the FVP port by
      overlaying the BL3-1/BL3-2 NOBITS sections on top of these at runtime.

   -  Made storing the FP register context optional, saving 0.5KB per context
      (8KB on the FVP port, with TSPD enabled and running on 8 CPUs).

   -  Implemented a leaner ``tf_printf()`` function, allowing the stack to be
      greatly reduced.

   -  Removed coherent stacks from the codebase. Stacks allocated in normal
      memory are now used before and after the MMU is enabled. This saves 768
      bytes per CPU in BL3-1.

   -  Reworked the crash reporting in BL3-1 to use less stack.

   -  Optimized the EL3 register state stored in the ``cpu_context`` structure
      so that registers that do not change during normal execution are
      re-initialized each time during cold/warm boot, rather than restored
      from memory. This saves about 1.2KB.

   -  As a result of some of the above, reduced the runtime stack size in all
      BL images. For BL3-1, this saves 1KB per CPU.

-  PSCI SMC handler improvements to correctly handle calls from secure states
   and from AArch32.

-  CPU contexts are now initialized from the ``entry_point_info``. BL3-1 fully
   determines the exception level to use for the non-trusted firmware (BL3-3)
   based on the SPSR value provided by the BL2 platform code (or otherwise
   provided to BL3-1). This allows platform code to directly run non-trusted
   firmware payloads at either EL2 or EL1 without requiring an EL2 stub or OS
   loader.

-  Code refactoring improvements:

   -  Refactored ``fvp_config`` into a common platform header.

   -  Refactored the fvp gic code to be a generic driver that no longer has an
      explicit dependency on platform code.

   -  Refactored the CCI-400 driver to not have dependency on platform code.

   -  Simplified the IO driver so it's no longer necessary to call ``io_init()``
      and moved all the IO storage framework code to one place.

   -  Simplified the interface the the TZC-400 driver.

   -  Clarified the platform porting interface to the TSP.

   -  Reworked the TSPD setup code to support the alternate BL3-2
      initialization flow where BL3-1 generic code hands control to BL3-2,
      rather than expecting the TSPD to hand control directly to BL3-2.

   -  Considerable rework to PSCI generic code to support CPU specific
      operations.

-  Improved console log output, by:

   -  Adding the concept of debug log levels.

   -  Rationalizing the existing debug messages and adding new ones.

   -  Printing out the version of each BL stage at runtime.

   -  Adding support for printing console output from assembler code,
      including when a crash occurs before the C runtime is initialized.

-  Moved up to the latest versions of the FVPs, toolchain, EDK2, kernel, Linaro
   file system and DS-5.

-  On the FVP port, made the use of the Trusted DRAM region optional at build
   time (off by default). Normal platforms will not have such a "ready-to-use"
   DRAM area so it is not a good example to use it.

-  Added support for PSCI ``SYSTEM_OFF`` and ``SYSTEM_RESET`` APIs.

-  Added support for CPU specific reset sequences, power down sequences and
   register dumping during crash reporting. The CPU specific reset sequences
   include support for errata workarounds.

-  Merged the Juno port into the master branch. Added support for CPU hotplug
   and CPU idle. Updated the user guide to describe how to build and run on the
   Juno platform.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Removed the concept of top/bottom image loading. The image loader now
   automatically detects the position of the image inside the current memory
   layout and updates the layout to minimize fragmentation. This resolves the
   image loader limitations of previously releases. There are currently no
   plans to support dynamic image loading.

-  CPU idle now works on the publicized version of the Foundation FVP.

-  All known issues relating to the compiler version used have now been
   resolved. This TF-A version uses Linaro toolchain 14.07 (based on GCC 4.9).

Known issues
^^^^^^^^^^^^

-  GICv3 support is experimental. The Linux kernel patches to support this are
   not widely available. There are known issues with GICv3 initialization in
   the TF-A.

-  While this version greatly reduces the on-chip RAM requirements, there are
   further RAM usage enhancements that could be made.

-  The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
   its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

-  The Juno-specific firmware design documentation is incomplete.

-  Some recent enhancements to the FVP port have not yet been translated into
   the Juno port. These will be tracked via the tf-issues project.

-  The Linux kernel version referred to in the user guide has DVFS and HMP
   support disabled due to some known instabilities at the time of this
   release. A future kernel version will re-enable these features.

-  DS-5 v5.19 does not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in
   CADI server mode. This is because the ``<SimName>`` reported by the FVP in
   this version has changed. For example, for the Cortex-A57x4-A53x4 Base FVP,
   the ``<SimName>`` reported by the FVP is ``FVP_Base_Cortex_A57x4_A53x4``, while
   DS-5 expects it to be ``FVP_Base_A57x4_A53x4``.

   The temporary fix to this problem is to change the name of the FVP in
   ``sw/debugger/configdb/Boards/ARM FVP/Base_A57x4_A53x4/cadi_config.xml``.
   Change the following line:

   ::

       <SimName>System Generator:FVP_Base_A57x4_A53x4</SimName>

   to
   System Generator:FVP_Base_Cortex-A57x4_A53x4

   A similar change can be made to the other Cortex-A57-A53 Base FVP variants.

Version 0.4
-----------

New features
^^^^^^^^^^^^

-  Makefile improvements:

   -  Improved dependency checking when building.

   -  Removed ``dump`` target (build now always produces dump files).

   -  Enabled platform ports to optionally make use of parts of the Trusted
      Firmware (e.g. BL3-1 only), rather than being forced to use all parts.
      Also made the ``fip`` target optional.

   -  Specified the full path to source files and removed use of the ``vpath``
      keyword.

-  Provided translation table library code for potential re-use by platforms
   other than the FVPs.

-  Moved architectural timer setup to platform-specific code.

-  Added standby state support to PSCI cpu_suspend implementation.

-  SRAM usage improvements:

   -  Started using the ``-ffunction-sections``, ``-fdata-sections`` and
      ``--gc-sections`` compiler/linker options to remove unused code and data
      from the images. Previously, all common functions were being built into
      all binary images, whether or not they were actually used.

   -  Placed all assembler functions in their own section to allow more unused
      functions to be removed from images.

   -  Updated BL1 and BL2 to use a single coherent stack each, rather than one
      per CPU.

   -  Changed variables that were unnecessarily declared and initialized as
      non-const (i.e. in the .data section) so they are either uninitialized
      (zero init) or const.

-  Moved the Test Secure-EL1 Payload (BL3-2) to execute in Trusted SRAM by
   default. The option for it to run in Trusted DRAM remains.

-  Implemented a TrustZone Address Space Controller (TZC-400) driver. A
   default configuration is provided for the Base FVPs. This means the model
   parameter ``-C bp.secure_memory=1`` is now supported.

-  Started saving the PSCI cpu_suspend 'power_state' parameter prior to
   suspending a CPU. This allows platforms that implement multiple power-down
   states at the same affinity level to identify a specific state.

-  Refactored the entire codebase to reduce the amount of nesting in header
   files and to make the use of system/user includes more consistent. Also
   split platform.h to separate out the platform porting declarations from the
   required platform porting definitions and the definitions/declarations
   specific to the platform port.

-  Optimized the data cache clean/invalidate operations.

-  Improved the BL3-1 unhandled exception handling and reporting. Unhandled
   exceptions now result in a dump of registers to the console.

-  Major rework to the handover interface between BL stages, in particular the
   interface to BL3-1. The interface now conforms to a specification and is
   more future proof.

-  Added support for optionally making the BL3-1 entrypoint a reset handler
   (instead of BL1). This allows platforms with an alternative image loading
   architecture to re-use BL3-1 with fewer modifications to generic code.

-  Reserved some DDR DRAM for secure use on FVP platforms to avoid future
   compatibility problems with non-secure software.

-  Added support for secure interrupts targeting the Secure-EL1 Payload (SP)
   (using GICv2 routing only). Demonstrated this working by adding an interrupt
   target and supporting test code to the TSP. Also demonstrated non-secure
   interrupt handling during TSP processing.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Now support use of the model parameter ``-C bp.secure_memory=1`` in the Base
   FVPs (see **New features**).

-  Support for secure world interrupt handling now available (see **New
   features**).

-  Made enough SRAM savings (see **New features**) to enable the Test Secure-EL1
   Payload (BL3-2) to execute in Trusted SRAM by default.

-  The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
   14.04) now correctly reports progress in the console.

-  Improved the Makefile structure to make it easier to separate out parts of
   the TF-A for re-use in platform ports. Also, improved target dependency
   checking.

Known issues
^^^^^^^^^^^^

-  GICv3 support is experimental. The Linux kernel patches to support this are
   not widely available. There are known issues with GICv3 initialization in
   the TF-A.

-  Dynamic image loading is not available yet. The current image loader
   implementation (used to load BL2 and all subsequent images) has some
   limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
   to loading errors, even if the images should theoretically fit in memory.

-  TF-A still uses too much on-chip Trusted SRAM. A number of RAM usage
   enhancements have been identified to rectify this situation.

-  CPU idle does not work on the advertised version of the Foundation FVP.
   Some FVP fixes are required that are not available externally at the time
   of writing. This can be worked around by disabling CPU idle in the Linux
   kernel.

-  Various bugs in TF-A, UEFI and the Linux kernel have been observed when
   using Linaro toolchain versions later than 13.11. Although most of these
   have been fixed, some remain at the time of writing. These mainly seem to
   relate to a subtle change in the way the compiler converts between 64-bit
   and 32-bit values (e.g. during casting operations), which reveals
   previously hidden bugs in client code.

-  The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
   its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

Version 0.3
-----------

New features
^^^^^^^^^^^^

-  Support for Foundation FVP Version 2.0 added.
   The documented UEFI configuration disables some devices that are unavailable
   in the Foundation FVP, including MMC and CLCD. The resultant UEFI binary can
   be used on the AEMv8 and Cortex-A57-A53 Base FVPs, as well as the Foundation
   FVP.

   .. note::
      The software will not work on Version 1.0 of the Foundation FVP.

-  Enabled third party contributions. Added a new contributing.md containing
   instructions for how to contribute and updated copyright text in all files
   to acknowledge contributors.

-  The PSCI CPU_SUSPEND API has been stabilised to the extent where it can be
   used for entry into power down states with the following restrictions:

   -  Entry into standby states is not supported.
   -  The API is only supported on the AEMv8 and Cortex-A57-A53 Base FVPs.

-  The PSCI AFFINITY_INFO api has undergone limited testing on the Base FVPs to
   allow experimental use.

-  Required C library and runtime header files are now included locally in
   TF-A instead of depending on the toolchain standard include paths. The
   local implementation has been cleaned up and reduced in scope.

-  Added I/O abstraction framework, primarily to allow generic code to load
   images in a platform-independent way. The existing image loading code has
   been reworked to use the new framework. Semi-hosting and NOR flash I/O
   drivers are provided.

-  Introduced Firmware Image Package (FIP) handling code and tools. A FIP
   combines multiple firmware images with a Table of Contents (ToC) into a
   single binary image. The new FIP driver is another type of I/O driver. The
   Makefile builds a FIP by default and the FVP platform code expect to load a
   FIP from NOR flash, although some support for image loading using semi-
   hosting is retained.

   .. note::
      Building a FIP by default is a non-backwards-compatible change.

   .. note::
      Generic BL2 code now loads a BL3-3 (non-trusted firmware) image into
      DRAM instead of expecting this to be pre-loaded at known location. This is
      also a non-backwards-compatible change.

   .. note::
      Some non-trusted firmware (e.g. UEFI) will need to be rebuilt so that
      it knows the new location to execute from and no longer needs to copy
      particular code modules to DRAM itself.

-  Reworked BL2 to BL3-1 handover interface. A new composite structure
   (bl31_args) holds the superset of information that needs to be passed from
   BL2 to BL3-1, including information on how handover execution control to
   BL3-2 (if present) and BL3-3 (non-trusted firmware).

-  Added library support for CPU context management, allowing the saving and
   restoring of

   -  Shared system registers between Secure-EL1 and EL1.
   -  VFP registers.
   -  Essential EL3 system registers.

-  Added a framework for implementing EL3 runtime services. Reworked the PSCI
   implementation to be one such runtime service.

-  Reworked the exception handling logic, making use of both SP_EL0 and SP_EL3
   stack pointers for determining the type of exception, managing general
   purpose and system register context on exception entry/exit, and handling
   SMCs. SMCs are directed to the correct EL3 runtime service.

-  Added support for a Test Secure-EL1 Payload (TSP) and a corresponding
   Dispatcher (TSPD), which is loaded as an EL3 runtime service. The TSPD
   implements Secure Monitor functionality such as world switching and
   EL1 context management, and is responsible for communication with the TSP.

   .. note::
      The TSPD does not yet contain support for secure world interrupts.
   .. note::
      The TSP/TSPD is not built by default.

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Support has been added for switching context between secure and normal
   worlds in EL3.

-  PSCI API calls ``AFFINITY_INFO`` & ``PSCI_VERSION`` have now been tested (to
   a limited extent).

-  The TF-A build artifacts are now placed in the ``./build`` directory and
   sub-directories instead of being placed in the root of the project.

-  TF-A is now free from build warnings. Build warnings are now treated as
   errors.

-  TF-A now provides C library support locally within the project to maintain
   compatibility between toolchains/systems.

-  The PSCI locking code has been reworked so it no longer takes locks in an
   incorrect sequence.

-  The RAM-disk method of loading a Linux file-system has been confirmed to
   work with the TF-A and Linux kernel version (based on version 3.13) used
   in this release, for both Foundation and Base FVPs.

Known issues
^^^^^^^^^^^^

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

-  The TrustZone Address Space Controller (TZC-400) is not being programmed
   yet. Use of model parameter ``-C bp.secure_memory=1`` is not supported.

-  No support yet for secure world interrupt handling.

-  GICv3 support is experimental. The Linux kernel patches to support this are
   not widely available. There are known issues with GICv3 initialization in
   TF-A.

-  Dynamic image loading is not available yet. The current image loader
   implementation (used to load BL2 and all subsequent images) has some
   limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
   to loading errors, even if the images should theoretically fit in memory.

-  TF-A uses too much on-chip Trusted SRAM. Currently the Test Secure-EL1
   Payload (BL3-2) executes in Trusted DRAM since there is not enough SRAM.
   A number of RAM usage enhancements have been identified to rectify this
   situation.

-  CPU idle does not work on the advertised version of the Foundation FVP.
   Some FVP fixes are required that are not available externally at the time
   of writing.

-  Various bugs in TF-A, UEFI and the Linux kernel have been observed when
   using Linaro toolchain versions later than 13.11. Although most of these
   have been fixed, some remain at the time of writing. These mainly seem to
   relate to a subtle change in the way the compiler converts between 64-bit
   and 32-bit values (e.g. during casting operations), which reveals
   previously hidden bugs in client code.

-  The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
   14.01) does not report progress correctly in the console. It only seems to
   produce error output, not standard output. It otherwise appears to function
   correctly. Other filesystem versions on the same software stack do not
   exhibit the problem.

-  The Makefile structure doesn't make it easy to separate out parts of the
   TF-A for re-use in platform ports, for example if only BL3-1 is required in
   a platform port. Also, dependency checking in the Makefile is flawed.

-  The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
   its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

Version 0.2
-----------

New features
^^^^^^^^^^^^

-  First source release.

-  Code for the PSCI suspend feature is supplied, although this is not enabled
   by default since there are known issues (see below).

Issues resolved since last release
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  The "psci" nodes in the FDTs provided in this release now fully comply
   with the recommendations made in the PSCI specification.

Known issues
^^^^^^^^^^^^

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

-  The TrustZone Address Space Controller (TZC-400) is not being programmed
   yet. Use of model parameter ``-C bp.secure_memory=1`` is not supported.

-  No support yet for secure world interrupt handling or for switching context
   between secure and normal worlds in EL3.

-  GICv3 support is experimental. The Linux kernel patches to support this are
   not widely available. There are known issues with GICv3 initialization in
   TF-A.

-  Dynamic image loading is not available yet. The current image loader
   implementation (used to load BL2 and all subsequent images) has some
   limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead
   to loading errors, even if the images should theoretically fit in memory.

-  Although support for PSCI ``CPU_SUSPEND`` is present, it is not yet stable
   and ready for use.

-  PSCI API calls ``AFFINITY_INFO`` & ``PSCI_VERSION`` are implemented but have
   not been tested.

-  The TF-A make files result in all build artifacts being placed in the root
   of the project. These should be placed in appropriate sub-directories.

-  The compilation of TF-A is not free from compilation warnings. Some of these
   warnings have not been investigated yet so they could mask real bugs.

-  TF-A currently uses toolchain/system include files like stdio.h. It should
   provide versions of these within the project to maintain compatibility
   between toolchains/systems.

-  The PSCI code takes some locks in an incorrect sequence. This may cause
   problems with suspend and hotplug in certain conditions.

-  The Linux kernel used in this release is based on version 3.12-rc4. Using
   this kernel with the TF-A fails to start the file-system as a RAM-disk. It
   fails to execute user-space ``init`` from the RAM-disk. As an alternative,
   the VirtioBlock mechanism can be used to provide a file-system to the
   kernel.

--------------

*Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.*

.. _SDEI Specification: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _tf-issue#501: https://github.com/ARM-software/tf-issues/issues/501
.. _PR#1002: https://github.com/ARM-software/arm-trusted-firmware/pull/1002#issuecomment-312650193
.. _mbed TLS releases: https://tls.mbed.org/tech-updates/releases
