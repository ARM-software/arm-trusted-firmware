PSCI Performance Measurement
============================

TF-A provides two instrumentation tools for performing analysis of the PSCI
implementation:

* PSCI STAT
* Runtime Instrumentation

This page explains how they may be enabled and used to perform all varieties of
analysis.

Performance Measurement Framework
---------------------------------

The Performance Measurement Framework :ref:`PMF <firmware_design_pmf>`
is a framework that provides mechanisms for collecting and retrieving timestamps
at runtime from the Performance Measurement Unit
(:ref:`PMU <Performance Monitoring Unit>`).
The PMU is a generalized abstraction for accessing CPU hardware registers used to
measure hardware events. This means, for instance, that the PMU might be used to
place instrumentation points at logical locations in code for tracing purposes.

TF-A utilises the PMF as a backend for the two instrumentation services it
provides--PSCI Statistics and Runtime Instrumentation. The PMF is used by
these services to facilitate collection and retrieval of timestamps. For
instance, the PSCI Statistics service registers the PMF service
``psci_svc`` to track its residency statistics.

This is reserved a unique ID, name, and space in memory by the PMF. The
framework provides a convenient interface for PSCI Statistics to retrieve
values from ``psci_svc`` at runtime.  Alternatively, the service may be
configured such that the PMF dumps those values to the console. A platform may
choose to expose SMCs that allow retrieval of these timestamps from the
service.

This feature is enabled with the Boolean flag ``ENABLE_PMF``.

PSCI Statistics
---------------

PSCI Statistics is a runtime service that provides residency statistics for
power states used by the platform. The service tracks residency time and
entry count. Residency time is the total time spent in a particular power
state by a PE. The entry count is the number of times the PE has entered
the power state. PSCI Statistics implements the optional functions
``PSCI_STAT_RESIDENCY`` and ``PSCI_STAT_COUNT`` from the `PSCI`_
specification.


.. c:macro:: PSCI_STAT_RESIDENCY

    :param target_cpu: Contains copy of affinity fields in the MPIDR register
      for identifying the target core (See section 5.1.4 of `PSCI`_
      specifications for more details).
    :param power_state: identifier for a specific local
      state. Generally, this parameter takes the same form as the power_state
      parameter described for CPU_SUSPEND in section 5.4.2.

    :returns: Time spent in ``power_state``, in microseconds, by ``target_cpu``
      and the highest level expressed in ``power_state``.


.. c:macro:: PSCI_STAT_COUNT

    :param target_cpu: follows the same format as ``PSCI_STAT_RESIDENCY``.
    :param power_state: follows the same format as ``PSCI_STAT_RESIDENCY``.

    :returns: Number of times the state expressed in ``power_state`` has been
      used by ``target_cpu`` and the highest level expressed in
      ``power_state``.

The implementation provides residency statistics only for low power states,
and does this regardless of the entry mechanism into those states. The
statistics it collects are set to 0 during shutdown or reset.

PSCI Statistics is enabled with the Boolean build flag
``ENABLE_PSCI_STAT``.  All Arm platforms utilise the PMF unless another
collection backend is provided (``ENABLE_PMF`` is implicitly enabled).

Runtime Instrumentation
-----------------------

The Runtime Instrumentation Service is an instrumentation tool that wraps
around the PMF to provide timestamp data. Although the service is not
restricted to PSCI, it is used primarily in TF-A to quantify the total time
spent in the PSCI implementation. The tool can be used to instrument other
components in TF-A as well. It is enabled with the Boolean flag
``ENABLE_RUNTIME_INSTRUMENTATION``, and as with PSCI STAT, requires PMF to
be enabled.

In PSCI, this service provides instrumentation points in the
following code paths:

* Entry into the PSCI SMC handler
* Exit from the PSCI SMC handler
* Entry to low power state
* Exit from low power state
* Entry into cache maintenance operations in PSCI
* Exit from cache maintenance operations in PSCI

The service captures the cycle count, which allows for the time spent in the
implementation to be calculated, given the frequency counter.

PSCI SMC Handler Instrumentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The timestamp during entry into the handler is captured as early as possible
during the runtime exception, prior to entry into the handler itself. All
timestamps are stored in memory for later retrieval. The exit timestamp is
captured after normal return from the PSCI SMC handler, or, if a low power state
was requested, it is captured in the warm boot path.

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _PSCI: https://developer.arm.com/documentation/den0022/latest/
