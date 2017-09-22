Platform Interrupt Controller API documentation
===============================================

.. section-numbering::
    :suffix: .

.. contents::

This document lists the optional platform interrupt controller API that
abstracts the runtime configuration and control of interrupt controller from the
generic code. The mandatory APIs are described in the `porting guide`__.

.. __: porting-guide.rst#interrupt-management-framework-in-bl31

Function: unsigned int plat_ic_get_running_priority(void); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : unsigned int

This API should return the priority of the interrupt the PE is currently
servicing. This must be be called only after an interrupt has already been
acknowledged via. ``plat_ic_acknowledge_interrupt``.

In the case of ARM standard platforms using GIC, the *Running Priority Register*
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

In case of ARM standard platforms using GIC, the implementation of the API reads
the GIC *Set Active Register* to read and return the active status of the
interrupt.

Function: void plat_ic_enable_interrupt(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should enable the interrupt ID specified by the first parameter,
``id``. PEs in the system are expected to receive only enabled interrupts.

In case of ARM standard platforms using GIC, the implementation of the API
inserts barrier to make memory updates visible before enabling interrupt, and
then writes to GIC *Set Enable Register* to enable the interrupt.

Function: void plat_ic_disable_interrupt(unsigned int id); [optional]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : unsigned int
    Return   : void

This API should disable the interrupt ID specified by the first parameter,
``id``. PEs in the system are not expected to receive disabled interrupts.

In case of ARM standard platforms using GIC, the implementation of the API
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

In case of ARM standard platforms using GIC, the implementation of the API
writes to GIC *Priority Register* set interrupt priority.

----

*Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.*
