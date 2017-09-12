Guide to migrate to new Platform porting interface
==================================================


.. section-numbering::
    :suffix: .

.. contents::

--------------

Introduction
------------

The PSCI implementation in Trusted Firmware has undergone a redesign because of
three requirements that the PSCI 1.0 specification introduced :

-  Removing the framework assumption about the structure of the MPIDR, and
   its relation to the power topology enables support for deeper and more
   complex hierarchies.

-  Reworking the power state coordination implementation in the framework
   to support the more detailed PSCI 1.0 requirements and reduce platform
   port complexity

-  Enable the use of the extended power\_state parameter and the larger StateID
   field

The PSCI 1.0 implementation introduces new frameworks to fulfill the above
requirements. These framework changes mean that the platform porting API must
also be modified. This document is a guide to assist migration of the existing
platform ports to the new platform API.

This document describes the new platform API and compares it with the
deprecated API. It also describes the compatibility layer that enables the
existing platform ports to work with the PSCI 1.0 implementation. The
deprecated platform API is documented for reference.

Platform API modification due to PSCI framework changes
-------------------------------------------------------

This section describes changes to the platform APIs.

Power domain topology framework platform API modifications
----------------------------------------------------------

This removes the assumption in the PSCI implementation that MPIDR
based affinity instances map directly to power domains. A power domain, as
described in section 4.2 of `PSCI`_, could contain a core or a logical group
of cores (a cluster) which share some state on which power management
operations can be performed. The existing affinity instance based APIs
``plat_get_aff_count()`` and ``plat_get_aff_state()`` are deprecated. The new
platform interfaces that are introduced for this framework are:

-  ``plat_core_pos_by_mpidr()``
-  ``plat_my_core_pos()``
-  ``plat_get_power_domain_tree_desc()``

``plat_my_core_pos()`` and ``plat_core_pos_by_mpidr()`` are mandatory
and are meant to replace the existing ``platform_get_core_pos()`` API.
The description of these APIs can be found in the `Porting Guide`_.
These are used by the power domain topology framework such that:

#. The generic PSCI code does not generate MPIDRs or use them to query the
   platform about the number of power domains at a particular power level. The
   ``plat_get_power_domain_tree_desc()`` provides a description of the power
   domain tree on the SoC through a pointer to the byte array containing the
   power domain topology tree description data structure.

#. The linear indices returned by ``plat_core_pos_by_mpidr()`` and
   ``plat_my_core_pos()`` are used to retrieve core power domain nodes from
   the power domain tree. These core indices are unique for a core and it is a
   number between ``0`` and ``PLATFORM_CORE_COUNT - 1``. The platform can choose
   to implement a static mapping between ``MPIDR`` and core index or implement
   a dynamic mapping, choosing to skip the unavailable/unused cores to compact
   the core indices.

In addition, the platforms must define the macros ``PLAT_NUM_PWR_DOMAINS`` and
``PLAT_MAX_PWR_LVL`` which replace the macros ``PLAT_NUM_AFFS`` and
``PLATFORM_MAX_AFFLVL`` respectively. On platforms where the affinity instances
correspond to power domains, the values of new macros remain the same as the
old ones.

More details on the power domain topology description and its platform
interface can be found in `psci pd tree`_.

Composite power state framework platform API modifications
----------------------------------------------------------

The state-ID field in the power-state parameter of a CPU\_SUSPEND call can be
used to describe the composite power states specific to a platform. The existing
PSCI state coordination had the limitation that it operates on a run/off
granularity of power states and it did not interpret the state-ID field. This
was acceptable as the specification requirement in PSCI 0.2 and the framework's
approach to coordination only required maintaining a reference
count of the number of cores that have requested the cluster to remain powered.

In the PSCI 1.0 specification, this approach is non optimal. If composite
power states are used, the PSCI implementation cannot make global
decisions about state coordination required because it does not understand the
platform specific states.

The PSCI 1.0 implementation now defines a generic representation of the
power-state parameter :

.. code:: c

    typedef struct psci_power_state {
        plat_local_state_t pwr_domain_state[PLAT_MAX_PWR_LVL + 1];
    } psci_power_state_t;

``pwr_domain_state`` is an array where each index corresponds to a power level.
Each entry in the array contains the local power state the power domain at
that power level could enter. The meaning of the local power state value is
platform defined, and can vary between levels in a single platform. The PSCI
implementation constraints the values only so that it can classify the state
as RUN, RETENTION or OFF as required by the specification:

#. Zero means RUN

#. All OFF state values at all levels must be higher than all
   RETENTION state values at all levels

The platform is required to define the macros ``PLAT_MAX_RET_STATE`` and
``PLAT_MAX_OFF_STATE`` to the framework. The requirement for these macros can
be found in the `Porting Guide <porting-guide.rst>`__.

The PSCI 1.0 implementation adds support to involve the platform in state
coordination. This enables the platform to decide the final target state.
During a request to place a power domain in a low power state, the platform
is passed an array of requested ``plat_local_state_t`` for that power domain by
each core within it through the ``plat_get_target_pwr_state()`` API. This API
coordinates amongst these requested states to determine a target
``plat_local_state_t`` for that power domain. A default weak implementation of
this API is provided in the platform layer which returns the minimum of the
requested local states back to the PSCI state coordination. More details
of ``plat_get_target_pwr_state()`` API can be found in the
`Porting Guide <porting-guide.rst#user-content-function--plat_get_target_pwr_state-optional>`__.

The PSCI Generic implementation expects platform ports to populate the handlers
for the ``plat_psci_ops`` structure which is declared as :

.. code:: c

    typedef struct plat_psci_ops {
        void (*cpu_standby)(plat_local_state_t cpu_state);
        int (*pwr_domain_on)(u_register_t mpidr);
        void (*pwr_domain_off)(const psci_power_state_t *target_state);
        void (*pwr_domain_suspend_early)(const psci_power_state_t *target_state);
        void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
        void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
        void (*pwr_domain_suspend_finish)(
                        const psci_power_state_t *target_state);
        void (*system_off)(void) __dead2;
        void (*system_reset)(void) __dead2;
        int (*validate_power_state)(unsigned int power_state,
                        psci_power_state_t *req_state);
        int (*validate_ns_entrypoint)(unsigned long ns_entrypoint);
        void (*get_sys_suspend_power_state)(
                        psci_power_state_t *req_state);
        int (*get_pwr_lvl_state_idx)(plat_local_state_t pwr_domain_state,
                                    int pwrlvl);
        int (*translate_power_state_by_mpidr)(u_register_t mpidr,
                                    unsigned int power_state,
                                    psci_power_state_t *output_state);
        int (*get_node_hw_state)(u_register_t mpidr, unsigned int power_level);
        int (*mem_protect_chk)(uintptr_t base, u_register_t length);
        int (*read_mem_protect)(int *val);
        int (*write_mem_protect)(int val);
        int (*system_reset2)(int is_vendor,
                                int reset_type, u_register_t cookie);
    } plat_psci_ops_t;

The description of these handlers can be found in the `Porting Guide <porting-guide.rst#user-content-function--plat_setup_psci_ops-mandatory>`__.
The previous ``plat_pm_ops`` structure is deprecated. Compared with the previous
handlers, the major differences are:

-  Difference in parameters

The PSCI 1.0 implementation depends on the ``validate_power_state`` handler to
convert the power-state parameter (possibly encoding a composite power state)
passed in a PSCI ``CPU_SUSPEND`` to the ``psci_power_state`` format. This handler
is now mandatory for PSCI ``CPU_SUSPEND`` support.

The ``plat_psci_ops`` handlers, ``pwr_domain_off``, ``pwr_domain_suspend_early``
and ``pwr_domain_suspend``, are passed the target local state for each affected
power domain. The platform must execute operations specific to these target
states. Similarly, ``pwr_domain_on_finish`` and ``pwr_domain_suspend_finish``
are passed the local states of the affected power domains before wakeup. The
platform must execute actions to restore these power domains from these specific
local states.

-  Difference in invocation

Whereas the power management handlers in ``plat_pm_ops`` used to be invoked
for each affinity level till the target affinity level, the new handlers
are only invoked once. The ``target_state`` encodes the target low power
state or the low power state woken up from for each affected power domain.

-  Difference in semantics

Although the previous ``suspend`` handlers could be used for power down as well
as retention at different affinity levels, the new handlers make this support
explicit. The ``pwr_domain_suspend`` can be used to specify powerdown and
retention at various power domain levels subject to the conditions mentioned
in section 4.2.1 of `PSCI`_

Unlike the previous ``standby`` handler, the ``cpu_standby()`` handler is only used
as a fast path for placing a core power domain into a standby or retention
state.

The below diagram shows the sequence of a PSCI SUSPEND call and the interaction
with the platform layer depicting the exchange of data between PSCI Generic
layer and the platform layer.

|Image 1|

Refer `plat/arm/board/fvp/fvp\_pm.c`_ for the implementation details of
these handlers for the FVP. The commit `38dce70f51fb83b27958ba3e2ad15f5635cb1061`_
demonstrates the migration of ARM reference platforms to the new platform API.

Miscellaneous modifications
---------------------------

In addition to the framework changes, unification of warm reset entry points on
wakeup from low power modes has led to a change in the platform API. In the
earlier implementation, the warm reset entry used to be programmed into the
mailboxes by the 'ON' and 'SUSPEND' power management hooks. In the PSCI 1.0
implementation, this information is not required, because it can figure that
out by querying affinity info state whether to execute the 'suspend\_finisher\`
or 'on\_finisher'.

As a result, the warm reset entry point must be programmed only once. The
``plat_setup_psci_ops()`` API takes the secure entry point as an
additional parameter to enable the platforms to configure their mailbox. The
plat\_psci\_ops handlers ``pwr_domain_on`` and ``pwr_domain_suspend`` no longer take
the warm reset entry point as a parameter.

Also, some platform APIs which took ``MPIDR`` as an argument were only ever
invoked to perform actions specific to the caller core which makes the argument
redundant. Therefore the platform APIs ``plat_get_my_entrypoint()``,
``plat_is_my_cpu_primary()``, ``plat_set_my_stack()`` and
``plat_get_my_stack()`` are defined which are meant to be invoked only for
operations on the current caller core instead of ``platform_get_entrypoint()``,
``platform_is_primary_cpu()``, ``platform_set_stack()`` and ``platform_get_stack()``.

Compatibility layer
-------------------

To ease the migration of the platform ports to the new porting interface,
a compatibility layer is introduced that essentially implements a glue layer
between the old platform API and the new API. The build flag
``ENABLE_PLAT_COMPAT`` (enabled by default), specifies whether to enable this
layer or not. A platform port which has migrated to the new API can disable
this flag within the platform specific makefile.

The compatibility layer works on the assumption that the onus of
state coordination, in case multiple low power states are supported,
is with the platform. The generic PSCI implementation only takes into
account whether the suspend request is power down or not. This corresponds
with the behavior of the PSCI implementation before the introduction of
new frameworks. Also, it assumes that the affinity levels of the platform
correspond directly to the power domain levels.

The compatibility layer dynamically constructs the new topology
description array by querying the platform using ``plat_get_aff_count()``
and ``plat_get_aff_state()`` APIs. The linear index returned by
``platform_get_core_pos()`` is used as the core index for the cores. The
higher level (non-core) power domain nodes must know the cores contained
within its domain. It does so by storing the core index of first core
within it and number of core indexes following it. This means that core
indices returned by ``platform_get_core_pos()`` for cores within a particular
power domain must be consecutive. We expect that this is the case for most
platform ports including ARM reference platforms.

The old PSCI helpers like ``psci_get_suspend_powerstate()``,
``psci_get_suspend_stateid()``, ``psci_get_suspend_stateid_by_mpidr()``,
``psci_get_max_phys_off_afflvl()`` and ``psci_get_suspend_afflvl()`` are also
implemented for the compatibility layer. This allows the existing
platform ports to work with the new PSCI frameworks without significant
rework.

Deprecated Platform API
-----------------------

This section documents the deprecated platform porting API.

Common mandatory modifications
------------------------------

The mandatory macros to be defined by the platform port in ``platform_def.h``

-  **#define : PLATFORM\_NUM\_AFFS**

   Defines the total number of nodes in the affinity hierarchy at all affinity
   levels used by the platform.

-  **#define : PLATFORM\_MAX\_AFFLVL**

   Defines the maximum affinity level that the power management operations
   should apply to. ARMv8-A has support for four affinity levels. It is likely
   that hardware will implement fewer affinity levels. This macro allows the
   PSCI implementation to consider only those affinity levels in the system
   that the platform implements. For example, the Base AEM FVP implements two
   clusters with a configurable number of cores. It reports the maximum
   affinity level as 1, resulting in PSCI power control up to the cluster
   level.

The following functions must be implemented by the platform port to enable
the reset vector code to perform the required tasks.

Function : platform\_get\_entrypoint() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned long
    Return   : unsigned long

This function is called with the ``SCTLR.M`` and ``SCTLR.C`` bits disabled. The core
is identified by its ``MPIDR``, which is passed as the argument. The function is
responsible for distinguishing between a warm and cold reset using platform-
specific means. If it is a warm reset, it returns the entrypoint into the
BL31 image that the core must jump to. If it is a cold reset, this function
must return zero.

This function is also responsible for implementing a platform-specific mechanism
to handle the condition where the core has been warm reset but there is no
entrypoint to jump to.

This function does not follow the Procedure Call Standard used by the
Application Binary Interface for the ARM 64-bit architecture. The caller should
not assume that callee saved registers are preserved across a call to this
function.

Function : platform\_is\_primary\_cpu() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned long
    Return   : unsigned int

This function identifies a core by its ``MPIDR``, which is passed as the argument,
to determine whether this core is the primary core or a secondary core. A return
value of zero indicates that the core is not the primary core, while a non-zero
return value indicates that the core is the primary core.

Common optional modifications
-----------------------------

Function : platform\_get\_core\_pos()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned long
    Return   : int

A platform may need to convert the ``MPIDR`` of a core to an absolute number, which
can be used as a core-specific linear index into blocks of memory (for example
while allocating per-core stacks). This routine contains a simple mechanism
to perform this conversion, using the assumption that each cluster contains a
maximum of four cores:

::

    linear index = cpu_id + (cluster_id * 4)

    cpu_id = 8-bit value in MPIDR at affinity level 0
    cluster_id = 8-bit value in MPIDR at affinity level 1

Function : platform\_set\_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned long
    Return   : void

This function sets the current stack pointer to the normal memory stack that
has been allocated for the core specified by MPIDR. For BL images that only
require a stack for the primary core the parameter is ignored. The size of
the stack allocated to each core is specified by the platform defined constant
``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in `plat/common/aarch64/platform\_up\_stack.S`_ and
`plat/common/aarch64/platform\_mp\_stack.S`_

Function : platform\_get\_stack()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned long
    Return   : unsigned long

This function returns the base address of the normal memory stack that
has been allocated for the core specificed by MPIDR. For BL images that only
require a stack for the primary core the parameter is ignored. The size of
the stack allocated to each core is specified by the platform defined constant
``PLATFORM_STACK_SIZE``.

Common implementations of this function for the UP and MP BL images are
provided in `plat/common/aarch64/platform\_up\_stack.S`_ and
`plat/common/aarch64/platform\_mp\_stack.S`_

Modifications for Power State Coordination Interface (in BL31)
--------------------------------------------------------------

The following functions must be implemented to initialize PSCI functionality in
the ARM Trusted Firmware.

Function : plat\_get\_aff\_count() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int, unsigned long
    Return   : unsigned int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``. It is only
called by the primary core.

This function is called by the PSCI initialization code to detect the system
topology. Its purpose is to return the number of affinity instances implemented
at a given ``affinity level`` (specified by the first argument) and a given
``MPIDR`` (specified by the second argument). For example, on a dual-cluster
system where first cluster implements two cores and the second cluster
implements four cores, a call to this function with an ``MPIDR`` corresponding
to the first cluster (``0x0``) and affinity level 0, would return 2. A call
to this function with an ``MPIDR`` corresponding to the second cluster (``0x100``)
and affinity level 0, would return 4.

Function : plat\_get\_aff\_state() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int, unsigned long
    Return   : unsigned int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``. It is only
called by the primary core.

This function is called by the PSCI initialization code. Its purpose is to
return the state of an affinity instance. The affinity instance is determined by
the affinity ID at a given ``affinity level`` (specified by the first argument)
and an ``MPIDR`` (specified by the second argument). The state can be one of
``PSCI_AFF_PRESENT`` or ``PSCI_AFF_ABSENT``. The latter state is used to cater for
system topologies where certain affinity instances are unimplemented. For
example, consider a platform that implements a single cluster with four cores and
another core implemented directly on the interconnect with the cluster. The
``MPIDR``\ s of the cluster would range from ``0x0-0x3``. The ``MPIDR`` of the single
core is 0x100 to indicate that it does not belong to cluster 0. Cluster 1
is missing but needs to be accounted for to reach this single core in the
topology tree. Therefore it is marked as ``PSCI_AFF_ABSENT``.

Function : platform\_setup\_pm() [mandatory]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : const plat_pm_ops **
    Return   : int

This function may execute with the MMU and data caches enabled if the platform
port does the necessary initializations in ``bl31_plat_arch_setup()``. It is only
called by the primary core.

This function is called by PSCI initialization code. Its purpose is to export
handler routines for platform-specific power management actions by populating
the passed pointer with a pointer to the private ``plat_pm_ops`` structure of
BL31.

A description of each member of this structure is given below. A platform port
is expected to implement these handlers if the corresponding PSCI operation
is to be supported and these handlers are expected to succeed if the return
type is ``void``.

plat\_pm\_ops.affinst\_standby()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Perform the platform-specific setup to enter the standby state indicated by the
passed argument. The generic code expects the handler to succeed.

plat\_pm\_ops.affinst\_on()
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Perform the platform specific setup to power on an affinity instance, specified
by the ``MPIDR`` (first argument) and ``affinity level`` (third argument). The
``state`` (fourth argument) contains the current state of that affinity instance
(ON or OFF). This is useful to determine whether any action must be taken. For
example, while powering on a core, the cluster that contains this core might
already be in the ON state. The platform decides what actions must be taken to
transition from the current state to the target state (indicated by the power
management operation). The generic code expects the platform to return
E\_SUCCESS on success or E\_INTERN\_FAIL for any failure.

plat\_pm\_ops.affinst\_off()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Perform the platform specific setup to power off an affinity instance of the
calling core. It is called by the PSCI ``CPU_OFF`` API implementation.

The ``affinity level`` (first argument) and ``state`` (second argument) have
a similar meaning as described in the ``affinst_on()`` operation. They
identify the affinity instance on which the call is made and its
current state. This gives the platform port an indication of the
state transition it must make to perform the requested action. For example, if
the calling core is the last powered on core in the cluster, after powering down
affinity level 0 (the core), the platform port should power down affinity
level 1 (the cluster) as well. The generic code expects the handler to succeed.

plat\_pm\_ops.affinst\_suspend()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Perform the platform specific setup to power off an affinity instance of the
calling core. It is called by the PSCI ``CPU_SUSPEND`` API and ``SYSTEM_SUSPEND``
API implementation

The ``affinity level`` (second argument) and ``state`` (third argument) have a
similar meaning as described in the ``affinst_on()`` operation. They are used to
identify the affinity instance on which the call is made and its current state.
This gives the platform port an indication of the state transition it must
make to perform the requested action. For example, if the calling core is the
last powered on core in the cluster, after powering down affinity level 0
(the core), the platform port should power down affinity level 1 (the cluster)
as well.

The difference between turning an affinity instance off and suspending it
is that in the former case, the affinity instance is expected to re-initialize
its state when it is next powered on (see ``affinst_on_finish()``). In the latter
case, the affinity instance is expected to save enough state so that it can
resume execution by restoring this state when it is powered on (see
``affinst_suspend_finish()``).The generic code expects the handler to succeed.

plat\_pm\_ops.affinst\_on\_finish()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is called by the PSCI implementation after the calling core is
powered on and released from reset in response to an earlier PSCI ``CPU_ON`` call.
It performs the platform-specific setup required to initialize enough state for
this core to enter the Normal world and also provide secure runtime firmware
services.

The ``affinity level`` (first argument) and ``state`` (second argument) have a
similar meaning as described in the previous operations. The generic code
expects the handler to succeed.

plat\_pm\_ops.affinst\_suspend\_finish()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is called by the PSCI implementation after the calling core is
powered on and released from reset in response to an asynchronous wakeup
event, for example a timer interrupt that was programmed by the core during the
``CPU_SUSPEND`` call or ``SYSTEM_SUSPEND`` call. It performs the platform-specific
setup required to restore the saved state for this core to resume execution
in the Normal world and also provide secure runtime firmware services.

The ``affinity level`` (first argument) and ``state`` (second argument) have a
similar meaning as described in the previous operations. The generic code
expects the platform to succeed.

plat\_pm\_ops.validate\_power\_state()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is called by the PSCI implementation during the ``CPU_SUSPEND``
call to validate the ``power_state`` parameter of the PSCI API. If the
``power_state`` is known to be invalid, the platform must return
PSCI\_E\_INVALID\_PARAMS as an error, which is propagated back to the Normal
world PSCI client.

plat\_pm\_ops.validate\_ns\_entrypoint()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is called by the PSCI implementation during the ``CPU_SUSPEND``,
``SYSTEM_SUSPEND`` and ``CPU_ON`` calls to validate the Non-secure ``entry_point``
parameter passed by the Normal world. If the ``entry_point`` is known to be
invalid, the platform must return PSCI\_E\_INVALID\_PARAMS as an error, which is
propagated back to the Normal world PSCI client.

plat\_pm\_ops.get\_sys\_suspend\_power\_state()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is called by the PSCI implementation during the ``SYSTEM_SUSPEND``
call to return the ``power_state`` parameter. This allows the platform to encode
the appropriate State-ID field within the ``power_state`` parameter which can be
utilized in ``affinst_suspend()`` to suspend to system affinity level. The
``power_state`` parameter should be in the same format as specified by the
PSCI specification for the CPU\_SUSPEND API.

--------------

*Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.*

.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
.. _Porting Guide: porting-guide.rst#user-content-function--plat_my_core_pos
.. _psci pd tree: psci-pd-tree.rst
.. _plat/arm/board/fvp/fvp\_pm.c: ../plat/arm/board/fvp/fvp_pm.c
.. _38dce70f51fb83b27958ba3e2ad15f5635cb1061: https://github.com/ARM-software/arm-trusted-firmware/commit/38dce70f51fb83b27958ba3e2ad15f5635cb1061
.. _plat/common/aarch64/platform\_up\_stack.S: ../plat/common/aarch64/platform_up_stack.S
.. _plat/common/aarch64/platform\_mp\_stack.S: ../plat/common/aarch64/platform_mp_stack.S

.. |Image 1| image:: diagrams/psci-suspend-sequence.png?raw=true
