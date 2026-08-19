Branch Record Buffer
===========================

The Branch Record Buffer (BRB) allows recording of branch paths for profiling
purposes.

TF-A supports ``FEAT_BRBE`` and ``FEAT_BRBEv1p1``, the latter of which
enables branch recording at EL3.

Branch Records
--------------

Taken branch instructions and exceptions can generate branch records - the BRB
may be configured to filter which circumstances they are generated under. Each
branch record consists of three registers:

- ``BRBINF<n>_EL1``: Stores information about a branch record
- ``BRBSRC<n>_EL1``: Stores the source address of a branch record
- ``BRBTGT<n>_EL1``: Stores the destination address of a branch record

Where ``n`` is ``0`` to at most ``31`` (``IMPLEMENTATION DEFINED``).

.. note::
   For more information, consult Chapter D19.4 of the `Arm ARM`_.

Using TF-A profiling
--------------------

TF-A provides methods to enable branch recording at all exception levels,
accessible from `include/lib/extensions/brbe.h`. Profiling can be carried
out as follows:

.. code:: c

  brbe_start_recording();
  // code to be profiled
  brbe_stop_recording();

If ``FEAT_BRBEv1p1`` is supported, then EL3 payloads may be profiled.

Since the BRB is of finite size, it is prone to overflowing; the resulting
buffer flush then causes already-captured profiling information to be lost.
TF-A will dump the contents of the BRB when it overflows. This relies on a
PMU counter (by default, counter 0) triggering a synchronous exception. Setup
of this counter is performed when recording starts, but relies on
``FEAT_EBEP`` being implemented for correct function.

.. note::
   For more information on the PMU, consult Chapter D13 of the `Arm ARM`_ and
   the `Performance Monitoring Unit` section of this documentation.

.. rubric:: References

-  `Arm ARM`_

--------------

*Copyright (c) 2019-2026, Arm Limited and Contributors. All rights reserved.*

.. _Arm ARM: https://developer.arm.com/docs/ddi0487/latest
