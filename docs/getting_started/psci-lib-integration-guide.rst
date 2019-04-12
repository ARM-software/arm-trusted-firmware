PSCI Library Integration guide for Armv8-A AArch32 systems
==========================================================

This document describes the PSCI library interface with a focus on how to
integrate with a suitable Trusted OS for an Armv8-A AArch32 system. The PSCI
Library implements the PSCI Standard as described in `PSCI spec`_ and is meant
to be integrated with EL3 Runtime Software which invokes the PSCI Library
interface appropriately. **EL3 Runtime Software** refers to software executing
at the highest secure privileged mode, which is EL3 in AArch64 or Secure SVC/
Monitor mode in AArch32, and provides runtime services to the non-secure world.
The runtime service request is made via SMC (Secure Monitor Call) and the call
must adhere to `SMCCC`_. In AArch32, EL3 Runtime Software may additionally
include Trusted OS functionality. A minimal AArch32 Secure Payload, SP-MIN, is
provided in Trusted Firmware-A (TF-A) to illustrate the usage and integration
of the PSCI library. The description of PSCI library interface and its
integration with EL3 Runtime Software in this document is targeted towards
AArch32 systems.

Generic call sequence for PSCI Library interface (AArch32)
----------------------------------------------------------

The generic call sequence of PSCI Library interfaces (see
`PSCI Library Interface`_) during cold boot in AArch32
system is described below:

#. After cold reset, the EL3 Runtime Software performs its cold boot
   initialization including the PSCI library pre-requisites mentioned in
   `PSCI Library Interface`_, and also the necessary platform
   setup.

#. Call ``psci_setup()`` in Monitor mode.

#. Optionally call ``psci_register_spd_pm_hook()`` to register callbacks to
   do bookkeeping for the EL3 Runtime Software during power management.

#. Call ``psci_prepare_next_non_secure_ctx()`` to initialize the non-secure CPU
   context.

#. Get the non-secure ``cpu_context_t`` for the current CPU by calling
   ``cm_get_context()`` , then programming the registers in the non-secure
   context and exiting to non-secure world. If the EL3 Runtime Software needs
   additional configuration to be set for non-secure context, like routing
   FIQs to the secure world, the values of the registers can be modified prior
   to programming. See `PSCI CPU context management`_ for more
   details on CPU context management.

The generic call sequence of PSCI library interfaces during warm boot in
AArch32 systems is described below:

#. After warm reset, the EL3 Runtime Software performs the necessary warm
   boot initialization including the PSCI library pre-requisites mentioned in
   `PSCI Library Interface`_ (Note that the Data cache
   **must not** be enabled).

#. Call ``psci_warmboot_entrypoint()`` in Monitor mode. This interface
   initializes/restores the non-secure CPU context as well.

#. Do step 5 of the cold boot call sequence described above.

The generic call sequence of PSCI library interfaces on receipt of a PSCI SMC
on an AArch32 system is described below:

#. On receipt of an SMC, save the register context as per `SMCCC`_.

#. If the SMC function identifier corresponds to a SMC32 PSCI API, construct
   the appropriate arguments and call the ``psci_smc_handler()`` interface.
   The invocation may or may not return back to the caller depending on
   whether the PSCI API resulted in power down of the CPU.

#. If ``psci_smc_handler()`` returns, populate the return value in R0 (AArch32)/
   X0 (AArch64) and restore other registers as per `SMCCC`_.

PSCI CPU context management
---------------------------

PSCI library is in charge of initializing/restoring the non-secure CPU system
registers according to `PSCI specification`_ during cold/warm boot.
This is referred to as ``PSCI CPU Context Management``. Registers that need to
be preserved across CPU power down/power up cycles are maintained in
``cpu_context_t`` data structure. The initialization of other non-secure CPU
system registers which do not require coordination with the EL3 Runtime
Software is done directly by the PSCI library (see ``cm_prepare_el3_exit()``).

The EL3 Runtime Software is responsible for managing register context
during switch between Normal and Secure worlds. The register context to be
saved and restored depends on the mechanism used to trigger the world switch.
For example, if the world switch was triggered by an SMC call, then the
registers need to be saved and restored according to `SMCCC`_. In AArch64,
due to the tight integration with BL31, both BL31 and PSCI library
use the same ``cpu_context_t`` data structure for PSCI CPU context management
and register context management during world switch. This cannot be assumed
for AArch32 EL3 Runtime Software since most AArch32 Trusted OSes already implement
a mechanism for register context management during world switch. Hence, when
the PSCI library is integrated with a AArch32 EL3 Runtime Software, the
``cpu_context_t`` is stripped down for just PSCI CPU context management.

During cold/warm boot, after invoking appropriate PSCI library interfaces, it
is expected that the EL3 Runtime Software will query the ``cpu_context_t`` and
write appropriate values to the corresponding system registers. This mechanism
resolves 2 additional problems for AArch32 EL3 Runtime Software:

#. Values for certain system registers like SCR and SCTLR cannot be
   unilaterally determined by PSCI library and need inputs from the EL3
   Runtime Software. Using ``cpu_context_t`` as an intermediary data store
   allows EL3 Runtime Software to modify the register values appropriately
   before programming them.

#. The PSCI library provides appropriate LR and SPSR values (entrypoint
   information) for exit into non-secure world. Using ``cpu_context_t`` as an
   intermediary data store allows the EL3 Runtime Software to store these
   values safely until it is ready for exit to non-secure world.

Currently the ``cpu_context_t`` data structure for AArch32 stores the following
registers: R0 - R3, LR (R14), SCR, SPSR, SCTLR.

The EL3 Runtime Software must implement accessors to get/set pointers
to CPU context ``cpu_context_t`` data and these are described in
`CPU Context management API`_.

PSCI Library Interface
----------------------

The PSCI library implements the `PSCI Specification`_. The interfaces
to this library are declared in ``psci_lib.h`` and are as listed below:

.. code:: c

        u_register_t psci_smc_handler(uint32_t smc_fid, u_register_t x1,
                                      u_register_t x2, u_register_t x3,
                                      u_register_t x4, void *cookie,
                                      void *handle, u_register_t flags);
        int psci_setup(const psci_lib_args_t *lib_args);
        void psci_warmboot_entrypoint(void);
        void psci_register_spd_pm_hook(const spd_pm_ops_t *pm);
        void psci_prepare_next_non_secure_ctx(entry_point_info_t *next_image_info);

The CPU context data 'cpu_context_t' is programmed to the registers differently
when PSCI is integrated with an AArch32 EL3 Runtime Software compared to
when the PSCI is integrated with an AArch64 EL3 Runtime Software (BL31). For
example, in the case of AArch64, there is no need to retrieve ``cpu_context_t``
data and program the registers as it will done implicitly as part of
``el3_exit``. The description below of the PSCI interfaces is targeted at
integration with an AArch32 EL3 Runtime Software.

The PSCI library is responsible for initializing/restoring the non-secure world
to an appropriate state after boot and may choose to directly program the
non-secure system registers. The PSCI generic code takes care not to directly
modify any of the system registers affecting the secure world and instead
returns the values to be programmed to these registers via ``cpu_context_t``.
The EL3 Runtime Software is responsible for programming those registers and
can use the proposed values provided in the ``cpu_context_t``, modifying the
values if required.

PSCI library needs the flexibility to access both secure and non-secure
copies of banked registers. Hence it needs to be invoked in Monitor mode
for AArch32 and in EL3 for AArch64. The NS bit in SCR (in AArch32) or SCR_EL3
(in AArch64) must be set to 0. Additional requirements for the PSCI library
interfaces are:

-  Instruction cache must be enabled
-  Both IRQ and FIQ must be masked for the current CPU
-  The page tables must be setup and the MMU enabled
-  The C runtime environment must be setup and stack initialized
-  The Data cache must be enabled prior to invoking any of the PSCI library
   interfaces except for ``psci_warmboot_entrypoint()``. For
   ``psci_warmboot_entrypoint()``, if the build option ``HW_ASSISTED_COHERENCY``
   is enabled however, data caches are expected to be enabled.

Further requirements for each interface can be found in the interface
description.

Interface : psci_setup()
~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : const psci_lib_args_t *lib_args
    Return   : void

This function is to be called by the primary CPU during cold boot before
any other interface to the PSCI library. It takes ``lib_args``, a const pointer
to ``psci_lib_args_t``, as the argument. The ``psci_lib_args_t`` is a versioned
structure and is declared in ``psci_lib.h`` header as follows:

.. code:: c

        typedef struct psci_lib_args {
            /* The version information of PSCI Library Interface */
            param_header_t        h;
            /* The warm boot entrypoint function */
            mailbox_entrypoint_t  mailbox_ep;
        } psci_lib_args_t;

The first field ``h``, of ``param_header_t`` type, provides the version
information. The second field ``mailbox_ep`` is the warm boot entrypoint address
and is used to configure the platform mailbox. Helper macros are provided in
``psci_lib.h`` to construct the ``lib_args`` argument statically or during
runtime. Prior to calling the ``psci_setup()`` interface, the platform setup for
cold boot must have completed. Major actions performed by this interface are:

-  Initializes architecture.
-  Initializes PSCI power domain and state coordination data structures.
-  Calls ``plat_setup_psci_ops()`` with warm boot entrypoint ``mailbox_ep`` as
   argument.
-  Calls ``cm_set_context_by_index()`` (see
   `CPU Context management API`_) for all the CPUs in the
   platform

Interface : psci_prepare_next_non_secure_ctx()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : entry_point_info_t *next_image_info
    Return   : void

After ``psci_setup()`` and prior to exit to the non-secure world, this function
must be called by the EL3 Runtime Software to initialize the non-secure world
context. The non-secure world entrypoint information ``next_image_info`` (first
argument) will be used to determine the non-secure context. After this function
returns, the EL3 Runtime Software must retrieve the ``cpu_context_t`` (using
cm_get_context()) for the current CPU and program the registers prior to exit
to the non-secure world.

Interface : psci_register_spd_pm_hook()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : const spd_pm_ops_t *
    Return   : void

As explained in `Secure payload power management callback`_,
the EL3 Runtime Software may want to perform some bookkeeping during power
management operations. This function is used to register the ``spd_pm_ops_t``
(first argument) callbacks with the PSCI library which will be called
appropriately during power management. Calling this function is optional and
need to be called by the primary CPU during the cold boot sequence after
``psci_setup()`` has completed.

Interface : psci_smc_handler()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : uint32_t smc_fid, u_register_t x1,
               u_register_t x2, u_register_t x3,
               u_register_t x4, void *cookie,
               void *handle, u_register_t flags
    Return   : u_register_t

This function is the top level handler for SMCs which fall within the
PSCI service range specified in `SMCCC`_. The function ID ``smc_fid`` (first
argument) determines the PSCI API to be called. The ``x1`` to ``x4`` (2nd to 5th
arguments), are the values of the registers r1 - r4 (in AArch32) or x1 - x4
(in AArch64) when the SMC is received. These are the arguments to PSCI API as
described in `PSCI spec`_. The 'flags' (8th argument) is a bit field parameter
and is detailed in 'smccc.h' header. It includes whether the call is from the
secure or non-secure world. The ``cookie`` (6th argument) and the ``handle``
(7th argument) are not used and are reserved for future use.

The return value from this interface is the return value from the underlying
PSCI API corresponding to ``smc_fid``. This function may not return back to the
caller if PSCI API causes power down of the CPU. In this case, when the CPU
wakes up, it will start execution from the warm reset address.

Interface : psci_warmboot_entrypoint()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function performs the warm boot initialization/restoration as mandated by
`PSCI spec`_. For AArch32, on wakeup from power down the CPU resets to secure SVC
mode and the EL3 Runtime Software must perform the prerequisite initializations
mentioned at top of this section. This function must be called with Data cache
disabled (unless build option ``HW_ASSISTED_COHERENCY`` is enabled) but with MMU
initialized and enabled. The major actions performed by this function are:

-  Invalidates the stack and enables the data cache.
-  Initializes architecture and PSCI state coordination.
-  Restores/Initializes the peripheral drivers to the required state via
   appropriate ``plat_psci_ops_t`` hooks
-  Restores the EL3 Runtime Software context via appropriate ``spd_pm_ops_t``
   callbacks.
-  Restores/Initializes the non-secure context and populates the
   ``cpu_context_t`` for the current CPU.

Upon the return of this function, the EL3 Runtime Software must retrieve the
non-secure ``cpu_context_t`` using ``cm_get_context()`` and program the registers
prior to exit to the non-secure world.

EL3 Runtime Software dependencies
---------------------------------

The PSCI Library includes supporting frameworks like context management,
cpu operations (cpu_ops) and per-cpu data framework. Other helper library
functions like bakery locks and spin locks are also included in the library.
The dependencies which must be fulfilled by the EL3 Runtime Software
for integration with PSCI library are described below.

General dependencies
~~~~~~~~~~~~~~~~~~~~

The PSCI library being a Multiprocessor (MP) implementation, EL3 Runtime
Software must provide an SMC handling framework capable of MP adhering to
`SMCCC`_ specification.

The EL3 Runtime Software must also export cache maintenance primitives
and some helper utilities for assert, print and memory operations as listed
below. The TF-A source tree provides implementations for all
these functions but the EL3 Runtime Software may use its own implementation.

**Functions : assert(), memcpy(), memset(), printf()**

These must be implemented as described in ISO C Standard.

**Function : flush_dcache_range()**

::

    Argument : uintptr_t addr, size_t size
    Return   : void

This function cleans and invalidates (flushes) the data cache for memory
at address ``addr`` (first argument) address and of size ``size`` (second argument).

**Function : inv_dcache_range()**

::

    Argument : uintptr_t addr, size_t size
    Return   : void

This function invalidates (flushes) the data cache for memory at address
``addr`` (first argument) address and of size ``size`` (second argument).

**Function : do_panic()**

::

    Argument : void
    Return   : void

This function will be called by the PSCI library on encountering a critical
failure that cannot be recovered from. This function **must not** return.

CPU Context management API
~~~~~~~~~~~~~~~~~~~~~~~~~~

The CPU context management data memory is statically allocated by PSCI library
in BSS section. The PSCI library requires the EL3 Runtime Software to implement
APIs to store and retrieve pointers to this CPU context data. SP-MIN
demonstrates how these APIs can be implemented but the EL3 Runtime Software can
choose a more optimal implementation (like dedicating the secure TPIDRPRW
system register (in AArch32) for storing these pointers).

**Function : cm_set_context_by_index()**

::

    Argument : unsigned int cpu_idx, void *context, unsigned int security_state
    Return   : void

This function is called during cold boot when the ``psci_setup()`` PSCI library
interface is called.

This function must store the pointer to the CPU context data, ``context`` (2nd
argument), for the specified ``security_state`` (3rd argument) and CPU identified
by ``cpu_idx`` (first argument). The ``security_state`` will always be non-secure
when called by PSCI library and this argument is retained for compatibility
with BL31. The ``cpu_idx`` will correspond to the index returned by the
``plat_core_pos_by_mpidr()`` for ``mpidr`` of the CPU.

The actual method of storing the ``context`` pointers is implementation specific.
For example, SP-MIN stores the pointers in the array ``sp_min_cpu_ctx_ptr``
declared in ``sp_min_main.c``.

**Function : cm_get_context()**

::

    Argument : uint32_t security_state
    Return   : void *

This function must return the pointer to the ``cpu_context_t`` structure for
the specified ``security_state`` (first argument) for the current CPU. The caller
must ensure that ``cm_set_context_by_index`` is called first and the appropriate
context pointers are stored prior to invoking this API. The ``security_state``
will always be non-secure when called by PSCI library and this argument
is retained for compatibility with BL31.

**Function : cm_get_context_by_index()**

::

    Argument : unsigned int cpu_idx, unsigned int security_state
    Return   : void *

This function must return the pointer to the ``cpu_context_t`` structure for
the specified ``security_state`` (second argument) for the CPU identified by
``cpu_idx`` (first argument). The caller must ensure that
``cm_set_context_by_index`` is called first and the appropriate context
pointers are stored prior to invoking this API. The ``security_state`` will
always be non-secure when called by PSCI library and this argument is
retained for compatibility with BL31. The ``cpu_idx`` will correspond to the
index returned by the ``plat_core_pos_by_mpidr()`` for ``mpidr`` of the CPU.

Platform API
~~~~~~~~~~~~

The platform layer abstracts the platform-specific details from the generic
PSCI library. The following platform APIs/macros must be defined by the EL3
Runtime Software for integration with the PSCI library.

The mandatory platform APIs are:

-  plat_my_core_pos
-  plat_core_pos_by_mpidr
-  plat_get_syscnt_freq2
-  plat_get_power_domain_tree_desc
-  plat_setup_psci_ops
-  plat_reset_handler
-  plat_panic_handler
-  plat_get_my_stack

The mandatory platform macros are:

-  PLATFORM_CORE_COUNT
-  PLAT_MAX_PWR_LVL
-  PLAT_NUM_PWR_DOMAINS
-  CACHE_WRITEBACK_GRANULE
-  PLAT_MAX_OFF_STATE
-  PLAT_MAX_RET_STATE
-  PLAT_MAX_PWR_LVL_STATES (optional)
-  PLAT_PCPU_DATA_SIZE (optional)

The details of these APIs/macros can be found in the :ref:`Porting Guide`.

All platform specific operations for power management are done via
``plat_psci_ops_t`` callbacks registered by the platform when
``plat_setup_psci_ops()`` API is called. The description of each of
the callbacks in ``plat_psci_ops_t`` can be found in PSCI section of the
:ref:`Porting Guide`. If any these callbacks are not registered, then the
PSCI API associated with that callback will not be supported by PSCI
library.

Secure payload power management callback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

During PSCI power management operations, the EL3 Runtime Software may
need to perform some bookkeeping, and PSCI library provides
``spd_pm_ops_t`` callbacks for this purpose. These hooks must be
populated and registered by using ``psci_register_spd_pm_hook()`` PSCI
library interface.

Typical bookkeeping during PSCI power management calls include save/restore
of the EL3 Runtime Software context. Also if the EL3 Runtime Software makes
use of secure interrupts, then these interrupts must also be managed
appropriately during CPU power down/power up. Any secure interrupt targeted
to the current CPU must be disabled or re-targeted to other running CPU prior
to power down of the current CPU. During power up, these interrupt can be
enabled/re-targeted back to the current CPU.

.. code:: c

        typedef struct spd_pm_ops {
                void (*svc_on)(u_register_t target_cpu);
                int32_t (*svc_off)(u_register_t __unused);
                void (*svc_suspend)(u_register_t max_off_pwrlvl);
                void (*svc_on_finish)(u_register_t __unused);
                void (*svc_suspend_finish)(u_register_t max_off_pwrlvl);
                int32_t (*svc_migrate)(u_register_t from_cpu, u_register_t to_cpu);
                int32_t (*svc_migrate_info)(u_register_t *resident_cpu);
                void (*svc_system_off)(void);
                void (*svc_system_reset)(void);
        } spd_pm_ops_t;

A brief description of each callback is given below:

-  svc_on, svc_off, svc_on_finish

   The ``svc_on``, ``svc_off`` callbacks are called during PSCI_CPU_ON,
   PSCI_CPU_OFF APIs respectively. The ``svc_on_finish`` is called when the
   target CPU of PSCI_CPU_ON API powers up and executes the
   ``psci_warmboot_entrypoint()`` PSCI library interface.

-  svc_suspend, svc_suspend_finish

   The ``svc_suspend`` callback is called during power down bu either
   PSCI_SUSPEND or PSCI_SYSTEM_SUSPEND APIs. The ``svc_suspend_finish`` is
   called when the CPU wakes up from suspend and executes the
   ``psci_warmboot_entrypoint()`` PSCI library interface. The ``max_off_pwrlvl``
   (first parameter) denotes the highest power domain level being powered down
   to or woken up from suspend.

-  svc_system_off, svc_system_reset

   These callbacks are called during PSCI_SYSTEM_OFF and PSCI_SYSTEM_RESET
   PSCI APIs respectively.

-  svc_migrate_info

   This callback is called in response to PSCI_MIGRATE_INFO_TYPE or
   PSCI_MIGRATE_INFO_UP_CPU APIs. The return value of this callback must
   correspond to the return value of PSCI_MIGRATE_INFO_TYPE API as described
   in `PSCI spec`_. If the secure payload is a Uniprocessor (UP)
   implementation, then it must update the mpidr of the CPU it is resident in
   via ``resident_cpu`` (first argument). The updates to ``resident_cpu`` is
   ignored if the secure payload is a multiprocessor (MP) implementation.

-  svc_migrate

   This callback is only relevant if the secure payload in EL3 Runtime
   Software is a Uniprocessor (UP) implementation and supports migration from
   the current CPU ``from_cpu`` (first argument) to another CPU ``to_cpu``
   (second argument). This callback is called in response to PSCI_MIGRATE
   API. This callback is never called if the secure payload is a
   Multiprocessor (MP) implementation.

CPU operations
~~~~~~~~~~~~~~

The CPU operations (cpu_ops) framework implement power down sequence specific
to the CPU and the details of which can be found at
:ref:`firmware_design_cpu_ops_fwk`. The TF-A tree implements the ``cpu_ops``
for various supported CPUs and the EL3 Runtime Software needs to include the
required ``cpu_ops`` in its build. The start and end of the ``cpu_ops``
descriptors must be exported by the EL3 Runtime Software via the
``__CPU_OPS_START__`` and ``__CPU_OPS_END__`` linker symbols.

The ``cpu_ops`` descriptors also include reset sequences and may include errata
workarounds for the CPU. The EL3 Runtime Software can choose to call this
during cold/warm reset if it does not implement its own reset sequence/errata
workarounds.

--------------

*Copyright (c) 2016-2019, Arm Limited and Contributors. All rights reserved.*

.. _PSCI spec: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _SMCCC: https://silver.arm.com/download/ARM_and_AMBA_Architecture/AR570-DA-80002-r0p0-00rel0/ARM_DEN0028A_SMC_Calling_Convention.pdf
.. _PSCI specification: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _PSCI Specification: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
