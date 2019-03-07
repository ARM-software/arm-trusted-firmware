Advisory TFV-5 (CVE-2017-15031)
===============================

+----------------+-------------------------------------------------------------+
| Title          | Not initializing or saving/restoring ``PMCR_EL0`` can leak  |
|                | secure world timing information                             |
+================+=============================================================+
| CVE ID         | `CVE-2017-15031`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | 02 Oct 2017                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v1.4                               |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Leakage of sensitive secure world timing information        |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #1127`_ (merged on 18 October 2017)           |
+----------------+-------------------------------------------------------------+
| Credit         | Arm                                                         |
+----------------+-------------------------------------------------------------+

The ``PMCR_EL0`` (Performance Monitors Control Register) provides details of the
Performance Monitors implementation, including the number of counters
implemented, and configures and controls the counters. If the ``PMCR_EL0.DP``
bit is set to zero, the cycle counter (when enabled) counts during secure world
execution, even when prohibited by the debug signals.

Since Arm TF does not save and restore ``PMCR_EL0`` when switching between the
normal and secure worlds, normal world code can set ``PMCR_EL0.DP`` to zero to
cause leakage of secure world timing information. This register should be added
to the list of saved/restored registers.

Furthermore, ``PMCR_EL0.DP`` has an architecturally ``UNKNOWN`` reset value.
Since Arm TF does not initialize this register, it's possible that on at least
some implementations, ``PMCR_EL0.DP`` is set to zero by default. This and other
bits with an architecturally UNKNOWN reset value should be initialized to
sensible default values in the secure context.

The same issue exists for the equivalent AArch32 register, ``PMCR``, except that
here ``PMCR_EL0.DP`` architecturally resets to zero.

.. _CVE-2017-15031: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-15031
.. _Pull Request #1127: https://github.com/ARM-software/arm-trusted-firmware/pull/1127
