ARM Trusted Firmware Porting Guide
==================================


.. section-numbering::
    :suffix: .

.. contents::

--------------

Introduction
------------

Please note that this document has been updated for the new platform API
as required by the PSCI v1.0 implementation. Please refer to the
`Migration Guide`_ for the previous platform API.

Porting the ARM Trusted Firmware to a new platform involves making some
mandatory and optional modifications for both the cold and warm boot paths.
Modifications consist of:

-  Implementing a platform-specific function or variable,
-  Setting up the execution context in a certain way, or
-  Defining certain constants (for example #defines).

The platform-specific functions and variables are declared in
`include/plat/common/platform.h`_. The firmware provides a default implementation
of variables and functions to fulfill the optional requirements. These
implementations are all weakly defined; they are provided to ease the porting
effort. Each platform port can override them with its own implementation if the
default implementation is inadequate.

Platform ports that want to be aligned with standard ARM platforms (for example
FVP and Juno) may also use `include/plat/arm/common/plat\_arm.h`_ and the
corresponding source files in ``plat/arm/common/``. These provide standard
implementations for some of the required platform porting functions. However,
using these functions requires the platform port to implement additional
ARM standard platform porting functions. These additional functions are not
documented here.

Some modifications are common to all Boot Loader (BL) stages. Section 2
discusses these in detail. The subsequent sections discuss the remaining
modifications for each BL stage in detail.

This document should be read in conjunction with the ARM Trusted Firmware
`User Guide`_.

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

Also, the only translation granule size supported in Trusted Firmware is 4KB, as
various parts of the code assume that is the case. It is not possible to switch
to 16 KB or 64 KB granule sizes at the moment.

In ARM standard platforms, each BL stage configures the MMU in the
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

File : platform\_def.h [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each platform must ensure that a header file of this name is in the system
include path with the following constants defined. This may require updating the
list of ``PLAT_INCLUDES`` in the ``platform.mk`` file. In the ARM development
platforms, this file is found in ``plat/arm/board/<plat_name>/include/``.

Platform ports may optionally use the file `include/plat/common/common\_def.h`_,
which provides typical values for some of the constants below. These values are
likely to be suitable for all platform ports.

Platform ports that want to be aligned with standard ARM platforms (for example
FVP and Juno) may also use `include/plat/arm/common/arm\_def.h`_, which provides
standard values for some of the constants below. However, this requires the
platform port to define additional platform porting constants in
``platform_def.h``. These additional constants are not documented here.

-  **#define : PLATFORM\_LINKER\_FORMAT**

   Defines the linker format used by the platform, for example
   ``elf64-littleaarch64``.

-  **#define : PLATFORM\_LINKER\_ARCH**

   Defines the processor architecture for the linker by the platform, for
   example ``aarch64``.

-  **#define : PLATFORM\_STACK\_SIZE**

   Defines the normal stack memory available to each CPU. This constant is used
   by `plat/common/aarch64/platform\_mp\_stack.S`_ and
   `plat/common/aarch64/platform\_up\_stack.S`_.

-  **define : CACHE\_WRITEBACK\_GRANULE**

   Defines the size in bits of the largest cache line across all the cache
   levels in the platform.

-  **#define : FIRMWARE\_WELCOME\_STR**

   Defines the character string printed by BL1 upon entry into the ``bl1_main()``
   function.

-  **#define : PLATFORM\_CORE\_COUNT**

   Defines the total number of CPUs implemented by the platform across all
   clusters in the system.

-  **#define : PLAT\_NUM\_PWR\_DOMAINS**

   Defines the total number of nodes in the power domain topology
   tree at all the power domain levels used by the platform.
   This macro is used by the PSCI implementation to allocate
   data structures to represent power domain topology.

-  **#define : PLAT\_MAX\_PWR\_LVL**

   Defines the maximum power domain level that the power management operations
   should apply to. More often, but not always, the power domain level
   corresponds to affinity level. This macro allows the PSCI implementation
   to know the highest power domain level that it should consider for power
   management operations in the system that the platform implements. For
   example, the Base AEM FVP implements two clusters with a configurable
   number of CPUs and it reports the maximum power domain level as 1.

-  **#define : PLAT\_MAX\_OFF\_STATE**

   Defines the local power state corresponding to the deepest power down
   possible at every power domain level in the platform. The local power
   states for each level may be sparsely allocated between 0 and this value
   with 0 being reserved for the RUN state. The PSCI implementation uses this
   value to initialize the local power states of the power domain nodes and
   to specify the requested power state for a PSCI\_CPU\_OFF call.

-  **#define : PLAT\_MAX\_RET\_STATE**

   Defines the local power state corresponding to the deepest retention state
   possible at every power domain level in the platform. This macro should be
   a value less than PLAT\_MAX\_OFF\_STATE and greater than 0. It is used by the
   PSCI implementation to distinguish between retention and power down local
   power states within PSCI\_CPU\_SUSPEND call.

-  **#define : PLAT\_MAX\_PWR\_LVL\_STATES**

   Defines the maximum number of local power states per power domain level
   that the platform supports. The default value of this macro is 2 since
   most platforms just support a maximum of two local power states at each
   power domain level (power-down and retention). If the platform needs to
   account for more local power states, then it must redefine this macro.

   Currently, this macro is used by the Generic PSCI implementation to size
   the array used for PSCI\_STAT\_COUNT/RESIDENCY accounting.

-  **#define : BL1\_RO\_BASE**

   Defines the base address in secure ROM where BL1 originally lives. Must be
   aligned on a page-size boundary.

-  **#define : BL1\_RO\_LIMIT**

   Defines the maximum address in secure ROM that BL1's actual content (i.e.
   excluding any data section allocated at runtime) can occupy.

-  **#define : BL1\_RW\_BASE**

   Defines the base address in secure RAM where BL1's read-write data will live
   at runtime. Must be aligned on a page-size boundary.

-  **#define : BL1\_RW\_LIMIT**

   Defines the maximum address in secure RAM that BL1's read-write data can
   occupy at runtime.

-  **#define : BL2\_BASE**

   Defines the base address in secure RAM where BL1 loads the BL2 binary image.
   Must be aligned on a page-size boundary.

-  **#define : BL2\_LIMIT**

   Defines the maximum address in secure RAM that the BL2 image can occupy.

-  **#define : BL31\_BASE**

   Defines the base address in secure RAM where BL2 loads the BL31 binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL31\_LIMIT**

   Defines the maximum address in secure RAM that the BL31 image can occupy.

For every image, the platform must define individual identifiers that will be
used by BL1 or BL2 to load the corresponding image into memory from non-volatile
storage. For the sake of performance, integer numbers will be used as
identifiers. The platform will use those identifiers to return the relevant
information about the image to be loaded (file handler, load address,
authentication information, etc.). The following image identifiers are
mandatory:

-  **#define : BL2\_IMAGE\_ID**

   BL2 image identifier, used by BL1 to load BL2.

-  **#define : BL31\_IMAGE\_ID**

   BL31 image identifier, used by BL2 to load BL31.

-  **#define : BL33\_IMAGE\_ID**

   BL33 image identifier, used by BL2 to load BL33.

If Trusted Board Boot is enabled, the following certificate identifiers must
also be defined:

-  **#define : TRUSTED\_BOOT\_FW\_CERT\_ID**

   BL2 content certificate identifier, used by BL1 to load the BL2 content
   certificate.

-  **#define : TRUSTED\_KEY\_CERT\_ID**

   Trusted key certificate identifier, used by BL2 to load the trusted key
   certificate.

-  **#define : SOC\_FW\_KEY\_CERT\_ID**

   BL31 key certificate identifier, used by BL2 to load the BL31 key
   certificate.

-  **#define : SOC\_FW\_CONTENT\_CERT\_ID**

   BL31 content certificate identifier, used by BL2 to load the BL31 content
   certificate.

-  **#define : NON\_TRUSTED\_FW\_KEY\_CERT\_ID**

   BL33 key certificate identifier, used by BL2 to load the BL33 key
   certificate.

-  **#define : NON\_TRUSTED\_FW\_CONTENT\_CERT\_ID**

   BL33 content certificate identifier, used by BL2 to load the BL33 content
   certificate.

-  **#define : FWU\_CERT\_ID**

   Firmware Update (FWU) certificate identifier, used by NS\_BL1U to load the
   FWU content certificate.

-  **#define : PLAT\_CRYPTOCELL\_BASE**

   This defines the base address of ARM® TrustZone® CryptoCell and must be
   defined if CryptoCell crypto driver is used for Trusted Board Boot. For
   capable ARM platforms, this driver is used if ``ARM_CRYPTOCELL_INTEG`` is
   set.

If the AP Firmware Updater Configuration image, BL2U is used, the following
must also be defined:

-  **#define : BL2U\_BASE**

   Defines the base address in secure memory where BL1 copies the BL2U binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL2U\_LIMIT**

   Defines the maximum address in secure memory that the BL2U image can occupy.

-  **#define : BL2U\_IMAGE\_ID**

   BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to BL2U.

If the SCP Firmware Update Configuration Image, SCP\_BL2U is used, the following
must also be defined:

-  **#define : SCP\_BL2U\_IMAGE\_ID**

   SCP\_BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to SCP\_BL2U.
   NOTE: TF does not provide source code for this image.

If the Non-Secure Firmware Updater ROM, NS\_BL1U is used, the following must
also be defined:

-  **#define : NS\_BL1U\_BASE**

   Defines the base address in non-secure ROM where NS\_BL1U executes.
   Must be aligned on a page-size boundary.
   NOTE: TF does not provide source code for this image.

-  **#define : NS\_BL1U\_IMAGE\_ID**

   NS\_BL1U image identifier, used by BL1 to fetch an image descriptor
   corresponding to NS\_BL1U.

If the Non-Secure Firmware Updater, NS\_BL2U is used, the following must also
be defined:

-  **#define : NS\_BL2U\_BASE**

   Defines the base address in non-secure memory where NS\_BL2U executes.
   Must be aligned on a page-size boundary.
   NOTE: TF does not provide source code for this image.

-  **#define : NS\_BL2U\_IMAGE\_ID**

   NS\_BL2U image identifier, used by BL1 to fetch an image descriptor
   corresponding to NS\_BL2U.

For the the Firmware update capability of TRUSTED BOARD BOOT, the following
macros may also be defined:

-  **#define : PLAT\_FWU\_MAX\_SIMULTANEOUS\_IMAGES**

   Total number of images that can be loaded simultaneously. If the platform
   doesn't specify any value, it defaults to 10.

If a SCP\_BL2 image is supported by the platform, the following constants must
also be defined:

-  **#define : SCP\_BL2\_IMAGE\_ID**

   SCP\_BL2 image identifier, used by BL2 to load SCP\_BL2 into secure memory
   from platform storage before being transfered to the SCP.

-  **#define : SCP\_FW\_KEY\_CERT\_ID**

   SCP\_BL2 key certificate identifier, used by BL2 to load the SCP\_BL2 key
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : SCP\_FW\_CONTENT\_CERT\_ID**

   SCP\_BL2 content certificate identifier, used by BL2 to load the SCP\_BL2
   content certificate (mandatory when Trusted Board Boot is enabled).

If a BL32 image is supported by the platform, the following constants must
also be defined:

-  **#define : BL32\_IMAGE\_ID**

   BL32 image identifier, used by BL2 to load BL32.

-  **#define : TRUSTED\_OS\_FW\_KEY\_CERT\_ID**

   BL32 key certificate identifier, used by BL2 to load the BL32 key
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : TRUSTED\_OS\_FW\_CONTENT\_CERT\_ID**

   BL32 content certificate identifier, used by BL2 to load the BL32 content
   certificate (mandatory when Trusted Board Boot is enabled).

-  **#define : BL32\_BASE**

   Defines the base address in secure memory where BL2 loads the BL32 binary
   image. Must be aligned on a page-size boundary.

-  **#define : BL32\_LIMIT**

   Defines the maximum address that the BL32 image can occupy.

If the Test Secure-EL1 Payload (TSP) instantiation of BL32 is supported by the
platform, the following constants must also be defined:

-  **#define : TSP\_SEC\_MEM\_BASE**

   Defines the base address of the secure memory used by the TSP image on the
   platform. This must be at the same address or below ``BL32_BASE``.

-  **#define : TSP\_SEC\_MEM\_SIZE**

   Defines the size of the secure memory used by the BL32 image on the
   platform. ``TSP_SEC_MEM_BASE`` and ``TSP_SEC_MEM_SIZE`` must fully accomodate
   the memory required by the BL32 image, defined by ``BL32_BASE`` and
   ``BL32_LIMIT``.

-  **#define : TSP\_IRQ\_SEC\_PHY\_TIMER**

   Defines the ID of the secure physical generic timer interrupt used by the
   TSP's interrupt handling code.

If the platform port uses the translation table library code, the following
constants must also be defined:

-  **#define : PLAT\_XLAT\_TABLES\_DYNAMIC**

   Optional flag that can be set per-image to enable the dynamic allocation of
   regions even when the MMU is enabled. If not defined, only static
   functionality will be available, if defined and set to 1 it will also
   include the dynamic functionality.

-  **#define : MAX\_XLAT\_TABLES**

   Defines the maximum number of translation tables that are allocated by the
   translation table library code. To minimize the amount of runtime memory
   used, choose the smallest value needed to map the required virtual addresses
   for each BL stage. If ``PLAT_XLAT_TABLES_DYNAMIC`` flag is enabled for a BL
   image, ``MAX_XLAT_TABLES`` must be defined to accommodate the dynamic regions
   as well.

-  **#define : MAX\_MMAP\_REGIONS**

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

-  **#define : ADDR\_SPACE\_SIZE**

   Defines the total size of the address space in bytes. For example, for a 32
   bit address space, this value should be ``(1ull << 32)``. This definition is
   now deprecated, platforms should use ``PLAT_PHY_ADDR_SPACE_SIZE`` and
   ``PLAT_VIRT_ADDR_SPACE_SIZE`` instead.

-  **#define : PLAT\_VIRT\_ADDR\_SPACE\_SIZE**

   Defines the total size of the virtual address space in bytes. For example,
   for a 32 bit virtual address space, this value should be ``(1ull << 32)``.

-  **#define : PLAT\_PHY\_ADDR\_SPACE\_SIZE**

   Defines the total size of the physical address space in bytes. For example,
   for a 32 bit physical address space, this value should be ``(1ull << 32)``.

If the platform port uses the IO storage framework, the following constants
must also be defined:

-  **#define : MAX\_IO\_DEVICES**

   Defines the maximum number of registered IO devices. Attempting to register
   more devices than this value using ``io_register_device()`` will fail with
   -ENOMEM.

-  **#define : MAX\_IO\_HANDLES**

   Defines the maximum number of open IO handles. Attempting to open more IO
   entities than this value using ``io_open()`` will fail with -ENOMEM.

-  **#define : MAX\_IO\_BLOCK\_DEVICES**

   Defines the maximum number of registered IO block devices. Attempting to
   register more devices this value using ``io_dev_open()`` will fail
   with -ENOMEM. MAX\_IO\_BLOCK\_DEVICES should be less than MAX\_IO\_DEVICES.
   With this macro, multiple block devices could be supported at the same
   time.

If the platform needs to allocate data within the per-cpu data framework in
BL31, it should define the following macro. Currently this is only required if
the platform decides not to use the coherent memory section by undefining the
``USE_COHERENT_MEM`` build flag. In this case, the framework allocates the
required memory within the the per-cpu data to minimize wastage.

-  **#define : PLAT\_PCPU\_DATA\_SIZE**

   Defines the memory (in bytes) to be reserved within the per-cpu data
   structure for use by the platform layer.

The following constants are optional. They should be defined when the platform
memory layout implies some image overlaying like in ARM standard platforms.

-  **#define : BL31\_PROGBITS\_LIMIT**

   Defines the maximum address in secure RAM that the BL31's progbits sections
   can occupy.

-  **#define : TSP\_PROGBITS\_LIMIT**

   Defines the maximum address that the TSP's progbits sections can occupy.

If the platform port uses the PL061 GPIO driver, the following constant may
optionally be defined:

-  **PLAT\_PL061\_MAX\_GPIOS**
   Maximum number of GPIOs required by the platform. This allows control how
   much memory is allocated for PL061 GPIO controllers. The default value is

   #. $(eval $(call add\_define,PLAT\_PL061\_MAX\_GPIOS))

If the platform port uses the partition driver, the following constant may
optionally be defined:

-  **PLAT\_PARTITION\_MAX\_ENTRIES**
   Maximum number of partition entries required by the platform. This allows
   control how much memory is allocated for partition entries. The default
   value is 128.
   `For example, define the build flag in platform.mk`_:
   PLAT\_PARTITION\_MAX\_ENTRIES := 12
   $(eval $(call add\_define,PLAT\_PARTITION\_MAX\_ENTRIES))

The following constant is optional. It should be defined to override the default
behaviour of the ``assert()`` function (for example, to save memory).

-  **PLAT\_LOG\_LEVEL\_ASSERT**
   If ``PLAT_LOG_LEVEL_ASSERT`` is higher or equal than ``LOG_LEVEL_VERBOSE``,
   ``assert()`` prints the name of the file, the line number and the asserted
   expression. Else if it is higher than ``LOG_LEVEL_INFO``, it prints the file
   name and the line number. Else if it is lower than ``LOG_LEVEL_INFO``, it
   doesn't print anything to the console. If ``PLAT_LOG_LEVEL_ASSERT`` isn't
   defined, it defaults to ``LOG_LEVEL``.

If the platform port uses the Activity Monitor Unit, the following constants
may be defined:

-  **PLAT\_AMU\_GROUP1\_COUNTERS\_MASK**
   This mask reflects the set of group counters that should be enabled.  The
   maximum number of group 1 counters supported by AMUv1 is 16 so the mask
   can be at most 0xffff. If the platform does not define this mask, no group 1
   counters are enabled. If the platform defines this mask, the following
   constant needs to also be defined.

-  **PLAT\_AMU\_GROUP1\_NR\_COUNTERS**
   This value is used to allocate an array to save and restore the counters
   specified by ``PLAT_AMU_GROUP1_COUNTERS_MASK`` on CPU suspend.
   This value should be equal to the highest bit position set in the
   mask, plus 1.  The maximum number of group 1 counters in AMUv1 is 16.

File : plat\_macros.S [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each platform must ensure a file of this name is in the system include path with
the following macro defined. In the ARM development platforms, this file is
found in ``plat/arm/board/<plat_name>/include/plat_macros.S``.

-  **Macro : plat\_crash\_print\_regs**

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

Function : plat\_get\_my\_entrypoint() [mandatory when PROGRAMMABLE\_RESET\_ADDRESS == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
Application Binary Interface for the ARM 64-bit architecture. The caller should
not assume that callee saved registers are preserved across a call to this
function.

This function fulfills requirement 1 and 3 listed above.

Note that for platforms that support programming the reset address, it is
expected that a CPU will start executing code directly at the right address,
both on a cold and warm reset. In this case, there is no need to identify the
type of reset nor to query the warm reset entrypoint. Therefore, implementing
this function is not required on such platforms.

Function : plat\_secondary\_cold\_boot\_setup() [mandatory when COLD\_BOOT\_SINGLE\_CPU == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

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

Function : plat\_is\_my\_cpu\_primary() [mandatory when COLD\_BOOT\_SINGLE\_CPU == 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function : platform\_mem\_init() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function is called before any access to data is made by the firmware, in
order to carry out any essential memory initialization.

Function: plat\_get\_rotpk\_info()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function: plat\_get\_nv\_ctr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, unsigned int *
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It returns the
non-volatile counter value stored in the platform in the second argument. The
cookie in the first argument may be used to select the counter in case the
platform provides more than one (for example, on platforms that use the default
TBBR CoT, the cookie will correspond to the OID values defined in
TRUSTED\_FW\_NVCOUNTER\_OID or NON\_TRUSTED\_FW\_NVCOUNTER\_OID).

The function returns 0 on success. Any other value means the counter value could
not be retrieved from the platform.

Function: plat\_set\_nv\_ctr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void *, unsigned int
    Return   : int

This function is mandatory when Trusted Board Boot is enabled. It sets a new
counter value in the platform. The cookie in the first argument may be used to
select the counter (as explained in plat\_get\_nv\_ctr()). The second argument is
the updated counter value to be written to the NV counter.

The function returns 0 on success. Any other value means the counter value could
not be updated.

Function: plat\_set\_nv\_ctr2()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function : plat\_my\_core\_pos()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This funtion returns the index of the calling CPU which is used as a
CPU-specific linear index into blocks of memory (for example while allocating
per-CPU stacks). This function will be invoked very early in the
initialization sequence which mandates that this function should be
implemented in assembly and should not rely on the avalability of a C
runtime environment. This function can clobber x0 - x8 and must preserve
x9 - x29.

This function plays a crucial role in the power domain topology framework in
PSCI and details of this can be found in `Power Domain Topology Design`_.

Function : plat\_core\_pos\_by\_mpidr()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : u_register_t
    Return   : int

This function validates the ``MPIDR`` of a CPU and converts it to an index,
which can be used as a CPU-specific linear index into blocks of memory. In
case the ``MPIDR`` is invalid, this function returns -1. This function will only
be invoked by BL31 after the power domain topology is initialized and can
utilize the C runtime environment. For further details about how ARM Trusted
Firmware represents the power domain topology and how this relates to the
linear CPU index, please refer `Power Domain Topology Design`_.

Common optional modifications
-----------------------------

The following are helper functions implemented by the firmware that perform
common platform-specific tasks. A platform may choose to override these
definitions.

Function : plat\_set\_my\_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function sets the current stack pointer to the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant ``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in `plat/common/aarch64/platform\_up\_stack.S`_ and
`plat/common/aarch64/platform\_mp\_stack.S`_

Function : plat\_get\_my\_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uintptr_t

This function returns the base address of the normal memory stack that
has been allocated for the current CPU. For BL images that only require a
stack for the primary CPU, the UP version of the function is used. The size
of the stack allocated to each CPU is specified by the platform defined
constant ``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in `plat/common/aarch64/platform\_up\_stack.S`_ and
`plat/common/aarch64/platform\_mp\_stack.S`_

Function : plat\_report\_exception()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
`include/common/bl\_common.h`_ header file. Note that these constants are not
related to any architectural exception code; they are just an ARM Trusted
Firmware convention.

For AArch32, this function receives the exception mode as its argument.
Possible values for exception modes are listed in the
`include/lib/aarch32/arch.h`_ header file.

Function : plat\_reset\_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

A platform may need to do additional initialization after reset. This function
allows the platform to do the platform specific intializations. Platform
specific errata workarounds could also be implemented here. The api should
preserve the values of callee saved registers x19 to x29.

The default implementation doesn't do anything. If a platform needs to override
the default implementation, refer to the `Firmware Design`_ for general
guidelines.

Function : plat\_disable\_acp()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This api allows a platform to disable the Accelerator Coherency Port (if
present) during a cluster power down sequence. The default weak implementation
doesn't do anything. Since this api is called during the power down sequence,
it has restrictions for stack usage and it can use the registers x0 - x17 as
scratch registers. It should preserve the value in x18 register as it is used
by the caller to store the return address.

Function : plat\_error\_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
-  ``-ENOMEM``: resources exhausted. Trusted Firmware does not use dynamic
   memory, so this error is usually an indication of an incorrect array size

The default implementation simply spins.

Function : plat\_panic\_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This API is called when the generic code encounters an unexpected error
situation from which it cannot recover. This function must not return,
and must be implemented in assembly because it may be called before the C
environment is initialized.

Note: The address from where it was called is stored in x30 (Link Register).
The default implementation simply spins.

Function : plat\_get\_bl\_image\_load\_info()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : bl_load_info_t *

This function returns pointer to the list of images that the platform has
populated to load. This function is currently invoked in BL2 to load the
BL3xx images, when LOAD\_IMAGE\_V2 is enabled.

Function : plat\_get\_next\_bl\_params()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : bl_params_t *

This function returns a pointer to the shared memory that the platform has
kept aside to pass trusted firmware related information that next BL image
needs. This function is currently invoked in BL2 to pass this information to
the next BL image, when LOAD\_IMAGE\_V2 is enabled.

Function : plat\_get\_stack\_protector\_canary()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : u_register_t

This function returns a random value that is used to initialize the canary used
when the stack protector is enabled with ENABLE\_STACK\_PROTECTOR. A predictable
value will weaken the protection as the attacker could easily write the right
value as part of the attack most of the time. Therefore, it should return a
true random number.

Note: For the protection to be effective, the global data need to be placed at
a lower address than the stack bases. Failure to do so would allow an attacker
to overwrite the canary as part of the stack buffer overflow attack.

Function : plat\_flush\_next\_bl\_params()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function flushes to main memory all the image params that are passed to
next image. This function is currently invoked in BL2 to flush this information
to the next BL image, when LOAD\_IMAGE\_V2 is enabled.

Function : plat\_log\_get\_prefix()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : const char *

This function defines the prefix string corresponding to the `log_level` to be
prepended to all the log output from ARM Trusted Firmware. The `log_level`
(argument) will correspond to one of the standard log levels defined in
debug.h. The platform can override the common implementation to define a
different prefix string for the log output.  The implementation should be
robust to future changes that increase the number of log levels.

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
       meminfo.free_base  = Base address of secure RAM available for
                            allocation to BL2
       meminfo.free_size  = Size of secure RAM available for allocation to BL2

   BL1 places this ``meminfo`` structure at the beginning of the free memory
   available for its use. Since BL1 cannot allocate memory dynamically at the
   moment, its free memory will be available for BL2's use as-is. However, this
   means that BL2 must read the ``meminfo`` structure before it starts using its
   free memory (this is discussed in Section 3.2).

   In future releases of the ARM Trusted Firmware it will be possible for
   the platform to decide where it wants to place the ``meminfo`` structure for
   BL2.

   BL1 implements the ``bl1_init_bl2_mem_layout()`` function to populate the
   BL2 ``meminfo`` structure. The platform may override this implementation, for
   example if the platform wants to restrict the amount of memory visible to
   BL2. Details of how to do this are given below.

The following functions need to be implemented by the platform port to enable
BL1 to perform the above tasks.

Function : bl1\_early\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

On ARM standard platforms, this function:

-  Enables a secure instance of SP805 to act as the Trusted Watchdog.

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL1.

-  Enables issuing of snoop and DVM (Distributed Virtual Memory) requests to
   the CCI slave interface corresponding to the cluster that includes the
   primary CPU.

Function : bl1\_plat\_arch\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function performs any platform-specific and architectural setup that the
platform requires. Platform-specific setup might include configuration of
memory controllers and the interconnect.

In ARM standard platforms, this function enables the MMU.

This function helps fulfill requirement 2 above.

Function : bl1\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches enabled. It is responsible
for performing any remaining platform-specific setup that can occur after the
MMU and data cache have been enabled.

if support for multiple boot sources is required, it initializes the boot
sequence used by plat\_try\_next\_boot\_source().

In ARM standard platforms, this function initializes the storage abstraction
layer used to load the next bootloader image.

This function helps fulfill requirement 4 above.

Function : bl1\_plat\_sec\_mem\_layout() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    meminfo.free_base  = Base address of secure RAM available for allocation
                         to BL1
    meminfo.free_size  = Size of secure RAM available for allocation to BL1

This information is used by BL1 to load the BL2 image in secure RAM. BL1 also
populates a similar structure to tell BL2 the extents of memory available for
its own use.

This function helps fulfill requirements 4 and 5 above.

Function : bl1\_init\_bl2\_mem\_layout() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *, meminfo *
    Return   : void

BL1 needs to tell the next stage the amount of secure RAM available
for it to use. This information is populated in a ``meminfo``
structure.

Depending upon where BL2 has been loaded in secure RAM (determined by
``BL2_BASE``), BL1 calculates the amount of free memory available for BL2 to use.
BL1 also ensures that its data sections resident in secure RAM are not visible
to BL2. An illustration of how this is done in ARM standard platforms is given
in the **Memory layout on ARM development platforms** section in the
`Firmware Design`_.

Function : bl1\_plat\_prepare\_exit() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : entry_point_info_t *
    Return   : void

This function is called prior to exiting BL1 in response to the
``BL1_SMC_RUN_IMAGE`` SMC request raised by BL2. It should be used to perform
platform specific clean up or bookkeeping operations before transferring
control to the next image. It receives the address of the ``entry_point_info_t``
structure passed from BL2. This function runs with MMU disabled.

Function : bl1\_plat\_set\_ep\_info() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id, entry_point_info_t *ep_info
    Return   : void

This function allows platforms to override ``ep_info`` for the given ``image_id``.

The default implementation just returns.

Function : bl1\_plat\_get\_next\_image\_id() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This and the following function must be overridden to enable the FWU feature.

BL1 calls this function after platform setup to identify the next image to be
loaded and executed. If the platform returns ``BL2_IMAGE_ID`` then BL1 proceeds
with the normal boot sequence, which loads and executes BL2. If the platform
returns a different image id, BL1 assumes that Firmware Update is required.

The default implementation always returns ``BL2_IMAGE_ID``. The ARM development
platforms override this function to detect if firmware update is required, and
if so, return the first image in the firmware update process.

Function : bl1\_plat\_get\_image\_desc() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id
    Return   : image_desc_t *

BL1 calls this function to get the image descriptor information ``image_desc_t``
for the provided ``image_id`` from the platform.

The default implementation always returns a common BL2 image descriptor. ARM
standard platforms return an image descriptor corresponding to BL2 or one of
the firmware update images defined in the Trusted Board Boot Requirements
specification.

Function : bl1\_plat\_fwu\_done() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int image_id, uintptr_t image_src,
               unsigned int image_size
    Return   : void

BL1 calls this function when the FWU process is complete. It must not return.
The platform may override this function to take platform specific action, for
example to initiate the normal boot flow.

The default implementation spins forever.

Function : bl1\_plat\_mem\_check() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
``BL2_BASE``. BL2 executes in Secure EL1 and is responsible for:

#. (Optional) Loading the SCP\_BL2 binary image (if present) from platform
   provided non-volatile storage. To load the SCP\_BL2 image, BL2 makes use of
   the ``meminfo`` returned by the ``bl2_plat_get_scp_bl2_meminfo()`` function.
   The platform also defines the address in memory where SCP\_BL2 is loaded
   through the optional constant ``SCP_BL2_BASE``. BL2 uses this information
   to determine if there is enough memory to load the SCP\_BL2 image.
   Subsequent handling of the SCP\_BL2 image is platform-specific and is
   implemented in the ``bl2_plat_handle_scp_bl2()`` function.
   If ``SCP_BL2_BASE`` is not defined then this step is not performed.

#. Loading the BL31 binary image into secure RAM from non-volatile storage. To
   load the BL31 image, BL2 makes use of the ``meminfo`` structure passed to it
   by BL1. This structure allows BL2 to calculate how much secure RAM is
   available for its use. The platform also defines the address in secure RAM
   where BL31 is loaded through the constant ``BL31_BASE``. BL2 uses this
   information to determine if there is enough memory to load the BL31 image.

#. (Optional) Loading the BL32 binary image (if present) from platform
   provided non-volatile storage. To load the BL32 image, BL2 makes use of
   the ``meminfo`` returned by the ``bl2_plat_get_bl32_meminfo()`` function.
   The platform also defines the address in memory where BL32 is loaded
   through the optional constant ``BL32_BASE``. BL2 uses this information
   to determine if there is enough memory to load the BL32 image.
   If ``BL32_BASE`` is not defined then this and the next step is not performed.

#. (Optional) Arranging to pass control to the BL32 image (if present) that
   has been pre-loaded at ``BL32_BASE``. BL2 populates an ``entry_point_info``
   structure in memory provided by the platform with information about how
   BL31 should pass control to the BL32 image.

#. (Optional) Loading the normal world BL33 binary image (if not loaded by
   other means) into non-secure DRAM from platform storage and arranging for
   BL31 to pass control to this image. This address is determined using the
   ``plat_get_ns_image_entrypoint()`` function described below.

#. BL2 populates an ``entry_point_info`` structure in memory provided by the
   platform with information about how BL31 should pass control to the
   other BL images.

The following functions must be implemented by the platform port to enable BL2
to perform the above tasks.

Function : bl2\_early\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function is the address of the
``meminfo`` structure populated by BL1.

The platform may copy the contents of the ``meminfo`` structure into a private
variable as the original memory may be subsequently overwritten by BL2. The
copied structure is made available to all BL2 code through the
``bl2_plat_sec_mem_layout()`` function.

On ARM standard platforms, this function also:

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL2.

-  Initializes the storage abstraction layer used to load further bootloader
   images. It is necessary to do this early on platforms with a SCP\_BL2 image,
   since the later ``bl2_platform_setup`` must be done after SCP\_BL2 is loaded.

Function : bl2\_plat\_arch\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On ARM standard platforms, this function enables the MMU.

Function : bl2\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl2_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2.

In ARM standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.

Function : bl2\_plat\_sec\_mem\_layout() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : meminfo *

This function should only be called on the cold boot path. It may execute with
the MMU and data caches enabled if the platform port does the necessary
initialization in ``bl2_plat_arch_setup()``. It is only called by the primary CPU.

The purpose of this function is to return a pointer to a ``meminfo`` structure
populated with the extents of secure RAM available for BL2 to use. See
``bl2_early_platform_setup()`` above.

Following function is required only when LOAD\_IMAGE\_V2 is enabled.

Function : bl2\_plat\_handle\_post\_image\_load() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This function can be used by the platforms to update/use image information
for given ``image_id``. This function is currently invoked in BL2 to handle
BL image specific information based on the ``image_id`` passed, when
LOAD\_IMAGE\_V2 is enabled.

Following functions are required only when LOAD\_IMAGE\_V2 is disabled.

Function : bl2\_plat\_get\_scp\_bl2\_meminfo() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
SCP\_BL2 image. The meminfo provided by this is used by load\_image() to
validate whether the SCP\_BL2 image can be loaded within the given
memory from the given base.

Function : bl2\_plat\_handle\_scp\_bl2() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : image_info *
    Return   : int

This function is called after loading SCP\_BL2 image and it is used to perform
any platform-specific actions required to handle the SCP firmware. Typically it
transfers the image into SCP memory using a platform-specific protocol and waits
until SCP executes it and signals to the Application Processor (AP) for BL2
execution to continue.

This function returns 0 on success, a negative error code otherwise.

Function : bl2\_plat\_get\_bl31\_params() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : bl31_params *

BL2 platform code needs to return a pointer to a ``bl31_params`` structure it
will use for passing information to BL31. The ``bl31_params`` structure carries
the following information.
- Header describing the version information for interpreting the bl31\_param
structure
- Information about executing the BL33 image in the ``bl33_ep_info`` field
- Information about executing the BL32 image in the ``bl32_ep_info`` field
- Information about the type and extents of BL31 image in the
``bl31_image_info`` field
- Information about the type and extents of BL32 image in the
``bl32_image_info`` field
- Information about the type and extents of BL33 image in the
``bl33_image_info`` field

The memory pointed by this structure and its sub-structures should be
accessible from BL31 initialisation code. BL31 might choose to copy the
necessary content, or maintain the structures until BL33 is initialised.

Funtion : bl2\_plat\_get\_bl31\_ep\_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : entry_point_info *

BL2 platform code returns a pointer which is used to populate the entry point
information for BL31 entry point. The location pointed by it should be
accessible from BL1 while processing the synchronous exception to run to BL31.

In ARM standard platforms this is allocated inside a bl2\_to\_bl31\_params\_mem
structure in BL2 memory.

Function : bl2\_plat\_set\_bl31\_ep\_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : image_info *, entry_point_info *
    Return   : void

In the normal boot flow, this function is called after loading BL31 image and
it can be used to overwrite the entry point set by loader and also set the
security state and SPSR which represents the entry point system state for BL31.

When booting an EL3 payload instead, this function is called after populating
its entry point address and can be used for the same purpose for the payload
image. It receives a null pointer as its first argument in this case.

Function : bl2\_plat\_set\_bl32\_ep\_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL32 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL32.

Function : bl2\_plat\_set\_bl33\_ep\_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : image_info *, entry_point_info *
    Return   : void

This function is called after loading BL33 image and it can be used to
overwrite the entry point set by loader and also set the security state
and SPSR which represents the entry point system state for BL33.

In the preloaded BL33 alternative boot flow, this function is called after
populating its entry point address. It is passed a null pointer as its first
argument in this case.

Function : bl2\_plat\_get\_bl32\_meminfo() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL32 image. The meminfo provided by this is used by load\_image() to
validate whether the BL32 image can be loaded with in the given
memory from the given base.

Function : bl2\_plat\_get\_bl33\_meminfo() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *
    Return   : void

This function is used to get the memory limits where BL2 can load the
BL33 image. The meminfo provided by this is used by load\_image() to
validate whether the BL33 image can be loaded with in the given
memory from the given base.

This function isn't needed if either ``PRELOADED_BL33_BASE`` or ``EL3_PAYLOAD_BASE``
build options are used.

Function : bl2\_plat\_flush\_bl31\_params() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

Once BL2 has populated all the structures that needs to be read by BL1
and BL31 including the bl31\_params structures and its sub-structures,
the bl31\_ep\_info structure and any platform specific data. It flushes
all these data to the main memory so that it is available when we jump to
later Bootloader stages with MMU off

Function : plat\_get\_ns\_image\_entrypoint() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uintptr_t

As previously described, BL2 is responsible for arranging for control to be
passed to a normal world BL image through BL31. This function returns the
entrypoint of that image, which BL31 uses to jump to it.

BL2 is responsible for loading the normal world BL33 image (e.g. UEFI).

This function isn't needed if either ``PRELOADED_BL33_BASE`` or ``EL3_PAYLOAD_BASE``
build options are used.

Function : bl2\_plat\_preload\_setup [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::
    Argument : void
    Return   : void

This optional function performs any BL2 platform initialization
required before image loading, that is not done later in
bl2\_platform\_setup(). Specifically, if support for multiple
boot sources is required, it initializes the boot sequence used by
plat\_try\_next\_boot\_source().

Function : plat\_try\_next\_boot\_source() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

When the platform has a non-TF Boot ROM it is desirable to jump
directly to BL2 instead of TF BL1. In this case BL2 is expected to
execute at EL3 instead of executing at EL1. Refer to the `Firmware
Design`_ for more information.

All mandatory functions of BL2 must be implemented, except the functions
bl2\_early\_platform\_setup and bl2\_el3\_plat\_arch\_setup, because
their work is done now by bl2\_el3\_early\_platform\_setup and
bl2\_el3\_plat\_arch\_setup. These functions should generally implement
the bl1\_plat\_xxx() and bl2\_plat\_xxx() functionality combined.


Function : bl2\_el3\_early\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::
	Argument : u_register_t, u_register_t, u_register_t, u_register_t
	Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. This function receives four parameters which can be used
by the platform to pass any needed information from the Boot ROM to BL2.

On ARM standard platforms, this function does the following:

-  Initializes a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL2.

-  Initializes the storage abstraction layer used to load further bootloader
   images. It is necessary to do this early on platforms with a SCP\_BL2 image,
   since the later ``bl2_platform_setup`` must be done after SCP\_BL2 is loaded.

- Initializes the private variables that define the memory layout used.

Function : bl2\_el3\_plat\_arch\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::
	Argument : void
	Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On ARM standard platforms, this function enables the MMU.

Function : bl2\_el3\_plat\_prepare\_exit() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

#. (Optional) Transfering the optional SCP\_BL2U binary image from AP secure
   memory to SCP RAM. BL2U uses the SCP\_BL2U ``image_info`` passed by BL1.
   ``SCP_BL2U_BASE`` defines the address in AP secure memory where SCP\_BL2U
   should be copied from. Subsequent handling of the SCP\_BL2U image is
   implemented by the platform specific ``bl2u_plat_handle_scp_bl2u()`` function.
   If ``SCP_BL2U_BASE`` is not defined then this step is not performed.

#. Any platform specific setup required to perform the FWU process. For
   example, ARM standard platforms initialize the TZC controller so that the
   normal world can access DDR memory.

The following functions must be implemented by the platform port to enable
BL2U to perform the tasks mentioned above.

Function : bl2u\_early\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : meminfo *mem_info, void *plat_info
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU. The arguments to this function is the address
of the ``meminfo`` structure and platform specific info provided by BL1.

The platform may copy the contents of the ``mem_info`` and ``plat_info`` into
private storage as the original memory may be subsequently overwritten by BL2U.

On ARM CSS platforms ``plat_info`` is interpreted as an ``image_info_t`` structure,
to extract SCP\_BL2U image information, which is then copied into a private
variable.

Function : bl2u\_plat\_arch\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only
called by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms, for example enabling the MMU (since the memory
map differs across platforms).

Function : bl2u\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl2u_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to perform any platform initialization
specific to BL2U.

In ARM standard platforms, this function performs security setup, including
configuration of the TrustZone controller to allow non-secure masters access
to most of DRAM. Part of DRAM is reserved for secure world use.

Function : bl2u\_plat\_handle\_scp\_bl2u() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This function is used to perform any platform-specific actions required to
handle the SCP firmware. Typically it transfers the image into SCP memory using
a platform-specific protocol and waits until SCP executes it and signals to the
Application Processor (AP) for BL2U execution to continue.

This function returns 0 on success, a negative error code otherwise.
This function is included if SCP\_BL2U\_BASE is defined.

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
   specific address by BL2. BL31 uses the ``entry_point_info`` structure that BL2
   populated in memory to do this.

#. Providing runtime firmware services. Currently, BL31 only implements a
   subset of the Power State Coordination Interface (PSCI) API as a runtime
   service. See Section 3.3 below for details of porting the PSCI
   implementation.

#. Optionally passing control to the BL32 image, pre-loaded at a platform-
   specific address by BL2. BL31 exports a set of apis that allow runtime
   services to specify the security state in which the next image should be
   executed and run the corresponding image. BL31 uses the ``entry_point_info``
   structure populated by BL2 to do this.

If BL31 is a reset vector, It also needs to handle the reset as specified in
section 2.2 before the tasks described above.

The following functions must be implemented by the platform port to enable BL31
to perform the above tasks.

Function : bl31\_early\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : bl31_params *, void *
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU. The arguments to this function are:

-  The address of the ``bl31_params`` structure populated by BL2.
-  An opaque pointer that the platform may use as needed.

The platform can copy the contents of the ``bl31_params`` structure and its
sub-structures into private variables if the original memory may be
subsequently overwritten by BL31 and similarly the ``void *`` pointing
to the platform data also needs to be saved.

In ARM standard platforms, BL2 passes a pointer to a ``bl31_params`` structure
in BL2 memory. BL31 copies the information in this pointer to internal data
structures. It also performs the following:

-  Initialize a UART (PL011 console), which enables access to the ``printf``
   family of functions in BL31.

-  Enable issuing of snoop and DVM (Distributed Virtual Memory) requests to the
   CCI slave interface corresponding to the cluster that includes the primary
   CPU.

Function : bl31\_plat\_arch\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function executes with the MMU and data caches disabled. It is only called
by the primary CPU.

The purpose of this function is to perform any architectural initialization
that varies across platforms.

On ARM standard platforms, this function enables the MMU.

Function : bl31\_platform\_setup() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initialization in ``bl31_plat_arch_setup()``. It is only
called by the primary CPU.

The purpose of this function is to complete platform initialization so that both
BL31 runtime services and normal world software can function correctly.

On ARM standard platforms, this function does the following:

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

Function : bl31\_plat\_runtime\_setup() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

The purpose of this function is allow the platform to perform any BL31 runtime
setup just prior to BL31 exit during cold boot. The default weak
implementation of this function will invoke ``console_uninit()`` which will
suppress any BL31 runtime logs.

In ARM Standard platforms, this function will initialize the BL31 runtime
console which will cause all further BL31 logs to be output to the
runtime console.

Function : bl31\_get\_next\_image\_info() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : entry_point_info *

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``.

This function is called by ``bl31_main()`` to retrieve information provided by
BL2 for the next image in the security state specified by the argument. BL31
uses this information to pass control to that image in the specified security
state. This function must return a pointer to the ``entry_point_info`` structure
(that was copied during ``bl31_early_platform_setup()``) if the image exists. It
should return NULL otherwise.

Function : plat\_get\_syscnt\_freq2() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This function is used by the architecture setup code to retrieve the counter
frequency for the CPU's generic timer. This value will be programmed into the
``CNTFRQ_EL0`` register. In ARM standard platforms, it returns the base frequency
of the system counter, which is retrieved from the first entry in the frequency
modes table.

#define : PLAT\_PERCPU\_BAKERY\_LOCK\_SIZE [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

SDEI porting requirements
~~~~~~~~~~~~~~~~~~~~~~~~~

The SDEI dispatcher requires the platform to provide the following macros
and functions, of which some are optional, and some others mandatory.

Macros
......

Macro: PLAT_SDEI_NORMAL_PRI [mandatory]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This macro must be defined to the EL3 exception priority level associated with
Normal SDEI events on the platform. This must have a higher value (therefore of
lower priority) than ``PLAT_SDEI_CRITICAL_PRI``.

Macro: PLAT_SDEI_CRITICAL_PRI [mandatory]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This macro must be defined to the EL3 exception priority level associated with
Critical SDEI events on the platform. This must have a lower value (therefore of
higher priority) than ``PLAT_SDEI_NORMAL_PRI``.

It's recommended that SDEI exception priorities in general are assigned the
lowest among Secure priorities. Among the SDEI exceptions, Critical SDEI
priority must be higher than Normal SDEI priority.

Functions
.........

Function: int plat_sdei_validate_entry_point(uintptr_t ep) [optional]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  Argument: uintptr_t
  Return: int

This function validates the address of client entry points provided for both
event registration and *Complete and Resume* SDEI calls. The function takes one
argument, which is the address of the handler the SDEI client requested to
register. The function must return ``0`` for successful validation, or ``-1``
upon failure.

The default implementation always returns ``0``. On ARM platforms, this function
is implemented to translate the entry point to physical address, and further to
ensure that the address is located in Non-secure DRAM.

Function: void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr) [optional]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  Argument: uint64_t
  Argument: unsigned int
  Return: void

SDEI specification requires that a PE comes out of reset with the events masked.
The client therefore is expected to call ``PE_UNMASK`` to unmask SDEI events on
the PE. No SDEI events can be dispatched until such time.

Should a PE receive an interrupt that was bound to an SDEI event while the
events are masked on the PE, the dispatcher implementation invokes the function
``plat_sdei_handle_masked_trigger``. The MPIDR of the PE that received the
interrupt and the interrupt ID are passed as parameters.

The default implementation only prints out a warning message.

Power State Coordination Interface (in BL31)
--------------------------------------------

The ARM Trusted Firmware's implementation of the PSCI API is based around the
concept of a *power domain*. A *power domain* is a CPU or a logical group of
CPUs which share some state on which power management operations can be
performed as specified by `PSCI`_. Each CPU in the system is assigned a cpu
index which is a unique number between ``0`` and ``PLATFORM_CORE_COUNT - 1``.
The *power domains* are arranged in a hierarchical tree structure and
each *power domain* can be identified in a system by the cpu index of any CPU
that is part of that domain and a *power domain level*. A processing element
(for example, a CPU) is at level 0. If the *power domain* node above a CPU is
a logical grouping of CPUs that share some state, then level 1 is that group
of CPUs (for example, a cluster), and level 2 is a group of clusters
(for example, the system). More details on the power domain topology and its
organization can be found in `Power Domain Topology Design`_.

BL31's platform initialization code exports a pointer to the platform-specific
power management operations required for the PSCI implementation to function
correctly. This information is populated in the ``plat_psci_ops`` structure. The
PSCI implementation calls members of the ``plat_psci_ops`` structure for performing
power management operations on the power domains. For example, the target
CPU is specified by its ``MPIDR`` in a PSCI ``CPU_ON`` call. The ``pwr_domain_on()``
handler (if present) is called for the CPU power domain.

The ``power-state`` parameter of a PSCI ``CPU_SUSPEND`` call can be used to
describe composite power states specific to a platform. The PSCI implementation
defines a generic representation of the power-state parameter viz which is an
array of local power states where each index corresponds to a power domain
level. Each entry contains the local power state the power domain at that power
level could enter. It depends on the ``validate_power_state()`` handler to
convert the power-state parameter (possibly encoding a composite power state)
passed in a PSCI ``CPU_SUSPEND`` call to this representation.

The following functions form part of platform port of PSCI functionality.

Function : plat\_psci\_stat\_accounting\_start() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function : plat\_psci\_stat\_accounting\_stop() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function : plat\_psci\_stat\_get\_residency() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int, const psci_power_state_t *, int
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

Function : plat\_get\_target\_pwr\_state() [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Function : plat\_get\_power\_domain\_tree\_desc() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : const unsigned char *

This function returns a pointer to the byte array containing the power domain
topology tree description. The format and method to construct this array are
described in `Power Domain Topology Design`_. The BL31 PSCI initilization code
requires this array to be described by the platform, either statically or
dynamically, to initialize the power domain topology tree. In case the array
is populated dynamically, then plat\_core\_pos\_by\_mpidr() and
plat\_my\_core\_pos() should also be implemented suitably so that the topology
tree description matches the CPU indices returned by these APIs. These APIs
together form the platform interface for the PSCI topology framework.

Function : plat\_setup\_psci\_ops() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
the ARM FVP specific implementation of these handlers in
`plat/arm/board/fvp/fvp\_pm.c`_ as an example. For each PSCI function that the
platform wants to support, the associated operation or operations in this
structure must be provided and implemented (Refer section 4 of
`Firmware Design`_ for the PSCI API supported in Trusted Firmware). To disable
a PSCI function in a platform port, the operation should be removed from this
structure instead of providing an empty implementation.

plat\_psci\_ops.cpu\_standby()
..............................

Perform the platform-specific actions to enter the standby state for a cpu
indicated by the passed argument. This provides a fast path for CPU standby
wherein overheads of PSCI state management and lock acquistion is avoided.
For this handler to be invoked by the PSCI ``CPU_SUSPEND`` API implementation,
the suspend state type specified in the ``power-state`` parameter should be
STANDBY and the target power domain level specified should be the CPU. The
handler should put the CPU into a low power retention state (usually by
issuing a wfi instruction) and ensure that it can be woken up from that
state by a normal interrupt. The generic code expects the handler to succeed.

plat\_psci\_ops.pwr\_domain\_on()
.................................

Perform the platform specific actions to power on a CPU, specified
by the ``MPIDR`` (first argument). The generic code expects the platform to
return PSCI\_E\_SUCCESS on success or PSCI\_E\_INTERN\_FAIL for any failure.

plat\_psci\_ops.pwr\_domain\_off()
..................................

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

plat\_psci\_ops.pwr\_domain\_suspend\_pwrdown\_early() [optional]
.................................................................

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

plat\_psci\_ops.pwr\_domain\_suspend()
......................................

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

plat\_psci\_ops.pwr\_domain\_pwr\_down\_wfi()
.............................................

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

plat\_psci\_ops.pwr\_domain\_on\_finish()
.........................................

This function is called by the PSCI implementation after the calling CPU is
powered on and released from reset in response to an earlier PSCI ``CPU_ON`` call.
It performs the platform-specific setup required to initialize enough state for
this CPU to enter the normal world and also provide secure runtime firmware
services.

The ``target_state`` (first argument) is the prior state of the power domains
immediately before the CPU was turned on. It indicates which power domains
above the CPU might require initialization due to having previously been in
low power states. The generic code expects the handler to succeed.

plat\_psci\_ops.pwr\_domain\_suspend\_finish()
..............................................

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

plat\_psci\_ops.system\_off()
.............................

This function is called by PSCI implementation in response to a ``SYSTEM_OFF``
call. It performs the platform-specific system poweroff sequence after
notifying the Secure Payload Dispatcher.

plat\_psci\_ops.system\_reset()
...............................

This function is called by PSCI implementation in response to a ``SYSTEM_RESET``
call. It performs the platform-specific system reset sequence after
notifying the Secure Payload Dispatcher.

plat\_psci\_ops.validate\_power\_state()
........................................

This function is called by the PSCI implementation during the ``CPU_SUSPEND``
call to validate the ``power_state`` parameter of the PSCI API and if valid,
populate it in ``req_state`` (second argument) array as power domain level
specific local states. If the ``power_state`` is invalid, the platform must
return PSCI\_E\_INVALID\_PARAMS as error, which is propagated back to the
normal world PSCI client.

plat\_psci\_ops.validate\_ns\_entrypoint()
..........................................

This function is called by the PSCI implementation during the ``CPU_SUSPEND``,
``SYSTEM_SUSPEND`` and ``CPU_ON`` calls to validate the non-secure ``entry_point``
parameter passed by the normal world. If the ``entry_point`` is invalid,
the platform must return PSCI\_E\_INVALID\_ADDRESS as error, which is
propagated back to the normal world PSCI client.

plat\_psci\_ops.get\_sys\_suspend\_power\_state()
.................................................

This function is called by the PSCI implementation during the ``SYSTEM_SUSPEND``
call to get the ``req_state`` parameter from platform which encodes the power
domain level specific local states to suspend to system affinity level. The
``req_state`` will be utilized to do the PSCI state coordination and
``pwr_domain_suspend()`` will be invoked with the coordinated target state to
enter system suspend.

plat\_psci\_ops.get\_pwr\_lvl\_state\_idx()
...........................................

This is an optional function and, if implemented, is invoked by the PSCI
implementation to convert the ``local_state`` (first argument) at a specified
``pwr_lvl`` (second argument) to an index between 0 and
``PLAT_MAX_PWR_LVL_STATES`` - 1. This function is only needed if the platform
supports more than two local power states at each power domain level, that is
``PLAT_MAX_PWR_LVL_STATES`` is greater than 2, and needs to account for these
local power states.

plat\_psci\_ops.translate\_power\_state\_by\_mpidr()
....................................................

This is an optional function and, if implemented, verifies the ``power_state``
(second argument) parameter of the PSCI API corresponding to a target power
domain. The target power domain is identified by using both ``MPIDR`` (first
argument) and the power domain level encoded in ``power_state``. The power domain
level specific local states are to be extracted from ``power_state`` and be
populated in the ``output_state`` (third argument) array. The functionality
is similar to the ``validate_power_state`` function described above and is
envisaged to be used in case the validity of ``power_state`` depend on the
targeted power domain. If the ``power_state`` is invalid for the targeted power
domain, the platform must return PSCI\_E\_INVALID\_PARAMS as error. If this
function is not implemented, then the generic implementation relies on
``validate_power_state`` function to translate the ``power_state``.

This function can also be used in case the platform wants to support local
power state encoding for ``power_state`` parameter of PSCI\_STAT\_COUNT/RESIDENCY
APIs as described in Section 5.18 of `PSCI`_.

plat\_psci\_ops.get\_node\_hw\_state()
......................................

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

plat\_psci\_ops.system\_reset2()
................................

This is an optional function. If implemented this function is
called during the ``SYSTEM_RESET2`` call to perform a reset
based on the first parameter ``reset_type`` as specified in
`PSCI`_. The parameter ``cookie`` can be used to pass additional
reset information. If the ``reset_type`` is not supported, the
function must return ``PSCI_E_NOT_SUPPORTED``. For architectural
resets, all failures must return ``PSCI_E_INVALID_PARAMETERS``
and vendor reset can return other PSCI error codes as defined
in `PSCI`_. On success this function will not return.

plat\_psci\_ops.write\_mem\_protect()
....................................

This is an optional function. If implemented it enables or disables the
``MEM_PROTECT`` functionality based on the value of ``val``.
A non-zero value enables ``MEM_PROTECT`` and a value of zero
disables it. Upon encountering failures it must return a negative value
and on success it must return 0.

plat\_psci\_ops.read\_mem\_protect()
.....................................

This is an optional function. If implemented it returns the current
state of ``MEM_PROTECT`` via the ``val`` parameter.  Upon encountering
failures it must return a negative value and on success it must
return 0.

plat\_psci\_ops.mem\_protect\_chk()
...................................

This is an optional function. If implemented it checks if a memory
region defined by a base address ``base`` and with a size of ``length``
bytes is protected by ``MEM_PROTECT``.  If the region is protected
then it must return 0, otherwise it must return a negative number.

Interrupt Management framework (in BL31)
----------------------------------------

BL31 implements an Interrupt Management Framework (IMF) to manage interrupts
generated in either security state and targeted to EL1 or EL2 in the non-secure
state or EL3/S-EL1 in the secure state. The design of this framework is
described in the `IMF Design Guide`_

A platform should export the following APIs to support the IMF. The following
text briefly describes each api and its implementation in ARM standard
platforms. The API implementation depends upon the type of interrupt controller
present in the platform. ARM standard platform layer supports both
`ARM Generic Interrupt Controller version 2.0 (GICv2)`_
and `3.0 (GICv3)`_. Juno builds the ARM
Standard layer to use GICv2 and the FVP can be configured to use either GICv2 or
GICv3 depending on the build flag ``FVP_USE_GIC_DRIVER`` (See FVP platform
specific build options in `User Guide`_ for more details).

See also: `Interrupt Controller Abstraction APIs`__.

.. __: platform-interrupt-controller-API.rst

Function : plat\_interrupt\_type\_to\_line() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t, uint32_t
    Return   : uint32_t

The ARM processor signals an interrupt exception either through the IRQ or FIQ
interrupt line. The specific line that is signaled depends on how the interrupt
controller (IC) reports different interrupt types from an execution context in
either security state. The IMF uses this API to determine which interrupt line
the platform IC uses to signal each type of interrupt supported by the framework
from a given security state. This API must be invoked at EL3.

The first parameter will be one of the ``INTR_TYPE_*`` values (see
`IMF Design Guide`_) indicating the target type of the interrupt, the second parameter is the
security state of the originating execution context. The return result is the
bit position in the ``SCR_EL3`` register of the respective interrupt trap: IRQ=1,
FIQ=2.

In the case of ARM standard platforms using GICv2, S-EL1 interrupts are
configured as FIQs and Non-secure interrupts as IRQs from either security
state.

In the case of ARM standard platforms using GICv3, the interrupt line to be
configured depends on the security state of the execution context when the
interrupt is signalled and are as follows:

-  The S-EL1 interrupts are signaled as IRQ in S-EL0/1 context and as FIQ in
   NS-EL0/1/2 context.
-  The Non secure interrupts are signaled as FIQ in S-EL0/1 context and as IRQ
   in the NS-EL0/1/2 context.
-  The EL3 interrupts are signaled as FIQ in both S-EL0/1 and NS-EL0/1/2
   context.

Function : plat\_ic\_get\_pending\_interrupt\_type() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API returns the type of the highest priority pending interrupt at the
platform IC. The IMF uses the interrupt type to retrieve the corresponding
handler function. ``INTR_TYPE_INVAL`` is returned when there is no interrupt
pending. The valid interrupt types that can be returned are ``INTR_TYPE_EL3``,
``INTR_TYPE_S_EL1`` and ``INTR_TYPE_NS``. This API must be invoked at EL3.

In the case of ARM standard platforms using GICv2, the *Highest Priority
Pending Interrupt Register* (``GICC_HPPIR``) is read to determine the id of
the pending interrupt. The type of interrupt depends upon the id value as
follows.

#. id < 1022 is reported as a S-EL1 interrupt
#. id = 1022 is reported as a Non-secure interrupt.
#. id = 1023 is reported as an invalid interrupt type.

In the case of ARM standard platforms using GICv3, the system register
``ICC_HPPIR0_EL1``, *Highest Priority Pending group 0 Interrupt Register*,
is read to determine the id of the pending interrupt. The type of interrupt
depends upon the id value as follows.

#. id = ``PENDING_G1S_INTID`` (1020) is reported as a S-EL1 interrupt
#. id = ``PENDING_G1NS_INTID`` (1021) is reported as a Non-secure interrupt.
#. id = ``GIC_SPURIOUS_INTERRUPT`` (1023) is reported as an invalid interrupt type.
#. All other interrupt id's are reported as EL3 interrupt.

Function : plat\_ic\_get\_pending\_interrupt\_id() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API returns the id of the highest priority pending interrupt at the
platform IC. ``INTR_ID_UNAVAILABLE`` is returned when there is no interrupt
pending.

In the case of ARM standard platforms using GICv2, the *Highest Priority
Pending Interrupt Register* (``GICC_HPPIR``) is read to determine the id of the
pending interrupt. The id that is returned by API depends upon the value of
the id read from the interrupt controller as follows.

#. id < 1022. id is returned as is.
#. id = 1022. The *Aliased Highest Priority Pending Interrupt Register*
   (``GICC_AHPPIR``) is read to determine the id of the non-secure interrupt.
   This id is returned by the API.
#. id = 1023. ``INTR_ID_UNAVAILABLE`` is returned.

In the case of ARM standard platforms using GICv3, if the API is invoked from
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
Register*, is returned if is not equal to GIC\_SPURIOUS\_INTERRUPT (1023) else
``INTR_ID_UNAVAILABLE`` is returned.

Function : plat\_ic\_acknowledge\_interrupt() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : uint32_t

This API is used by the CPU to indicate to the platform IC that processing of
the highest pending interrupt has begun. It should return the raw, unmodified
value obtained from the interrupt controller when acknowledging an interrupt.
The actual interrupt number shall be extracted from this raw value using the API
`plat_ic_get_interrupt_id()`__.

.. __: platform-interrupt-controller-API.rst#function-unsigned-int-plat-ic-get-interrupt-id-unsigned-int-raw-optional

This function in ARM standard platforms using GICv2, reads the *Interrupt
Acknowledge Register* (``GICC_IAR``). This changes the state of the highest
priority pending interrupt from pending to active in the interrupt controller.
It returns the value read from the ``GICC_IAR``, unmodified.

In the case of ARM standard platforms using GICv3, if the API is invoked
from EL3, the function reads the system register ``ICC_IAR0_EL1``, *Interrupt
Acknowledge Register group 0*. If the API is invoked from S-EL1, the function
reads the system register ``ICC_IAR1_EL1``, *Interrupt Acknowledge Register
group 1*. The read changes the state of the highest pending interrupt from
pending to active in the interrupt controller. The value read is returned
unmodified.

The TSP uses this API to start processing of the secure physical timer
interrupt.

Function : plat\_ic\_end\_of\_interrupt() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : void

This API is used by the CPU to indicate to the platform IC that processing of
the interrupt corresponding to the id (passed as the parameter) has
finished. The id should be the same as the id returned by the
``plat_ic_acknowledge_interrupt()`` API.

ARM standard platforms write the id to the *End of Interrupt Register*
(``GICC_EOIR``) in case of GICv2, and to ``ICC_EOIR0_EL1`` or ``ICC_EOIR1_EL1``
system register in case of GICv3 depending on where the API is invoked from,
EL3 or S-EL1. This deactivates the corresponding interrupt in the interrupt
controller.

The TSP uses this API to finish processing of the secure physical timer
interrupt.

Function : plat\_ic\_get\_interrupt\_type() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t
    Return   : uint32_t

This API returns the type of the interrupt id passed as the parameter.
``INTR_TYPE_INVAL`` is returned if the id is invalid. If the id is valid, a valid
interrupt type (one of ``INTR_TYPE_EL3``, ``INTR_TYPE_S_EL1`` and ``INTR_TYPE_NS``) is
returned depending upon how the interrupt has been configured by the platform
IC. This API must be invoked at EL3.

ARM standard platforms using GICv2 configures S-EL1 interrupts as Group0 interrupts
and Non-secure interrupts as Group1 interrupts. It reads the group value
corresponding to the interrupt id from the relevant *Interrupt Group Register*
(``GICD_IGROUPRn``). It uses the group value to determine the type of interrupt.

In the case of ARM standard platforms using GICv3, both the *Interrupt Group
Register* (``GICD_IGROUPRn``) and *Interrupt Group Modifier Register*
(``GICD_IGRPMODRn``) is read to figure out whether the interrupt is configured
as Group 0 secure interrupt, Group 1 secure interrupt or Group 1 NS interrupt.

Crash Reporting mechanism (in BL31)
-----------------------------------

BL31 implements a crash reporting mechanism which prints the various registers
of the CPU to enable quick crash analysis and debugging. It requires that a
console is designated as the crash console by the platform which will be used to
print the register dump.

The following functions must be implemented by the platform if it wants crash
reporting mechanism in BL31. The functions are implemented in assembly so that
they can be invoked without a C Runtime stack.

Function : plat\_crash\_console\_init
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to initialize the crash
console. It must only use the general purpose registers x0 to x4 to do the
initialization and returns 1 on success.

Function : plat\_crash\_console\_putc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : int

This API is used by the crash reporting mechanism to print a character on the
designated crash console. It must only use general purpose registers x1 and
x2 to do its work. The parameter and the return value are in general purpose
register x0.

Function : plat\_crash\_console\_flush
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This API is used by the crash reporting mechanism to force write of all buffered
data on the designated crash console. It should only use general purpose
registers x0 and x1 to do its work. The return value is 0 on successful
completion; otherwise the return value is -1.

Build flags
-----------

-  **ENABLE\_PLAT\_COMPAT**
   All the platforms ports conforming to this API specification should define
   the build flag ``ENABLE_PLAT_COMPAT`` to 0 as the compatibility layer should
   be disabled. For more details on compatibility layer, refer
   `Migration Guide`_.

There are some build flags which can be defined by the platform to control
inclusion or exclusion of certain BL stages from the FIP image. These flags
need to be defined in the platform makefile which will get included by the
build system.

-  **NEED\_BL33**
   By default, this flag is defined ``yes`` by the build system and ``BL33``
   build option should be supplied as a build option. The platform has the
   option of excluding the BL33 image in the ``fip`` image by defining this flag
   to ``no``. If any of the options ``EL3_PAYLOAD_BASE`` or ``PRELOADED_BL33_BASE``
   are used, this flag will be set to ``no`` automatically.

C Library
---------

To avoid subtle toolchain behavioral dependencies, the header files provided
by the compiler are not used. The software is built with the ``-nostdinc`` flag
to ensure no headers are included from the toolchain inadvertently. Instead the
required headers are included in the ARM Trusted Firmware source tree. The
library only contains those C library definitions required by the local
implementation. If more functionality is required, the needed library functions
will need to be added to the local implementation.

Versions of `FreeBSD`_ headers can be found in ``include/lib/stdlib``. Some of
these headers have been cut down in order to simplify the implementation. In
order to minimize changes to the header files, the `FreeBSD`_ layout has been
maintained. The generic C library definitions can be found in
``include/lib/stdlib`` with more system and machine specific declarations in
``include/lib/stdlib/sys`` and ``include/lib/stdlib/machine``.

The local C library implementations can be found in ``lib/stdlib``. In order to
extend the C library these files may need to be modified. It is recommended to
use a release version of `FreeBSD`_ as a starting point.

The C library header files in the `FreeBSD`_ source tree are located in the
``include`` and ``sys/sys`` directories. `FreeBSD`_ machine specific definitions
can be found in the ``sys/<machine-type>`` directories. These files define things
like 'the size of a pointer' and 'the range of an integer'. Since an AArch64
port for `FreeBSD`_ does not yet exist, the machine specific definitions are
based on existing machine types with similar properties (for example SPARC64).

Where possible, C library function implementations were taken from `FreeBSD`_
as found in the ``lib/libc`` directory.

A copy of the `FreeBSD`_ sources can be downloaded with ``git``.

::

    git clone git://github.com/freebsd/freebsd.git -b origin/release/9.2.0

Storage abstraction layer
-------------------------

In order to improve platform independence and portability an storage abstraction
layer is used to load data from non-volatile platform storage.

Each platform should register devices and their drivers via the Storage layer.
These drivers then need to be initialized by bootloader phases as
required in their respective ``blx_platform_setup()`` functions. Currently
storage access is only required by BL1 and BL2 phases. The ``load_image()``
function uses the storage layer to access non-volatile platform storage.

It is mandatory to implement at least one storage driver. For the ARM
development platforms the Firmware Image Package (FIP) driver is provided as
the default means to load data from storage (see the "Firmware Image Package"
section in the `User Guide`_). The storage layer is described in the header file
``include/drivers/io/io_storage.h``. The implementation of the common library
is in ``drivers/io/io_storage.c`` and the driver files are located in
``drivers/io/``.

Each IO driver must provide ``io_dev_*`` structures, as described in
``drivers/io/io_driver.h``. These are returned via a mandatory registration
function that is called on platform initialization. The semi-hosting driver
implementation in ``io_semihosting.c`` can be used as an example.

The Storage layer provides mechanisms to initialize storage devices before
IO operations are called. The basic operations supported by the layer
include ``open()``, ``close()``, ``read()``, ``write()``, ``size()`` and ``seek()``.
Drivers do not have to implement all operations, but each platform must
provide at least one driver for a device capable of supporting generic
operations such as loading a bootloader image.

The current implementation only allows for known images to be loaded by the
firmware. These images are specified by using their identifiers, as defined in
[include/plat/common/platform\_def.h] (or a separate header file included from
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

*Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.*

.. _Migration Guide: platform-migration-guide.rst
.. _include/plat/common/platform.h: ../include/plat/common/platform.h
.. _include/plat/arm/common/plat\_arm.h: ../include/plat/arm/common/plat_arm.h%5D
.. _User Guide: user-guide.rst
.. _include/plat/common/common\_def.h: ../include/plat/common/common_def.h
.. _include/plat/arm/common/arm\_def.h: ../include/plat/arm/common/arm_def.h
.. _plat/common/aarch64/platform\_mp\_stack.S: ../plat/common/aarch64/platform_mp_stack.S
.. _plat/common/aarch64/platform\_up\_stack.S: ../plat/common/aarch64/platform_up_stack.S
.. _For example, define the build flag in platform.mk: PLAT_PL061_MAX_GPIOS%20:=%20160
.. _Power Domain Topology Design: psci-pd-tree.rst
.. _include/common/bl\_common.h: ../include/common/bl_common.h
.. _include/lib/aarch32/arch.h: ../include/lib/aarch32/arch.h
.. _Firmware Design: firmware-design.rst
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _plat/arm/board/fvp/fvp\_pm.c: ../plat/arm/board/fvp/fvp_pm.c
.. _IMF Design Guide: interrupt-framework-design.rst
.. _ARM Generic Interrupt Controller version 2.0 (GICv2): http://infocenter.arm.com/help/topic/com.arm.doc.ihi0048b/index.html
.. _3.0 (GICv3): http://infocenter.arm.com/help/topic/com.arm.doc.ihi0069b/index.html
.. _FreeBSD: http://www.freebsd.org
