ARM Trusted Firmware Porting Guide
==================================

Contents
--------

1.  Introduction
2.  Common Modifications
    *   Common mandatory modifications
    *   Common optional modifications
3.  Boot Loader stage specific modifications
    *   Boot Loader stage 1 (BL1)
    *   Boot Loader stage 2 (BL2)
    *   Boot Loader stage 3-1 (BL3-1)
    *   PSCI implementation (in BL3-1)
4. C Library

- - - - - - - - - - - - - - - - - -

1.  Introduction
----------------

Porting the ARM Trusted Firmware to a new platform involves making some
mandatory and optional modifications for both the cold and warm boot paths.
Modifications consist of:

*   Implementing a platform-specific function or variable,
*   Setting up the execution context in a certain way, or
*   Defining certain constants (for example #defines).

The firmware provides a default implementation of variables and functions to
fulfill the optional requirements. These implementations are all weakly defined;
they are provided to ease the porting effort. Each platform port can override
them with its own implementation if the default implementation is inadequate.

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


### File : platform.h [mandatory]

Each platform must export a header file of this name with the following
constants defined. In the ARM FVP port, this file is found in
[../plat/fvp/platform.h].

*   **#define : PLATFORM_LINKER_FORMAT**

    Defines the linker format used by the platform, for example
    `elf64-littleaarch64` used by the FVP.

*   **#define : PLATFORM_LINKER_ARCH**

    Defines the processor architecture for the linker by the platform, for
    example `aarch64` used by the FVP.

*   **#define : PLATFORM_STACK_SIZE**

    Defines the normal stack memory available to each CPU. This constant is used
    by `platform_set_stack()`.

*   **#define : FIRMWARE_WELCOME_STR**

    Defines the character string printed by BL1 upon entry into the `bl1_main()`
    function.

*   **#define : BL2_IMAGE_NAME**

    Name of the BL2 binary image on the host file-system. This name is used by
    BL1 to load BL2 into secure memory using semi-hosting.

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

*   **#define : PRIMARY_CPU**

    Defines the `MPIDR` of the primary CPU on the platform. This value is used
    after a cold boot to distinguish between primary and secondary CPUs.

*   **#define : TZROM_BASE**

    Defines the base address of secure ROM on the platform, where the BL1 binary
    is loaded. This constant is used by the linker scripts to ensure that the
    BL1 image fits into the available memory.

*   **#define : TZROM_SIZE**

    Defines the size of secure ROM on the platform. This constant is used by the
    linker scripts to ensure that the BL1 image fits into the available memory.

*   **#define : TZRAM_BASE**

    Defines the base address of the secure RAM on platform, where the data
    section of the BL1 binary is loaded. The BL2 and BL3-1 images are also
    loaded in this secure RAM region. This constant is used by the linker
    scripts to ensure that the BL1 data section and BL2/BL3-1 binary images fit
    into the available memory.

*   **#define : TZRAM_SIZE**

    Defines the size of the secure RAM on the platform. This constant is used by
    the linker scripts to ensure that the BL1 data section and BL2/BL3-1 binary
    images fit into the available memory.

*   **#define : SYS_CNTCTL_BASE**

    Defines the base address of the `CNTCTLBase` frame of the memory mapped
    counter and timer in the system level implementation of the generic timer.

*   **#define : BL2_BASE**

    Defines the base address in secure RAM where BL1 loads the BL2 binary image.
    Must be aligned on a page-size boundary.

*   **#define : BL31_BASE**

    Defines the base address in secure RAM where BL2 loads the BL3-1 binary
    image. Must be aligned on a page-size boundary.


### Other mandatory modifications

The following mandatory modifications may be implemented in any file
the implementer chooses. In the ARM FVP port, they are implemented in
[../plat/fvp/aarch64/plat_common.c].

*   **Variable : unsigned char platform_normal_stacks[X][Y]**

        where  X = PLATFORM_STACK_SIZE
          and  Y = PLATFORM_CORE_COUNT

    Each platform must allocate a block of memory with Normal Cacheable, Write
    back, Write allocate and Inner Shareable attributes aligned to the size (in
    bytes) of the largest cache line amongst all caches implemented in the
    system. A pointer to this memory should be exported with the name
    `platform_normal_stacks`. This pointer is used by the common platform helper
    functions `platform_set_stack()` (to allocate a stack for each CPU in the
    platform)  & `platform_get_stack()` (to return the base address of that
    stack) (see [../plat/common/aarch64/platform_helpers.S]).


2.2 Common optional modifications
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


### Function : platform_set_coherent_stack()

    Argument : unsigned long
    Return   : void

A platform may need stack memory that is coherent with main memory to perform
certain operations like:

*   Turning the MMU on, or
*   Flushing caches prior to powering down a CPU or cluster.

Each BL stage allocates this coherent stack memory for each CPU in the
`tzfw_coherent_mem` section. A pointer to this memory (`pcpu_dv_mem_stack`) is
used by this function to allocate a coherent stack for each CPU. A CPU is
identified by its `MPIDR`, which is passed as an argument to this function.

The size of the stack allocated to each CPU is specified by the constant
`PCPU_DV_MEM_STACK_SIZE`.


### Function : platform_is_primary_cpu()

    Argument : unsigned long
    Return   : unsigned int

This function identifies a CPU by its `MPIDR`, which is passed as the argument,
to determine whether this CPU is the primary CPU or a secondary CPU. A return
value of zero indicates that the CPU is not the primary CPU, while a non-zero
return value indicates that the CPU is the primary CPU.


### Function : platform_set_stack()

    Argument : unsigned long
    Return   : void

This function uses the `platform_normal_stacks` pointer variable to allocate
stacks to each CPU. Further details are given in the description of the
`platform_normal_stacks` variable below. A CPU is identified by its `MPIDR`,
which is passed as the argument.

The size of the stack allocated to each CPU is specified by the platform defined
constant `PLATFORM_STACK_SIZE`.


### Function : platform_get_stack()

    Argument : unsigned long
    Return   : unsigned long

This function uses the `platform_normal_stacks` pointer variable to return the
base address of the stack memory reserved for a CPU. Further details are given
in the description of the `platform_normal_stacks` variable below. A CPU is
identified by its `MPIDR`, which is passed as the argument.

The size of the stack allocated to each CPU is specified by the platform defined
constant `PLATFORM_STACK_SIZE`.


### Function : plat_report_exception()

    Argument : unsigned int
    Return   : void

A platform may need to report various information about its status when an
exception is taken, for example the current exception level, the CPU security
state (secure/non-secure), the exception type, and so on. This function is
called in the following circumstances:

*   In BL1, whenever an exception is taken.
*   In BL2, whenever an exception is taken.
*   In BL3-1, whenever an asynchronous exception or a synchronous exception
    other than an SMC32/SMC64 exception is taken.

The default implementation doesn't do anything, to avoid making assumptions
about the way the platform displays its status information.

This function receives the exception type as its argument. Possible values for
exceptions types are listed in the [../include/runtime_svc.h] header file. Note
that these constants are not related to any architectural exception code; they
are just an ARM Trusted Firmware convention.


3.  Modifications specific to a Boot Loader stage
-------------------------------------------------

3.1 Boot Loader Stage 1 (BL1)
-----------------------------

BL1 implements the reset vector where execution starts from after a cold or
warm boot. For each CPU, BL1 is responsible for the following tasks:

1.  Distinguishing between a cold boot and a warm boot.

2.  In the case of a cold boot and the CPU being the primary CPU, ensuring that
    only this CPU executes the remaining BL1 code, including loading and passing
    control to the BL2 stage.

3.  In the case of a cold boot and the CPU being a secondary CPU, ensuring that
    the CPU is placed in a platform-specific state until the primary CPU
    performs the necessary steps to remove it from this state.

4.  In the case of a warm boot, ensuring that the CPU jumps to a platform-
    specific address in the BL3-1 image in the same processor mode as it was
    when released from reset.

5.  Loading the BL2 image in secure memory using semi-hosting at the
    address specified by the platform defined constant `BL2_BASE`.

6.  Populating a `meminfo` structure with the following information in memory,
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

    BL1 implements the `init_bl2_mem_layout()` function to populate the
    BL2 `meminfo` structure. The platform may override this implementation, for
    example if the platform wants to restrict the amount of memory visible to
    BL2. Details of how to do this are given below.

The following functions need to be implemented by the platform port to enable
BL1 to perform the above tasks.


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

This function fulfills requirement 1 listed above.


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

This function fulfills requirement 3 above.


### Function : platform_cold_boot_init() [mandatory]

    Argument : unsigned long
    Return   : unsigned int

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The argument to this function is the address of the
`bl1_main()` routine where the generic BL1-specific actions are performed.
This function performs any platform-specific and architectural setup that the
platform requires to make execution of `bl1_main()` possible.

The platform must enable the MMU with identity mapped page tables and enable
caches by setting the `SCTLR.I` and `SCTLR.C` bits.

Platform-specific setup might include configuration of memory controllers,
configuration of the interconnect to allow the cluster to service cache snoop
requests from another cluster, zeroing of the ZI section, and so on.

In the ARM FVP port, this function enables CCI snoops into the cluster that the
primary CPU is part of. It also enables the MMU and initializes the ZI section
in the BL1 image through the use of linker defined symbols.

This function helps fulfill requirement 2 above.


### Function : bl1_platform_setup() [mandatory]

    Argument : void
    Return   : void

This function executes with the MMU and data caches enabled. It is responsible
for performing any remaining platform-specific setup that can occur after the
MMU and data cache have been enabled.

In the ARM FVP port, it zeros out the ZI section, enables the system level
implementation of the generic timer counter and initializes the console.

This function helps fulfill requirement 5 above.


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

This function helps fulfill requirement 5 above.


### Function : init_bl2_mem_layout() [optional]

    Argument : meminfo *, meminfo *, unsigned int, unsigned long
    Return   : void

Each BL stage needs to tell the next stage the amount of secure RAM available
for it to use. For example, as part of handing control to BL2, BL1 informs BL2
of the extents of secure RAM available for BL2 to use. BL2 must do the same when
passing control to BL3-1. This information is populated in a `meminfo`
structure.

Depending upon where BL2 has been loaded in secure RAM (determined by
`BL2_BASE`), BL1 calculates the amount of free memory available for BL2 to use.
BL1 also ensures that its data sections resident in secure RAM are not visible
to BL2. An illustration of how this is done in the ARM FVP port is given in the
[User Guide], in the Section "Memory layout on Base FVP".


3.2 Boot Loader Stage 2 (BL2)
-----------------------------

The BL2 stage is executed only by the primary CPU, which is determined in BL1
using the `platform_is_primary_cpu()` function. BL1 passed control to BL2 at
`BL2_BASE`. BL2 executes in Secure EL1 and is responsible for:

1.  Loading the BL3-1 binary image in secure RAM using semi-hosting. To load the
    BL3-1 image, BL2 makes use of the `meminfo` structure passed to it by BL1.
    This structure allows BL2 to calculate how much secure RAM is available for
    its use. The platform also defines the address in secure RAM where BL3-1 is
    loaded through the constant `BL31_BASE`. BL2 uses this information to
    determine if there is enough memory to load the BL3-1 image.

2.  Arranging to pass control to a normal world BL image that has been
    pre-loaded at a platform-specific address. This address is determined using
    the `plat_get_ns_image_entrypoint()` function described below.

    BL2 populates an `el_change_info` structure in memory provided by the
    platform with information about how BL3-1 should pass control to the normal
    world BL image.

3.  Populating a `meminfo` structure with the following information in
    memory that is accessible by BL3-1 immediately upon entry.

        meminfo.total_base = Base address of secure RAM visible to BL3-1
        meminfo.total_size = Size of secure RAM visible to BL3-1
        meminfo.free_base  = Base address of secure RAM available for allocation
                             to BL3-1
        meminfo.free_size  = Size of secure RAM available for allocation to
                             BL3-1

    BL2 places this `meminfo` structure in memory provided by the
    platform (`bl2_el_change_mem_ptr`). BL2 implements the
    `init_bl31_mem_layout()` function to populate the BL3-1 meminfo structure
    described above. The platform may override this implementation, for example
    if the platform wants to restrict the amount of memory visible to BL3-1.
    Details of this function are given below.

The following functions must be implemented by the platform port to enable BL2
to perform the above tasks.


### Function : bl2_early_platform_setup() [mandatory]

    Argument : meminfo *, void *
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function are:

*   The address of the `meminfo` structure populated by BL1
*   An opaque pointer that the platform may use as needed.

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

The purpose of this function is to perform any platform initialization specific
to BL2. This function must initialize a pointer to memory
(`bl2_el_change_mem_ptr`), which can then be used to populate an
`el_change_info` structure. The underlying requirement is that the platform must
initialize this pointer before the `get_el_change_mem_ptr()` function
accesses it in `bl2_main()`.

The ARM FVP port initializes this pointer to the base address of Secure DRAM
(`0x06000000`).


### Variable : unsigned char bl2_el_change_mem_ptr[EL_CHANGE_MEM_SIZE] [mandatory]

As mentioned in the description of `bl2_platform_setup()`, this pointer is
initialized by the platform to point to memory where an `el_change_info`
structure can be populated.


### Function : bl2_plat_sec_mem_layout() [mandatory]

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It may execute with
the MMU and data caches enabled if the platform port does the necessary
initialization in `bl2_plat_arch_setup()`. It is only called by the primary CPU.

The purpose of this function is to return a pointer to a `meminfo` structure
populated with the extents of secure RAM available for BL2 to use. See
`bl2_early_platform_setup()` above.


### Function : init_bl31_mem_layout() [optional]

    Argument : meminfo *, meminfo *, unsigned int
    Return   : void

Each BL stage needs to tell the next stage the amount of secure RAM that is
available for it to use. For example, as part of handing control to BL2, BL1
must inform BL2 about the extents of secure RAM that is available for BL2 to
use. BL2 must do the same when passing control to BL3-1. This information is
populated in a `meminfo` structure.

Depending upon where BL3-1 has been loaded in secure RAM (determined by
`BL31_BASE`), BL2 calculates the amount of free memory available for BL3-1 to
use. BL2 also ensures that BL3-1 is able reclaim memory occupied by BL2. This
is done because BL2 never executes again after passing control to BL3-1.
An illustration of how this is done in the ARM FVP port is given in the
[User Guide], in the section "Memory layout on Base FVP".


### Function : plat_get_ns_image_entrypoint() [mandatory]

    Argument : void
    Return   : unsigned long

As previously described, BL2 is responsible for arranging for control to be
passed to a normal world BL image through BL3-1. This function returns the
entrypoint of that image, which BL3-1 uses to jump to it.

The ARM FVP port assumes that flash memory has been pre-loaded with the UEFI
image, and so returns the base address of flash memory.


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
    specific address by BL2. BL3-1 uses the `el_change_info` structure that BL2
    populated in memory to do this.

3.  Providing runtime firmware services. Currently, BL3-1 only implements a
    subset of the Power State Coordination Interface (PSCI) API as a runtime
    service. See Section 3.3 below for details of porting the PSCI
    implementation.

The following functions must be implemented by the platform port to enable BL3-1
to perform the above tasks.


### Function : bl31_early_platform_setup() [mandatory]

    Argument : meminfo *, void *, unsigned long
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function are:

*   The address of the `meminfo` structure populated by BL2.
*   An opaque pointer that the platform may use as needed.
*   The `MPIDR` of the primary CPU.

The platform must copy the contents of the `meminfo` structure into a private
variable as the original memory may be subsequently overwritten by BL3-1. The
copied structure is made available to all BL3-1 code through the
`bl31_plat_sec_mem_layout()` function.


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
*   Grants access to the system counter timer module
*   Initializes the FVP power controller device
*   Detects the system topology.


### Function : bl31_get_next_image_info() [mandatory]

    Argument : unsigned long
    Return   : el_change_info *

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`.

This function is called by `bl31_main()` to retrieve information provided by
BL2, so that BL3-1 can pass control to the normal world software image. This
function must return a pointer to the `el_change_info` structure (that was
copied during `bl31_early_platform_setup()`).


### Function : bl31_plat_sec_mem_layout() [mandatory]

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. This function may
execute with the MMU and data caches enabled if the platform port does the
necessary initializations in `bl31_plat_arch_setup()`. It is only called by the
primary CPU.

The purpose of this function is to return a pointer to a `meminfo` structure
populated with the extents of secure RAM available for BL3-1 to use. See
`bl31_early_platform_setup()` above.


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

    Argument : plat_pm_ops **
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in `bl31_plat_arch_setup()`. It is only
called by the primary CPU.

This function is called by PSCI initialization code. Its purpose is to export
handler routines for platform-specific power management actions by populating
the passed pointer with a pointer to BL3-1's private `plat_pm_ops` structure.

A description of each member of this structure is given below. Please refer to
the ARM FVP specific implementation of these handlers in [../plat/fvp/plat_pm.c]
as an example. A platform port may choose not implement some of the power
management operations. For example, the ARM FVP port does not implement the
`affinst_standby()` function.

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

This function is called with coherent stacks. This allows the PSCI
implementation to flush caches at a given affinity level without running into
stale stack state after turning off the caches. On ARMv8-A cache hits do not
occur after the cache has been turned off.

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

This function is called with coherent stacks. This allows the PSCI
implementation to flush caches at a given affinity level without running into
stale stack state after turning off the caches. On ARMv8-A cache hits do not
occur after the cache has been turned off.

#### plat_pm_ops.affinst_on_finish()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an earlier PSCI `CPU_ON` call.
It performs the platform-specific setup required to initialize enough state for
this CPU to enter the normal world and also provide secure runtime firmware
services.

The `MPIDR` (first argument), `affinity level` (second argument) and `state`
(third argument) have a similar meaning as described in the previous operations.

This function is called with coherent stacks. This allows the PSCI
implementation to flush caches at a given affinity level without running into
stale stack state after turning off the caches. On ARMv8-A cache hits do not
occur after the cache has been turned off.

#### plat_pm_ops.affinst_on_suspend()

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an asynchronous wakeup
event, for example a timer interrupt that was programmed by the CPU during the
`CPU_SUSPEND` call. It performs the platform-specific setup required to
restore the saved state for this CPU to resume execution in the normal world
and also provide secure runtime firmware services.

The `MPIDR` (first argument), `affinity level` (second argument) and `state`
(third argument) have a similar meaning as described in the previous operations.

This function is called with coherent stacks. This allows the PSCI
implementation to flush caches at a given affinity level without running into
stale stack state after turning off the caches. On ARMv8-A cache hits do not
occur after the cache has been turned off.

BL3-1 platform initialization code must also detect the system topology and
the state of each affinity instance in the topology. This information is
critical for the PSCI runtime service to function correctly. More details are
provided in the description of the `plat_get_aff_count()` and
`plat_get_aff_state()` functions above.


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


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013, ARM Limited and Contributors. All rights reserved._


[User Guide]: user-guide.md
[FreeBSD]:    http://www.freebsd.org

[../plat/common/aarch64/platform_helpers.S]: ../plat/common/aarch64/platform_helpers.S
[../plat/fvp/platform.h]:                    ../plat/fvp/platform.h
[../plat/fvp/aarch64/plat_common.c]:          ../plat/fvp/aarch64/plat_common.c
[../plat/fvp/plat_pm.c]:                      ../plat/fvp/plat_pm.c
[../include/runtime_svc.h]:                  ../include/runtime_svc.h
