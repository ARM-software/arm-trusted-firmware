Reliability, Availability, and Serviceability (RAS) Extensions
==============================================================

.. |EHF| replace:: Exception Handling Framework
.. |TF-A| replace:: Trusted Firmware-A

This document describes |TF-A| support for Arm Reliability, Availability, and
Serviceability (RAS) extensions. RAS is a mandatory extension for Armv8.2 and
later CPUs, and also an optional extension to the base Armv8.0 architecture.

In conjunction with the |EHF|, support for RAS extension enables firmware-first
paradigm for handling platform errors: exceptions resulting from errors are
routed to and handled in EL3. Said errors are Synchronous External Abort (SEA),
Asynchronous External Abort (signalled as SErrors), Fault Handling and Error
Recovery interrupts.  The |EHF| document mentions various `error handling
use-cases`__.

.. __: exception-handling.rst#delegation-use-cases

For the description of Arm RAS extensions, Standard Error Records, and the
precise definition of RAS terminology, please refer to the Arm Architecture
Reference Manual. The rest of this document assumes familiarity with
architecture and terminology.

Overview
--------

As mentioned above, the RAS support in |TF-A| enables routing to and handling of
exceptions resulting from platform errors in EL3. It allows the platform to
define an External Abort handler, and to register RAS nodes and interrupts. RAS
framework also provides `helpers`__ for accessing Standard Error Records as
introduced by the RAS extensions.

.. __: `Standard Error Record helpers`_

The build option ``RAS_EXTENSION`` when set to ``1`` includes the RAS in run
time firmware; ``EL3_EXCEPTION_HANDLING`` and ``HANDLE_EA_EL3_FIRST`` must also
be set ``1``.

.. _ras-figure:

.. image:: ../resources/diagrams/draw.io/ras.svg

See more on `Engaging the RAS framework`_.

Platform APIs
-------------

The RAS framework allows the platform to define handlers for External Abort,
Uncontainable Errors, Double Fault, and errors rising from EL3 execution. Please
refer to the porting guide for the `RAS platform API descriptions`__.

.. __: ../getting_started/porting-guide.rst#external-abort-handling-and-ras-support

Registering RAS error records
-----------------------------

RAS nodes are components in the system capable of signalling errors to PEs
through one one of the notification mechanisms—SEAs, SErrors, or interrupts. RAS
nodes contain one or more error records, which are registers through which the
nodes advertise various properties of the signalled error. Arm recommends that
error records are implemented in the Standard Error Record format. The RAS
architecture allows for error records to be accessible via system or
memory-mapped registers.

The platform should enumerate the error records providing for each of them:

-  A handler to probe error records for errors;
-  When the probing identifies an error, a handler to handle it;
-  For memory-mapped error record, its base address and size in KB; for a system
   register-accessed record, the start index of the record and number of
   continuous records from that index;
-  Any node-specific auxiliary data.

With this information supplied, when the run time firmware receives one of the
notification mechanisms, the RAS framework can iterate through and probe error
records for error, and invoke the appropriate handler to handle it.

The RAS framework provides the macros to populate error record information. The
macros are versioned, and the latest version as of this writing is 1. These
macros create a structure of type ``struct err_record_info`` from its arguments,
which are later passed to probe and error handlers.

For memory-mapped error records:

.. code:: c

    ERR_RECORD_MEMMAP_V1(base_addr, size_num_k, probe, handler, aux)

And, for system register ones:

.. code:: c

    ERR_RECORD_SYSREG_V1(idx_start, num_idx, probe, handler, aux)

The probe handler must have the following prototype:

.. code:: c

    typedef int (*err_record_probe_t)(const struct err_record_info *info,
                    int *probe_data);

The probe handler must return a non-zero value if an error was detected, or 0
otherwise. The ``probe_data`` output parameter can be used to pass any useful
information resulting from probe to the error handler (see `below`__). For
example, it could return the index of the record.

.. __: `Standard Error Record helpers`_

The error handler must have the following prototype:

.. code:: c

    typedef int (*err_record_handler_t)(const struct err_record_info *info,
               int probe_data, const struct err_handler_data *const data);

The ``data`` constant parameter describes the various properties of the error,
including the reason for the error, exception syndrome, and also ``flags``,
``cookie``, and ``handle`` parameters from the `top-level exception handler`__.

.. __: interrupt-framework-design.rst#el3-interrupts

The platform is expected populate an array using the macros above, and register
the it with the RAS framework using the macro ``REGISTER_ERR_RECORD_INFO()``,
passing it the name of the array describing the records. Note that the macro
must be used in the same file where the array is defined.

Standard Error Record helpers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The |TF-A| RAS framework provides probe handlers for Standard Error Records, for
both memory-mapped and System Register accesses:

.. code:: c

    int ras_err_ser_probe_memmap(const struct err_record_info *info,
                int *probe_data);

    int ras_err_ser_probe_sysreg(const struct err_record_info *info,
                int *probe_data);

When the platform enumerates error records, for those records in the Standard
Error Record format, these helpers maybe used instead of rolling out their own.
Both helpers above:

-  Return non-zero value when an error is detected in a Standard Error Record;
-  Set ``probe_data`` to the index of the error record upon detecting an error.

Registering RAS interrupts
--------------------------

RAS nodes can signal errors to the PE by raising Fault Handling and/or Error
Recovery interrupts. For the firmware-first handling paradigm for interrupts to
work, the platform must setup and register with |EHF|. See `Interaction with
Exception Handling Framework`_.

For each RAS interrupt, the platform has to provide structure of type ``struct
ras_interrupt``:

-  Interrupt number;
-  The associated error record information (pointer to the corresponding
   ``struct err_record_info``);
-  Optionally, a cookie.

The platform is expected to define an array of ``struct ras_interrupt``, and
register it with the RAS framework using the macro
``REGISTER_RAS_INTERRUPTS()``, passing it the name of the array. Note that the
macro must be used in the same file where the array is defined.

The array of ``struct ras_interrupt`` must be sorted in the increasing order of
interrupt number. This allows for fast look of handlers in order to service RAS
interrupts.

Double-fault handling
---------------------

A Double Fault condition arises when an error is signalled to the PE while
handling of a previously signalled error is still underway. When a Double Fault
condition arises, the Arm RAS extensions only require for handler to perform
orderly shutdown of the system, as recovery may be impossible.

The RAS extensions part of Armv8.4 introduced new architectural features to deal
with Double Fault conditions, specifically, the introduction of ``NMEA`` and
``EASE`` bits to ``SCR_EL3`` register. These were introduced to assist EL3
software which runs part of its entry/exit routines with exceptions momentarily
masked—meaning, in such systems, External Aborts/SErrors are not immediately
handled when they occur, but only after the exceptions are unmasked again.

|TF-A|, for legacy reasons, executes entire EL3 with all exceptions unmasked.
This means that all exceptions routed to EL3 are handled immediately. |TF-A|
thus is able to detect a Double Fault conditions in software, without needing
the intended advantages of Armv8.4 Double Fault architecture extensions.

Double faults are fatal, and terminate at the platform double fault handler, and
doesn't return.

Engaging the RAS framework
--------------------------

Enabling RAS support is a platform choice constructed from three distinct, but
related, build options:

-  ``RAS_EXTENSION=1`` includes the RAS framework in the run time firmware;

-  ``EL3_EXCEPTION_HANDLING=1`` enables handling of exceptions at EL3. See
   `Interaction with Exception Handling Framework`_;

-  ``HANDLE_EA_EL3_FIRST=1`` enables routing of External Aborts and SErrors to
   EL3.

The RAS support in |TF-A| introduces a default implementation of
``plat_ea_handler``, the External Abort handler in EL3. When ``RAS_EXTENSION``
is set to ``1``, it'll first call ``ras_ea_handler()`` function, which is the
top-level RAS exception handler. ``ras_ea_handler`` is responsible for iterating
to through platform-supplied error records, probe them, and when an error is
identified, look up and invoke the corresponding error handler.

Note that, if the platform chooses to override the ``plat_ea_handler`` function
and intend to use the RAS framework, it must explicitly call
``ras_ea_handler()`` from within.

Similarly, for RAS interrupts, the framework defines
``ras_interrupt_handler()``. The RAS framework arranges for it to be invoked
when  a RAS interrupt taken at EL3. The function bisects the platform-supplied
sorted array of interrupts to look up the error record information associated
with the interrupt number. That error handler for that record is then invoked to
handle the error.

Interaction with Exception Handling Framework
---------------------------------------------

As mentioned in earlier sections, RAS framework interacts with the |EHF| to
arbitrate handling of RAS exceptions with others that are routed to EL3. This
means that the platform must partition a `priority level`__ for handling RAS
exceptions. The platform must then define the macro ``PLAT_RAS_PRI`` to the
priority level used for RAS exceptions. Platforms would typically want to
allocate the highest secure priority for RAS handling.

.. __: exception-handling.rst#partitioning-priority-levels

Handling of both `interrupt`__ and `non-interrupt`__ exceptions follow the
sequences outlined in the |EHF| documentation. I.e., for interrupts, the
priority management is implicit; but for non-interrupt exceptions, they're
explicit using `EHF APIs`__.

.. __: exception-handling.rst#interrupt-flow
.. __: exception-handling.rst#non-interrupt-flow
.. __: exception-handling.rst#activating-and-deactivating-priorities

----

*Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.*
