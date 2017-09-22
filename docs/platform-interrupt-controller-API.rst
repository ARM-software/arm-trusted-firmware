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

----

*Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.*
