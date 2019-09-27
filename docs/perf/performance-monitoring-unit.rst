Performance Monitoring Unit
===========================

The Performance Monitoring Unit (PMU) allows recording of architectural and
microarchitectural events for profiling purposes.

This document gives an overview of the PMU counter configuration to assist with
implementation and to complement the PMU security guidelines given in the
:ref:`Secure Development Guidelines` document.

.. note::
   This section applies to Armv8-A implementations which have version 3
   of the Performance Monitors Extension (PMUv3).

PMU Counters
------------

The PMU makes 32 counters available at all privilege levels:

-  31 programmable event counters: ``PMEVCNTR<n>``, where ``n`` is ``0`` to
   ``30``.
-  A dedicated cycle counter: ``PMCCNTR``.

Architectural mappings
~~~~~~~~~~~~~~~~~~~~~~

+--------------+---------+----------------------------+
| Counters     | State   | System Register Name       |
+==============+=========+============================+
|              | AArch64 | ``PMEVCNTR<n>_EL0[63*:0]`` |
| Programmable +---------+----------------------------+
|              | AArch32 | ``PMEVCNTR<n>[31:0]``      |
+--------------+---------+----------------------------+
|              | AArch64 | ``PMCCNTR_EL0[63:0]``      |
| Cycle        +---------+----------------------------+
|              | AArch32 | ``PMCCNTR[63:0]``          |
+--------------+---------+----------------------------+

.. note::
   Bits [63:32] are only available if ARMv8.5-PMU is implemented. Refer to the
   `Arm ARM`_ for a detailed description of ARMv8.5-PMU features.

Configuring the PMU for counting events
---------------------------------------

Each programmable counter has an associated register, ``PMEVTYPER<n>`` which
configures it. The cycle counter has the ``PMCCFILTR_EL0`` register, which has
an identical function and bit field layout as ``PMEVTYPER<n>``. In addition,
the counters are enabled (permitted to increment) via the ``PMCNTENSET`` and
``PMCR`` registers. These can be accessed at all privilege levels.

Architectural mappings
~~~~~~~~~~~~~~~~~~~~~~

+-----------------------------+------------------------+
| AArch64                     | AArch32                |
+=============================+========================+
| ``PMEVTYPER<n>_EL0[63*:0]`` | ``PMEVTYPER<n>[31:0]`` |
+-----------------------------+------------------------+
| ``PMCCFILTR_EL0[63*:0]``    | ``PMCCFILTR[31:0]``    |
+-----------------------------+------------------------+
| ``PMCNTENSET_EL0[63*:0]``   | ``PMCNTENSET[31:0]``   |
+-----------------------------+------------------------+
| ``PMCR_EL0[63*:0]``         | ``PMCR[31:0]``         |
+-----------------------------+------------------------+

.. note::
   Bits [63:32] are reserved.

Relevant register fields
~~~~~~~~~~~~~~~~~~~~~~~~

For ``PMEVTYPER<n>_EL0``/``PMEVTYPER<n>`` and ``PMCCFILTR_EL0/PMCCFILTR``, the
most important fields are:

-  ``P``:

   -  Bit 31.
   -  If set to ``0``, will increment the associated ``PMEVCNTR<n>`` at EL1.

-  ``NSK``:

   -  Bit 29.
   -  If equal to the ``P`` bit it enables the associated ``PMEVCNTR<n>`` at
      Non-secure EL1.
   -  Reserved if EL3 not implemented.

-  ``NSH``:

   -  Bit 27.
   -  If set to ``1``, will increment the associated ``PMEVCNTR<n>`` at EL2.
   -  Reserved if EL2 not implemented.

-  ``SH``:

   -  Bit 24.
   -  If different to the ``NSH`` bit it enables the associated ``PMEVCNTR<n>``
      at Secure EL2.
   -  Reserved if Secure EL2 not implemented.

-  ``M``:

   -  Bit 26.
   -  If equal to the ``P`` bit it enables the associated ``PMEVCNTR<n>`` at
      EL3.

-  ``evtCount[15:10]``:

   -  Extension to ``evtCount[9:0]``. Reserved unless ARMv8.1-PMU implemented.

-  ``evtCount[9:0]``:

   -  The event number that the associated ``PMEVCNTR<n>`` will count.

For ``PMCNTENSET_EL0``/``PMCNTENSET``, the most important fields are:

-  ``P[30:0]``:

   -  Setting bit ``P[n]`` to ``1`` enables counter ``PMEVCNTR<n>``.
   -  The effects of ``PMEVTYPER<n>`` are applied on top of this.
      In other words, the counter will not increment at any privilege level or
      security state unless it is enabled here.

-  ``C``:

   -  Bit 31.
   -  If set to ``1`` enables the cycle counter ``PMCCNTR``.

For ``PMCR``/``PMCR_EL0``, the most important fields are:

-  ``DP``:

   -  Bit 5.
   -  If set to ``1`` it disables the cycle counter ``PMCCNTR`` where event
      counting (by ``PMEVCNTR<n>``) is prohibited (e.g. EL2 and the Secure
      world).
   -  If set to ``0``, ``PMCCNTR`` will not be affected by this bit and
      therefore will be able to count where the programmable counters are
      prohibited.

-  ``E``:

   -  Bit 0.
   -  Enables/disables counting altogether.
   -  The effects of ``PMCNTENSET`` and ``PMCR.DP`` are applied on top of this.
      In other words, if this bit is ``0`` then no counters will increment
      regardless of how the other PMU system registers or bit fields are
      configured.

.. rubric:: References

-  `Arm ARM`_

--------------

*Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.*

.. _Arm ARM: https://developer.arm.com/docs/ddi0487/latest
