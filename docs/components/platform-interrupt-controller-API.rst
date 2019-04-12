Platform Interrupt Controller API
=================================

This document lists the optional platform interrupt controller API that
abstracts the runtime configuration and control of interrupt controller from the
generic code. The mandatory APIs are described in the
:ref:`Porting Guide <porting_guide_imf_in_bl31>`.

Function: unsigned int plat_ic_get_running_priority(void); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This API should return the priority of the interrupt the PE is currently
servicing. This must be be called only after an interrupt has already been
acknowledged via ``plat_ic_acknowledge_interrupt``.

In the case of Arm standard platforms using GIC, the *Running Priority Register*
is read to determine the priority of the interrupt.

Function: int plat_ic_is_spi(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

The API should return whether the interrupt ID (first parameter) is categorized
as a Shared Peripheral Interrupt. Shared Peripheral Interrupts are typically
associated to system-wide peripherals, and these interrupts can target any PE in
the system.

Function: int plat_ic_is_ppi(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

The API should return whether the interrupt ID (first parameter) is categorized
as a Private Peripheral Interrupt. Private Peripheral Interrupts are typically
associated with peripherals that are private to each PE. Interrupts from private
peripherals target to that PE only.

Function: int plat_ic_is_sgi(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

The API should return whether the interrupt ID (first parameter) is categorized
as a Software Generated Interrupt. Software Generated Interrupts are raised by
explicit programming by software, and are typically used in inter-PE
communication. Secure SGIs are reserved for use by Secure world software.

Function: unsigned int plat_ic_get_interrupt_active(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This API should return the *active* status of the interrupt ID specified by the
first parameter, ``id``.

In case of Arm standard platforms using GIC, the implementation of the API reads
the GIC *Set Active Register* to read and return the active status of the
interrupt.

Function: void plat_ic_enable_interrupt(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should enable the interrupt ID specified by the first parameter,
``id``. PEs in the system are expected to receive only enabled interrupts.

In case of Arm standard platforms using GIC, the implementation of the API
inserts barrier to make memory updates visible before enabling interrupt, and
then writes to GIC *Set Enable Register* to enable the interrupt.

Function: void plat_ic_disable_interrupt(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should disable the interrupt ID specified by the first parameter,
``id``. PEs in the system are not expected to receive disabled interrupts.

In case of Arm standard platforms using GIC, the implementation of the API
writes to GIC *Clear Enable Register* to disable the interrupt, and inserts
barrier to make memory updates visible afterwards.

Function: void plat_ic_set_interrupt_priority(unsigned int id, unsigned int priority); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Argument : unsigned int
    Return   : void

This API should set the priority of the interrupt specified by first parameter
``id`` to the value set by the second parameter ``priority``.

In case of Arm standard platforms using GIC, the implementation of the API
writes to GIC *Priority Register* set interrupt priority.

Function: int plat_ic_has_interrupt_type(unsigned int type); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This API should return whether the platform supports a given interrupt type. The
parameter ``type`` shall be one of ``INTR_TYPE_EL3``, ``INTR_TYPE_S_EL1``, or
``INTR_TYPE_NS``.

In case of Arm standard platforms using GICv3, the implementation of the API
returns ``1`` for all interrupt types.

In case of Arm standard platforms using GICv2, the API always return ``1`` for
``INTR_TYPE_NS``. Return value for other types depends on the value of build
option ``GICV2_G0_FOR_EL3``:

- For interrupt type ``INTR_TYPE_EL3``:

  - When ``GICV2_G0_FOR_EL3`` is ``0``, it returns ``0``, indicating no support
    for EL3 interrupts.

  - When ``GICV2_G0_FOR_EL3`` is ``1``, it returns ``1``, indicating support for
    EL3 interrupts.

- For interrupt type ``INTR_TYPE_S_EL1``:

  - When ``GICV2_G0_FOR_EL3`` is ``0``, it returns ``1``, indicating support for
    Secure EL1 interrupts.

  - When ``GICV2_G0_FOR_EL3`` is ``1``, it returns ``0``, indicating no support
    for Secure EL1 interrupts.

Function: void plat_ic_set_interrupt_type(unsigned int id, unsigned int type); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Argument : unsigned int
    Return   : void

This API should set the interrupt specified by first parameter ``id`` to the
type specified by second parameter ``type``. The ``type`` parameter can be
one of:

- ``INTR_TYPE_NS``: interrupt is meant to be consumed by the Non-secure world.

- ``INTR_TYPE_S_EL1``: interrupt is meant to be consumed by Secure EL1.

- ``INTR_TYPE_EL3``: interrupt is meant to be consumed by EL3.

In case of Arm standard platforms using GIC, the implementation of the API
writes to the GIC *Group Register* and *Group Modifier Register* (only GICv3) to
assign the interrupt to the right group.

For GICv3:

- ``INTR_TYPE_NS`` maps to Group 1 interrupt.

- ``INTR_TYPE_S_EL1`` maps to Secure Group 1 interrupt.

- ``INTR_TYPE_EL3`` maps to Secure Group 0 interrupt.

For GICv2:

- ``INTR_TYPE_NS`` maps to Group 1 interrupt.

- When the build option ``GICV2_G0_FOR_EL3`` is set to ``0`` (the default),
  ``INTR_TYPE_S_EL1`` maps to Group 0. Otherwise, ``INTR_TYPE_EL3`` maps to
  Group 0 interrupt.

Function: void plat_ic_raise_el3_sgi(int sgi_num, u_register_t target); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Argument : u_register_t
    Return   : void

This API should raise an EL3 SGI. The first parameter, ``sgi_num``, specifies
the ID of the SGI. The second parameter, ``target``, must be the MPIDR of the
target PE.

In case of Arm standard platforms using GIC, the implementation of the API
inserts barrier to make memory updates visible before raising SGI, then writes
to appropriate *SGI Register* in order to raise the EL3 SGI.

Function: void plat_ic_set_spi_routing(unsigned int id, unsigned int routing_mode, u_register_t mpidr); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Argument : unsigned int
    Argument : u_register_t
    Return   : void

This API should set the routing mode of Share Peripheral Interrupt (SPI)
specified by first parameter ``id`` to that specified by the second parameter
``routing_mode``.

The ``routing_mode`` parameter can be one of:

- ``INTR_ROUTING_MODE_ANY`` means the interrupt can be routed to any PE in the
  system. The ``mpidr`` parameter is ignored in this case.

- ``INTR_ROUTING_MODE_PE`` means the interrupt is routed to the PE whose MPIDR
  value is specified by the parameter ``mpidr``.

In case of Arm standard platforms using GIC, the implementation of the API
writes to the GIC *Target Register* (GICv2) or *Route Register* (GICv3) to set
the routing.

Function: void plat_ic_set_interrupt_pending(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should set the interrupt specified by first parameter ``id`` to
*Pending*.

In case of Arm standard platforms using GIC, the implementation of the API
inserts barrier to make memory updates visible before setting interrupt pending,
and writes to the GIC *Set Pending Register* to set the interrupt pending
status.

Function: void plat_ic_clear_interrupt_pending(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should clear the *Pending* status of the interrupt specified by first
parameter ``id``.

In case of Arm standard platforms using GIC, the implementation of the API
writes to the GIC *Clear Pending Register* to clear the interrupt pending
status, and inserts barrier to make memory updates visible afterwards.

Function: unsigned int plat_ic_set_priority_mask(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : int

This API should set the priority mask (first parameter) in the interrupt
controller such that only interrupts of higher priority than the supplied one
may be signalled to the PE. The API should return the current priority value
that it's overwriting.

In case of Arm standard platforms using GIC, the implementation of the API
inserts to order memory updates before updating mask, then writes to the GIC
*Priority Mask Register*, and make sure memory updates are visible before
potential trigger due to mask update.

Function: unsigned int plat_ic_get_interrupt_id(unsigned int raw); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : unsigned int

This API should extract and return the interrupt number from the raw value
obtained by the acknowledging the interrupt (read using
``plat_ic_acknowledge_interrupt()``). If the interrupt ID is invalid, this API
should return ``INTR_ID_UNAVAILABLE``.

In case of Arm standard platforms using GIC, the implementation of the API
masks out the interrupt ID field from the acknowledged value from GIC.

--------------

*Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.*
