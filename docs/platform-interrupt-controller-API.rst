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

----

*Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.*
