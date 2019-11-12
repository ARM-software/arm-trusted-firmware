Advisory TFV-5 (CVE-2017-15031)
===============================

+----------------+-------------------------------------------------------------+
| Title          | Not initializing or saving/restoring ``PMCR_EL0`` can leak  |
|                | secure world timing information                             |
+================+=============================================================+
| CVE ID         | `CVE-2017-15031`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | 02 Oct 2017, updated on 04 Nov 2019                         |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v2.1                               |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Leakage of sensitive secure world timing information        |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #1127`_ (merged on 18 October 2017)           |
|                |                                                             |
|                | `Commit e290a8fcbc`_ (merged on 23 August 2019)             |
|                |                                                             |
|                | `Commit c3e8b0be9b`_ (merged on 27 September 2019)          |
+----------------+-------------------------------------------------------------+
| Credit         | Arm, Marek Bykowski                                         |
+----------------+-------------------------------------------------------------+

The ``PMCR_EL0`` (Performance Monitors Control Register) provides details of the
Performance Monitors implementation, including the number of counters
implemented, and configures and controls the counters. If the ``PMCR_EL0.DP``
bit is set to zero, the cycle counter (when enabled) counts during secure world
execution, even when prohibited by the debug signals.

Since TF-A does not save and restore ``PMCR_EL0`` when switching between the
normal and secure worlds, normal world code can set ``PMCR_EL0.DP`` to zero to
cause leakage of secure world timing information. This register should be added
to the list of saved/restored registers both when entering EL3 and also
transitioning to S-EL1.

Furthermore, ``PMCR_EL0.DP`` has an architecturally ``UNKNOWN`` reset value.
Since Arm TF does not initialize this register, it's possible that on at least
some implementations, ``PMCR_EL0.DP`` is set to zero by default. This and other
bits with an architecturally UNKNOWN reset value should be initialized to
sensible default values in the secure context.

The same issue exists for the equivalent AArch32 register, ``PMCR``, except that
here ``PMCR_EL0.DP`` architecturally resets to zero.

NOTE: The original pull request referenced above only fixed the issue for S-EL1
whereas the EL3 was fixed in the later commits.

.. _CVE-2017-15031: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-15031
.. _Pull Request #1127: https://github.com/ARM-software/arm-trusted-firmware/pull/1127
.. _Commit e290a8fcbc: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=e290a8fcbc
.. _Commit c3e8b0be9b: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=c3e8b0be9b

