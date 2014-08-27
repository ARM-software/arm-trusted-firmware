ARM Trusted Firmware Porting Guide
==================================

Contents
--------

1.  Introduction
2.  Common Modifications
    *   Common mandatory modifications
    *   Handling reset
    *   Common optional modifications
3.  Boot Loader stage specific modifications
    *   Boot Loader stage 1 (BL1)
    *   Boot Loader stage 2 (BL2)
    *   Boot Loader stage 3-1 (BL3-1)
    *   PSCI implementation (in BL3-1)
    *   Interrupt Management framework (in BL3-1)
    *   Crash Reporting mechanism (in BL3-1)
4.  C Library
5.  Storage abstraction layer

- - - - - - - - - - - - - - - - - -

1.  Introduction
----------------

Porting the ARM Trusted Firmware to a new platform involves making some
mandatory and optional modifications for both the cold and warm boot paths.
Modifications consist of:

*   Implementing a platform-specific function or variable,
*   Setting up the execution context in a certain way, or
*   Defining certain constants (for example #defines).

The platform-specific functions and variables are all declared in
[include/plat/common/platform.h]. The firmware provides a default implementation
of variables and functions to fulfill the optional requirements. These
implementations are all weakly defined; they are provided to ease the porting
effort. Each platform port can override them with its own implementation if the
default implementation is inadequate.

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
stage. In the ARM FVP port, each BL stage configures the MMU in its platform-
specific architecture setup function, for example `blX_plat_arch_setup()`.

Each platform must allocate a block of identity mapped secure memory with
Device-nGnRE attributes aligned to page boundary (4K) for each BL stage. This
memory is identified by the section name `tzfw_coherent_mem` so that its
possible for the firmware to place variables in it using the following C code
directive:

    __attribute__ ((section("tzfw_coherent_mem")))

Or alternatively the following assembler code directive:

    .section tzfw_coherent_mem

The `tzfw_coherent_mem` section is used to allocate any data structures that are
accessed both when a CPU is executing with its MMU and caches enabled, and when
it's running with its MMU and caches disabled. Examples are given below.

The following variables, functions and constants must be defined by the platform
for the firmware to work correctly.


### File : platform_def.h [mandatory]

Each platform must ensure that a header file of this name is in the system
include path with the following constants defined. This may require updating the
list of `PLAT_INCLUDES` in the `platform.mk` file. In the ARM FVP port, this
file is found in [plat/fvp/include/platform_def.h].

*   **#define : PLATFORM_LINKER_FORMAT**

    Defines the linker format used by the platform, for example
    `elf64-littleaarch64` used by the FVP.

*   **#define : PLATFORM_LINKER_ARCH**

    Defines the processor architecture for the linker by the platform, for
    example `aarch64` used by the FVP.

*   **#define : PLATFORM_STACK_SIZE**

    Defines the normal stack memory available to each CPU. This constant is used
    by [plat/common/aarch64/platform_mp_stack.S] and
    [plat/common/aarch64/platform_up_stack.S].

*   **#define : FIRMWARE_WELCOME_STR**

    Defines the character string printed by BL1 upon entry into the `bl1_main()`
    function.

*   **#define : BL2_IMAGE_NAME**

    Name of the BL2 binary image on the host file-system. This name is used by
    BL1 to load BL2 into secure memory from non-volatile storage.

*   **#define : BL31_IMAGE_NAME**

    Name of the BL3-1 binary image on the host file-system. This name is used by
    BL2 to load BL3-1 into secure memory from platform storage.

*   **#define : BL33_IMAGE_NAME**

    Name of the BL3-3 binary image on the host file-system. This name is used by
    BL2 to load BL3-3 into non-secure memory from platform storage.

*   **#define : PLATFORM_CACHE_LINE_SIZE**

    Defines the size (in bytes) of the largest cache line across all the cache
    levels in the platform.

*   **#define : PLATFORM_CLUSTER_COUNT**

    Defines the total number of clusters implemented by the platform in the
    system.

*   **#define : PLATFORM_CORE_COUNT**

    Defines the total number of CPUs implemented by the platform across all
    clusters in the system.

*   **#define : PLATFORM_MAX_CPUS_PER_CLUSTER**

    Defines the maximum number of CPUs that can be implemented within a cluster
    on the platform.

*   **#define : PLATFORM_NUM_AFFS**

    Defines the total number of nodes in the affinity heirarchy at all affinity
    levels used by the platform.

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

    Defines the base address in secure RAM where BL2 loads the BL3-1 binary
    image. Must be aligned on a page-size boundary.

*   **#define : BL31_LIMIT**

    Defines the maximum address in secure RAM that the BL3-1 image can occupy.

*   **#define : NS_IMAGE_OFFSET**

    Defines the base address in non-secure DRAM where BL2 loads the BL3-3 binary
    image. Must be aligned on a page-size boundary.

If a BL3-2 image is supported by the platform, the following constants must
also be defined:

*   **#define : BL32_IMAGE_NAME**

    Name of the BL3-2 binary image on the host file-system. This name is used by
    BL2 to load BL3-2 into secure memory from platform storage.

*   **#define : BL32_BASE**

    Defines the base address in secure memory where BL2 loads the BL3-2 binary
    image. Must be aligned on a page-size boundary.

*   **#define : BL32_LIMIT**

    Defines the maximum address that the BL3-2 image can occupy.

If the Test Secure-EL1 Payload (TSP) instantiation of BL3-2 is supported by the
platform, the following constants must also be defined:

*   **#define : TSP_SEC_MEM_BASE**

    Defines the base address of the secure memory used by the TSP image on the
    platform. This must be at the same address or below `BL32_BASE`.

*   **#define : TSP_SEC_MEM_SIZE**

    Defines the size of the secure memory used by the BL3-2 image on the
    platform. `TSP_SEC_MEM_BASE` and `TSP_SEC_MEM_SIZE` must fully accomodate
    the memory required by the BL3-2 image, defined by `BL32_BASE` and
    `BL32_LIMIT`.

*   **#define : TSP_IRQ_SEC_PHY_TIMER**

    Defines the ID of the secure physical generic timer interrupt used by the
    TSP's interrupt handling code.

If the platform port uses the IO storage framework, the following constants
must also be defined:

*   **#define : MAX_IO_DEVICES**

    Defines the maximum number of registered IO devices. Attempting to register
    more devices than this value using `io_register_device()` will fail with
    IO_RESOURCES_EXHAUSTED.

*   **#define : MAX_IO_HANDLES**

    Defines the maximum number of open IO handles. Attempting to open more IO
    entities than this value using `io_open()` will fail with
    IO_RESOURCES_EXHAUSTED.

The following constants are optional. They should be defined when the platform
memory layout implies some image overlaying like on FVP.

*   **#define : BL31_PROGBITS_LIMIT**

    Defines the maximum address in secure RAM that the BL3-1's progbits sections
    can occupy.

*   **#define : TSP_PROGBITS_LIMIT**

    Defines the maximum address that the TSP's progbits sections can occupy.

### File : plat_macros.S [mandatory]

Each platform must ensure a file of this name is in the system include path with
the following macro defined. In the ARM FVP port, this file is found in
[plat/fvp/include/plat_macros.S].

*   **Macro : plat_print_gic_regs**

    This macro allows the crash reporting routine to print GIC registers
    in case of an unhandled exception in BL3-1. This aids in debugging and
    this macro can be defined to be empty in case GIC register reporting is
    not desired.

*   **Macro : plat_print_interconnect_regs**

    This macro allows the crash reporting routine to print interconnect registers
    in case of an unhandled exception in BL3-1. This aids in debugging and
    this macro can be defined to be empty in case interconnect register reporting
    is not desired. In the ARM FVP port, the CCI snoop control registers are
    reported.

### Other mandatory modifications

The following mandatory modifications may be implemented in any file
the implementer chooses. In the ARM FVP port, they are implemented in
[plat/fvp/aarch64/plat_common.c].

*   **Function : uint64_t plat_get_syscnt_freq(void)**

    This function is used by the architecture setup code to retrieve the
    counter frequency for the CPU's generic timer.  This value will be
    programmed into the `CNTFRQ_EL0` register.
    In the ARM FVP port, it returns the base frequency of the system counter,
    which is retrieved from the first entry in the frequency modes table.


2.2 Handling Reset
------------------

BL1 by default implements the reset vector where execution starts from a cold
or warm boot. BL3-1 can be optionally set as a reset vector using the
RESET_TO_BL31 make variable.

For each CPU, the reset vector code is responsible for the following tasks:

1.  Distinguishing between a cold boot and a warm boot.

2.  In the case of a cold boot and the CPU being a secondary CPU, ensuring that
    the CPU is placed in a platform-specific state until the primary CPU
    performs the necessary steps to remove it from this state.

3.  In the case of a warm boot, ensuring that the CPU jumps to a platform-
    specific address in the BL3-1 image in the same processor mode as it was
    when released from reset.

The following functions need to be implemented by the platform port to enable
reset vector code to perform the above tasks.


### Function : platform_get_entrypoint() [mandatory]

    Argument : unsigned long
    Return   : unsigned int

This function is called with the `SCTLR.M` and `SCTLR.C` bits disabled. The CPU
is identified by its `MPIDR`, which is passed as the argument. The function is
responsible for distinguishing between a warm and cold reset using platform-
specific means. If it's a warm reset then it returns the entrypoint into the
BL3-1 image that the CPU must jump to. If it's a cold reset then this function
must return zero.

This function is also responsible for implementing a platform-specific mechanism
to handle the condition where the CPU has been warm reset but there is no
entrypoint to jump to.

This function does not follow the Procedure Call Standard used by the
Application Binary Interface for the ARM 64-bit architecture. The caller should
not assume that callee saved registers are preserved across a call to this
function.

This function fulfills requirement 1 and 3 listed above.


### Function : plat_secondary_cold_boot_setup() [mandatory]

    Argument : void
    Return   : void

This function is called with the MMU and data caches disabled. It is responsible
for placing the executing secondary CPU in a platform-specific state until the
primary CPU performs the necessary actions to bring it out of that state and
allow entry into the OS.

In the ARM FVP port, each secondary CPU powers itself off. The primary CPU is
responsible for powering up the secondary CPU when normal world software
requires them.

This function fulfills requirement 2 above.


### Function : platform_is_primary_cpu() [mandatory]

    Argument : unsigned long
    Return   : unsigned int

This function identifies a CPU by its `MPIDR`, which is passed as the argument,
to determine whether this CPU is the primary CPU or a secondary CPU. A return
value of zero indicates that the CPU is not the primary CPU, while a non-zero
return value indicates that the CPU is the primary CPU.


### Function : platform_mem_init() [mandatory]

    Argument : void
    Return   : void

This function is called before any access to data is made by the firmware, in
order to carry out any essential memory initialization.

The ARM FVP port uses this function to initialize the mailbox memory used for
providing the warm-boot entry-point addresses.



2.3 Common optional modifications
---------------------------------

The following are helper functions implemented by the firmware that perform
common platform-specific tasks. A platform may choose to override these
definitions.


### Function : platform_get_core_pos()

    Argument : unsigned long
    Return   : int

A platform may need to convert the `MPIDR` of a CPU to an absolute number, which
can be used as a CPU-specific linear index into blocks of memory (for example
while allocating per-CPU stacks). This routine contains a simple mechanism
to perform this conversion, using the assumption that each cluster contains a
maximum of 4 CPUs:

    linear index = cpu_id + (cluster_id * 4)

    cpu_id = 8-bit value in MPIDR at affinity level 0
    cluster_id = 8-bit value in MPIDR at affinity level 1


### Function : platform_set_stack()

    Argument : unsigned long
    Return   : void

This function sets the current stack pointer to the normal memory stack that
has been allocated for the CPU specificed by MPIDR. For BL images that only
require a stack for the primary CPU the parameter is ignored. The size of
the stack allocated to each CPU is specified by the platform defined constant
`PLATFORM_STACK_SIZE`.

Common implementations of this function for the UP and MP BL images are
provided in [plat/common/aarch64/platform_up_stack.S] and
[plat/common/aarch64/platform_mp_stack.S]


### Function : platform_get_stack()

    Argument : unsigned long
    Return   : unsigned long

This function returns the base address of the normal memory stack that
has been allocated for the CPU specificed by MPIDR. For BL images that only
require a stack for the primary CPU the parameter is ignored. The size of
the stack allocated to each CPU is specified by the platform defined constant
`PLATFORM_STACK_SIZE`.

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
exceptions types are listed in the [include/runtime_svc.h] header file. Note
that these constants are not related to any architectural exception code; they
are just an ARM Trusted Firmware convention.


### Function : plat_reset_handler()

    Argument : void
    Return   : void

A platform may need to do additional initialization after reset. This function
allows the platform to do the platform specific intializations. Platform
specific errata workarounds could also be implemented here. The api should
preserve the value in x10 register as it is used by the caller to store the
return address.

The default implementation doesn't do anything.

### Function : plat_disable_acp()

    Argument : void
    Return   : void

This api allows a platform to disable the Accelerator Coherency Port (if
present) during a cluster power down sequence. The default weak implementation
doesn't do anything. Since this api is called during the power down sequence,
it has restrictions for stack usage and it can use the registers x0 - x17 as
scratch registers. It should preserve the value in x18 register as it is used
by the caller to store the return address.


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

3.  Loading the BL2 image from non-volatile storage into secure memory at the
    address specified by the platform defined constant `BL2_BASE`.

4.  Populating a `meminfo` structure with the following information in memory,
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


### Function : bl1_plat_arch_setup() [mandatory]

    Argument : void
    Return   : void

This function performs any platform-specific and architectural setup that the
platform requires.  Platform-specific setup might include configuration of
memory controllers, configuration of the interconnect to allow the cluster
to service cache snoop requests from another cluster, and so on.

In the ARM FVP port, this function enables CCI snoops into the cluster that the
primary CPU is part of. It also enables the MMU.

This function helps fulfill requirement 2 above.


### Function : bl1_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches enabled. It is responsible
for performing any remaining platform-specific setup that can occur after the
MMU and data cache have been enabled.

This function is also responsible for initializing the storage abstraction layer
which is used to load further bootloader images.

This function helps fulfill requirement 3 above.


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

This function helps fulfill requirement 3 above.


### Function : bl1_init_bl2_mem_layout() [optional]

    Argument : meminfo *, meminfo *, unsigned int, unsigned long
    Return   : void

BL1 needs to tell the next stage the amount of secure RAM available
for it to use. This information is populated in a `meminfo`
structure.

Depending upon where BL2 has been loaded in secure RAM (determined by
`BL2_BASE`), BL1 calculates the amount of free memory available for BL2 to use.
BL1 also ensures that its data sections resident in secure RAM are not visible
to BL2. An illustration of how this is done in the ARM FVP port is given in the
[User Guide], in the Section "Memory layout on Base FVP".


### Function : bl1_plat_set_bl2_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL2 image and it can be used to overwrite
the entry point set by loader and also set the security state and SPSR which
represents the entry point system state for BL2.

On FVP, we are setting the security state and the SPSR for the BL2 entrypoint


3.2 Boot Loader Stage 2 (BL2)
-----------------------------

The BL2 stage is executed only by the primary CPU, which is determined in BL1
using the `platform_is_primary_cpu()` function. BL1 passed control to BL2 at
`BL2_BASE`. BL2 executes in Secure EL1 and is responsible for:

1.  (Optional) Loading the BL3-0 binary image (if present) from platform
    provided non-volatile storage. To load the BL3-0 image, BL2 makes use of
    the `meminfo` returned by the `bl2_plat_get_bl30_meminfo()` function.
    The platform also defines the address in memory where BL3-0 is loaded
    through the optional constant `BL30_BASE`. BL2 uses this information
    to determine if there is enough memory to load the BL3-0 image.
    Subsequent handling of the BL3-0 image is platform-specific and is
    implemented in the `bl2_plat_handle_bl30()` function.
    If `BL30_BASE` is not defined then this step is not performed.

2.  Loading the BL3-1 binary image into secure RAM from non-volatile storage. To
    load the BL3-1 image, BL2 makes use of the `meminfo` structure passed to it
    by BL1. This structure allows BL2 to calculate how much secure RAM is
    available for its use. The platform also defines the address in secure RAM
    where BL3-1 is loaded through the constant `BL31_BASE`. BL2 uses this
    information to determine if there is enough memory to load the BL3-1 image.

3.  (Optional) Loading the BL3-2 binary image (if present) from platform
    provided non-volatile storage. To load the BL3-2 image, BL2 makes use of
    the `meminfo` returned by the `bl2_plat_get_bl32_meminfo()` function.
    The platform also defines the address in memory where BL3-2 is loaded
    through the optional constant `BL32_BASE`. BL2 uses this information
    to determine if there is enough memory to load the BL3-2 image.
    If `BL32_BASE` is not defined then this and the next step is not performed.

4.  (Optional) Arranging to pass control to the BL3-2 image (if present) that
    has been pre-loaded at `BL32_BASE`. BL2 populates an `entry_point_info`
    structure in memory provided by the platform with information about how
    BL3-1 should pass control to the BL3-2 image.

5.  Loading the normal world BL3-3 binary image into non-secure DRAM from
    platform storage and arranging for BL3-1 to pass control to this image. This
    address is determined using the `plat_get_ns_image_entrypoint()` function
    described below.

6.  BL2 populates an `entry_point_info` structure in memory provided by the
    platform with information about how BL3-1 should pass control to the
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
specific to BL2. Platform security components are configured if required.
For the Base FVP the TZC-400 TrustZone controller is configured to only
grant non-secure access to DRAM. This avoids aliasing between secure and
non-secure accesses in the TLB and cache - secure execution states can use
the NS attributes in the MMU translation tables to access the DRAM.

This function is also responsible for initializing the storage abstraction layer
which is used to load further bootloader images.


### Function : bl2_plat_sec_mem_layout() [mandatory]

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It may execute with
the MMU and data caches enabled if the platform port does the necessary
initialization in `bl2_plat_arch_setup()`. It is only called by the primary CPU.

The purpose of this function is to return a pointer to a `meminfo` structure
populated with the extents of secure RAM available for BL2 to use. See
`bl2_early_platform_setup()` above.


### Function : bl2_plat_get_bl30_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL3-0 image. The meminfo provided by this is used by load_image() to
validate whether the BL3-0 image can be loaded within the given
memory from the given base.


### Function : bl2_plat_handle_bl30() [mandatory]

    Argument : image_info *
    Return   : int

This function is called after loading BL3-0 image and it is used to perform any
platform-specific actions required to handle the SCP firmware. Typically it
transfers the image into SCP memory using a platform-specific protocol and waits
until SCP executes it and signals to the Application Processor (AP) for BL2
execution to continue.

This function returns 0 on success, a negative error code otherwise.


### Function : bl2_plat_get_bl31_params() [mandatory]

    Argument : void
    Return   : bl31_params *

BL2 platform code needs to return a pointer to a `bl31_params` structure it
will use for passing information to BL3-1. The `bl31_params` structure carries
the following information.
    - Header describing the version information for interpreting the bl31_param
      structure
    - Information about executing the BL3-3 image in the `bl33_ep_info` field
    - Information about executing the BL3-2 image in the `bl32_ep_info` field
    - Information about the type and extents of BL3-1 image in the
      `bl31_image_info` field
    - Information about the type and extents of BL3-2 image in the
      `bl32_image_info` field
    - Information about the type and extents of BL3-3 image in the
      `bl33_image_info` field

The memory pointed by this structure and its sub-structures should be
accessible from BL3-1 initialisation code. BL3-1 might choose to copy the
necessary content, or maintain the structures until BL3-3 is initialised.


### Funtion : bl2_plat_get_bl31_ep_info() [mandatory]

    Argument : void
    Return   : entry_point_info *

BL2 platform code returns a pointer which is used to populate the entry point
information for BL3-1 entry point. The location pointed by it should be
accessible from BL1 while processing the synchronous exception to run to BL3-1.

On FVP this is allocated inside an bl2_to_bl31_params_mem structure which
is allocated at an address pointed by PARAMS_BASE.


### Function : bl2_plat_set_bl31_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL3-1 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL3-1.

On FVP, we are setting the security state and the SPSR for the BL3-1
entrypoint.

### Function : bl2_plat_set_bl32_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL3-2 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL3-2.

On FVP, we are setting the security state and the SPSR for the BL3-2
entrypoint

### Function : bl2_plat_set_bl33_ep_info() [mandatory]

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL3-3 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL3-3.

On FVP, we are setting the security state and the SPSR for the BL3-3
entrypoint

### Function : bl2_plat_get_bl32_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL3-2 image. The meminfo provided by this is used by load_image() to
validate whether the BL3-2 image can be loaded with in the given
memory from the given base.

### Function : bl2_plat_get_bl33_meminfo() [mandatory]

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL3-3 image. The meminfo provided by this is used by load_image() to
validate whether the BL3-3 image can be loaded with in the given
memory from the given base.

### Function : bl2_plat_flush_bl31_params() [mandatory]

    Argument : void
    Return   : void

Once BL2 has populated all the structures that needs to be read by BL1
and BL3-1 including the bl31_params structures and its sub-structures,
the bl31_ep_info structure and any platform specific data. It flushes
all these data to the main memory so that it is available when we jump to
later Bootloader stages with MMU off

### Function : plat_get_ns_image_entrypoint() [mandatory]

    Argument : void
    Return   : unsigned long

As previously described, BL2 is responsible for arranging for control to be
passed to a normal world BL image through BL3-1. This function returns the
entrypoint of that image, which BL3-1 uses to jump to it.

BL2 is responsible for loading the normal world BL3-3 image (e.g. UEFI).


3.2 Boot Loader Stage 3-1 (BL3-1)
---------------------------------

During cold boot, the BL3-1 stage is executed only by the primary CPU. This is
determined in BL1 using the `platform_is_primary_cpu()` function. BL1 passes
control to BL3-1 at `BL31_BASE`. During warm boot, BL3-1 is executed by all
CPUs. BL3-1 executes at EL3 and is responsible for:

1.  Re-initializing all architectural and platform state. Although BL1 performs
    some of this initialization, BL3-1 remains resident in EL3 and must ensure
    that EL3 architectural and platform state is completely initialized. It
    should make no assumptions about the system state when it receives control.

2.  Passing control to a normal world BL image, pre-loaded at a platform-
    specific address by BL2. BL3-1 uses the `entry_point_info` structure that BL2
    populated in memory to do this.

3.  Providing runtime firmware services. Currently, BL3-1 only implements a
    subset of the Power State Coordination Interface (PSCI) API as a runtime
    service. See Section 3.3 below for details of porting the PSCI
    implementation.

4.  Optionally passing control to the BL3-2 image, pre-loaded at a platform-
    specific address by BL2. BL3-1 exports a set of apis that allow runtime
    services to specify the security state in which the next image should be
    executed and run the corresponding image. BL3-1 uses the `entry_point_info`
    structure populated by BL2 to do this.

If BL3-1 is a reset vector, It also needs to handle the reset as specified in
section 2.2 before the tasks described above.

The following functions must be implemented by the platform port to enable BL3-1
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
subsequently overwritten by BL3-1 and similarly the `void *` pointing
to the platform data also needs to be saved.

On the ARM FVP port, BL2 passes a pointer to a `bl31_params` structure populated
in the secure DRAM at address `0x6000000` in the bl31_params * argument and it
does not use opaque pointer mentioned earlier. BL3-1 does not copy this
information to internal data structures as it guarantees that the secure
DRAM memory will not be overwritten. It maintains an internal reference to this
information in the `bl2_to_bl31_params` variable.

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
BL3-1 runtime services and normal world software can function correctly.

The ARM FVP port does the following:
*   Initializes the generic interrupt controller.
*   Configures the CLCD controller.
*   Enables system-level implementation of the generic timer counter.
*   Grants access to the system counter timer module
*   Initializes the FVP power controller device
*   Detects the system topology.


### Function : bl31_get_next_image_info() [mandatory]

    Argument : unsigned int
    Return   : entry_point_info *

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`.

This function is called by `bl31_main()` to retrieve information provided by
BL2 for the next image in the security state specified by the argument. BL3-1
uses this information to pass control to that image in the specified security
state. This function must return a pointer to the `entry_point_info` structure
(that was copied during `bl31_early_platform_setup()`) if the image exists. It
should return NULL otherwise.


3.3 Power State Coordination Interface (in BL3-1)
------------------------------------------------

The ARM Trusted Firmware's implementation of the PSCI API is based around the
concept of an _affinity instance_. Each _affinity instance_ can be uniquely
identified in a system by a CPU ID (the processor `MPIDR` is used in the PSCI
interface) and an _affinity level_. A processing element (for example, a
CPU) is at level 0. If the CPUs in the system are described in a tree where the
node above a CPU is a logical grouping of CPUs that share some state, then
affinity level 1 is that group of CPUs (for example, a cluster), and affinity
level 2 is a group of clusters (for example, the system). The implementation
assumes that the affinity level 1 ID can be computed from the affinity level 0
ID (for example, a unique cluster ID can be computed from the CPU ID). The
current implementation computes this on the basis of the recommended use of
`MPIDR` affinity fields in the ARM Architecture Reference Manual.

BL3-1's platform initialization code exports a pointer to the platform-specific
power management operations required for the PSCI implementation to function
correctly. This information is populated in the `plat_pm_ops` structure. The
PSCI implementation calls members of the `plat_pm_ops` structure for performing
power management operations for each affinity instance. For example, the target
CPU is specified by its `MPIDR` in a PSCI `CPU_ON` call. The `affinst_on()`
handler (if present) is called for each affinity instance as the PSCI
implementation powers up each affinity level implemented in the `MPIDR` (for
example, CPU, cluster and system).

The following functions must be implemented to initialize PSCI functionality in
the ARM Trusted Firmware.


### Function : plat_get_aff_count() [mandatory]

    Argument : unsigned int, unsigned long
    Return   : unsigned int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by the PSCI initialization code to detect the system
topology. Its purpose is to return the number of affinity instances implemented
at a given `affinity level` (specified by the first argument) and a given
`MPIDR` (specified by the second argument). For example, on a dual-cluster
system where first cluster implements 2 CPUs and the second cluster implements 4
CPUs, a call to this function with an `MPIDR` corresponding to the first cluster
(`0x0`) and affinity level 0, would return 2. A call to this function with an
`MPIDR` corresponding to the second cluster (`0x100`) and affinity level 0,
would return 4.


### Function : plat_get_aff_state() [mandatory]

    Argument : unsigned int, unsigned long
    Return   : unsigned int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by the PSCI initialization code. Its purpose is to
return the state of an affinity instance. The affinity instance is determined by
the affinity ID at a given `affinity level` (specified by the first argument)
and an `MPIDR` (specified by the second argument). The state can be one of
`PSCI_AFF_PRESENT` or `PSCI_AFF_ABSENT`. The latter state is used to cater for
system topologies where certain affinity instances are unimplemented. For
example, consider a platform that implements a single cluster with 4 CPUs and
another CPU implemented directly on the interconnect with the cluster. The
`MPIDR`s of the cluster would range from `0x0-0x3`. The `MPIDR` of the single
CPU would be 0x100 to indicate that it does not belong to cluster 0. Cluster 1
is missing but needs to be accounted for to reach this single CPU in the
topology tree. Hence it is marked as `PSCI_AFF_ABSENT`.


### Function : plat_get_max_afflvl() [mandatory]

    Argument : void
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by the PSCI implementation both during cold and warm
boot, to determine the maximum affinity level that the power management
operations should apply to. ARMv8-A has support for 4 affinity levels. It is
likely that hardware will implement fewer affinity levels. This function allows
the PSCI implementation to consider only those affinity levels in the system
that the platform implements. For example, the Base AEM FVP implements two
clusters with a configurable number of CPUs. It reports the maximum affinity
level as 1, resulting in PSCI power control up to the cluster level.


### Function : platform_setup_pm() [mandatory]

    Argument : const plat_pm_ops **
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by PSCI initialization code. Its purpose is to export
handler routines for platform-specific power management actions by populating
the passed pointer with a pointer to BL3-1's private `plat_pm_ops` structure.

A description of each member of this structure is given below. Please refer to
the ARM FVP specific implementation of these handlers in [plat/fvp/fvp_pm.c]
as an example. A platform port may choose not implement some of the power
management operations.

#### plat_pm_ops.affinst_standby()

Perform the platform-specific setup to enter the standby state indicated by the
passed argument.

#### plat_pm_ops.affinst_on()

Perform the platform specific setup to power on an affinity instance, specified
by the `MPIDR` (first argument) and `affinity level` (fourth argument). The
`state` (fifth argument) contains the current state of that affinity instance
(ON or OFF). This is useful to determine whether any action must be taken. For
example, while powering on a CPU, the cluster that contains this CPU might
already be in the ON state. The platform decides what actions must be taken to
transition from the current state to the target state (indicated by the power
management operation).

#### plat_pm_ops.affinst_off()

Perform the platform specific setup to power off an affinity instance in the
`MPIDR` of the calling CPU. It is called by the PSCI `CPU_OFF` API
implementation.

The `MPIDR` (first argument), `affinity level` (second argument) and `state`
(third argument) have a similar meaning as described in the `affinst_on()`
operation. They are used to identify the affinity instance on which the call
is made and its current state. This gives the platform port an indication of the
state transition it must make to perform the requested action. For example, if
the calling CPU is the last powered on CPU in the cluster, after powering down
affinity level 0 (CPU), the platform port should power down affinity level 1
(the cluster) as well.

#### plat_pm_ops.affinst_suspend()

Perform the platform specific setup to power off an affinity instance in the
`MPIDR` of the calling CPU. It is called by the PSCI `CPU_SUSPEND` API
implementation.

The `MPIDR` (first argument), `affinity level` (third argument) and `state`
(fifth argument) have a similar meaning as described in the `affinst_on()`
operation. They are used to identify the affinity instance on which the call
is made and its current state. This gives the platform port an indication of the
state transition it must make to perform the requested action. For example, if
the calling CPU is the last powered on CPU in the cluster, after powering down
affinity level 0 (CPU), the platform port should power down affinity level 1
(the cluster) as well.

The difference between turning an affinity instance off versus suspending it
is that in the former case, the affinity instance is expected to re-initialize
its state when its next powered on (see `affinst_on_finish()`). In the latter
case, the affinity instance is expected to save enough state so that it can
resume execution by restoring this state when its powered on (see
`affinst_suspend_finish()`).

#### plat_pm_ops.affinst_on_finish()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an earlier PSCI `CPU_ON` call.
It performs the platform-specific setup required to initialize enough state for
this CPU to enter the normal world and also provide secure runtime firmware
services.

The `MPIDR` (first argument), `affinity level` (second argument) and `state`
(third argument) have a similar meaning as described in the previous operations.

#### plat_pm_ops.affinst_on_suspend()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an asynchronous wakeup
event, for example a timer interrupt that was programmed by the CPU during the
`CPU_SUSPEND` call. It performs the platform-specific setup required to
restore the saved state for this CPU to resume execution in the normal world
and also provide secure runtime firmware services.

The `MPIDR` (first argument), `affinity level` (second argument) and `state`
(third argument) have a similar meaning as described in the previous operations.

BL3-1 platform initialization code must also detect the system topology and
the state of each affinity instance in the topology. This information is
critical for the PSCI runtime service to function correctly. More details are
provided in the description of the `plat_get_aff_count()` and
`plat_get_aff_state()` functions above.

3.4  Interrupt Management framework (in BL3-1)
----------------------------------------------
BL3-1 implements an Interrupt Management Framework (IMF) to manage interrupts
generated in either security state and targeted to EL1 or EL2 in the non-secure
state or EL3/S-EL1 in the secure state.  The design of this framework is
described in the [IMF Design Guide]

A platform should export the following APIs to support the IMF. The following
text briefly describes each api and its implementation on the FVP port. The API
implementation depends upon the type of interrupt controller present in the
platform. The FVP implements an ARM Generic Interrupt Controller (ARM GIC) as
per the version 2.0 of the [ARM GIC Architecture Specification]

### Function : plat_interrupt_type_to_line() [mandatory]

    Argument : uint32_t, uint32_t
    Return   : uint32_t

The ARM processor signals an interrupt exception either through the IRQ or FIQ
interrupt line. The specific line that is signaled depends on how the interrupt
controller (IC) reports different interrupt types from an execution context in
either security state. The IMF uses this API to determine which interrupt line
the platform IC uses to signal each type of interrupt supported by the framework
from a given security state.

The first parameter will be one of the `INTR_TYPE_*` values (see [IMF Design
Guide]) indicating the target type of the interrupt, the second parameter is the
security state of the originating execution context. The return result is the
bit position in the `SCR_EL3` register of the respective interrupt trap: IRQ=1,
FIQ=2.

The FVP port configures the ARM GIC to signal S-EL1 interrupts as FIQs and
Non-secure interrupts as IRQs from either security state.


### Function : plat_ic_get_pending_interrupt_type() [mandatory]

    Argument : void
    Return   : uint32_t

This API returns the type of the highest priority pending interrupt at the
platform IC. The IMF uses the interrupt type to retrieve the corresponding
handler function. `INTR_TYPE_INVAL` is returned when there is no interrupt
pending. The valid interrupt types that can be returned are `INTR_TYPE_EL3`,
`INTR_TYPE_S_EL1` and `INTR_TYPE_NS`.

The FVP port reads the _Highest Priority Pending Interrupt Register_
(`GICC_HPPIR`) to determine the id of the pending interrupt. The type of interrupt
depends upon the id value as follows.

1. id < 1022 is reported as a S-EL1 interrupt
2. id = 1022 is reported as a Non-secure interrupt.
3. id = 1023 is reported as an invalid interrupt type.


### Function : plat_ic_get_pending_interrupt_id() [mandatory]

    Argument : void
    Return   : uint32_t

This API returns the id of the highest priority pending interrupt at the
platform IC. The IMF passes the id returned by this API to the registered
handler for the pending interrupt if the `IMF_READ_INTERRUPT_ID` build time flag
is set. INTR_ID_UNAVAILABLE is returned when there is no interrupt pending.

The FVP port reads the _Highest Priority Pending Interrupt Register_
(`GICC_HPPIR`) to determine the id of the pending interrupt. The id that is
returned by API depends upon the value of the id read from the interrupt
controller as follows.

1. id < 1022. id is returned as is.
2. id = 1022. The _Aliased Highest Priority Pending Interrupt Register_
   (`GICC_AHPPIR`) is read to determine the id of the non-secure interrupt. This
   id is returned by the API.
3. id = 1023. `INTR_ID_UNAVAILABLE` is returned.


### Function : plat_ic_acknowledge_interrupt() [mandatory]

    Argument : void
    Return   : uint32_t

This API is used by the CPU to indicate to the platform IC that processing of
the highest pending interrupt has begun. It should return the id of the
interrupt which is being processed.

The FVP port reads the _Interrupt Acknowledge Register_ (`GICC_IAR`). This
changes the state of the highest priority pending interrupt from pending to
active in the interrupt controller. It returns the value read from the
`GICC_IAR`. This value is the id of the interrupt whose state has been changed.

The TSP uses this API to start processing of the secure physical timer
interrupt.


### Function : plat_ic_end_of_interrupt() [mandatory]

    Argument : uint32_t
    Return   : void

This API is used by the CPU to indicate to the platform IC that processing of
the interrupt corresponding to the id (passed as the parameter) has
finished. The id should be the same as the id returned by the
`plat_ic_acknowledge_interrupt()` API.

The FVP port writes the id to the _End of Interrupt Register_
(`GICC_EOIR`). This deactivates the corresponding interrupt in the interrupt
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
IC.

The FVP port configures S-EL1 interrupts as Group0 interrupts and Non-secure
interrupts as Group1 interrupts. It reads the group value corresponding to the
interrupt id from the relevant _Interrupt Group Register_ (`GICD_IGROUPRn`). It
uses the group value to determine the type of interrupt.

3.5  Crash Reporting mechanism (in BL3-1)
----------------------------------------------
BL3-1 implements a crash reporting mechanism which prints the various registers
of the CPU to enable quick crash analysis and debugging. It requires that a
console is designated as the crash console by the platform which will be used to
print the register dump.

The following functions must be implemented by the platform if it wants crash
reporting mechanism in BL3-1. The functions are implemented in assembly so that
they can be invoked without a C Runtime stack.

### Function : plat_crash_console_init

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to initialize the crash
console. It should only use the general purpose registers x0 to x2 to do the
initialization and returns 1 on success.

The FVP port designates the `PL011_UART0` as the crash console and calls the
console_core_init() to initialize the console.

### Function : plat_crash_console_putc

    Argument : int
    Return   : int

This API is used by the crash reporting mechanism to print a character on the
designated crash console. It should only use general purpose registers x1 and
x2 to do its work. The parameter and the return value are in general purpose
register x0.

The FVP port designates the `PL011_UART0` as the crash console and calls the
console_core_putc() to print the character on the console.

4.  C Library
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


5.  Storage abstraction layer
-----------------------------

In order to improve platform independence and portability an storage abstraction
layer is used to load data from non-volatile platform storage.

Each platform should register devices and their drivers via the Storage layer.
These drivers then need to be initialized by bootloader phases as
required in their respective `blx_platform_setup()` functions.  Currently
storage access is only required by BL1 and BL2 phases. The `load_image()`
function uses the storage layer to access non-volatile platform storage.

It is mandatory to implement at least one storage driver. For the FVP the
Firmware Image Package(FIP) driver is provided as the default means to load data
from storage (see the "Firmware Image Package" section in the [User Guide]).
The storage layer is described in the header file `include/io_storage.h`.  The
implementation of the common library is in `lib/io_storage.c` and the driver
files are located in `drivers/io/`.

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
firmware.  These images are specified by using their names, as defined in
[include/plat/common/platform.h]. The platform layer (`plat_get_image_source()`)
then returns a reference to a device and a driver-specific `spec` which will be
understood by the driver to allow access to the image data.

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

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[ARM GIC Architecture Specification]: http://arminfo.emea.arm.com/help/topic/com.arm.doc.ihi0048b/IHI0048B_gic_architecture_specification.pdf
[IMF Design Guide]:                   interrupt-framework-design.md
[User Guide]:                         user-guide.md
[FreeBSD]:                            http://www.freebsd.org

[plat/common/aarch64/platform_mp_stack.S]: ../plat/common/aarch64/platform_mp_stack.S
[plat/common/aarch64/platform_up_stack.S]: ../plat/common/aarch64/platform_up_stack.S
[plat/fvp/include/platform_def.h]:         ../plat/fvp/include/platform_def.h
[plat/fvp/include/plat_macros.S]:          ../plat/fvp/include/plat_macros.S
[plat/fvp/aarch64/plat_common.c]:          ../plat/fvp/aarch64/plat_common.c
[plat/fvp/plat_pm.c]:                      ../plat/fvp/plat_pm.c
[include/runtime_svc.h]:                   ../include/runtime_svc.h
[include/plat/common/platform.h]:          ../include/plat/common/platform.h
