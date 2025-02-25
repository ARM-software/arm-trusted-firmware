Maximum Power Mitigation Mechanism (MPMM)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

|MPMM| is an optional microarchitectural power management mechanism supported by
some Arm Armv9-A cores, beginning with the Cortex-X2, Cortex-A710 and
Cortex-A510 cores. This mechanism detects and limits high-activity events to
assist in |SoC| processor power domain dynamic power budgeting and limit the
triggering of whole-rail (i.e. clock chopping) responses to overcurrent
conditions.

|MPMM| is enabled on a per-core basis by the EL3 runtime firmware.

.. warning::

    |MPMM| exposes gear metrics through the auxiliary |AMU| counters. An
    external power controller can use these metrics to budget SoC power by
    limiting the number of cores that can execute higher-activity workloads or
    switching to a different DVFS operating point. When this is the case, the
    |AMU| counters that make up the |MPMM| gears must be enabled by the EL3
    runtime firmware - please see :ref:`Activity Monitor Auxiliary Counters` for
    documentation on enabling auxiliary |AMU| counters.
