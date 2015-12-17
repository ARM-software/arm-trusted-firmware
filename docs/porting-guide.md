ARM Trusted Firmware Porting Guide
==================================

Contents
--------

1.  [Introduction](#1--introduction)
2.  [Common Modifications](#2--common-modifications)
    *   [Common mandatory modifications](#21-common-mandatory-modifications)
    *   [Handling reset](#22-handling-reset)
    *   [Common mandatory modifications](#23-common-mandatory-modifications)
    *   [Common optional modifications](#24-common-optional-modifications)
3.  [Boot Loader stage specific modifications](#3--modifications-specific-to-a-boot-loader-stage)
    *   [Boot Loader stage 1 (BL1)](#31-boot-loader-stage-1-bl1)
    *   [Boot Loader stage 2 (BL2)](#32-boot-loader-stage-2-bl2)
    *   [FWU Boot Loader stage 2 (BL2U)](#33-fwu-boot-loader-stage-2-bl2u)
    *   [Boot Loader stage 3-1 (BL31)](#34-boot-loader-stage-3-1-bl31)
    *   [PSCI implementation (in BL31)](#35-power-state-coordination-interface-in-bl31)
    *   [Interrupt Management framework (in BL31)](#36--interrupt-management-framework-in-bl31)
    *   [Crash Reporting mechanism (in BL31)](#37--crash-reporting-mechanism-in-bl31)
4.  [Build flags](#4--build-flags)
5.  [C Library](#5--c-library)
6.  [Storage abstraction layer](#6--storage-abstraction-layer)

- - - - - - - - - - - - - - - - - -

1.  Introduction
----------------

Please note that this document has been updated for the new platform API
as required by the PSCI v1.0 implementation. Please refer to the
[Migration Guide] for the previous platform API.

Porting the ARM Trusted Firmware to a new platform involves making some
mandatory and optional modifications for both the cold and warm boot paths.
Modifications consist of:

*   Implementing a platform-specific function or variable,
*   Setting up the execution context in a certain way, or
*   Defining certain constants (for example #defines).

The platform-specific functions and variables are declared in
[include/plat/common/platform.h]. The firmware provides a default implementation
of variables and functions to fulfill the optional requirements. These
implementations are all weakly defined; they are provided to ease the porting
effort. Each platform port can override them with its own implementation if the
default implementation is inadequate.

Platform ports that want to be aligned with standard ARM platforms (for example
FVP and Juno) may also use [include/plat/arm/common/plat_arm.h] and the
corresponding source files in `plat/arm/common/`. These provide standard
implementations for some of the required platform porting functions. However,
using these functions requires the platform port to implement additional
ARM standard platform porting functions. These additional functions are not
documented here.

Some modifications are common to all Boot Loader (BL) stages. Section 2
discusses these in detail. The subsequent sections discuss the remaining
modifications for each BL stage in detail.

This document should be read in conjunction with the ARM Trusted Firmware
[User Guide].


2.  Common modifications
------------------------

This section covers the modifications that should be made by the platform for
each BL stage to correctly port the firmware stack. They are categorized as
either mandatory or optional.


2.1 Common mandatory modifications
----------------------------------
A platform port must enable the Memory Management Unit (MMU) with identity
mapped page tables, and enable both the instruction and data caches for each BL
stage. In ARM standard platforms, each BL stage configures the MMU in
the platform-specific architecture setup function, `blX_plat_arch_setup()`.

If the build option `USE_COHERENT_MEM` is enabled, each platform can allocate a
block of identity mapped secure memory with Device-nGnRE attributes aligned to
page boundary (4K) for each BL stage. All sections which allocate coherent
memory are grouped under `coherent_ram`. For ex: Bakery locks are placed in a
section identified by name `bakery_lock` inside `coherent_ram` so that its
possible for the firmware to place variables in it using the following C code
directive:

    __attribute__ ((section("bakery_lock")))

Or alternatively the following assembler code directive:

    .section bakery_lock

The `coherent_ram` section is a sum of all sections like `bakery_lock` which are
used to allocate any data structures that are accessed both when a CPU is
executing with its MMU and caches enabled, and when it's running with its MMU
and caches disabled. Examples are given below.

The following variables, functions and constants must be defined by the platform
for the firmware to work correctly.


### File : platform_def.h [mandatory]

Each platform must ensure that a header file of this name is in the system
include path with the following constants defined. This may require updating the
list of `PLAT_INCLUDES` in the `platform.mk` file. In the ARM development
platforms, this file is found in `plat/arm/board/<plat_name>/include/`.

Platform ports may optionally use the file [include/plat/common/common_def.h],
which provides typical values for some of the constants below. These values are
likely to be suitable for all platform ports.

Platform ports that want to be aligned with standard ARM platforms (for example
FVP and Juno) may also use [include/plat/arm/common/arm_def.h], which provides
standard values for some of the constants below. However, this requires the
platform port to define additional platform porting constants in
`platform_def.h`. These additional constants are not documented here.

*   **#define : PLATFORM_LINKER_FORMAT**

    Defines the linker format used by the platform, for example
    `elf64-littleaarch64`.

*   **#define : PLATFORM_LINKER_ARCH**

    Defines the processor architecture for the linker by the platform, for
    example `aarch64`.

*   **#define : PLATFORM_STACK_SIZE**

    Defines the normal stack memory available to each CPU. This constant is used
    by [plat/common/aarch64/platform_mp_stack.S] and
    [plat/common/aarch64/platform_up_stack.S].

*   **define  : CACHE_WRITEBACK_GRANULE**

    Defines the size in bits of the largest cache line across all the cache
    levels in the platform.

*   **#define : FIRMWARE_WELCOME_STR**

    Defines the character string printed by BL1 upon entry into the `bl1_main()`
    function.

*   **#define : PLATFORM_CORE_COUNT**

    Defines the total number of CPUs implemented by the platform across all
    clusters in the system.

*   **#define : PLAT_NUM_PWR_DOMAINS**

    Defines the total number of nodes in the power domain topology
    tree at all the power domain levels used by the platform.
    This macro is used by the PSCI implementation to allocate
    data structures to represent power domain topology.

*   **#define : PLAT_MAX_PWR_LVL**

    Defines the maximum power domain level that the power management operations
    should apply to. More often, but not always, the power domain level
    corresponds to affinity level. This macro allows the PSCI implementation
    to know the highest power domain level that it should consider for power
    management operations in the system that the platform implements. For
    example, the Base AEM FVP implements two  clusters with a configurable
    number of CPUs and it reports the maximum power domain level as 1.

*   **#define : PLAT_MAX_OFF_STATE**

    Defines the local power state corresponding to the deepest power down
    possible at every power domain level in the platform. The local power
    states for each level may be sparsely allocated between 0 and this value
    with 0 being reserved for the RUN state. The PSCI implementation uses this
    value to initialize the local power states of the power domain nodes and
    to specify the requested power state for a PSCI_CPU_OFF call.

*   **#define : PLAT_MAX_RET_STATE**

    Defines the local power state corresponding to the deepest retention state
    possible at every power domain level in the platform. This macro should be
    a value less than PLAT_MAX_OFF_STATE and greater than 0. It is used by the
    PSCI implementation to distuiguish between retention and power down local
    power states within PSCI_CPU_SUSPEND call.

*   **#define : BL1_RO_BASE**

    Defines the base address in secure ROM where BL1 originally lives. Must be
    aligned on a page-size boundary.

*   **#define : BL1_RO_LIMIT**

    Defines the maximum address in secure ROM that BL1's actual content (i.e.
    excluding any data section allocated at runtime) can occupy.

*   **#define : BL1_RW_BASE**

    Defines the base address in secure RAM where BL1's read-write data will live
    at runtime. Must be aligned on a page-size boundary.

*   **#define : BL1_RW_LIMIT**

    Defines the maximum address in secure RAM that BL1's read-write data can
    occupy at runtime.

*   **#define : BL2_BASE**

    Defines the base address in secure RAM where BL1 loads the BL2 binary image.
    Must be aligned on a page-size boundary.

*   **#define : BL2_LIMIT**

    Defines the maximum address in secure RAM that the BL2 image can occupy.

*   **#define : BL31_BASE**

    Defines the base address in secure RAM where BL2 loads the BL31 binary
    image. Must be aligned on a page-size boundary.

*   **#define : BL31_LIMIT**

    Defines the maximum address in secure RAM that the BL31 image can occupy.

*   **#define : NS_IMAGE_OFFSET**

    Defines the base address in non-secure DRAM where BL2 loads the BL33 binary
    image. Must be aligned on a page-size boundary.

For every image, the platform must define individual identifiers that will be
used by BL1 or BL2 to load the corresponding image into memory from non-volatile
storage. For the sake of performance, integer numbers will be used as
identifiers. The platform will use those identifiers to return the relevant
information about the image to be loaded (file handler, load address,
authentication information, etc.). The following image identifiers are
mandatory:

*   **#define : BL2_IMAGE_ID**

    BL2 image identifier, used by BL1 to load BL2.

*   **#define : BL31_IMAGE_ID**

    BL31 image identifier, used by BL2 to load BL31.

*   **#define : BL33_IMAGE_ID**

    BL33 image identifier, used by BL2 to load BL33.

If Trusted Board Boot is enabled, the following certificate identifiers must
also be defined:

*   **#define : TRUSTED_BOOT_FW_CERT_ID**

    BL2 content certificate identifier, used by BL1 to load the BL2 content
    certificate.

*   **#define : TRUSTED_KEY_CERT_ID**

    Trusted key certificate identifier, used by BL2 to load the trusted key
    certificate.

*   **#define : SOC_FW_KEY_CERT_ID**

    BL31 key certificate identifier, used by BL2 to load the BL31 key
    certificate.

*   **#define : SOC_FW_CONTENT_CERT_ID**

    BL31 content certificate identifier, used by BL2 to load the BL31 content
    certificate.

*   **#define : NON_TRUSTED_FW_KEY_CERT_ID**

    BL33 key certificate identifier, used by BL2 to load the BL33 key
    certificate.

*   **#define : NON_TRUSTED_FW_CONTENT_CERT_ID**

    BL33 content certificate identifier, used by BL2 to load the BL33 content
    certificate.

*   **#define : FWU_CERT_ID**

    Firmware Update (FWU) certificate identifier, used by NS_BL1U to load the
    FWU content certificate.


If the AP Firmware Updater Configuration image, BL2U is used, the following
must also be defined:

*   **#define : BL2U_BASE**

    Defines the base address in secure memory where BL1 copies the BL2U binary
    image. Must be aligned on a page-size boundary.

*   **#define : BL2U_LIMIT**

    Defines the maximum address in secure memory that the BL2U image can occupy.

*   **#define : BL2U_IMAGE_ID**

    BL2U image identifier, used by BL1 to fetch an image descriptor
    corresponding to BL2U.

If the SCP Firmware Update Configuration Image, SCP_BL2U is used, the following
must also be defined:

*   **#define : SCP_BL2U_IMAGE_ID**

    SCP_BL2U image identifier, used by BL1 to fetch an image descriptor
    corresponding to SCP_BL2U.
    NOTE: TF does not provide source code for this image.

If the Non-Secure Firmware Updater ROM, NS_BL1U is used, the following must
also be defined:

*   **#define : NS_BL1U_BASE**

    Defines the base address in non-secure ROM where NS_BL1U executes.
    Must be aligned on a page-size boundary.
    NOTE: TF does not provide source code for this image.

*   **#define : NS_BL1U_IMAGE_ID**

    NS_BL1U image identifier, used by BL1 to fetch an image descriptor
    corresponding to NS_BL1U.

If the Non-Secure Firmware Updater, NS_BL2U is used, the following must also
be defined:

*   **#define : NS_BL2U_BASE**

    Defines the base address in non-secure memory where NS_BL2U executes.
    Must be aligned on a page-size boundary.
    NOTE: TF does not provide source code for this image.

*   **#define : NS_BL2U_IMAGE_ID**

    NS_BL2U image identifier, used by BL1 to fetch an image descriptor
    corresponding to NS_BL2U.


If a SCP_BL2 image is supported by the platform, the following constants must
also be defined:

*   **#define : SCP_BL2_IMAGE_ID**

    SCP_BL2 image identifier, used by BL2 to load SCP_BL2 into secure memory
    from platform storage before being transfered to the SCP.

*   **#define : SCP_FW_KEY_CERT_ID**

    SCP_BL2 key certificate identifier, used by BL2 to load the SCP_BL2 key
    certificate (mandatory when Trusted Board Boot is enabled).

*   **#define : SCP_FW_CONTENT_CERT_ID**

    SCP_BL2 content certificate identifier, used by BL2 to load the SCP_BL2
    content certificate (mandatory when Trusted Board Boot is enabled).

If a BL32 image is supported by the platform, the following constants must
also be defined:

*   **#define : BL32_IMAGE_ID**

    BL32 image identifier, used by BL2 to load BL32.

*   **#define : TRUSTED_OS_FW_KEY_CERT_ID**

    BL32 key certificate identifier, used by BL2 to load the BL32 key
    certificate (mandatory when Trusted Board Boot is enabled).

*   **#define : TRUSTED_OS_FW_CONTENT_CERT_ID**

    BL32 content certificate identifier, used by BL2 to load the BL32 content
    certificate (mandatory when Trusted Board Boot is enabled).

*   **#define : BL32_BASE**

    Defines the base address in secure memory where BL2 loads the BL32 binary
    image. Must be aligned on a page-size boundary.

*   **#define : BL32_LIMIT**

    Defines the maximum address that the BL32 image can occupy.

If the Test Secure-EL1 Payload (TSP) instantiation of BL32 is supported by the
platform, the following constants must also be defined:

*   **#define : TSP_SEC_MEM_BASE**

    Defines the base address of the secure memory used by the TSP image on the
    platform. This must be at the same address or below `BL32_BASE`.

*   **#define : TSP_SEC_MEM_SIZE**

    Defines the size of the secure memory used by the BL32 image on the
    platform. `TSP_SEC_MEM_BASE` and `TSP_SEC_MEM_SIZE` must fully accomodate
    the memory required by the BL32 image, defined by `BL32_BASE` and
    `BL32_LIMIT`.

*   **#define : TSP_IRQ_SEC_PHY_TIMER**

    Defines the ID of the secure physical generic timer interrupt used by the
    TSP's interrupt handling code.

If the platform port uses the translation table library code, the following
constant must also be defined:

*   **#define : MAX_XLAT_TABLES**

    Defines the maximum number of translation tables that are allocated by the
    translation table library code. To minimize the amount of runtime memory
    used, choose the smallest value needed to map the required virtual addresses
    for each BL stage.

If the platform port uses the IO storage framework, the following constants
must also be defined:

*   **#define : MAX_IO_DEVICES**

    Defines the maximum number of registered IO devices. Attempting to register
    more devices than this value using `io_register_device()` will fail with
    -ENOMEM.

*   **#define : MAX_IO_HANDLES**

    Defines the maximum number of open IO handles. Attempting to open more IO
    entities than this value using `io_open()` will fail with -ENOMEM.

If the platform needs to allocate data within the per-cpu data framework in
BL31, it should define the following macro. Currently this is only required if
the platform decides not to use the coherent memory section by undefining the
`USE_COHERENT_MEM` build flag. In this case, the framework allocates the
required memory within the the per-cpu data to minimize wastage.

*   **#define : PLAT_PCPU_DATA_SIZE**

    Defines the memory (in bytes) to be reserved within the per-cpu data
    structure for use by the platform layer.

The following constants are optional. They should be defined when the platform
memory layout implies some image overlaying like in ARM standard platforms.

*   **#define : BL31_PROGBITS_LIMIT**

    Defines the maximum address in secure RAM that the BL31's progbits sections
    can occupy.

*   **#define : TSP_PROGBITS_LIMIT**

    Defines the maximum address that the TSP's progbits sections can occupy.

### File : plat_macros.S [mandatory]

Each platform must ensure a file of this name is in the system include path with
the following macro defined. In the ARM development platforms, this file is
found in `plat/arm/board/<plat_name>/include/plat_macros.S`.

*   **Macro : plat_print_gic_regs**

    This macro allows the crash reporting routine to print GIC registers
    in case of an unhandled exception in BL31. This aids in debugging and
    this macro can be defined to be empty in case GIC register reporting is
    not desired.

*   **Macro : plat_print_interconnect_regs**

    This macro allows the crash reporting routine to print interconnect
    registers in case of an unhandled exception in BL31. This aids in debugging
    and this macro can be defined to be empty in case interconnect register
    reporting is not desired. In ARM standard platforms, the CCI snoop
    control registers are reported.


2.2 Handling Reset
------------------

BL1 by default implements the reset vector where execution starts from a cold
or warm boot. BL31 can be optionally set as a reset vector using the
`RESET_TO_BL31` make variable.

For each CPU, the reset vector code is responsible for the following tasks:

1.  Distinguishing between a cold boot and a warm boot.

2.  In the case of a cold boot and the CPU being a secondary CPU, ensuring that
    the CPU is placed in a platform-specific state until the primary CPU
    performs the necessary steps to remove it from this state.

3.  In the case of a warm boot, ensuring that the CPU jumps to a platform-
    specific address in the BL31 image in the same processor mode as it was
    when released from reset.

The following functions need to be implemented by the platform port to enable
reset vector code to perform the above tasks.


### Function : plat_get_my_entrypoint() [mandatory when PROGRAMMABLE_RESET_ADDRESS == 0]

    Argument : void
    Return   : unsigned long

This function is called with the called with the MMU and caches disabled
(`SCTLR_EL3.M` = 0 and `SCTLR_EL3.C` = 0). The function is responsible for
distinguishing between a warm and cold reset for the current CPU using
platform-specific means. If it's a warm reset, then it returns the warm
reset entrypoint point provided to `plat_setup_psci_ops()` during
BL31 initialization. If it's a cold reset then this function must return zero.

This function does not follow the Procedure Call Standard used by the
Application Binary Interface for the ARM 64-bit architecture. The caller should
not assume that callee saved registers are preserved across a call to this
function.

This function fulfills requirement 1 and 3 listed above.

Note that for platforms that support programming the reset address, it is
expected that a CPU will start executing code directly at the right address,
both on a cold and warm reset. In this case, there is no need to identify the
type of reset nor to query the warm reset entrypoint. Therefore, implementing
this function is not required on such platforms.


### Function : plat_secondary_cold_boot_setup() [mandatory when COLD_BOOT_SINGLE_CPU == 0]

    Argument : void

This function is called with the MMU and data caches disabled. It is responsible
for placing the executing secondary CPU in a platform-specific state until the
primary CPU performs the necessary actions to bring it out of that state and
allow entry into the OS. This function must not return.

In the ARM FVP port, when using the normal boot flow, each secondary CPU powers
itself off. The primary CPU is responsible for powering up the secondary CPUs
when normal world software requires them. When booting an EL3 payload instead,
they stay powered on and are put in a holding pen until their mailbox gets
populated.

This function fulfills requirement 2 above.

Note that for platforms that can't release secondary CPUs out of reset, only the
primary CPU will execute the cold boot code. Therefore, implementing this
function is not required on such platforms.


### Function : plat_is_my_cpu_primary() [mandatory when COLD_BOOT_SINGLE_CPU == 0]

    Argument : void
    Return   : unsigned int

This function identifies whether the current CPU is the primary CPU or a
secondary CPU. A return value of zero indicates that the CPU is not the
primary CPU, while a non-zero return value indicates that the CPU is the
primary CPU.

Note that for platforms that can't release secondary CPUs out of reset, only the
primary CPU will execute the cold boot code. Therefore, there is no need to
distinguish between primary and secondary CPUs and implementing this function is
not required.


### Function : platform_mem_init() [mandatory]

    Argument : void
    Return   : void

This function is called before any access to data is made by the firmware, in
order to carry out any essential memory initialization.


### Function: plat_get_rotpk_info()

    Argument : void *, void **, unsigned int *, unsigned int *
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It returns a
pointer to the ROTPK stored in the platform (or a hash of it) and its length.
The ROTPK must be encoded in DER format according to the following ASN.1
structure:

    AlgorithmIdentifier  ::=  SEQUENCE  {
        algorithm         OBJECT IDENTIFIER,
        parameters        ANY DEFINED BY algorithm OPTIONAL
    }

    SubjectPublicKeyInfo  ::=  SEQUENCE  {
        algorithm         AlgorithmIdentifier,
        subjectPublicKey  BIT STRING
    }

In case the function returns a hash of the key:

    DigestInfo ::= SEQUENCE {
        digestAlgorithm   AlgorithmIdentifier,
        digest            OCTET STRING
    }

The function returns 0 on success. Any other value means the ROTPK could not be
retrieved from the platform. The function also reports extra information related
to the ROTPK in the flags parameter.


2.3 Common mandatory modifications
---------------------------------

The following functions are mandatory functions which need to be implemented
by the platform port.

### Function : plat_my_core_pos()

    Argument : void
    Return   : unsigned int

This funtion returns the index of the calling CPU which is used as a
CPU-specific linear index into blocks of memory (for example while allocating
per-CPU stacks). This function will be invoked very early in the
initialization sequence which mandates that this function should be
implemented in assembly and should not rely on the avalability of a C
runtime environment.

This function plays a crucial role in the power domain topology framework in
PSCI and details of this can be found in [Power Domain Topology Design].

### Function : plat_core_pos_by_mpidr()

    Argument : u_register_t
    Return   : int

This function validates the `MPIDR` of a CPU and converts it to an index,
which can be used as a CPU-specific linear index into blocks of memory. In
case the `MPIDR` is invalid, this function returns -1. This function will only
be invoked by BL31 after the power domain topology is initialized and can
utilize the C runtime environment. For further details about how ARM Trusted
Firmware represents the power domain topology and how this relates to the
linear CPU index, please refer [Power Domain Topology Design].



2.4 Common optional modifications
---------------------------------

The following are helper functions implemented by the firmware that perform
common platform-specific tasks. A platform may choose to override these
definitions.

### Function : plat_set_my_stack()

    Argument : void
    Return   : void

This function sets the current stack pointer to the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant `PLATFORM_STACK_SIZE`.

Common implementations of this function for the UP and MP BL images are
provided in [plat/common/aarch64/platform_up_stack.S] and
[plat/common/aarch64/platform_mp_stack.S]


### Function : plat_get_my_stack()

    Argument : void
    Return   : unsigned long

This function returns the base address of the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant `PLATFORM_STACK_SIZE`.

Common implementations of this function for the UP and MP BL images are
provided in [plat/common/aarch64/platform_up_stack.S] and
[plat/common/aarch64/platform_mp_stack.S]


### Function : plat_report_exception()

    Argument : unsigned int
    Return   : void

A platform may need to report various information about its status when an
exception is taken, for example the current exception level, the CPU security
state (secure/non-secure), the exception type, and so on. This function is
called in the following circumstances:

*   In BL1, whenever an exception is taken.
*   In BL2, whenever an exception is taken.

The default implementation doesn't do anything, to avoid making assumptions
about the way the platform displays its status information.

This function receives the exception type as its argument. Possible values for
exceptions types are listed in the [include/common/bl_common.h] header file.
Note that these constants are not related to any architectural exception code;
they are just an ARM Trusted Firmware convention.


### Function : plat_reset_handler()

    Argument : void
    Return   : void

A platform may need to do additional initialization after reset. This function
allows the platform to do the platform specific intializations. Platform
specific errata workarounds could also be implemented here. The api should
preserve the values of callee saved registers x19 to x29.

The default implementation doesn't do anything. If a platform needs to override
the default implementation, refer to the [Firmware Design] for general
guidelines.

### Function : plat_disable_acp()

    Argument : void
    Return   : void

This api allows a platform to disable the Accelerator Coherency Port (if
present) during a cluster power down sequence. The default weak implementation
doesn't do anything. Since this api is called during the power down sequence,
it has restrictions for stack usage and it can use the registers x0 - x17 as
scratch registers. It should preserve the value in x18 register as it is used
by the caller to store the return address.

### Function : plat_error_handler()

    Argument : int
    Return   : void

This API is called when the generic code encounters an error situation from
which it cannot continue. It allows the platform to perform error reporting or
recovery actions (for example, reset the system). This function must not return.

The parameter indicates the type of error using standard codes from `errno.h`.
Possible errors reported by the generic code are:

*   `-EAUTH`: a certificate or image could not be authenticated (when Trusted
    Board Boot is enabled)
*   `-ENOENT`: the requested image or certificate could not be found or an IO
    error was detected
*   `-ENOMEM`: resources exhausted. Trusted Firmware does not use dynamic
    memory, so this error is usually an indication of an incorrect array size

The default implementation simply spins.


3.  Modifications specific to a Boot Loader stage
-------------------------------------------------

3.1 Boot Loader Stage 1 (BL1)
-----------------------------

BL1 implements the reset vector where execution starts from after a cold or
warm boot. For each CPU, BL1 is responsible for the following tasks:

1.  Handling the reset as described in section 2.2

2.  In the case of a cold boot and the CPU being the primary CPU, ensuring that
    only this CPU executes the remaining BL1 code, including loading and passing
    control to the BL2 stage.

3.  Identifying and starting the Firmware Update process (if required).

4.  Loading the BL2 image from non-volatile storage into secure memory at the
    address specified by the platform defined constant `BL2_BASE`.

5.  Populating a `meminfo` structure with the following information in memory,
    accessible by BL2 immediately upon entry.

        meminfo.total_base = Base address of secure RAM visible to BL2
        meminfo.total_size = Size of secure RAM visible to BL2
        meminfo.free_base  = Base address of secure RAM available for
                             allocation to BL2
        meminfo.free_size  = Size of secure RAM available for allocation to BL2

    BL1 places this `meminfo` structure at the beginning of the free memory
    available for its use. Since BL1 cannot allocate memory dynamically at the
    moment, its free memory will be available for BL2's use as-is. However, this
    means that BL2 must read the `meminfo` structure before it starts using its
    free memory (this is discussed in Section 3.2).

    In future releases of the ARM Trusted Firmware it will be possible for
    the platform to decide where it wants to place the `meminfo` structure for
    BL2.

    BL1 implements the `bl1_init_bl2_mem_layout()` function to populate the
    BL2 `meminfo` structure. The platform may override this implementation, for
    example if the platform wants to restrict the amount of memory visible to
    BL2. Details of how to do this are given below.

The following functions need to be implemented by the platform port to enable
BL1 to perform the above tasks.


### Function : bl1_early_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

In ARM standard platforms, this function initializes the console and enables
snoop requests into the primary CPU's cluster.

### Function : bl1_plat_arch_setup() [mandatory]

    Argument : void
    Return   : void

This function performs any platform-specific and architectural setup that the
platform requires. Platform-specific setup might include configuration of
memory controllers and the interconnect.

In ARM standard platforms, this function enables the MMU.

This function helps fulfill requirement 2 above.


### Function : bl1_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches enabled. It is responsible
for performing any remaining platform-specific setup that can occur after the
MMU and data cache have been enabled.

In ARM standard platforms, this function initializes the storage abstraction
layer used to load the next bootloader image.

This function helps fulfill requirement 4 above.


### Function : bl1_plat_sec_mem_layout() [mandatory]

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It executes with the
MMU and data caches enabled. The pointer returned by this function must point to
a `meminfo` structure containing the extents and availability of secure RAM for
the BL1 stage.

    meminfo.total_base = Base address of secure RAM visible to BL1
    meminfo.total_size = Size of secure RAM visible to BL1
    meminfo.free_base  = Base address of secure RAM available for allocation
                         to BL1
    meminfo.free_size  = Size of secure RAM available for allocation to BL1

This information is used by BL1 to load the BL2 image in secure RAM. BL1 also
populates a similar structure to tell BL2 the extents of memory available for
its own use.

This function helps fulfill requirements 4 and 5 above.


### Function : bl1_init_bl2_mem_layout() [optional]

    Argument : meminfo *, meminfo *, unsigned int, unsigned long
    Return   : void

BL1 needs to tell the next stage the amount of secure RAM available
for it to use. This information is populated in a `meminfo`
structure.

Depending upon where BL2 has been loaded in secure RAM (determined by
`BL2_BASE`), BL1 calculates the amount of free memory available for BL2 to use.
BL1 also ensures that its data sections resident in secure RAM are not visible
to BL2. An illustration of how this is done in ARM standard platforms is given
in the **Memory layout on ARM development platforms** section in the
[Firmware Design].


### Function : bl1_plat_prepare_exit() [optional]

    Argument : entry_point_info_t *
    Return   : void

This function is called prior to exiting BL1 in response to the
`BL1_SMC_RUN_IMAGE` SMC request raised by BL2. It should be used to perform
platform specific clean up or bookkeeping operations before transferring
control to the next image. It receives the address of the `entry_point_info_t`
structure passed from BL2. This function runs with MMU disabled.

### Function : bl1_plat_set_ep_info() [optional]

    Argument : unsigned int image_id, entry_point_info_t *ep_info
    Return   : void

This function allows platforms to override `ep_info` for the given `image_id`.

The default implementation just returns.

### Function : bl1_plat_get_next_image_id() [optional]

    Argument : void
    Return   : unsigned int

This and the following function must be overridden to enable the FWU feature.

BL1 calls this function after platform setup to identify the next image to be
loaded and executed. If the platform returns `BL2_IMAGE_ID` then BL1 proceeds
with the normal boot sequence, which loads and executes BL2. If the platform
returns a different image id, BL1 assumes that Firmware Update is required.

The default implementation always returns `BL2_IMAGE_ID`. The ARM development
platforms override this function to detect if firmware update is required, and
if so, return the first image in the firmware update process.

### Function : bl1_plat_get_image_desc() [optional]

    Argument : unsigned int image_id
    Return   : image_desc_t *

BL1 calls this function to get the image descriptor information `image_desc_t`
for the provided `image_id` from the platform.

The default implementation always returns a common BL2 image descriptor. ARM
standard platforms return an image descriptor corresponding to BL2 or one of
the firmware update images defined in the Trusted Board Boot Requirements
specification.

### Function : bl1_plat_fwu_done() [optional]

    Argument : unsigned int image_id, uintptr_t image_src,
               unsigned int image_size
    Return   : void

BL1 calls this function when the FWU process is complete. It must not return.
The platform may override this function to take platform specific action, for
example to initiate the normal boot flow.

The default implementation spins forever.

### Function : bl1_plat_mem_check() [mandatory]

    Argument : uintptr_t mem_base, unsigned int mem_size,
               unsigned int flags
    Return   : void

BL1 calls this function while handling FWU copy and authenticate SMCs. The
platform must ensure that the provided `mem_base` and `mem_size` are mapped into
BL1, and that this memory corresponds to either a secure or non-secure memory
region as indicated by the security state of the `flags` argument.

The default implementation of this function asserts therefore platforms must
override it when using the FWU feature.


3.2 Boot Loader Stage 2 (BL2)
-----------------------------

The BL2 stage is executed only by the primary CPU, which is determined in BL1
using the `platform_is_primary_cpu()` function. BL1 passed control to BL2 at
`BL2_BASE`. BL2 executes in Secure EL1 and is responsible for:

1.  (Optional) Loading the SCP_BL2 binary image (if present) from platform
    provided non-volatile storage. To load the SCP_BL2 image, BL2 makes use of
    the `meminfo` returned by the `bl2_plat_get_scp_bl2_meminfo()` function.
    The platform also defines the address in memory where SCP_BL2 is loaded
    through the optional constant `SCP_BL2_BASE`. BL2 uses this information
    to determine if there is enough memory to load the SCP_BL2 image.
    Subsequent handling of the SCP_BL2 image is platform-specific and is
    implemented in the `bl2_plat_handle_scp_bl2()` function.
    If `SCP_BL2_BASE` is not defined then this step is not performed.

2.  Loading the BL31 binary image into secure RAM from non-volatile storage. To
    load the BL31 image, BL2 makes use of the `meminfo` structure passed to it
    by BL1. This structure allows BL2 to calculate how much secure RAM is
    available for its use. The platform also defines the address in secure RAM
    where BL31 is loaded through the constant `BL31_BASE`. BL2 uses this
    information to determine if there is enough memory to load the BL31 image.

3.  (Optional) Loading the BL32 binary image (if present) from platform
    provided non-volatile storage. To load the BL32 image, BL2 makes use of
    the `meminfo` returned by the `bl2_plat_get_bl32_meminfo()` function.
    The platform also defines the address in memory where BL32 is loaded
    through the optional constant `BL32_BASE`. BL2 uses this information
    to determine if there is enough memory to load the BL32 image.
    If `BL32_BASE` is not defined then this and the next step is not performed.

4.  (Optional) Arranging to pass control to the BL32 image (if present) that
    has been pre-loaded at `BL32_BASE`. BL2 populates an `entry_point_info`
    structure in memory provided by the platform with information about how
    BL31 should pass control to the BL32 image.

5.  Loading the normal world BL33 binary image into non-secure DRAM from
    platform storage and arranging for BL31 to pass control to this image. This
    address is determined using the `plat_get_ns_image_entrypoint()` function
    described below.

6.  BL2 populates an `entry_point_info` structure in memory provided by the
    platform with information about how BL31 should pass control to the
    other BL images.

The following functions must be implemented by the platform port to enable BL2
to perform the above tasks.


### Function : bl2_early_platform_setup() [mandatory]

    Argument : meminfo *
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function is the address of the
`meminfo` structure populated by BL1.

The platform must copy the contents of the `meminfo` structure into a private
variable as the original memory may be subsequently overwritten by BL2. The
copied structure is made available to all BL2 code through the
`bl2_plat_sec_mem_layout()` function.

In ARM standard platforms, this function also initializes the storage
abstraction layer used to load further bootloader images. It is necessary to do
this early on platforms with a SCP_BL2 image, since the later
`bl2_platform_setup` must be done after SCP_BL2 is loaded.


### Function : bl2_plat_arch_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms, for example enabling the MMU (since the memory
map differs across platforms).


### Function : bl2_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in `bl2_plat_arch_setup()`. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2.

In ARM standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.


### Function : bl2_plat_sec_mem_layout() [mandatory]

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It may execute with
the MMU and data caches enabled if the platform port does the necessary
initialization in `bl2_plat_arch_setup()`. It is only called by the primary CPU.

The purpose of this function is to return a pointer to a `meminfo` structure
populated with the extents of secure RAM available for BL2 to use. See
`bl2_early_platform_setup()` above.


### Function : bl2_plat_get_scp_bl2_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
SCP_BL2 image. The meminfo provided by this is used by load_image() to
validate whether the SCP_BL2 image can be loaded within the given
memory from the given base.


### Function : bl2_plat_handle_scp_bl2() [mandatory]

    Argument : image_info *
    Return   : int

This function is called after loading SCP_BL2 image and it is used to perform
any platform-specific actions required to handle the SCP firmware. Typically it
transfers the image into SCP memory using a platform-specific protocol and waits
until SCP executes it and signals to the Application Processor (AP) for BL2
execution to continue.

This function returns 0 on success, a negative error code otherwise.


### Function : bl2_plat_get_bl31_params() [mandatory]

    Argument : void
    Return   : bl31_params *

BL2 platform code needs to return a pointer to a `bl31_params` structure it
will use for passing information to BL31. The `bl31_params` structure carries
the following information.
    - Header describing the version information for interpreting the bl31_param
      structure
    - Information about executing the BL33 image in the `bl33_ep_info` field
    - Information about executing the BL32 image in the `bl32_ep_info` field
    - Information about the type and extents of BL31 image in the
      `bl31_image_info` field
    - Information about the type and extents of BL32 image in the
      `bl32_image_info` field
    - Information about the type and extents of BL33 image in the
      `bl33_image_info` field

The memory pointed by this structure and its sub-structures should be
accessible from BL31 initialisation code. BL31 might choose to copy the
necessary content, or maintain the structures until BL33 is initialised.


### Funtion : bl2_plat_get_bl31_ep_info() [mandatory]

    Argument : void
    Return   : entry_point_info *

BL2 platform code returns a pointer which is used to populate the entry point
information for BL31 entry point. The location pointed by it should be
accessible from BL1 while processing the synchronous exception to run to BL31.

In ARM standard platforms this is allocated inside a bl2_to_bl31_params_mem
structure in BL2 memory.


### Function : bl2_plat_set_bl31_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

In the normal boot flow, this function is called after loading BL31 image and
it can be used to overwrite the entry point set by loader and also set the
security state and SPSR which represents the entry point system state for BL31.

When booting an EL3 payload instead, this function is called after populating
its entry point address and can be used for the same purpose for the payload
image. It receives a null pointer as its first argument in this case.

### Function : bl2_plat_set_bl32_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL32 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL32.


### Function : bl2_plat_set_bl33_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL33 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL33.


### Function : bl2_plat_get_bl32_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL32 image. The meminfo provided by this is used by load_image() to
validate whether the BL32 image can be loaded with in the given
memory from the given base.

### Function : bl2_plat_get_bl33_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL33 image. The meminfo provided by this is used by load_image() to
validate whether the BL33 image can be loaded with in the given
memory from the given base.

### Function : bl2_plat_flush_bl31_params() [mandatory]

    Argument : void
    Return   : void

Once BL2 has populated all the structures that needs to be read by BL1
and BL31 including the bl31_params structures and its sub-structures,
the bl31_ep_info structure and any platform specific data. It flushes
all these data to the main memory so that it is available when we jump to
later Bootloader stages with MMU off

### Function : plat_get_ns_image_entrypoint() [mandatory]

    Argument : void
    Return   : unsigned long

As previously described, BL2 is responsible for arranging for control to be
passed to a normal world BL image through BL31. This function returns the
entrypoint of that image, which BL31 uses to jump to it.

BL2 is responsible for loading the normal world BL33 image (e.g. UEFI).


3.3 FWU Boot Loader Stage 2 (BL2U)
----------------------------------

The AP Firmware Updater Configuration, BL2U, is an optional part of the FWU
process and is executed only by the primary CPU. BL1 passes control to BL2U at
`BL2U_BASE`. BL2U executes in Secure-EL1 and is responsible for:

1.  (Optional) Transfering the optional SCP_BL2U binary image from AP secure
    memory to SCP RAM. BL2U uses the SCP_BL2U `image_info` passed by BL1.
    `SCP_BL2U_BASE` defines the address in AP secure memory where SCP_BL2U
    should be copied from. Subsequent handling of the SCP_BL2U image is
    implemented by the platform specific `bl2u_plat_handle_scp_bl2u()` function.
    If `SCP_BL2U_BASE` is not defined then this step is not performed.

2.  Any platform specific setup required to perform the FWU process. For
    example, ARM standard platforms initialize the TZC controller so that the
    normal world can access DDR memory.

The following functions must be implemented by the platform port to enable
BL2U to perform the tasks mentioned above.

### Function : bl2u_early_platform_setup() [mandatory]

    Argument : meminfo *mem_info, void *plat_info
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU. The arguments to this function is the address
of the `meminfo` structure and platform specific info provided by BL1.

The platform must copy the contents of the `mem_info` and `plat_info` into
private storage as the original memory may be subsequently overwritten by BL2U.

On ARM CSS platforms `plat_info` is interpreted as an `image_info_t` structure,
to extract SCP_BL2U image information, which is then copied into a private
variable.

### Function : bl2u_plat_arch_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms, for example enabling the MMU (since the memory
map differs across platforms).

### Function : bl2u_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in `bl2u_plat_arch_setup()`. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2U.

In ARM standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.

### Function : bl2u_plat_handle_scp_bl2u() [optional]

    Argument : void
    Return   : int

This function is used to perform any platform-specific actions required to
handle the SCP firmware. Typically it transfers the image into SCP memory using
a platform-specific protocol and waits until SCP executes it and signals to the
Application Processor (AP) for BL2U execution to continue.

This function returns 0 on success, a negative error code otherwise.
This function is included if SCP_BL2U_BASE is defined.


3.4 Boot Loader Stage 3-1 (BL31)
---------------------------------

During cold boot, the BL31 stage is executed only by the primary CPU. This is
determined in BL1 using the `platform_is_primary_cpu()` function. BL1 passes
control to BL31 at `BL31_BASE`. During warm boot, BL31 is executed by all
CPUs. BL31 executes at EL3 and is responsible for:

1.  Re-initializing all architectural and platform state. Although BL1 performs
    some of this initialization, BL31 remains resident in EL3 and must ensure
    that EL3 architectural and platform state is completely initialized. It
    should make no assumptions about the system state when it receives control.

2.  Passing control to a normal world BL image, pre-loaded at a platform-
    specific address by BL2. BL31 uses the `entry_point_info` structure that BL2
    populated in memory to do this.

3.  Providing runtime firmware services. Currently, BL31 only implements a
    subset of the Power State Coordination Interface (PSCI) API as a runtime
    service. See Section 3.3 below for details of porting the PSCI
    implementation.

4.  Optionally passing control to the BL32 image, pre-loaded at a platform-
    specific address by BL2. BL31 exports a set of apis that allow runtime
    services to specify the security state in which the next image should be
    executed and run the corresponding image. BL31 uses the `entry_point_info`
    structure populated by BL2 to do this.

If BL31 is a reset vector, It also needs to handle the reset as specified in
section 2.2 before the tasks described above.

The following functions must be implemented by the platform port to enable BL31
to perform the above tasks.


### Function : bl31_early_platform_setup() [mandatory]

    Argument : bl31_params *, void *
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function are:

*   The address of the `bl31_params` structure populated by BL2.
*   An opaque pointer that the platform may use as needed.

The platform can copy the contents of the `bl31_params` structure and its
sub-structures into private variables if the original memory may be
subsequently overwritten by BL31 and similarly the `void *` pointing
to the platform data also needs to be saved.

In ARM standard platforms, BL2 passes a pointer to a `bl31_params` structure
in BL2 memory. BL31 copies the information in this pointer to internal data
structures.


### Function : bl31_plat_arch_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms, for example enabling the MMU (since the memory
map differs across platforms).


### Function : bl31_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

The purpose of this function is to complete platform initialization so that both
BL31 runtime services and normal world software can function correctly.

In ARM standard platforms, this function does the following:
*   Initializes the generic interrupt controller.
*   Enables system-level implementation of the generic timer counter.
*   Grants access to the system counter timer module
*   Initializes the power controller device
*   Detects the system topology.


### Function : bl31_plat_runtime_setup() [optional]

    Argument : void
    Return   : void

The purpose of this function is allow the platform to perform any BL31 runtime
setup just prior to BL31 exit during cold boot. The default weak
implementation of this function will invoke `console_uninit()` which will
suppress any BL31 runtime logs.

In ARM Standard platforms, this function will initialize the BL31 runtime
console which will cause all further BL31 logs to be output to the
runtime console.


### Function : bl31_get_next_image_info() [mandatory]

    Argument : unsigned int
    Return   : entry_point_info *

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`.

This function is called by `bl31_main()` to retrieve information provided by
BL2 for the next image in the security state specified by the argument. BL31
uses this information to pass control to that image in the specified security
state. This function must return a pointer to the `entry_point_info` structure
(that was copied during `bl31_early_platform_setup()`) if the image exists. It
should return NULL otherwise.

### Function : plat_get_syscnt_freq() [mandatory]

    Argument : void
    Return   : uint64_t

This function is used by the architecture setup code to retrieve the counter
frequency for the CPU's generic timer.  This value will be programmed into the
`CNTFRQ_EL0` register. In ARM standard platforms, it returns the base frequency
of the system counter, which is retrieved from the first entry in the frequency
modes table.


### #define : PLAT_PERCPU_BAKERY_LOCK_SIZE [optional]

   When `USE_COHERENT_MEM = 0`, this constant defines the total memory (in
   bytes) aligned to the cache line boundary that should be allocated per-cpu to
   accommodate all the bakery locks.

   If this constant is not defined when `USE_COHERENT_MEM = 0`, the linker
   calculates the size of the `bakery_lock` input section, aligns it to the
   nearest `CACHE_WRITEBACK_GRANULE`, multiplies it with `PLATFORM_CORE_COUNT`
   and stores the result in a linker symbol. This constant prevents a platform
   from relying on the linker and provide a more efficient mechanism for
   accessing per-cpu bakery lock information.

   If this constant is defined and its value is not equal to the value
   calculated by the linker then a link time assertion is raised. A compile time
   assertion is raised if the value of the constant is not aligned to the cache
   line boundary.

3.5 Power State Coordination Interface (in BL31)
------------------------------------------------

The ARM Trusted Firmware's implementation of the PSCI API is based around the
concept of a _power domain_. A _power domain_ is a CPU or a logical group of
CPUs which share some state on which power management operations can be
performed as specified by [PSCI]. Each CPU in the system is assigned a cpu
index which is a unique number between `0` and `PLATFORM_CORE_COUNT - 1`.
The _power domains_ are arranged in a hierarchical tree structure and
each _power domain_ can be identified in a system by the cpu index of any CPU
that is part of that domain and a _power domain level_. A processing element
(for example, a CPU) is at level 0. If the _power domain_ node above a CPU is
a logical grouping of CPUs that share some state, then level 1 is that group
of CPUs (for example, a cluster), and level 2 is a group of clusters
(for example, the system). More details on the power domain topology and its
organization can be found in [Power Domain Topology Design].

BL31's platform initialization code exports a pointer to the platform-specific
power management operations required for the PSCI implementation to function
correctly. This information is populated in the `plat_psci_ops` structure. The
PSCI implementation calls members of the `plat_psci_ops` structure for performing
power management operations on the power domains. For example, the target
CPU is specified by its `MPIDR` in a PSCI `CPU_ON` call. The `pwr_domain_on()`
handler (if present) is called for the CPU power domain.

The `power-state` parameter of a PSCI `CPU_SUSPEND` call can be used to
describe composite power states specific to a platform. The PSCI implementation
defines a generic representation of the power-state parameter viz which is an
array of local power states where each index corresponds to a power domain
level. Each entry contains the local power state the power domain at that power
level could enter. It depends on the `validate_power_state()` handler to
convert the power-state parameter (possibly encoding a composite power state)
passed in a PSCI `CPU_SUSPEND` call to this representation.

The following functions must be implemented to initialize PSCI functionality in
the ARM Trusted Firmware.


### Function : plat_get_target_pwr_state() [optional]

    Argument : unsigned int, const plat_local_state_t *, unsigned int
    Return   : plat_local_state_t

The PSCI generic code uses this function to let the platform participate in
state coordination during a power management operation. The function is passed
a pointer to an array of platform specific local power state `states` (second
argument) which contains the requested power state for each CPU at a particular
power domain level `lvl` (first argument) within the power domain. The function
is expected to traverse this array of upto `ncpus` (third argument) and return
a coordinated target power state by the comparing all the requested power
states. The target power state should not be deeper than any of the requested
power states.

A weak definition of this API is provided by default wherein it assumes
that the platform assigns a local state value in order of increasing depth
of the power state i.e. for two power states X & Y, if X < Y
then X represents a shallower power state than Y. As a result, the
coordinated target local power state for a power domain will be the minimum
of the requested local power state values.


### Function : plat_get_power_domain_tree_desc() [mandatory]

    Argument : void
    Return   : const unsigned char *

This function returns a pointer to the byte array containing the power domain
topology tree description. The format and method to construct this array are
described in [Power Domain Topology Design]. The BL31 PSCI initilization code
requires this array to be described by the platform, either statically or
dynamically, to initialize the power domain topology tree. In case the array
is populated dynamically, then plat_core_pos_by_mpidr() and
plat_my_core_pos() should also be implemented suitably so that the topology
tree description matches the CPU indices returned by these APIs. These APIs
together form the platform interface for the PSCI topology framework.


## Function : plat_setup_psci_ops() [mandatory]

    Argument : uintptr_t, const plat_psci_ops **
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by PSCI initialization code. Its purpose is to let
the platform layer know about the warm boot entrypoint through the
`sec_entrypoint` (first argument) and to export handler routines for
platform-specific psci power management actions by populating the passed
pointer with a pointer to BL31's private `plat_psci_ops` structure.

A description of each member of this structure is given below. Please refer to
the ARM FVP specific implementation of these handlers in
[plat/arm/board/fvp/fvp_pm.c] as an example. For each PSCI function that the
platform wants to support, the associated operation or operations in this
structure must be provided and implemented (Refer section 4 of
[Firmware Design] for the PSCI API supported in Trusted Firmware). To disable
a PSCI function in a platform port, the operation should be removed from this
structure instead of providing an empty implementation.

#### plat_psci_ops.cpu_standby()

Perform the platform-specific actions to enter the standby state for a cpu
indicated by the passed argument. This provides a fast path for CPU standby
wherein overheads of PSCI state management and lock acquistion is avoided.
For this handler to be invoked by the PSCI `CPU_SUSPEND` API implementation,
the suspend state type specified in the `power-state` parameter should be
STANDBY and the target power domain level specified should be the CPU. The
handler should put the CPU into a low power retention state (usually by
issuing a wfi instruction) and ensure that it can be woken up from that
state by a normal interrupt. The generic code expects the handler to succeed.

#### plat_psci_ops.pwr_domain_on()

Perform the platform specific actions to power on a CPU, specified
by the `MPIDR` (first argument). The generic code expects the platform to
return PSCI_E_SUCCESS on success or PSCI_E_INTERN_FAIL for any failure.

#### plat_psci_ops.pwr_domain_off()

Perform the platform specific actions to prepare to power off the calling CPU
and its higher parent power domain levels as indicated by the `target_state`
(first argument). It is called by the PSCI `CPU_OFF` API implementation.

The `target_state` encodes the platform coordinated target local power states
for the CPU power domain and its parent power domain levels. The handler
needs to perform power management operation corresponding to the local state
at each power level.

For this handler, the local power state for the CPU power domain will be a
power down state where as it could be either power down, retention or run state
for the higher power domain levels depending on the result of state
coordination. The generic code expects the handler to succeed.

#### plat_psci_ops.pwr_domain_suspend()

Perform the platform specific actions to prepare to suspend the calling
CPU and its higher parent power domain levels as indicated by the
`target_state` (first argument). It is called by the PSCI `CPU_SUSPEND`
API implementation.

The `target_state` has a similar meaning as described in
the `pwr_domain_off()` operation. It encodes the platform coordinated
target local power states for the CPU power domain and its parent
power domain levels. The handler needs to perform power management operation
corresponding to the local state at each power level. The generic code
expects the handler to succeed.

The difference between turning a power domain off versus suspending it
is that in the former case, the power domain is expected to re-initialize
its state when it is next powered on (see `pwr_domain_on_finish()`). In the
latter case, the power domain is expected to save enough state so that it can
resume execution by restoring this state when its powered on (see
`pwr_domain_suspend_finish()`).

#### plat_psci_ops.pwr_domain_on_finish()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an earlier PSCI `CPU_ON` call.
It performs the platform-specific setup required to initialize enough state for
this CPU to enter the normal world and also provide secure runtime firmware
services.

The `target_state` (first argument) is the prior state of the power domains
immediately before the CPU was turned on. It indicates which power domains
above the CPU might require initialization due to having previously been in
low power states. The generic code expects the handler to succeed.

#### plat_psci_ops.pwr_domain_suspend_finish()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an asynchronous wakeup
event, for example a timer interrupt that was programmed by the CPU during the
`CPU_SUSPEND` call or `SYSTEM_SUSPEND` call. It performs the platform-specific
setup required to restore the saved state for this CPU to resume execution
in the normal world and also provide secure runtime firmware services.

The `target_state` (first argument) has a similar meaning as described in
the `pwr_domain_on_finish()` operation. The generic code expects the platform
to succeed.

#### plat_psci_ops.validate_power_state()

This function is called by the PSCI implementation during the `CPU_SUSPEND`
call to validate the `power_state` parameter of the PSCI API and if valid,
populate it in `req_state` (second argument) array as power domain level
specific local states. If the `power_state` is invalid, the platform must
return PSCI_E_INVALID_PARAMS as error, which is propagated back to the
normal world PSCI client.

#### plat_psci_ops.validate_ns_entrypoint()

This function is called by the PSCI implementation during the `CPU_SUSPEND`,
`SYSTEM_SUSPEND` and `CPU_ON` calls to validate the non-secure `entry_point`
parameter passed by the normal world. If the `entry_point` is invalid,
the platform must return PSCI_E_INVALID_ADDRESS as error, which is
propagated back to the normal world PSCI client.

#### plat_psci_ops.get_sys_suspend_power_state()

This function is called by the PSCI implementation during the `SYSTEM_SUSPEND`
call to get the `req_state` parameter from platform which encodes the power
domain level specific local states to suspend to system affinity level. The
`req_state` will be utilized to do the PSCI state coordination and
`pwr_domain_suspend()` will be invoked with the coordinated target state to
enter system suspend.


3.6  Interrupt Management framework (in BL31)
----------------------------------------------
BL31 implements an Interrupt Management Framework (IMF) to manage interrupts
generated in either security state and targeted to EL1 or EL2 in the non-secure
state or EL3/S-EL1 in the secure state.  The design of this framework is
described in the [IMF Design Guide]

A platform should export the following APIs to support the IMF. The following
text briefly describes each api and its implementation in ARM standard
platforms. The API implementation depends upon the type of interrupt controller
present in the platform. ARM standard platform layer supports both [ARM Generic
Interrupt Controller version 2.0 (GICv2)][ARM GIC Architecture Specification 2.0]
and [3.0 (GICv3)][ARM GIC Architecture Specification 3.0]. Juno builds the ARM
Standard layer to use GICv2 and the FVP can be configured to use either GICv2 or
GICv3 depending on the build flag `FVP_USE_GIC_DRIVER` (See FVP platform
specific build options in [User Guide] for more details).

### Function : plat_interrupt_type_to_line() [mandatory]

    Argument : uint32_t, uint32_t
    Return   : uint32_t

The ARM processor signals an interrupt exception either through the IRQ or FIQ
interrupt line. The specific line that is signaled depends on how the interrupt
controller (IC) reports different interrupt types from an execution context in
either security state. The IMF uses this API to determine which interrupt line
the platform IC uses to signal each type of interrupt supported by the framework
from a given security state. This API must be invoked at EL3.

The first parameter will be one of the `INTR_TYPE_*` values (see [IMF Design
Guide]) indicating the target type of the interrupt, the second parameter is the
security state of the originating execution context. The return result is the
bit position in the `SCR_EL3` register of the respective interrupt trap: IRQ=1,
FIQ=2.

In the case of ARM standard platforms using GICv2, S-EL1 interrupts are
configured as FIQs and Non-secure interrupts as IRQs from either security
state.

In the case of ARM standard platforms using GICv3, the interrupt line to be
configured depends on the security state of the execution context when the
interrupt is signalled and are as follows:
*  The S-EL1 interrupts are signaled as IRQ in S-EL0/1 context and as FIQ in
   NS-EL0/1/2 context.
*  The Non secure interrupts are signaled as FIQ in S-EL0/1 context and as IRQ
   in the NS-EL0/1/2 context.
*  The EL3 interrupts are signaled as FIQ in both S-EL0/1 and NS-EL0/1/2
   context.


### Function : plat_ic_get_pending_interrupt_type() [mandatory]

    Argument : void
    Return   : uint32_t

This API returns the type of the highest priority pending interrupt at the
platform IC. The IMF uses the interrupt type to retrieve the corresponding
handler function. `INTR_TYPE_INVAL` is returned when there is no interrupt
pending. The valid interrupt types that can be returned are `INTR_TYPE_EL3`,
`INTR_TYPE_S_EL1` and `INTR_TYPE_NS`. This API must be invoked at EL3.

In the case of ARM standard platforms using GICv2, the _Highest Priority
Pending Interrupt Register_ (`GICC_HPPIR`) is read to determine the id of
the pending interrupt. The type of interrupt depends upon the id value as
follows.

1. id < 1022 is reported as a S-EL1 interrupt
2. id = 1022 is reported as a Non-secure interrupt.
3. id = 1023 is reported as an invalid interrupt type.

In the case of ARM standard platforms using GICv3, the system register
`ICC_HPPIR0_EL1`, _Highest Priority Pending group 0 Interrupt Register_,
is read to determine the id of the pending interrupt. The type of interrupt
depends upon the id value as follows.

1. id = `PENDING_G1S_INTID` (1020) is reported as a S-EL1 interrupt
2. id = `PENDING_G1NS_INTID` (1021) is reported as a Non-secure interrupt.
3. id = `GIC_SPURIOUS_INTERRUPT` (1023) is reported as an invalid interrupt type.
4. All other interrupt id's are reported as EL3 interrupt.


### Function : plat_ic_get_pending_interrupt_id() [mandatory]

    Argument : void
    Return   : uint32_t

This API returns the id of the highest priority pending interrupt at the
platform IC. `INTR_ID_UNAVAILABLE` is returned when there is no interrupt
pending.

In the case of ARM standard platforms using GICv2, the _Highest Priority
Pending Interrupt Register_ (`GICC_HPPIR`) is read to determine the id of the
pending interrupt. The id that is returned by API depends upon the value of
the id read from the interrupt controller as follows.

1. id < 1022. id is returned as is.
2. id = 1022. The _Aliased Highest Priority Pending Interrupt Register_
   (`GICC_AHPPIR`) is read to determine the id of the non-secure interrupt.
   This id is returned by the API.
3. id = 1023. `INTR_ID_UNAVAILABLE` is returned.

In the case of ARM standard platforms using GICv3, if the API is invoked from
EL3, the system register `ICC_HPPIR0_EL1`, _Highest Priority Pending Interrupt
group 0 Register_, is read to determine the id of the pending interrupt. The id
that is returned by API depends upon the value of the id read from the
interrupt controller as follows.

1. id < `PENDING_G1S_INTID` (1020). id is returned as is.
2. id = `PENDING_G1S_INTID` (1020) or `PENDING_G1NS_INTID` (1021). The system
   register `ICC_HPPIR1_EL1`, _Highest Priority Pending Interrupt group 1
   Register_ is read to determine the id of the group 1 interrupt. This id
   is returned by the API as long as it is a valid interrupt id
3. If the id is any of the special interrupt identifiers,
   `INTR_ID_UNAVAILABLE` is returned.

When the API invoked from S-EL1 for GICv3 systems, the id read from system
register `ICC_HPPIR1_EL1`, _Highest Priority Pending group 1 Interrupt
Register_, is returned if is not equal to GIC_SPURIOUS_INTERRUPT (1023) else
`INTR_ID_UNAVAILABLE` is returned.

### Function : plat_ic_acknowledge_interrupt() [mandatory]

    Argument : void
    Return   : uint32_t

This API is used by the CPU to indicate to the platform IC that processing of
the highest pending interrupt has begun. It should return the id of the
interrupt which is being processed.

This function in ARM standard platforms using GICv2, reads the _Interrupt
Acknowledge Register_ (`GICC_IAR`). This changes the state of the highest
priority pending interrupt from pending to active in the interrupt controller.
It returns the value read from the `GICC_IAR`. This value is the id of the
interrupt whose state has been changed.

In the case of ARM standard platforms using GICv3, if the API is invoked
from EL3, the function reads the system register `ICC_IAR0_EL1`, _Interrupt
Acknowledge Register group 0_. If the API is invoked from S-EL1, the function
reads the system register `ICC_IAR1_EL1`, _Interrupt Acknowledge Register
group 1_. The read changes the state of the highest pending interrupt from
pending to active in the interrupt controller. The value read is returned
and is the id of the interrupt whose state has been changed.

The TSP uses this API to start processing of the secure physical timer
interrupt.


### Function : plat_ic_end_of_interrupt() [mandatory]

    Argument : uint32_t
    Return   : void

This API is used by the CPU to indicate to the platform IC that processing of
the interrupt corresponding to the id (passed as the parameter) has
finished. The id should be the same as the id returned by the
`plat_ic_acknowledge_interrupt()` API.

ARM standard platforms write the id to the _End of Interrupt Register_
(`GICC_EOIR`) in case of GICv2, and to `ICC_EOIR0_EL1` or `ICC_EOIR1_EL1`
system register in case of GICv3 depending on where the API is invoked from,
EL3 or S-EL1. This deactivates the corresponding interrupt in the interrupt
controller.

The TSP uses this API to finish processing of the secure physical timer
interrupt.


### Function : plat_ic_get_interrupt_type() [mandatory]

    Argument : uint32_t
    Return   : uint32_t

This API returns the type of the interrupt id passed as the parameter.
`INTR_TYPE_INVAL` is returned if the id is invalid. If the id is valid, a valid
interrupt type (one of `INTR_TYPE_EL3`, `INTR_TYPE_S_EL1` and `INTR_TYPE_NS`) is
returned depending upon how the interrupt has been configured by the platform
IC. This API must be invoked at EL3.

ARM standard platforms using GICv2 configures S-EL1 interrupts as Group0 interrupts
and Non-secure interrupts as Group1 interrupts. It reads the group value
corresponding to the interrupt id from the relevant _Interrupt Group Register_
(`GICD_IGROUPRn`). It uses the group value to determine the type of interrupt.

In the case of ARM standard platforms using GICv3, both the _Interrupt Group
Register_ (`GICD_IGROUPRn`) and _Interrupt Group Modifier Register_
(`GICD_IGRPMODRn`) is read to figure out whether the interrupt is configured
as Group 0 secure interrupt, Group 1 secure interrupt or Group 1 NS interrupt.


3.7  Crash Reporting mechanism (in BL31)
----------------------------------------------
BL31 implements a crash reporting mechanism which prints the various registers
of the CPU to enable quick crash analysis and debugging. It requires that a
console is designated as the crash console by the platform which will be used to
print the register dump.

The following functions must be implemented by the platform if it wants crash
reporting mechanism in BL31. The functions are implemented in assembly so that
they can be invoked without a C Runtime stack.

### Function : plat_crash_console_init

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to initialize the crash
console. It should only use the general purpose registers x0 to x2 to do the
initialization and returns 1 on success.

### Function : plat_crash_console_putc

    Argument : int
    Return   : int

This API is used by the crash reporting mechanism to print a character on the
designated crash console. It should only use general purpose registers x1 and
x2 to do its work. The parameter and the return value are in general purpose
register x0.

4.  Build flags
---------------

*   **ENABLE_PLAT_COMPAT**
    All the platforms ports conforming to this API specification should define
    the build flag `ENABLE_PLAT_COMPAT` to 0 as the compatibility layer should
    be disabled. For more details on compatibility layer, refer
    [Migration Guide].

There are some build flags which can be defined by the platform to control
inclusion or exclusion of certain BL stages from the FIP image. These flags
need to be defined in the platform makefile which will get included by the
build system.

*   **NEED_BL33**
    By default, this flag is defined `yes` by the build system and `BL33`
    build option should be supplied as a build option. The platform has the option
    of excluding the BL33 image in the `fip` image by defining this flag to
    `no`.

5.  C Library
-------------

To avoid subtle toolchain behavioral dependencies, the header files provided
by the compiler are not used. The software is built with the `-nostdinc` flag
to ensure no headers are included from the toolchain inadvertently. Instead the
required headers are included in the ARM Trusted Firmware source tree. The
library only contains those C library definitions required by the local
implementation. If more functionality is required, the needed library functions
will need to be added to the local implementation.

Versions of [FreeBSD] headers can be found in `include/stdlib`. Some of these
headers have been cut down in order to simplify the implementation. In order to
minimize changes to the header files, the [FreeBSD] layout has been maintained.
The generic C library definitions can be found in `include/stdlib` with more
system and machine specific declarations in `include/stdlib/sys` and
`include/stdlib/machine`.

The local C library implementations can be found in `lib/stdlib`. In order to
extend the C library these files may need to be modified. It is recommended to
use a release version of [FreeBSD] as a starting point.

The C library header files in the [FreeBSD] source tree are located in the
`include` and `sys/sys` directories. [FreeBSD] machine specific definitions
can be found in the `sys/<machine-type>` directories. These files define things
like 'the size of a pointer' and 'the range of an integer'. Since an AArch64
port for [FreeBSD] does not yet exist, the machine specific definitions are
based on existing machine types with similar properties (for example SPARC64).

Where possible, C library function implementations were taken from [FreeBSD]
as found in the `lib/libc` directory.

A copy of the [FreeBSD] sources can be downloaded with `git`.

    git clone git://github.com/freebsd/freebsd.git -b origin/release/9.2.0


6.  Storage abstraction layer
-----------------------------

In order to improve platform independence and portability an storage abstraction
layer is used to load data from non-volatile platform storage.

Each platform should register devices and their drivers via the Storage layer.
These drivers then need to be initialized by bootloader phases as
required in their respective `blx_platform_setup()` functions.  Currently
storage access is only required by BL1 and BL2 phases. The `load_image()`
function uses the storage layer to access non-volatile platform storage.

It is mandatory to implement at least one storage driver. For the ARM
development platforms the Firmware Image Package (FIP) driver is provided as
the default means to load data from storage (see the "Firmware Image Package"
section in the [User Guide]). The storage layer is described in the header file
`include/drivers/io/io_storage.h`. The implementation of the common library
is in `drivers/io/io_storage.c` and the driver files are located in
`drivers/io/`.

Each IO driver must provide `io_dev_*` structures, as described in
`drivers/io/io_driver.h`.  These are returned via a mandatory registration
function that is called on platform initialization.  The semi-hosting driver
implementation in `io_semihosting.c` can be used as an example.

The Storage layer provides mechanisms to initialize storage devices before
IO operations are called.  The basic operations supported by the layer
include `open()`, `close()`, `read()`, `write()`, `size()` and `seek()`.
Drivers do not have to implement all operations, but each platform must
provide at least one driver for a device capable of supporting generic
operations such as loading a bootloader image.

The current implementation only allows for known images to be loaded by the
firmware. These images are specified by using their identifiers, as defined in
[include/plat/common/platform_def.h] (or a separate header file included from
there). The platform layer (`plat_get_image_source()`) then returns a reference
to a device and a driver-specific `spec` which will be understood by the driver
to allow access to the image data.

The layer is designed in such a way that is it possible to chain drivers with
other drivers.  For example, file-system drivers may be implemented on top of
physical block devices, both represented by IO devices with corresponding
drivers.  In such a case, the file-system "binding" with the block device may
be deferred until the file-system device is initialised.

The abstraction currently depends on structures being statically allocated
by the drivers and callers, as the system does not yet provide a means of
dynamically allocating memory.  This may also have the affect of limiting the
amount of open resources per driver.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved._


[ARM GIC Architecture Specification 2.0]: http://arminfo.emea.arm.com/help/topic/com.arm.doc.ihi0048b/IHI0048B_gic_architecture_specification.pdf
[ARM GIC Architecture Specification 3.0]: http://arminfo.emea.arm.com/help/topic/com.arm.doc.ihi0069a/IHI0069A_gic_architecture_specification.pdf
[IMF Design Guide]:                       interrupt-framework-design.md
[User Guide]:                             user-guide.md
[FreeBSD]:                                http://www.freebsd.org
[Firmware Design]:                        firmware-design.md
[Power Domain Topology Design]:           psci-pd-tree.md
[PSCI]:                                   http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
[Migration Guide]:                        platform-migration-guide.md
[Firmware Update]:                        firmware-update.md

[plat/common/aarch64/platform_mp_stack.S]: ../plat/common/aarch64/platform_mp_stack.S
[plat/common/aarch64/platform_up_stack.S]: ../plat/common/aarch64/platform_up_stack.S
[plat/arm/board/fvp/fvp_pm.c]:             ../plat/arm/board/fvp/fvp_pm.c
[include/common/bl_common.h]:              ../include/common/bl_common.h
[include/plat/arm/common/arm_def.h]:       ../include/plat/arm/common/arm_def.h
[include/plat/common/common_def.h]:        ../include/plat/common/common_def.h
[include/plat/common/platform.h]:          ../include/plat/common/platform.h
[include/plat/arm/common/plat_arm.h]:      ../include/plat/arm/common/plat_arm.h]
