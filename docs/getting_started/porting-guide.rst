Porting Guide
=============

Introduction
------------

Porting Trusted Firmware-A (TF-A) to a new platform involves making some
mandatory and optional modifications for both the cold and warm boot paths.
Modifications consist of:

-  Implementing a platform-specific function or variable,
-  Setting up the execution context in a certain way, or
-  Defining certain constants (for example #defines).

The platform-specific functions and variables are declared in
``include/plat/common/platform.h``. The firmware provides a default
implementation of variables and functions to fulfill the optional requirements.
These implementations are all weakly defined; they are provided to ease the
porting effort. Each platform port can override them with its own implementation
if the default implementation is inadequate.

Some modifications are common to all Boot Loader (BL) stages. Section 2
discusses these in detail. The subsequent sections discuss the remaining
modifications for each BL stage in detail.

Please refer to the :ref:`Platform Compatibility Policy` for the policy
regarding compatibility and deprecation of these porting interfaces.

Only Arm development platforms (such as FVP and Juno) may use the
functions/definitions in ``include/plat/arm/common/`` and the corresponding
source files in ``plat/arm/common/``. This is done so that there are no
dependencies between platforms maintained by different people/companies. If you
want to use any of the functionality present in ``plat/arm`` files, please
create a pull request that moves the code to ``plat/common`` so that it can be
discussed.

Common modifications
--------------------

This section covers the modifications that should be made by the platform for
each BL stage to correctly port the firmware stack. They are categorized as
either mandatory or optional.

Common mandatory modifications
------------------------------

A platform port must enable the Memory Management Unit (MMU) as well as the
instruction and data caches for each BL stage. Setting up the translation
tables is the responsibility of the platform port because memory maps differ
across platforms. A memory translation library (see ``lib/xlat_tables/``) is
provided to help in this setup.

Note that although this library supports non-identity mappings, this is intended
only for re-mapping peripheral physical addresses and allows platforms with high
I/O addresses to reduce their virtual address space. All other addresses
corresponding to code and data must currently use an identity mapping.

Also, the only translation granule size supported in TF-A is 4KB, as various
parts of the code assume that is the case. It is not possible to switch to
16 KB or 64 KB granule sizes at the moment.

In Arm standard platforms, each BL stage configures the MMU in the
platform-specific architecture setup function, ``blX_plat_arch_setup()``, and uses
an identity mapping for all addresses.

If the build option ``USE_COHERENT_MEM`` is enabled, each platform can allocate a
block of identity mapped secure memory with Device-nGnRE attributes aligned to
page boundary (4K) for each BL stage. All sections which allocate coherent
memory are grouped under ``coherent_ram``. For ex: Bakery locks are placed in a
section identified by name ``bakery_lock`` inside ``coherent_ram`` so that its
possible for the firmware to place variables in it using the following C code
directive:

::

    __section("bakery_lock")

Or alternatively the following assembler code directive:

::

    .section bakery_lock

The ``coherent_ram`` section is a sum of all sections like ``bakery_lock`` which are
used to allocate any data structures that are accessed both when a CPU is
executing with its MMU and caches enabled, and when it's running with its MMU
and caches disabled. Examples are given below.

The following variables, functions and constants must be defined by the platform
for the firmware to work correctly.

File : platform_def.h [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each platform must ensure that a header file of this name is in the system
include path with the following constants defined. This will require updating
the list of ``PLAT_INCLUDES`` in the ``platform.mk`` file.

Platform ports may optionally use the file ``include/plat/common/common_def.h``,
which provides typical values for some of the constants below. These values are
likely to be suitable for all platform ports.

-  **#define : PLATFORM_LINKER_FORMAT**

   Defines the linker format used by the platform, for example
   ``elf64-littleaarch64``.

-  **#define : PLATFORM_LINKER_ARCH**

   Defines the processor architecture for the linker by the platform, for
   example ``aarch64``.

-  **#define : PLATFORM_STACK_SIZE**

   Defines the normal stack memory available to each CPU. This constant is used
   by ``plat/common/aarch64/platform_mp_stack.S`` and
   ``plat/common/aarch64/platform_up_stack.S``.

-  **define : CACHE_WRITEBACK_GRANULE**

   Defines the size in bits of the largest cache line across all the cache
   levels in the platform.

-  **#define : FIRMWARE_WELCOME_STR**

   Defines the character string printed by BL1 upon entry into the ``bl1_main()``
   function.

-  **#define : PLATFORM_CORE_COUNT**

   Defines the total number of CPUs implemented by the platform across all
   clusters in the system.

-  **#define : PLAT_NUM_PWR_DOMAINS**

   Defines the total number of nodes in the power domain topology
   tree at all the power domain levels used by the platform.
   This macro is used by the PSCI implementation to allocate
   data structures to represent power domain topology.

-  **#define : PLAT_MAX_PWR_LVL**

   Defines the maximum power domain level that the power management operations
   should apply to. More often, but not always, the power domain level
   corresponds to affinity level. This macro allows the PSCI implementation
   to know the highest power domain level that it should consider for power
   management operations in the system that the platform implements. For
   example, the Base AEM FVP implements two clusters with a configurable
   number of CPUs and it reports the maximum power domain level as 1.

-  **#define : PLAT_MAX_OFF_STATE**

   Defines the local power state corresponding to the deepest power down
   possible at every power domain level in the platform. The local power
   states for each level may be sparsely allocated between 0 and this value
   with 0 being reserved for the RUN state. The PSCI implementation uses this
   value to initialize the local power states of the power domain nodes and
   to specify the requested power state for a PSCI_CPU_OFF call.

-  **#define : PLAT_MAX_RET_STATE**

   Defines the local power state corresponding to the deepest retention state
   possible at every power domain level in the platform. This macro should be
   a value less than PLAT_MAX_OFF_STATE and greater than 0. It is used by the
   PSCI implementation to distinguish between retention and power down local
   power states within PSCI_CPU_SUSPEND call.

-  **#define : PLAT_MAX_PWR_LVL_STATES**

   Defines the maximum number of local power states per power domain level
   that the platform supports. The default value of this macro is 2 since
   most platforms just support a maximum of two local power states at each
   power domain level (power-down and retention). If the platform needs to
   account for more local power states, then it must redefine this macro.

   Currently, this macro is used by the Generic PSCI implementation to size
   the array used for PSCI_STAT_COUNT/RESIDENCY accounting.

-  **#define : BL1_RO_BASE**

   Defines the base address in secure ROM where BL1 originally lives. Must be
   aligned on a page-size boundary.

-  **#define : BL1_RO_LIMIT**

   Defines the maximum address in secure ROM that BL1's actual content (i.e.
   excluding any data section allocated at runtime) can occupy.

-  **#define : BL1_RW_BASE**

   Defines the base address in secure RAM where BL1's read-write data will live
   at runtime. Must be aligned on a page-size boundary.

-  **#define : BL1_RW_LIMIT**

   Defines the maximum address in secure RAM that BL1's read-write data can
   occupy at runtime.

-  **#define : BL2_BASE**

   Defines the base address in secure RAM where BL1 loads the BL2 binary image.
   Must be aligned on a page-size boundary. This constant is not applicable
   when BL2_IN_XIP_MEM is set to '1'.

-  **#define : BL2_LIMIT**

   Defines the maximum address in secure RAM that the BL2 image can occupy.
   This constant is not applicable when BL2_IN_XIP_MEM is set to '1'.

-  **#define : BL2_RO_BASE**

   Defines the base address in secure XIP memory where BL2 RO section originally
   lives. Must be aligned on a page-size boundary. This constant is only needed
   when BL2_IN_XIP_MEM is set to '1'.

-  **#define : BL2_RO_LIMIT**

   Defines the maximum address in secure XIP memory that BL2's actual content
   (i.e. excluding any data section allocated at runtime) can occupy. This
   constant is only needed when BL2_IN_XIP_MEM is set to '1'.

-  **#define : BL2_RW_BASE**

   Defines the base address in secure RAM where BL2's read-write data will live
   at runtime. Must be aligned on a page-size boundary. This constant is only
   needed when BL2_IN_XIP_MEM is set to '1'.

-  **#define : BL2_RW_LIMIT**

   Defines the maximum address in secure RAM that BL2's read-write data can
   occupy at runtime. This constant is only needed when BL2_IN_XIP_MEM is set
   to '1'.

-  **#define : BL31_BASE**

   Defines the base address in secure RAM where BL2 loads the BL31 binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL31_LIMIT**

   Defines the maximum address in secure RAM that the BL31 image can occupy.

For every image, the platform must define individual identifiers that will be
used by BL1 or BL2 to load the corresponding image into memory from non-volatile
storage. For the sake of performance, integer numbers will be used as
identifiers. The platform will use those identifiers to return the relevant
information about the image to be loaded (file handler, load address,
authentication information, etc.). The following image identifiers are
mandatory:

-  **#define : BL2_IMAGE_ID**

   BL2 image identifier, used by BL1 to load BL2.

-  **#define : BL31_IMAGE_ID**

   BL31 image identifier, used by BL2 to load BL31.

-  **#define : BL33_IMAGE_ID**

   BL33 image identifier, used by BL2 to load BL33.

If Trusted Board Boot is enabled, the following certificate identifiers must
also be defined:

-  **#define : TRUSTED_BOOT_FW_CERT_ID**

   BL2 content certificate identifier, used by BL1 to load the BL2 content
   certificate.

-  **#define : TRUSTED_KEY_CERT_ID**

   Trusted key certificate identifier, used by BL2 to load the trusted key
   certificate.

-  **#define : SOC_FW_KEY_CERT_ID**

   BL31 key certificate identifier, used by BL2 to load the BL31 key
   certificate.

-  **#define : SOC_FW_CONTENT_CERT_ID**

   BL31 content certificate identifier, used by BL2 to load the BL31 content
   certificate.

-  **#define : NON_TRUSTED_FW_KEY_CERT_ID**

   BL33 key certificate identifier, used by BL2 to load the BL33 key
   certificate.

-  **#define : NON_TRUSTED_FW_CONTENT_CERT_ID**

   BL33 content certificate identifier, used by BL2 to load the BL33 content
   certificate.

-  **#define : FWU_CERT_ID**

   Firmware Update (FWU) certificate identifier, used by NS_BL1U to load the
   FWU content certificate.

-  **#define : PLAT_CRYPTOCELL_BASE**

   This defines the base address of Arm® TrustZone® CryptoCell and must be
   defined if CryptoCell crypto driver is used for Trusted Board Boot. For
   capable Arm platforms, this driver is used if ``ARM_CRYPTOCELL_INTEG`` is
   set.

If the AP Firmware Updater Configuration image, BL2U is used, the following
must also be defined:

-  **#define : BL2U_BASE**

   Defines the base address in secure memory where BL1 copies the BL2U binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL2U_LIMIT**

   Defines the maximum address in secure memory that the BL2U image can occupy.

-  **#define : BL2U_IMAGE_ID**

   BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to BL2U.

If the SCP Firmware Update Configuration Image, SCP_BL2U is used, the following
must also be defined:

-  **#define : SCP_BL2U_IMAGE_ID**

   SCP_BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to SCP_BL2U.

   .. note::
      TF-A does not provide source code for this image.

If the Non-Secure Firmware Updater ROM, NS_BL1U is used, the following must
also be defined:

-  **#define : NS_BL1U_BASE**

   Defines the base address in non-secure ROM where NS_BL1U executes.
   Must be aligned on a page-size boundary.

   .. note::
      TF-A does not provide source code for this image.

-  **#define : NS_BL1U_IMAGE_ID**

   NS_BL1U image identifier, used by BL1 to fetch an image descriptor
   corresponding to NS_BL1U.

If the Non-Secure Firmware Updater, NS_BL2U is used, the following must also
be defined:

-  **#define : NS_BL2U_BASE**

   Defines the base address in non-secure memory where NS_BL2U executes.
   Must be aligned on a page-size boundary.

   .. note::
      TF-A does not provide source code for this image.

-  **#define : NS_BL2U_IMAGE_ID**

   NS_BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to NS_BL2U.

For the the Firmware update capability of TRUSTED BOARD BOOT, the following
macros may also be defined:

-  **#define : PLAT_FWU_MAX_SIMULTANEOUS_IMAGES**

   Total number of images that can be loaded simultaneously. If the platform
   doesn't specify any value, it defaults to 10.

If a SCP_BL2 image is supported by the platform, the following constants must
also be defined:

-  **#define : SCP_BL2_IMAGE_ID**

   SCP_BL2 image identifier, used by BL2 to load SCP_BL2 into secure memory
   from platform storage before being transferred to the SCP.

-  **#define : SCP_FW_KEY_CERT_ID**

   SCP_BL2 key certificate identifier, used by BL2 to load the SCP_BL2 key
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : SCP_FW_CONTENT_CERT_ID**

   SCP_BL2 content certificate identifier, used by BL2 to load the SCP_BL2
   content certificate (mandatory when Trusted Board Boot is enabled).

If a BL32 image is supported by the platform, the following constants must
also be defined:

-  **#define : BL32_IMAGE_ID**

   BL32 image identifier, used by BL2 to load BL32.

-  **#define : TRUSTED_OS_FW_KEY_CERT_ID**

   BL32 key certificate identifier, used by BL2 to load the BL32 key
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : TRUSTED_OS_FW_CONTENT_CERT_ID**

   BL32 content certificate identifier, used by BL2 to load the BL32 content
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : BL32_BASE**

   Defines the base address in secure memory where BL2 loads the BL32 binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL32_LIMIT**

   Defines the maximum address that the BL32 image can occupy.

If the Test Secure-EL1 Payload (TSP) instantiation of BL32 is supported by the
platform, the following constants must also be defined:

-  **#define : TSP_SEC_MEM_BASE**

   Defines the base address of the secure memory used by the TSP image on the
   platform. This must be at the same address or below ``BL32_BASE``.

-  **#define : TSP_SEC_MEM_SIZE**

   Defines the size of the secure memory used by the BL32 image on the
   platform. ``TSP_SEC_MEM_BASE`` and ``TSP_SEC_MEM_SIZE`` must fully
   accommodate the memory required by the BL32 image, defined by ``BL32_BASE``
   and ``BL32_LIMIT``.

-  **#define : TSP_IRQ_SEC_PHY_TIMER**

   Defines the ID of the secure physical generic timer interrupt used by the
   TSP's interrupt handling code.

If the platform port uses the translation table library code, the following
constants must also be defined:

-  **#define : PLAT_XLAT_TABLES_DYNAMIC**

   Optional flag that can be set per-image to enable the dynamic allocation of
   regions even when the MMU is enabled. If not defined, only static
   functionality will be available, if defined and set to 1 it will also
   include the dynamic functionality.

-  **#define : MAX_XLAT_TABLES**

   Defines the maximum number of translation tables that are allocated by the
   translation table library code. To minimize the amount of runtime memory
   used, choose the smallest value needed to map the required virtual addresses
   for each BL stage. If ``PLAT_XLAT_TABLES_DYNAMIC`` flag is enabled for a BL
   image, ``MAX_XLAT_TABLES`` must be defined to accommodate the dynamic regions
   as well.

-  **#define : MAX_MMAP_REGIONS**

   Defines the maximum number of regions that are allocated by the translation
   table library code. A region consists of physical base address, virtual base
   address, size and attributes (Device/Memory, RO/RW, Secure/Non-Secure), as
   defined in the ``mmap_region_t`` structure. The platform defines the regions
   that should be mapped. Then, the translation table library will create the
   corresponding tables and descriptors at runtime. To minimize the amount of
   runtime memory used, choose the smallest value needed to register the
   required regions for each BL stage. If ``PLAT_XLAT_TABLES_DYNAMIC`` flag is
   enabled for a BL image, ``MAX_MMAP_REGIONS`` must be defined to accommodate
   the dynamic regions as well.

-  **#define : PLAT_VIRT_ADDR_SPACE_SIZE**

   Defines the total size of the virtual address space in bytes. For example,
   for a 32 bit virtual address space, this value should be ``(1ULL << 32)``.

-  **#define : PLAT_PHY_ADDR_SPACE_SIZE**

   Defines the total size of the physical address space in bytes. For example,
   for a 32 bit physical address space, this value should be ``(1ULL << 32)``.

If the platform port uses the IO storage framework, the following constants
must also be defined:

-  **#define : MAX_IO_DEVICES**

   Defines the maximum number of registered IO devices. Attempting to register
   more devices than this value using ``io_register_device()`` will fail with
   -ENOMEM.

-  **#define : MAX_IO_HANDLES**

   Defines the maximum number of open IO handles. Attempting to open more IO
   entities than this value using ``io_open()`` will fail with -ENOMEM.

-  **#define : MAX_IO_BLOCK_DEVICES**

   Defines the maximum number of registered IO block devices. Attempting to
   register more devices this value using ``io_dev_open()`` will fail
   with -ENOMEM. MAX_IO_BLOCK_DEVICES should be less than MAX_IO_DEVICES.
   With this macro, multiple block devices could be supported at the same
   time.

If the platform needs to allocate data within the per-cpu data framework in
BL31, it should define the following macro. Currently this is only required if
the platform decides not to use the coherent memory section by undefining the
``USE_COHERENT_MEM`` build flag. In this case, the framework allocates the
required memory within the the per-cpu data to minimize wastage.

-  **#define : PLAT_PCPU_DATA_SIZE**

   Defines the memory (in bytes) to be reserved within the per-cpu data
   structure for use by the platform layer.

The following constants are optional. They should be defined when the platform
memory layout implies some image overlaying like in Arm standard platforms.

-  **#define : BL31_PROGBITS_LIMIT**

   Defines the maximum address in secure RAM that the BL31's progbits sections
   can occupy.

-  **#define : TSP_PROGBITS_LIMIT**

   Defines the maximum address that the TSP's progbits sections can occupy.

If the platform port uses the PL061 GPIO driver, the following constant may
optionally be defined:

-  **PLAT_PL061_MAX_GPIOS**
   Maximum number of GPIOs required by the platform. This allows control how
   much memory is allocated for PL061 GPIO controllers. The default value is

   #. $(eval $(call add_define,PLAT_PL061_MAX_GPIOS))

If the platform port uses the partition driver, the following constant may
optionally be defined:

-  **PLAT_PARTITION_MAX_ENTRIES**
   Maximum number of partition entries required by the platform. This allows
   control how much memory is allocated for partition entries. The default
   value is 128.
   For example, define the build flag in ``platform.mk``:
   PLAT_PARTITION_MAX_ENTRIES := 12
   $(eval $(call add_define,PLAT_PARTITION_MAX_ENTRIES))

-  **PLAT_PARTITION_BLOCK_SIZE**
   The size of partition block. It could be either 512 bytes or 4096 bytes.
   The default value is 512.
   For example, define the build flag in ``platform.mk``:
   PLAT_PARTITION_BLOCK_SIZE := 4096
   $(eval $(call add_define,PLAT_PARTITION_BLOCK_SIZE))

The following constant is optional. It should be defined to override the default
behaviour of the ``assert()`` function (for example, to save memory).

-  **PLAT_LOG_LEVEL_ASSERT**
   If ``PLAT_LOG_LEVEL_ASSERT`` is higher or equal than ``LOG_LEVEL_VERBOSE``,
   ``assert()`` prints the name of the file, the line number and the asserted
   expression. Else if it is higher than ``LOG_LEVEL_INFO``, it prints the file
   name and the line number. Else if it is lower than ``LOG_LEVEL_INFO``, it
   doesn't print anything to the console. If ``PLAT_LOG_LEVEL_ASSERT`` isn't
   defined, it defaults to ``LOG_LEVEL``.

If the platform port uses the Activity Monitor Unit, the following constants
may be defined:

-  **PLAT_AMU_GROUP1_COUNTERS_MASK**
   This mask reflects the set of group counters that should be enabled.  The
   maximum number of group 1 counters supported by AMUv1 is 16 so the mask
   can be at most 0xffff. If the platform does not define this mask, no group 1
   counters are enabled. If the platform defines this mask, the following
   constant needs to also be defined.

-  **PLAT_AMU_GROUP1_NR_COUNTERS**
   This value is used to allocate an array to save and restore the counters
   specified by ``PLAT_AMU_GROUP1_COUNTERS_MASK`` on CPU suspend.
   This value should be equal to the highest bit position set in the
   mask, plus 1.  The maximum number of group 1 counters in AMUv1 is 16.

File : plat_macros.S [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each platform must ensure a file of this name is in the system include path with
the following macro defined. In the Arm development platforms, this file is
found in ``plat/arm/board/<plat_name>/include/plat_macros.S``.

-  **Macro : plat_crash_print_regs**

   This macro allows the crash reporting routine to print relevant platform
   registers in case of an unhandled exception in BL31. This aids in debugging
   and this macro can be defined to be empty in case register reporting is not
   desired.

   For instance, GIC or interconnect registers may be helpful for
   troubleshooting.

Handling Reset
--------------

BL1 by default implements the reset vector where execution starts from a cold
or warm boot. BL31 can be optionally set as a reset vector using the
``RESET_TO_BL31`` make variable.

For each CPU, the reset vector code is responsible for the following tasks:

#. Distinguishing between a cold boot and a warm boot.

#. In the case of a cold boot and the CPU being a secondary CPU, ensuring that
   the CPU is placed in a platform-specific state until the primary CPU
   performs the necessary steps to remove it from this state.

#. In the case of a warm boot, ensuring that the CPU jumps to a platform-
   specific address in the BL31 image in the same processor mode as it was
   when released from reset.

The following functions need to be implemented by the platform port to enable
reset vector code to perform the above tasks.

Function : plat_get_my_entrypoint() [mandatory when PROGRAMMABLE_RESET_ADDRESS == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uintptr_t

This function is called with the MMU and caches disabled
(``SCTLR_EL3.M`` = 0 and ``SCTLR_EL3.C`` = 0). The function is responsible for
distinguishing between a warm and cold reset for the current CPU using
platform-specific means. If it's a warm reset, then it returns the warm
reset entrypoint point provided to ``plat_setup_psci_ops()`` during
BL31 initialization. If it's a cold reset then this function must return zero.

This function does not follow the Procedure Call Standard used by the
Application Binary Interface for the Arm 64-bit architecture. The caller should
not assume that callee saved registers are preserved across a call to this
function.

This function fulfills requirement 1 and 3 listed above.

Note that for platforms that support programming the reset address, it is
expected that a CPU will start executing code directly at the right address,
both on a cold and warm reset. In this case, there is no need to identify the
type of reset nor to query the warm reset entrypoint. Therefore, implementing
this function is not required on such platforms.

Function : plat_secondary_cold_boot_setup() [mandatory when COLD_BOOT_SINGLE_CPU == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void

This function is called with the MMU and data caches disabled. It is responsible
for placing the executing secondary CPU in a platform-specific state until the
primary CPU performs the necessary actions to bring it out of that state and
allow entry into the OS. This function must not return.

In the Arm FVP port, when using the normal boot flow, each secondary CPU powers
itself off. The primary CPU is responsible for powering up the secondary CPUs
when normal world software requires them. When booting an EL3 payload instead,
they stay powered on and are put in a holding pen until their mailbox gets
populated.

This function fulfills requirement 2 above.

Note that for platforms that can't release secondary CPUs out of reset, only the
primary CPU will execute the cold boot code. Therefore, implementing this
function is not required on such platforms.

Function : plat_is_my_cpu_primary() [mandatory when COLD_BOOT_SINGLE_CPU == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

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

Function : platform_mem_init() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function is called before any access to data is made by the firmware, in
order to carry out any essential memory initialization.

Function: plat_get_rotpk_info()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, void **, unsigned int *, unsigned int *
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It returns a
pointer to the ROTPK stored in the platform (or a hash of it) and its length.
The ROTPK must be encoded in DER format according to the following ASN.1
structure:

::

    AlgorithmIdentifier  ::=  SEQUENCE  {
        algorithm         OBJECT IDENTIFIER,
        parameters        ANY DEFINED BY algorithm OPTIONAL
    }

    SubjectPublicKeyInfo  ::=  SEQUENCE  {
        algorithm         AlgorithmIdentifier,
        subjectPublicKey  BIT STRING
    }

In case the function returns a hash of the key:

::

    DigestInfo ::= SEQUENCE {
        digestAlgorithm   AlgorithmIdentifier,
        digest            OCTET STRING
    }

The function returns 0 on success. Any other value is treated as error by the
Trusted Board Boot. The function also reports extra information related
to the ROTPK in the flags parameter:

::

    ROTPK_IS_HASH      : Indicates that the ROTPK returned by the platform is a
                         hash.
    ROTPK_NOT_DEPLOYED : This allows the platform to skip certificate ROTPK
                         verification while the platform ROTPK is not deployed.
                         When this flag is set, the function does not need to
                         return a platform ROTPK, and the authentication
                         framework uses the ROTPK in the certificate without
                         verifying it against the platform value. This flag
                         must not be used in a deployed production environment.

Function: plat_get_nv_ctr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, unsigned int *
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It returns the
non-volatile counter value stored in the platform in the second argument. The
cookie in the first argument may be used to select the counter in case the
platform provides more than one (for example, on platforms that use the default
TBBR CoT, the cookie will correspond to the OID values defined in
TRUSTED_FW_NVCOUNTER_OID or NON_TRUSTED_FW_NVCOUNTER_OID).

The function returns 0 on success. Any other value means the counter value could
not be retrieved from the platform.

Function: plat_set_nv_ctr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, unsigned int
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It sets a new
counter value in the platform. The cookie in the first argument may be used to
select the counter (as explained in plat_get_nv_ctr()). The second argument is
the updated counter value to be written to the NV counter.

The function returns 0 on success. Any other value means the counter value could
not be updated.

Function: plat_set_nv_ctr2()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, const auth_img_desc_t *, unsigned int
    Return   : int

This function is optional when Trusted Board Boot is enabled. If this
interface is defined, then ``plat_set_nv_ctr()`` need not be defined. The
first argument passed is a cookie and is typically used to
differentiate between a Non Trusted NV Counter and a Trusted NV
Counter. The second argument is a pointer to an authentication image
descriptor and may be used to decide if the counter is allowed to be
updated or not. The third argument is the updated counter value to
be written to the NV counter.

The function returns 0 on success. Any other value means the counter value
either could not be updated or the authentication image descriptor indicates
that it is not allowed to be updated.

Common mandatory function modifications
---------------------------------------

The following functions are mandatory functions which need to be implemented
by the platform port.

Function : plat_my_core_pos()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This function returns the index of the calling CPU which is used as a
CPU-specific linear index into blocks of memory (for example while allocating
per-CPU stacks). This function will be invoked very early in the
initialization sequence which mandates that this function should be
implemented in assembly and should not rely on the availability of a C
runtime environment. This function can clobber x0 - x8 and must preserve
x9 - x29.

This function plays a crucial role in the power domain topology framework in
PSCI and details of this can be found in
:ref:`PSCI Power Domain Tree Structure`.

Function : plat_core_pos_by_mpidr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : u_register_t
    Return   : int

This function validates the ``MPIDR`` of a CPU and converts it to an index,
which can be used as a CPU-specific linear index into blocks of memory. In
case the ``MPIDR`` is invalid, this function returns -1. This function will only
be invoked by BL31 after the power domain topology is initialized and can
utilize the C runtime environment. For further details about how TF-A
represents the power domain topology and how this relates to the linear CPU
index, please refer :ref:`PSCI Power Domain Tree Structure`.

Function : plat_get_mbedtls_heap() [when TRUSTED_BOARD_BOOT == 1]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Arguments : void **heap_addr, size_t *heap_size
    Return    : int

This function is invoked during Mbed TLS library initialisation to get a heap,
by means of a starting address and a size. This heap will then be used
internally by the Mbed TLS library. Hence, each BL stage that utilises Mbed TLS
must be able to provide a heap to it.

A helper function can be found in `drivers/auth/mbedtls/mbedtls_common.c` in
which a heap is statically reserved during compile time inside every image
(i.e. every BL stage) that utilises Mbed TLS. In this default implementation,
the function simply returns the address and size of this "pre-allocated" heap.
For a platform to use this default implementation, only a call to the helper
from inside plat_get_mbedtls_heap() body is enough and nothing else is needed.

However, by writting their own implementation, platforms have the potential to
optimise memory usage. For example, on some Arm platforms, the Mbed TLS heap is
shared between BL1 and BL2 stages and, thus, the necessary space is not reserved
twice.

On success the function should return 0 and a negative error code otherwise.

Common optional modifications
-----------------------------

The following are helper functions implemented by the firmware that perform
common platform-specific tasks. A platform may choose to override these
definitions.

Function : plat_set_my_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function sets the current stack pointer to the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant ``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in ``plat/common/aarch64/platform_up_stack.S`` and
``plat/common/aarch64/platform_mp_stack.S``

Function : plat_get_my_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uintptr_t

This function returns the base address of the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant ``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in ``plat/common/aarch64/platform_up_stack.S`` and
``plat/common/aarch64/platform_mp_stack.S``

Function : plat_report_exception()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

A platform may need to report various information about its status when an
exception is taken, for example the current exception level, the CPU security
state (secure/non-secure), the exception type, and so on. This function is
called in the following circumstances:

-  In BL1, whenever an exception is taken.
-  In BL2, whenever an exception is taken.

The default implementation doesn't do anything, to avoid making assumptions
about the way the platform displays its status information.

For AArch64, this function receives the exception type as its argument.
Possible values for exceptions types are listed in the
``include/common/bl_common.h`` header file. Note that these constants are not
related to any architectural exception code; they are just a TF-A convention.

For AArch32, this function receives the exception mode as its argument.
Possible values for exception modes are listed in the
``include/lib/aarch32/arch.h`` header file.

Function : plat_reset_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

A platform may need to do additional initialization after reset. This function
allows the platform to do the platform specific initializations. Platform
specific errata workarounds could also be implemented here. The API should
preserve the values of callee saved registers x19 to x29.

The default implementation doesn't do anything. If a platform needs to override
the default implementation, refer to the :ref:`Firmware Design` for general
guidelines.

Function : plat_disable_acp()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This API allows a platform to disable the Accelerator Coherency Port (if
present) during a cluster power down sequence. The default weak implementation
doesn't do anything. Since this API is called during the power down sequence,
it has restrictions for stack usage and it can use the registers x0 - x17 as
scratch registers. It should preserve the value in x18 register as it is used
by the caller to store the return address.

Function : plat_error_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : void

This API is called when the generic code encounters an error situation from
which it cannot continue. It allows the platform to perform error reporting or
recovery actions (for example, reset the system). This function must not return.

The parameter indicates the type of error using standard codes from ``errno.h``.
Possible errors reported by the generic code are:

-  ``-EAUTH``: a certificate or image could not be authenticated (when Trusted
   Board Boot is enabled)
-  ``-ENOENT``: the requested image or certificate could not be found or an IO
   error was detected
-  ``-ENOMEM``: resources exhausted. TF-A does not use dynamic memory, so this
   error is usually an indication of an incorrect array size

The default implementation simply spins.

Function : plat_panic_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This API is called when the generic code encounters an unexpected error
situation from which it cannot recover. This function must not return,
and must be implemented in assembly because it may be called before the C
environment is initialized.

.. note::
   The address from where it was called is stored in x30 (Link Register).
   The default implementation simply spins.

Function : plat_get_bl_image_load_info()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : bl_load_info_t *

This function returns pointer to the list of images that the platform has
populated to load. This function is invoked in BL2 to load the
BL3xx images.

Function : plat_get_next_bl_params()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : bl_params_t *

This function returns a pointer to the shared memory that the platform has
kept aside to pass TF-A related information that next BL image needs. This
function is invoked in BL2 to pass this information to the next BL
image.

Function : plat_get_stack_protector_canary()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : u_register_t

This function returns a random value that is used to initialize the canary used
when the stack protector is enabled with ENABLE_STACK_PROTECTOR. A predictable
value will weaken the protection as the attacker could easily write the right
value as part of the attack most of the time. Therefore, it should return a
true random number.

.. warning::
   For the protection to be effective, the global data need to be placed at
   a lower address than the stack bases. Failure to do so would allow an
   attacker to overwrite the canary as part of the stack buffer overflow attack.

Function : plat_flush_next_bl_params()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function flushes to main memory all the image params that are passed to
next image. This function is invoked in BL2 to flush this information
to the next BL image.

Function : plat_log_get_prefix()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : const char *

This function defines the prefix string corresponding to the `log_level` to be
prepended to all the log output from TF-A. The `log_level` (argument) will
correspond to one of the standard log levels defined in debug.h. The platform
can override the common implementation to define a different prefix string for
the log output. The implementation should be robust to future changes that
increase the number of log levels.

Modifications specific to a Boot Loader stage
---------------------------------------------

Boot Loader Stage 1 (BL1)
-------------------------

BL1 implements the reset vector where execution starts from after a cold or
warm boot. For each CPU, BL1 is responsible for the following tasks:

#. Handling the reset as described in section 2.2

#. In the case of a cold boot and the CPU being the primary CPU, ensuring that
   only this CPU executes the remaining BL1 code, including loading and passing
   control to the BL2 stage.

#. Identifying and starting the Firmware Update process (if required).

#. Loading the BL2 image from non-volatile storage into secure memory at the
   address specified by the platform defined constant ``BL2_BASE``.

#. Populating a ``meminfo`` structure with the following information in memory,
   accessible by BL2 immediately upon entry.

   ::

       meminfo.total_base = Base address of secure RAM visible to BL2
       meminfo.total_size = Size of secure RAM visible to BL2

   By default, BL1 places this ``meminfo`` structure at the end of secure
   memory visible to BL2.

   It is possible for the platform to decide where it wants to place the
   ``meminfo`` structure for BL2 or restrict the amount of memory visible to
   BL2 by overriding the weak default implementation of
   ``bl1_plat_handle_post_image_load`` API.

The following functions need to be implemented by the platform port to enable
BL1 to perform the above tasks.

Function : bl1_early_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

On Arm standard platforms, this function:

-  Enables a secure instance of SP805 to act as the Trusted Watchdog.

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL1.

-  Enables issuing of snoop and DVM (Distributed Virtual Memory) requests to
   the CCI slave interface corresponding to the cluster that includes the
   primary CPU.

Function : bl1_plat_arch_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function performs any platform-specific and architectural setup that the
platform requires. Platform-specific setup might include configuration of
memory controllers and the interconnect.

In Arm standard platforms, this function enables the MMU.

This function helps fulfill requirement 2 above.

Function : bl1_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches enabled. It is responsible
for performing any remaining platform-specific setup that can occur after the
MMU and data cache have been enabled.

if support for multiple boot sources is required, it initializes the boot
sequence used by plat_try_next_boot_source().

In Arm standard platforms, this function initializes the storage abstraction
layer used to load the next bootloader image.

This function helps fulfill requirement 4 above.

Function : bl1_plat_sec_mem_layout() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It executes with the
MMU and data caches enabled. The pointer returned by this function must point to
a ``meminfo`` structure containing the extents and availability of secure RAM for
the BL1 stage.

::

    meminfo.total_base = Base address of secure RAM visible to BL1
    meminfo.total_size = Size of secure RAM visible to BL1

This information is used by BL1 to load the BL2 image in secure RAM. BL1 also
populates a similar structure to tell BL2 the extents of memory available for
its own use.

This function helps fulfill requirements 4 and 5 above.

Function : bl1_plat_prepare_exit() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : entry_point_info_t *
    Return   : void

This function is called prior to exiting BL1 in response to the
``BL1_SMC_RUN_IMAGE`` SMC request raised by BL2. It should be used to perform
platform specific clean up or bookkeeping operations before transferring
control to the next image. It receives the address of the ``entry_point_info_t``
structure passed from BL2. This function runs with MMU disabled.

Function : bl1_plat_set_ep_info() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id, entry_point_info_t *ep_info
    Return   : void

This function allows platforms to override ``ep_info`` for the given ``image_id``.

The default implementation just returns.

Function : bl1_plat_get_next_image_id() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This and the following function must be overridden to enable the FWU feature.

BL1 calls this function after platform setup to identify the next image to be
loaded and executed. If the platform returns ``BL2_IMAGE_ID`` then BL1 proceeds
with the normal boot sequence, which loads and executes BL2. If the platform
returns a different image id, BL1 assumes that Firmware Update is required.

The default implementation always returns ``BL2_IMAGE_ID``. The Arm development
platforms override this function to detect if firmware update is required, and
if so, return the first image in the firmware update process.

Function : bl1_plat_get_image_desc() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id
    Return   : image_desc_t *

BL1 calls this function to get the image descriptor information ``image_desc_t``
for the provided ``image_id`` from the platform.

The default implementation always returns a common BL2 image descriptor. Arm
standard platforms return an image descriptor corresponding to BL2 or one of
the firmware update images defined in the Trusted Board Boot Requirements
specification.

Function : bl1_plat_handle_pre_image_load() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id
    Return   : int

This function can be used by the platforms to update/use image information
corresponding to ``image_id``. This function is invoked in BL1, both in cold
boot and FWU code path, before loading the image.

Function : bl1_plat_handle_post_image_load() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id
    Return   : int

This function can be used by the platforms to update/use image information
corresponding to ``image_id``. This function is invoked in BL1, both in cold
boot and FWU code path, after loading and authenticating the image.

The default weak implementation of this function calculates the amount of
Trusted SRAM that can be used by BL2 and allocates a ``meminfo_t``
structure at the beginning of this free memory and populates it. The address
of ``meminfo_t`` structure is updated in ``arg1`` of the entrypoint
information to BL2.

Function : bl1_plat_fwu_done() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id, uintptr_t image_src,
               unsigned int image_size
    Return   : void

BL1 calls this function when the FWU process is complete. It must not return.
The platform may override this function to take platform specific action, for
example to initiate the normal boot flow.

The default implementation spins forever.

Function : bl1_plat_mem_check() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uintptr_t mem_base, unsigned int mem_size,
               unsigned int flags
    Return   : int

BL1 calls this function while handling FWU related SMCs, more specifically when
copying or authenticating an image. Its responsibility is to ensure that the
region of memory identified by ``mem_base`` and ``mem_size`` is mapped in BL1, and
that this memory corresponds to either a secure or non-secure memory region as
indicated by the security state of the ``flags`` argument.

This function can safely assume that the value resulting from the addition of
``mem_base`` and ``mem_size`` fits into a ``uintptr_t`` type variable and does not
overflow.

This function must return 0 on success, a non-null error code otherwise.

The default implementation of this function asserts therefore platforms must
override it when using the FWU feature.

Boot Loader Stage 2 (BL2)
-------------------------

The BL2 stage is executed only by the primary CPU, which is determined in BL1
using the ``platform_is_primary_cpu()`` function. BL1 passed control to BL2 at
``BL2_BASE``. BL2 executes in Secure EL1 and and invokes
``plat_get_bl_image_load_info()`` to retrieve the list of images to load from
non-volatile storage to secure/non-secure RAM. After all the images are loaded
then BL2 invokes ``plat_get_next_bl_params()`` to get the list of executable
images to be passed to the next BL image.

The following functions must be implemented by the platform port to enable BL2
to perform the above tasks.

Function : bl2_early_platform_setup2() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : u_register_t, u_register_t, u_register_t, u_register_t
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The 4 arguments are passed by BL1 to BL2 and these arguments
are platform specific.

On Arm standard platforms, the arguments received are :

    arg0 - Points to load address of HW_CONFIG if present

    arg1 - ``meminfo`` structure populated by BL1. The platform copies
    the contents of ``meminfo`` as it may be subsequently overwritten by BL2.

On Arm standard platforms, this function also:

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL2.

-  Initializes the storage abstraction layer used to load further bootloader
   images. It is necessary to do this early on platforms with a SCP_BL2 image,
   since the later ``bl2_platform_setup`` must be done after SCP_BL2 is loaded.

Function : bl2_plat_arch_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On Arm standard platforms, this function enables the MMU.

Function : bl2_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl2_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2.

In Arm standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.

Function : bl2_plat_handle_pre_image_load() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This function can be used by the platforms to update/use image information
for given ``image_id``. This function is currently invoked in BL2 before
loading each image.

Function : bl2_plat_handle_post_image_load() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This function can be used by the platforms to update/use image information
for given ``image_id``. This function is currently invoked in BL2 after
loading each image.

Function : bl2_plat_preload_setup [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This optional function performs any BL2 platform initialization
required before image loading, that is not done later in
bl2_platform_setup(). Specifically, if support for multiple
boot sources is required, it initializes the boot sequence used by
plat_try_next_boot_source().

Function : plat_try_next_boot_source() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This optional function passes to the next boot source in the redundancy
sequence.

This function moves the current boot redundancy source to the next
element in the boot sequence. If there are no more boot sources then it
must return 0, otherwise it must return 1. The default implementation
of this always returns 0.

Boot Loader Stage 2 (BL2) at EL3
--------------------------------

When the platform has a non-TF-A Boot ROM it is desirable to jump
directly to BL2 instead of TF-A BL1. In this case BL2 is expected to
execute at EL3 instead of executing at EL1. Refer to the :ref:`Firmware Design`
document for more information.

All mandatory functions of BL2 must be implemented, except the functions
bl2_early_platform_setup and bl2_el3_plat_arch_setup, because
their work is done now by bl2_el3_early_platform_setup and
bl2_el3_plat_arch_setup. These functions should generally implement
the bl1_plat_xxx() and bl2_plat_xxx() functionality combined.


Function : bl2_el3_early_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

	Argument : u_register_t, u_register_t, u_register_t, u_register_t
	Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. This function receives four parameters which can be used
by the platform to pass any needed information from the Boot ROM to BL2.

On Arm standard platforms, this function does the following:

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL2.

-  Initializes the storage abstraction layer used to load further bootloader
   images. It is necessary to do this early on platforms with a SCP_BL2 image,
   since the later ``bl2_platform_setup`` must be done after SCP_BL2 is loaded.

- Initializes the private variables that define the memory layout used.

Function : bl2_el3_plat_arch_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

	Argument : void
	Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On Arm standard platforms, this function enables the MMU.

Function : bl2_el3_plat_prepare_exit() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

	Argument : void
	Return   : void

This function is called prior to exiting BL2 and run the next image.
It should be used to perform platform specific clean up or bookkeeping
operations before transferring control to the next image. This function
runs with MMU disabled.

FWU Boot Loader Stage 2 (BL2U)
------------------------------

The AP Firmware Updater Configuration, BL2U, is an optional part of the FWU
process and is executed only by the primary CPU. BL1 passes control to BL2U at
``BL2U_BASE``. BL2U executes in Secure-EL1 and is responsible for:

#. (Optional) Transferring the optional SCP_BL2U binary image from AP secure
   memory to SCP RAM. BL2U uses the SCP_BL2U ``image_info`` passed by BL1.
   ``SCP_BL2U_BASE`` defines the address in AP secure memory where SCP_BL2U
   should be copied from. Subsequent handling of the SCP_BL2U image is
   implemented by the platform specific ``bl2u_plat_handle_scp_bl2u()`` function.
   If ``SCP_BL2U_BASE`` is not defined then this step is not performed.

#. Any platform specific setup required to perform the FWU process. For
   example, Arm standard platforms initialize the TZC controller so that the
   normal world can access DDR memory.

The following functions must be implemented by the platform port to enable
BL2U to perform the tasks mentioned above.

Function : bl2u_early_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *mem_info, void *plat_info
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU. The arguments to this function is the address
of the ``meminfo`` structure and platform specific info provided by BL1.

The platform may copy the contents of the ``mem_info`` and ``plat_info`` into
private storage as the original memory may be subsequently overwritten by BL2U.

On Arm CSS platforms ``plat_info`` is interpreted as an ``image_info_t`` structure,
to extract SCP_BL2U image information, which is then copied into a private
variable.

Function : bl2u_plat_arch_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms, for example enabling the MMU (since the memory
map differs across platforms).

Function : bl2u_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl2u_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2U.

In Arm standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.

Function : bl2u_plat_handle_scp_bl2u() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This function is used to perform any platform-specific actions required to
handle the SCP firmware. Typically it transfers the image into SCP memory using
a platform-specific protocol and waits until SCP executes it and signals to the
Application Processor (AP) for BL2U execution to continue.

This function returns 0 on success, a negative error code otherwise.
This function is included if SCP_BL2U_BASE is defined.

Boot Loader Stage 3-1 (BL31)
----------------------------

During cold boot, the BL31 stage is executed only by the primary CPU. This is
determined in BL1 using the ``platform_is_primary_cpu()`` function. BL1 passes
control to BL31 at ``BL31_BASE``. During warm boot, BL31 is executed by all
CPUs. BL31 executes at EL3 and is responsible for:

#. Re-initializing all architectural and platform state. Although BL1 performs
   some of this initialization, BL31 remains resident in EL3 and must ensure
   that EL3 architectural and platform state is completely initialized. It
   should make no assumptions about the system state when it receives control.

#. Passing control to a normal world BL image, pre-loaded at a platform-
   specific address by BL2. On ARM platforms, BL31 uses the ``bl_params`` list
   populated by BL2 in memory to do this.

#. Providing runtime firmware services. Currently, BL31 only implements a
   subset of the Power State Coordination Interface (PSCI) API as a runtime
   service. See Section 3.3 below for details of porting the PSCI
   implementation.

#. Optionally passing control to the BL32 image, pre-loaded at a platform-
   specific address by BL2. BL31 exports a set of APIs that allow runtime
   services to specify the security state in which the next image should be
   executed and run the corresponding image. On ARM platforms, BL31 uses the
   ``bl_params`` list populated by BL2 in memory to do this.

If BL31 is a reset vector, It also needs to handle the reset as specified in
section 2.2 before the tasks described above.

The following functions must be implemented by the platform port to enable BL31
to perform the above tasks.

Function : bl31_early_platform_setup2() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : u_register_t, u_register_t, u_register_t, u_register_t
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. BL2 can pass 4 arguments to BL31 and these arguments are
platform specific.

In Arm standard platforms, the arguments received are :

    arg0 - The pointer to the head of `bl_params_t` list
    which is list of executable images following BL31,

    arg1 - Points to load address of SOC_FW_CONFIG if present

    arg2 - Points to load address of HW_CONFIG if present

    arg3 - A special value to verify platform parameters from BL2 to BL31. Not
    used in release builds.

The function runs through the `bl_param_t` list and extracts the entry point
information for BL32 and BL33. It also performs the following:

-  Initialize a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL31.

-  Enable issuing of snoop and DVM (Distributed Virtual Memory) requests to the
   CCI slave interface corresponding to the cluster that includes the primary
   CPU.

Function : bl31_plat_arch_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On Arm standard platforms, this function enables the MMU.

Function : bl31_platform_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl31_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to complete platform initialization so that both
BL31 runtime services and normal world software can function correctly.

On Arm standard platforms, this function does the following:

-  Initialize the generic interrupt controller.

   Depending on the GIC driver selected by the platform, the appropriate GICv2
   or GICv3 initialization will be done, which mainly consists of:

   -  Enable secure interrupts in the GIC CPU interface.
   -  Disable the legacy interrupt bypass mechanism.
   -  Configure the priority mask register to allow interrupts of all priorities
      to be signaled to the CPU interface.
   -  Mark SGIs 8-15 and the other secure interrupts on the platform as secure.
   -  Target all secure SPIs to CPU0.
   -  Enable these secure interrupts in the GIC distributor.
   -  Configure all other interrupts as non-secure.
   -  Enable signaling of secure interrupts in the GIC distributor.

-  Enable system-level implementation of the generic timer counter through the
   memory mapped interface.

-  Grant access to the system counter timer module

-  Initialize the power controller device.

   In particular, initialise the locks that prevent concurrent accesses to the
   power controller device.

Function : bl31_plat_runtime_setup() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

The purpose of this function is allow the platform to perform any BL31 runtime
setup just prior to BL31 exit during cold boot. The default weak
implementation of this function will invoke ``console_switch_state()`` to switch
console output to consoles marked for use in the ``runtime`` state.

Function : bl31_plat_get_next_image_ep_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : entry_point_info *

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``.

This function is called by ``bl31_main()`` to retrieve information provided by
BL2 for the next image in the security state specified by the argument. BL31
uses this information to pass control to that image in the specified security
state. This function must return a pointer to the ``entry_point_info`` structure
(that was copied during ``bl31_early_platform_setup()``) if the image exists. It
should return NULL otherwise.

Function : bl31_plat_enable_mmu [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : void

This function enables the MMU. The boot code calls this function with MMU and
caches disabled. This function should program necessary registers to enable
translation, and upon return, the MMU on the calling PE must be enabled.

The function must honor flags passed in the first argument. These flags are
defined by the translation library, and can be found in the file
``include/lib/xlat_tables/xlat_mmu_helpers.h``.

On DynamIQ systems, this function must not use stack while enabling MMU, which
is how the function in xlat table library version 2 is implemented.

Function : plat_init_apkey [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint128_t

This function returns the 128-bit value which can be used to program ARMv8.3
pointer authentication keys.

The value should be obtained from a reliable source of randomness.

This function is only needed if ARMv8.3 pointer authentication is used in the
Trusted Firmware by building with ``BRANCH_PROTECTION`` option set to non-zero.

Function : plat_get_syscnt_freq2() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This function is used by the architecture setup code to retrieve the counter
frequency for the CPU's generic timer. This value will be programmed into the
``CNTFRQ_EL0`` register. In Arm standard platforms, it returns the base frequency
of the system counter, which is retrieved from the first entry in the frequency
modes table.

#define : PLAT_PERCPU_BAKERY_LOCK_SIZE [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``USE_COHERENT_MEM = 0``, this constant defines the total memory (in
bytes) aligned to the cache line boundary that should be allocated per-cpu to
accommodate all the bakery locks.

If this constant is not defined when ``USE_COHERENT_MEM = 0``, the linker
calculates the size of the ``bakery_lock`` input section, aligns it to the
nearest ``CACHE_WRITEBACK_GRANULE``, multiplies it with ``PLATFORM_CORE_COUNT``
and stores the result in a linker symbol. This constant prevents a platform
from relying on the linker and provide a more efficient mechanism for
accessing per-cpu bakery lock information.

If this constant is defined and its value is not equal to the value
calculated by the linker then a link time assertion is raised. A compile time
assertion is raised if the value of the constant is not aligned to the cache
line boundary.

.. _porting_guide_sdei_requirements:

SDEI porting requirements
~~~~~~~~~~~~~~~~~~~~~~~~~

The |SDEI| dispatcher requires the platform to provide the following macros
and functions, of which some are optional, and some others mandatory.

Macros
......

Macro: PLAT_SDEI_NORMAL_PRI [mandatory]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This macro must be defined to the EL3 exception priority level associated with
Normal |SDEI| events on the platform. This must have a higher value
(therefore of lower priority) than ``PLAT_SDEI_CRITICAL_PRI``.

Macro: PLAT_SDEI_CRITICAL_PRI [mandatory]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This macro must be defined to the EL3 exception priority level associated with
Critical |SDEI| events on the platform. This must have a lower value
(therefore of higher priority) than ``PLAT_SDEI_NORMAL_PRI``.

**Note**: |SDEI| exception priorities must be the lowest among Secure
priorities. Among the |SDEI| exceptions, Critical |SDEI| priority must
be higher than Normal |SDEI| priority.

Functions
.........

Function: int plat_sdei_validate_entry_point(uintptr_t ep) [optional]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  Argument: uintptr_t
  Return: int

This function validates the address of client entry points provided for both
event registration and *Complete and Resume* |SDEI| calls. The function
takes one argument, which is the address of the handler the |SDEI| client
requested to register. The function must return ``0`` for successful validation,
or ``-1`` upon failure.

The default implementation always returns ``0``. On Arm platforms, this function
is implemented to translate the entry point to physical address, and further to
ensure that the address is located in Non-secure DRAM.

Function: void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr) [optional]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  Argument: uint64_t
  Argument: unsigned int
  Return: void

|SDEI| specification requires that a PE comes out of reset with the events
masked. The client therefore is expected to call ``PE_UNMASK`` to unmask
|SDEI| events on the PE. No |SDEI| events can be dispatched until such
time.

Should a PE receive an interrupt that was bound to an |SDEI| event while the
events are masked on the PE, the dispatcher implementation invokes the function
``plat_sdei_handle_masked_trigger``. The MPIDR of the PE that received the
interrupt and the interrupt ID are passed as parameters.

The default implementation only prints out a warning message.

Power State Coordination Interface (in BL31)
--------------------------------------------

The TF-A implementation of the PSCI API is based around the concept of a
*power domain*. A *power domain* is a CPU or a logical group of CPUs which
share some state on which power management operations can be performed as
specified by `PSCI`_. Each CPU in the system is assigned a cpu index which is
a unique number between ``0`` and ``PLATFORM_CORE_COUNT - 1``. The
*power domains* are arranged in a hierarchical tree structure and each
*power domain* can be identified in a system by the cpu index of any CPU that
is part of that domain and a *power domain level*. A processing element (for
example, a CPU) is at level 0. If the *power domain* node above a CPU is a
logical grouping of CPUs that share some state, then level 1 is that group of
CPUs (for example, a cluster), and level 2 is a group of clusters (for
example, the system). More details on the power domain topology and its
organization can be found in :ref:`PSCI Power Domain Tree Structure`.

BL31's platform initialization code exports a pointer to the platform-specific
power management operations required for the PSCI implementation to function
correctly. This information is populated in the ``plat_psci_ops`` structure. The
PSCI implementation calls members of the ``plat_psci_ops`` structure for performing
power management operations on the power domains. For example, the target
CPU is specified by its ``MPIDR`` in a PSCI ``CPU_ON`` call. The ``pwr_domain_on()``
handler (if present) is called for the CPU power domain.

The ``power-state`` parameter of a PSCI ``CPU_SUSPEND`` call can be used to
describe composite power states specific to a platform. The PSCI implementation
defines a generic representation of the power-state parameter, which is an
array of local power states where each index corresponds to a power domain
level. Each entry contains the local power state the power domain at that power
level could enter. It depends on the ``validate_power_state()`` handler to
convert the power-state parameter (possibly encoding a composite power state)
passed in a PSCI ``CPU_SUSPEND`` call to this representation.

The following functions form part of platform port of PSCI functionality.

Function : plat_psci_stat_accounting_start() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : const psci_power_state_t *
    Return   : void

This is an optional hook that platforms can implement for residency statistics
accounting before entering a low power state. The ``pwr_domain_state`` field of
``state_info`` (first argument) can be inspected if stat accounting is done
differently at CPU level versus higher levels. As an example, if the element at
index 0 (CPU power level) in the ``pwr_domain_state`` array indicates a power down
state, special hardware logic may be programmed in order to keep track of the
residency statistics. For higher levels (array indices > 0), the residency
statistics could be tracked in software using PMF. If ``ENABLE_PMF`` is set, the
default implementation will use PMF to capture timestamps.

Function : plat_psci_stat_accounting_stop() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : const psci_power_state_t *
    Return   : void

This is an optional hook that platforms can implement for residency statistics
accounting after exiting from a low power state. The ``pwr_domain_state`` field
of ``state_info`` (first argument) can be inspected if stat accounting is done
differently at CPU level versus higher levels. As an example, if the element at
index 0 (CPU power level) in the ``pwr_domain_state`` array indicates a power down
state, special hardware logic may be programmed in order to keep track of the
residency statistics. For higher levels (array indices > 0), the residency
statistics could be tracked in software using PMF. If ``ENABLE_PMF`` is set, the
default implementation will use PMF to capture timestamps.

Function : plat_psci_stat_get_residency() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int, const psci_power_state_t *, unsigned int
    Return   : u_register_t

This is an optional interface that is is invoked after resuming from a low power
state and provides the time spent resident in that low power state by the power
domain at a particular power domain level. When a CPU wakes up from suspend,
all its parent power domain levels are also woken up. The generic PSCI code
invokes this function for each parent power domain that is resumed and it
identified by the ``lvl`` (first argument) parameter. The ``state_info`` (second
argument) describes the low power state that the power domain has resumed from.
The current CPU is the first CPU in the power domain to resume from the low
power state and the ``last_cpu_idx`` (third parameter) is the index of the last
CPU in the power domain to suspend and may be needed to calculate the residency
for that power domain.

Function : plat_get_target_pwr_state() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int, const plat_local_state_t *, unsigned int
    Return   : plat_local_state_t

The PSCI generic code uses this function to let the platform participate in
state coordination during a power management operation. The function is passed
a pointer to an array of platform specific local power state ``states`` (second
argument) which contains the requested power state for each CPU at a particular
power domain level ``lvl`` (first argument) within the power domain. The function
is expected to traverse this array of upto ``ncpus`` (third argument) and return
a coordinated target power state by the comparing all the requested power
states. The target power state should not be deeper than any of the requested
power states.

A weak definition of this API is provided by default wherein it assumes
that the platform assigns a local state value in order of increasing depth
of the power state i.e. for two power states X & Y, if X < Y
then X represents a shallower power state than Y. As a result, the
coordinated target local power state for a power domain will be the minimum
of the requested local power state values.

Function : plat_get_power_domain_tree_desc() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : const unsigned char *

This function returns a pointer to the byte array containing the power domain
topology tree description. The format and method to construct this array are
described in :ref:`PSCI Power Domain Tree Structure`. The BL31 PSCI
initialization code requires this array to be described by the platform, either
statically or dynamically, to initialize the power domain topology tree. In case
the array is populated dynamically, then plat_core_pos_by_mpidr() and
plat_my_core_pos() should also be implemented suitably so that the topology tree
description matches the CPU indices returned by these APIs. These APIs together
form the platform interface for the PSCI topology framework.

Function : plat_setup_psci_ops() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uintptr_t, const plat_psci_ops **
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``. It is only
called by the primary CPU.

This function is called by PSCI initialization code. Its purpose is to let
the platform layer know about the warm boot entrypoint through the
``sec_entrypoint`` (first argument) and to export handler routines for
platform-specific psci power management actions by populating the passed
pointer with a pointer to BL31's private ``plat_psci_ops`` structure.

A description of each member of this structure is given below. Please refer to
the Arm FVP specific implementation of these handlers in
``plat/arm/board/fvp/fvp_pm.c`` as an example. For each PSCI function that the
platform wants to support, the associated operation or operations in this
structure must be provided and implemented (Refer section 4 of
:ref:`Firmware Design` for the PSCI API supported in TF-A). To disable a PSCI
function in a platform port, the operation should be removed from this
structure instead of providing an empty implementation.

plat_psci_ops.cpu_standby()
...........................

Perform the platform-specific actions to enter the standby state for a cpu
indicated by the passed argument. This provides a fast path for CPU standby
wherein overheads of PSCI state management and lock acquisition is avoided.
For this handler to be invoked by the PSCI ``CPU_SUSPEND`` API implementation,
the suspend state type specified in the ``power-state`` parameter should be
STANDBY and the target power domain level specified should be the CPU. The
handler should put the CPU into a low power retention state (usually by
issuing a wfi instruction) and ensure that it can be woken up from that
state by a normal interrupt. The generic code expects the handler to succeed.

plat_psci_ops.pwr_domain_on()
.............................

Perform the platform specific actions to power on a CPU, specified
by the ``MPIDR`` (first argument). The generic code expects the platform to
return PSCI_E_SUCCESS on success or PSCI_E_INTERN_FAIL for any failure.

plat_psci_ops.pwr_domain_off()
..............................

Perform the platform specific actions to prepare to power off the calling CPU
and its higher parent power domain levels as indicated by the ``target_state``
(first argument). It is called by the PSCI ``CPU_OFF`` API implementation.

The ``target_state`` encodes the platform coordinated target local power states
for the CPU power domain and its parent power domain levels. The handler
needs to perform power management operation corresponding to the local state
at each power level.

For this handler, the local power state for the CPU power domain will be a
power down state where as it could be either power down, retention or run state
for the higher power domain levels depending on the result of state
coordination. The generic code expects the handler to succeed.

plat_psci_ops.pwr_domain_suspend_pwrdown_early() [optional]
...........................................................

This optional function may be used as a performance optimization to replace
or complement pwr_domain_suspend() on some platforms. Its calling semantics
are identical to pwr_domain_suspend(), except the PSCI implementation only
calls this function when suspending to a power down state, and it guarantees
that data caches are enabled.

When HW_ASSISTED_COHERENCY = 0, the PSCI implementation disables data caches
before calling pwr_domain_suspend(). If the target_state corresponds to a
power down state and it is safe to perform some or all of the platform
specific actions in that function with data caches enabled, it may be more
efficient to move those actions to this function. When HW_ASSISTED_COHERENCY
= 1, data caches remain enabled throughout, and so there is no advantage to
moving platform specific actions to this function.

plat_psci_ops.pwr_domain_suspend()
..................................

Perform the platform specific actions to prepare to suspend the calling
CPU and its higher parent power domain levels as indicated by the
``target_state`` (first argument). It is called by the PSCI ``CPU_SUSPEND``
API implementation.

The ``target_state`` has a similar meaning as described in
the ``pwr_domain_off()`` operation. It encodes the platform coordinated
target local power states for the CPU power domain and its parent
power domain levels. The handler needs to perform power management operation
corresponding to the local state at each power level. The generic code
expects the handler to succeed.

The difference between turning a power domain off versus suspending it is that
in the former case, the power domain is expected to re-initialize its state
when it is next powered on (see ``pwr_domain_on_finish()``). In the latter
case, the power domain is expected to save enough state so that it can resume
execution by restoring this state when its powered on (see
``pwr_domain_suspend_finish()``).

When suspending a core, the platform can also choose to power off the GICv3
Redistributor and ITS through an implementation-defined sequence. To achieve
this safely, the ITS context must be saved first. The architectural part is
implemented by the ``gicv3_its_save_disable()`` helper, but most of the needed
sequence is implementation defined and it is therefore the responsibility of
the platform code to implement the necessary sequence. Then the GIC
Redistributor context can be saved using the ``gicv3_rdistif_save()`` helper.
Powering off the Redistributor requires the implementation to support it and it
is the responsibility of the platform code to execute the right implementation
defined sequence.

When a system suspend is requested, the platform can also make use of the
``gicv3_distif_save()`` helper to save the context of the GIC Distributor after
it has saved the context of the Redistributors and ITS of all the cores in the
system. The context of the Distributor can be large and may require it to be
allocated in a special area if it cannot fit in the platform's global static
data, for example in DRAM. The Distributor can then be powered down using an
implementation-defined sequence.

plat_psci_ops.pwr_domain_pwr_down_wfi()
.......................................

This is an optional function and, if implemented, is expected to perform
platform specific actions including the ``wfi`` invocation which allows the
CPU to powerdown. Since this function is invoked outside the PSCI locks,
the actions performed in this hook must be local to the CPU or the platform
must ensure that races between multiple CPUs cannot occur.

The ``target_state`` has a similar meaning as described in the ``pwr_domain_off()``
operation and it encodes the platform coordinated target local power states for
the CPU power domain and its parent power domain levels. This function must
not return back to the caller.

If this function is not implemented by the platform, PSCI generic
implementation invokes ``psci_power_down_wfi()`` for power down.

plat_psci_ops.pwr_domain_on_finish()
....................................

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an earlier PSCI ``CPU_ON`` call.
It performs the platform-specific setup required to initialize enough state for
this CPU to enter the normal world and also provide secure runtime firmware
services.

The ``target_state`` (first argument) is the prior state of the power domains
immediately before the CPU was turned on. It indicates which power domains
above the CPU might require initialization due to having previously been in
low power states. The generic code expects the handler to succeed.

plat_psci_ops.pwr_domain_on_finish_late() [optional]
...........................................................

This optional function is called by the PSCI implementation after the calling
CPU is fully powered on with respective data caches enabled. The calling CPU and
the associated cluster are guaranteed to be participating in coherency. This
function gives the flexibility to perform any platform-specific actions safely,
such as initialization or modification of shared data structures, without the
overhead of explicit cache maintainace operations.

The ``target_state`` has a similar meaning as described in the ``pwr_domain_on_finish()``
operation. The generic code expects the handler to succeed.

plat_psci_ops.pwr_domain_suspend_finish()
.........................................

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an asynchronous wakeup
event, for example a timer interrupt that was programmed by the CPU during the
``CPU_SUSPEND`` call or ``SYSTEM_SUSPEND`` call. It performs the platform-specific
setup required to restore the saved state for this CPU to resume execution
in the normal world and also provide secure runtime firmware services.

The ``target_state`` (first argument) has a similar meaning as described in
the ``pwr_domain_on_finish()`` operation. The generic code expects the platform
to succeed.

If the Distributor, Redistributors or ITS have been powered off as part of a
suspend, their context must be restored in this function in the reverse order
to how they were saved during suspend sequence.

plat_psci_ops.system_off()
..........................

This function is called by PSCI implementation in response to a ``SYSTEM_OFF``
call. It performs the platform-specific system poweroff sequence after
notifying the Secure Payload Dispatcher.

plat_psci_ops.system_reset()
............................

This function is called by PSCI implementation in response to a ``SYSTEM_RESET``
call. It performs the platform-specific system reset sequence after
notifying the Secure Payload Dispatcher.

plat_psci_ops.validate_power_state()
....................................

This function is called by the PSCI implementation during the ``CPU_SUSPEND``
call to validate the ``power_state`` parameter of the PSCI API and if valid,
populate it in ``req_state`` (second argument) array as power domain level
specific local states. If the ``power_state`` is invalid, the platform must
return PSCI_E_INVALID_PARAMS as error, which is propagated back to the
normal world PSCI client.

plat_psci_ops.validate_ns_entrypoint()
......................................

This function is called by the PSCI implementation during the ``CPU_SUSPEND``,
``SYSTEM_SUSPEND`` and ``CPU_ON`` calls to validate the non-secure ``entry_point``
parameter passed by the normal world. If the ``entry_point`` is invalid,
the platform must return PSCI_E_INVALID_ADDRESS as error, which is
propagated back to the normal world PSCI client.

plat_psci_ops.get_sys_suspend_power_state()
...........................................

This function is called by the PSCI implementation during the ``SYSTEM_SUSPEND``
call to get the ``req_state`` parameter from platform which encodes the power
domain level specific local states to suspend to system affinity level. The
``req_state`` will be utilized to do the PSCI state coordination and
``pwr_domain_suspend()`` will be invoked with the coordinated target state to
enter system suspend.

plat_psci_ops.get_pwr_lvl_state_idx()
.....................................

This is an optional function and, if implemented, is invoked by the PSCI
implementation to convert the ``local_state`` (first argument) at a specified
``pwr_lvl`` (second argument) to an index between 0 and
``PLAT_MAX_PWR_LVL_STATES`` - 1. This function is only needed if the platform
supports more than two local power states at each power domain level, that is
``PLAT_MAX_PWR_LVL_STATES`` is greater than 2, and needs to account for these
local power states.

plat_psci_ops.translate_power_state_by_mpidr()
..............................................

This is an optional function and, if implemented, verifies the ``power_state``
(second argument) parameter of the PSCI API corresponding to a target power
domain. The target power domain is identified by using both ``MPIDR`` (first
argument) and the power domain level encoded in ``power_state``. The power domain
level specific local states are to be extracted from ``power_state`` and be
populated in the ``output_state`` (third argument) array. The functionality
is similar to the ``validate_power_state`` function described above and is
envisaged to be used in case the validity of ``power_state`` depend on the
targeted power domain. If the ``power_state`` is invalid for the targeted power
domain, the platform must return PSCI_E_INVALID_PARAMS as error. If this
function is not implemented, then the generic implementation relies on
``validate_power_state`` function to translate the ``power_state``.

This function can also be used in case the platform wants to support local
power state encoding for ``power_state`` parameter of PSCI_STAT_COUNT/RESIDENCY
APIs as described in Section 5.18 of `PSCI`_.

plat_psci_ops.get_node_hw_state()
.................................

This is an optional function. If implemented this function is intended to return
the power state of a node (identified by the first parameter, the ``MPIDR``) in
the power domain topology (identified by the second parameter, ``power_level``),
as retrieved from a power controller or equivalent component on the platform.
Upon successful completion, the implementation must map and return the final
status among ``HW_ON``, ``HW_OFF`` or ``HW_STANDBY``. Upon encountering failures, it
must return either ``PSCI_E_INVALID_PARAMS`` or ``PSCI_E_NOT_SUPPORTED`` as
appropriate.

Implementations are not expected to handle ``power_levels`` greater than
``PLAT_MAX_PWR_LVL``.

plat_psci_ops.system_reset2()
.............................

This is an optional function. If implemented this function is
called during the ``SYSTEM_RESET2`` call to perform a reset
based on the first parameter ``reset_type`` as specified in
`PSCI`_. The parameter ``cookie`` can be used to pass additional
reset information. If the ``reset_type`` is not supported, the
function must return ``PSCI_E_NOT_SUPPORTED``. For architectural
resets, all failures must return ``PSCI_E_INVALID_PARAMETERS``
and vendor reset can return other PSCI error codes as defined
in `PSCI`_. On success this function will not return.

plat_psci_ops.write_mem_protect()
.................................

This is an optional function. If implemented it enables or disables the
``MEM_PROTECT`` functionality based on the value of ``val``.
A non-zero value enables ``MEM_PROTECT`` and a value of zero
disables it. Upon encountering failures it must return a negative value
and on success it must return 0.

plat_psci_ops.read_mem_protect()
................................

This is an optional function. If implemented it returns the current
state of ``MEM_PROTECT`` via the ``val`` parameter.  Upon encountering
failures it must return a negative value and on success it must
return 0.

plat_psci_ops.mem_protect_chk()
...............................

This is an optional function. If implemented it checks if a memory
region defined by a base address ``base`` and with a size of ``length``
bytes is protected by ``MEM_PROTECT``.  If the region is protected
then it must return 0, otherwise it must return a negative number.

.. _porting_guide_imf_in_bl31:

Interrupt Management framework (in BL31)
----------------------------------------

BL31 implements an Interrupt Management Framework (IMF) to manage interrupts
generated in either security state and targeted to EL1 or EL2 in the non-secure
state or EL3/S-EL1 in the secure state. The design of this framework is
described in the :ref:`Interrupt Management Framework`

A platform should export the following APIs to support the IMF. The following
text briefly describes each API and its implementation in Arm standard
platforms. The API implementation depends upon the type of interrupt controller
present in the platform. Arm standard platform layer supports both
`Arm Generic Interrupt Controller version 2.0 (GICv2)`_
and `3.0 (GICv3)`_. Juno builds the Arm platform layer to use GICv2 and the
FVP can be configured to use either GICv2 or GICv3 depending on the build flag
``FVP_USE_GIC_DRIVER`` (See :ref:`build_options_arm_fvp_platform` for more
details).

See also: `Interrupt Controller Abstraction APIs`__.

.. __: ../design/platform-interrupt-controller-API.rst

Function : plat_interrupt_type_to_line() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t, uint32_t
    Return   : uint32_t

The Arm processor signals an interrupt exception either through the IRQ or FIQ
interrupt line. The specific line that is signaled depends on how the interrupt
controller (IC) reports different interrupt types from an execution context in
either security state. The IMF uses this API to determine which interrupt line
the platform IC uses to signal each type of interrupt supported by the framework
from a given security state. This API must be invoked at EL3.

The first parameter will be one of the ``INTR_TYPE_*`` values (see
:ref:`Interrupt Management Framework`) indicating the target type of the
interrupt, the second parameter is the security state of the originating
execution context. The return result is the bit position in the ``SCR_EL3``
register of the respective interrupt trap: IRQ=1, FIQ=2.

In the case of Arm standard platforms using GICv2, S-EL1 interrupts are
configured as FIQs and Non-secure interrupts as IRQs from either security
state.

In the case of Arm standard platforms using GICv3, the interrupt line to be
configured depends on the security state of the execution context when the
interrupt is signalled and are as follows:

-  The S-EL1 interrupts are signaled as IRQ in S-EL0/1 context and as FIQ in
   NS-EL0/1/2 context.
-  The Non secure interrupts are signaled as FIQ in S-EL0/1 context and as IRQ
   in the NS-EL0/1/2 context.
-  The EL3 interrupts are signaled as FIQ in both S-EL0/1 and NS-EL0/1/2
   context.

Function : plat_ic_get_pending_interrupt_type() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API returns the type of the highest priority pending interrupt at the
platform IC. The IMF uses the interrupt type to retrieve the corresponding
handler function. ``INTR_TYPE_INVAL`` is returned when there is no interrupt
pending. The valid interrupt types that can be returned are ``INTR_TYPE_EL3``,
``INTR_TYPE_S_EL1`` and ``INTR_TYPE_NS``. This API must be invoked at EL3.

In the case of Arm standard platforms using GICv2, the *Highest Priority
Pending Interrupt Register* (``GICC_HPPIR``) is read to determine the id of
the pending interrupt. The type of interrupt depends upon the id value as
follows.

#. id < 1022 is reported as a S-EL1 interrupt
#. id = 1022 is reported as a Non-secure interrupt.
#. id = 1023 is reported as an invalid interrupt type.

In the case of Arm standard platforms using GICv3, the system register
``ICC_HPPIR0_EL1``, *Highest Priority Pending group 0 Interrupt Register*,
is read to determine the id of the pending interrupt. The type of interrupt
depends upon the id value as follows.

#. id = ``PENDING_G1S_INTID`` (1020) is reported as a S-EL1 interrupt
#. id = ``PENDING_G1NS_INTID`` (1021) is reported as a Non-secure interrupt.
#. id = ``GIC_SPURIOUS_INTERRUPT`` (1023) is reported as an invalid interrupt type.
#. All other interrupt id's are reported as EL3 interrupt.

Function : plat_ic_get_pending_interrupt_id() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API returns the id of the highest priority pending interrupt at the
platform IC. ``INTR_ID_UNAVAILABLE`` is returned when there is no interrupt
pending.

In the case of Arm standard platforms using GICv2, the *Highest Priority
Pending Interrupt Register* (``GICC_HPPIR``) is read to determine the id of the
pending interrupt. The id that is returned by API depends upon the value of
the id read from the interrupt controller as follows.

#. id < 1022. id is returned as is.
#. id = 1022. The *Aliased Highest Priority Pending Interrupt Register*
   (``GICC_AHPPIR``) is read to determine the id of the non-secure interrupt.
   This id is returned by the API.
#. id = 1023. ``INTR_ID_UNAVAILABLE`` is returned.

In the case of Arm standard platforms using GICv3, if the API is invoked from
EL3, the system register ``ICC_HPPIR0_EL1``, *Highest Priority Pending Interrupt
group 0 Register*, is read to determine the id of the pending interrupt. The id
that is returned by API depends upon the value of the id read from the
interrupt controller as follows.

#. id < ``PENDING_G1S_INTID`` (1020). id is returned as is.
#. id = ``PENDING_G1S_INTID`` (1020) or ``PENDING_G1NS_INTID`` (1021). The system
   register ``ICC_HPPIR1_EL1``, *Highest Priority Pending Interrupt group 1
   Register* is read to determine the id of the group 1 interrupt. This id
   is returned by the API as long as it is a valid interrupt id
#. If the id is any of the special interrupt identifiers,
   ``INTR_ID_UNAVAILABLE`` is returned.

When the API invoked from S-EL1 for GICv3 systems, the id read from system
register ``ICC_HPPIR1_EL1``, *Highest Priority Pending group 1 Interrupt
Register*, is returned if is not equal to GIC_SPURIOUS_INTERRUPT (1023) else
``INTR_ID_UNAVAILABLE`` is returned.

Function : plat_ic_acknowledge_interrupt() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API is used by the CPU to indicate to the platform IC that processing of
the highest pending interrupt has begun. It should return the raw, unmodified
value obtained from the interrupt controller when acknowledging an interrupt.
The actual interrupt number shall be extracted from this raw value using the API
`plat_ic_get_interrupt_id()`__.

.. __: ../design/platform-interrupt-controller-API.rst#function-unsigned-int-plat-ic-get-interrupt-id-unsigned-int-raw-optional

This function in Arm standard platforms using GICv2, reads the *Interrupt
Acknowledge Register* (``GICC_IAR``). This changes the state of the highest
priority pending interrupt from pending to active in the interrupt controller.
It returns the value read from the ``GICC_IAR``, unmodified.

In the case of Arm standard platforms using GICv3, if the API is invoked
from EL3, the function reads the system register ``ICC_IAR0_EL1``, *Interrupt
Acknowledge Register group 0*. If the API is invoked from S-EL1, the function
reads the system register ``ICC_IAR1_EL1``, *Interrupt Acknowledge Register
group 1*. The read changes the state of the highest pending interrupt from
pending to active in the interrupt controller. The value read is returned
unmodified.

The TSP uses this API to start processing of the secure physical timer
interrupt.

Function : plat_ic_end_of_interrupt() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : void

This API is used by the CPU to indicate to the platform IC that processing of
the interrupt corresponding to the id (passed as the parameter) has
finished. The id should be the same as the id returned by the
``plat_ic_acknowledge_interrupt()`` API.

Arm standard platforms write the id to the *End of Interrupt Register*
(``GICC_EOIR``) in case of GICv2, and to ``ICC_EOIR0_EL1`` or ``ICC_EOIR1_EL1``
system register in case of GICv3 depending on where the API is invoked from,
EL3 or S-EL1. This deactivates the corresponding interrupt in the interrupt
controller.

The TSP uses this API to finish processing of the secure physical timer
interrupt.

Function : plat_ic_get_interrupt_type() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : uint32_t

This API returns the type of the interrupt id passed as the parameter.
``INTR_TYPE_INVAL`` is returned if the id is invalid. If the id is valid, a valid
interrupt type (one of ``INTR_TYPE_EL3``, ``INTR_TYPE_S_EL1`` and ``INTR_TYPE_NS``) is
returned depending upon how the interrupt has been configured by the platform
IC. This API must be invoked at EL3.

Arm standard platforms using GICv2 configures S-EL1 interrupts as Group0 interrupts
and Non-secure interrupts as Group1 interrupts. It reads the group value
corresponding to the interrupt id from the relevant *Interrupt Group Register*
(``GICD_IGROUPRn``). It uses the group value to determine the type of interrupt.

In the case of Arm standard platforms using GICv3, both the *Interrupt Group
Register* (``GICD_IGROUPRn``) and *Interrupt Group Modifier Register*
(``GICD_IGRPMODRn``) is read to figure out whether the interrupt is configured
as Group 0 secure interrupt, Group 1 secure interrupt or Group 1 NS interrupt.

Crash Reporting mechanism (in BL31)
-----------------------------------

BL31 implements a crash reporting mechanism which prints the various registers
of the CPU to enable quick crash analysis and debugging. This mechanism relies
on the platform implementing ``plat_crash_console_init``,
``plat_crash_console_putc`` and ``plat_crash_console_flush``.

The file ``plat/common/aarch64/crash_console_helpers.S`` contains sample
implementation of all of them. Platforms may include this file to their
makefiles in order to benefit from them. By default, they will cause the crash
output to be routed over the normal console infrastructure and get printed on
consoles configured to output in crash state. ``console_set_scope()`` can be
used to control whether a console is used for crash output.

.. note::
   Platforms are responsible for making sure that they only mark consoles for
   use in the crash scope that are able to support this, i.e. that are written
   in assembly and conform with the register clobber rules for putc()
   (x0-x2, x16-x17) and flush() (x0-x3, x16-x17) crash callbacks.

In some cases (such as debugging very early crashes that happen before the
normal boot console can be set up), platforms may want to control crash output
more explicitly. These platforms may instead provide custom implementations for
these. They are executed outside of a C environment and without a stack. Many
console drivers provide functions named ``console_xxx_core_init/putc/flush``
that are designed to be used by these functions. See Arm platforms (like juno)
for an example of this.

Function : plat_crash_console_init [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to initialize the crash
console. It must only use the general purpose registers x0 through x7 to do the
initialization and returns 1 on success.

Function : plat_crash_console_putc [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : int

This API is used by the crash reporting mechanism to print a character on the
designated crash console. It must only use general purpose registers x1 and
x2 to do its work. The parameter and the return value are in general purpose
register x0.

Function : plat_crash_console_flush [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to force write of all buffered
data on the designated crash console. It should only use general purpose
registers x0 through x5 to do its work. The return value is 0 on successful
completion; otherwise the return value is -1.

External Abort handling and RAS Support
---------------------------------------

Function : plat_ea_handler
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Argument : uint64_t
    Argument : void *
    Argument : void *
    Argument : uint64_t
    Return   : void

This function is invoked by the RAS framework for the platform to handle an
External Abort received at EL3. The intention of the function is to attempt to
resolve the cause of External Abort and return; if that's not possible, to
initiate orderly shutdown of the system.

The first parameter (``int ea_reason``) indicates the reason for External Abort.
Its value is one of ``ERROR_EA_*`` constants defined in ``ea_handle.h``.

The second parameter (``uint64_t syndrome``) is the respective syndrome
presented to EL3 after having received the External Abort. Depending on the
nature of the abort (as can be inferred from the ``ea_reason`` parameter), this
can be the content of either ``ESR_EL3`` or ``DISR_EL1``.

The third parameter (``void *cookie``) is unused for now. The fourth parameter
(``void *handle``) is a pointer to the preempted context. The fifth parameter
(``uint64_t flags``) indicates the preempted security state. These parameters
are received from the top-level exception handler.

If ``RAS_EXTENSION`` is set to ``1``, the default implementation of this
function iterates through RAS handlers registered by the platform. If any of the
RAS handlers resolve the External Abort, no further action is taken.

If ``RAS_EXTENSION`` is set to ``0``, or if none of the platform RAS handlers
could resolve the External Abort, the default implementation prints an error
message, and panics.

Function : plat_handle_uncontainable_ea
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Argument : uint64_t
    Return   : void

This function is invoked by the RAS framework when an External Abort of
Uncontainable type is received at EL3. Due to the critical nature of
Uncontainable errors, the intention of this function is to initiate orderly
shutdown of the system, and is not expected to return.

This function must be implemented in assembly.

The first and second parameters are the same as that of ``plat_ea_handler``.

The default implementation of this function calls
``report_unhandled_exception``.

Function : plat_handle_double_fault
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Argument : uint64_t
    Return   : void

This function is invoked by the RAS framework when another External Abort is
received at EL3 while one is already being handled. I.e., a call to
``plat_ea_handler`` is outstanding. Due to its critical nature, the intention of
this function is to initiate orderly shutdown of the system, and is not expected
recover or return.

This function must be implemented in assembly.

The first and second parameters are the same as that of ``plat_ea_handler``.

The default implementation of this function calls
``report_unhandled_exception``.

Function : plat_handle_el3_ea
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Return   : void

This function is invoked when an External Abort is received while executing in
EL3. Due to its critical nature, the intention of this function is to initiate
orderly shutdown of the system, and is not expected recover or return.

This function must be implemented in assembly.

The default implementation of this function calls
``report_unhandled_exception``.

Build flags
-----------

There are some build flags which can be defined by the platform to control
inclusion or exclusion of certain BL stages from the FIP image. These flags
need to be defined in the platform makefile which will get included by the
build system.

-  **NEED_BL33**
   By default, this flag is defined ``yes`` by the build system and ``BL33``
   build option should be supplied as a build option. The platform has the
   option of excluding the BL33 image in the ``fip`` image by defining this flag
   to ``no``. If any of the options ``EL3_PAYLOAD_BASE`` or ``PRELOADED_BL33_BASE``
   are used, this flag will be set to ``no`` automatically.

Platform include paths
----------------------

Platforms are allowed to add more include paths to be passed to the compiler.
The ``PLAT_INCLUDES`` variable is used for this purpose. This is needed in
particular for the file ``platform_def.h``.

Example:

.. code:: c

  PLAT_INCLUDES  += -Iinclude/plat/myplat/include

C Library
---------

To avoid subtle toolchain behavioral dependencies, the header files provided
by the compiler are not used. The software is built with the ``-nostdinc`` flag
to ensure no headers are included from the toolchain inadvertently. Instead the
required headers are included in the TF-A source tree. The library only
contains those C library definitions required by the local implementation. If
more functionality is required, the needed library functions will need to be
added to the local implementation.

Some C headers have been obtained from `FreeBSD`_ and `SCC`_, while others have
been written specifically for TF-A. Some implementation files have been obtained
from `FreeBSD`_, others have been written specifically for TF-A as well. The
files can be found in ``include/lib/libc`` and ``lib/libc``.

SCC can be found in http://www.simple-cc.org/. A copy of the `FreeBSD`_ sources
can be obtained from http://github.com/freebsd/freebsd.

Storage abstraction layer
-------------------------

In order to improve platform independence and portability a storage abstraction
layer is used to load data from non-volatile platform storage. Currently
storage access is only required by BL1 and BL2 phases and performed inside the
``load_image()`` function in ``bl_common.c``.

.. uml:: ../resources/diagrams/plantuml/io_framework_usage_overview.puml

It is mandatory to implement at least one storage driver. For the Arm
development platforms the Firmware Image Package (FIP) driver is provided as
the default means to load data from storage (see :ref:`firmware_design_fip`).
The storage layer is described in the header file
``include/drivers/io/io_storage.h``. The implementation of the common library is
in ``drivers/io/io_storage.c`` and the driver files are located in
``drivers/io/``.

.. uml:: ../resources/diagrams/plantuml/io_arm_class_diagram.puml

Each IO driver must provide ``io_dev_*`` structures, as described in
``drivers/io/io_driver.h``. These are returned via a mandatory registration
function that is called on platform initialization. The semi-hosting driver
implementation in ``io_semihosting.c`` can be used as an example.

Each platform should register devices and their drivers via the storage
abstraction layer. These drivers then need to be initialized by bootloader
phases as required in their respective ``blx_platform_setup()`` functions.

.. uml:: ../resources/diagrams/plantuml/io_dev_registration.puml

The storage abstraction layer provides mechanisms (``io_dev_init()``) to
initialize storage devices before IO operations are called.

.. uml:: ../resources/diagrams/plantuml/io_dev_init_and_check.puml

The basic operations supported by the layer
include ``open()``, ``close()``, ``read()``, ``write()``, ``size()`` and ``seek()``.
Drivers do not have to implement all operations, but each platform must
provide at least one driver for a device capable of supporting generic
operations such as loading a bootloader image.

The current implementation only allows for known images to be loaded by the
firmware. These images are specified by using their identifiers, as defined in
``include/plat/common/common_def.h`` (or a separate header file included from
there). The platform layer (``plat_get_image_source()``) then returns a reference
to a device and a driver-specific ``spec`` which will be understood by the driver
to allow access to the image data.

The layer is designed in such a way that is it possible to chain drivers with
other drivers. For example, file-system drivers may be implemented on top of
physical block devices, both represented by IO devices with corresponding
drivers. In such a case, the file-system "binding" with the block device may
be deferred until the file-system device is initialised.

The abstraction currently depends on structures being statically allocated
by the drivers and callers, as the system does not yet provide a means of
dynamically allocating memory. This may also have the affect of limiting the
amount of open resources per driver.

--------------

*Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.*

.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _Arm Generic Interrupt Controller version 2.0 (GICv2): http://infocenter.arm.com/help/topic/com.arm.doc.ihi0048b/index.html
.. _3.0 (GICv3): http://infocenter.arm.com/help/topic/com.arm.doc.ihi0069b/index.html
.. _FreeBSD: https://www.freebsd.org
.. _SCC: http://www.simple-cc.org/
