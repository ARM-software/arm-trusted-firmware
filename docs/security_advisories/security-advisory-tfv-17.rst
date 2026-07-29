Advisory TFV-17 (CVE-2025-10263)
================================

+----------------+-------------------------------------------------------------+
| Title          | TLBI+DSB might complete too early                           |
+================+=============================================================+
| CVE ID         | `CVE-2025-10263`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | Reported on 13 August 2025                                  |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v2.15                              |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All platforms containing any of the following cores: (note  |
| Affected       | that if even a single affected core is present that all     |
|                | cores must implement the workaround, which the TFA patch    |
|                | does)                                                       |
|                |                                                             |
|                | - Cortex-A76                                                |
|                | - Cortex-A76AE                                              |
|                | - Cortex-A77                                                |
|                | - Cortex-X1                                                 |
|                | - Cortex-X1C                                                |
|                | - Cortex-A78                                                |
|                | - Cortex-A78C                                               |
|                | - Cortex-A78AE                                              |
|                | - Cortex-A710                                               |
|                | - Cortex-X2                                                 |
|                | - Cortex-X3                                                 |
|                | - Cortex-X4                                                 |
|                | - Cortex-X925                                               |
|                | - Neoverse-N1                                               |
|                | - Neoverse-N2                                               |
|                | - Neoverse-V1                                               |
|                | - Neoverse-V2                                               |
|                | - Neoverse-V3                                               |
|                | - Neoverse-V3AE                                             |
|                | - C1-Ultra                                                  |
|                | - C1-Premium                                                |
+----------------+-------------------------------------------------------------+
| Impact         | Exploitation may allow an attacker to corrupt page table    |
|                | entries or other memory-management data structures during   |
|                | translation invalidation operations.                        |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Commit 08fab0f083db` (merged on 16 June 2026)              |
|                |                                                             |
|                | `Gerrit topic #gr/CVE-2025-10263`_                          |
+----------------+-------------------------------------------------------------+
| Credit         | Arm                                                         |
+----------------+-------------------------------------------------------------+

Description
-----------

A race condition exists in affected Arm CPUs whereby a memory store performed by
one Processing Element (PE) may remain incomplete after another PE has
invalidated the corresponding translation and completed the architecturally
required TLBI and DSB sequence. Under specific microarchitectural conditions,
the assumption that all memory accesses made using the invalidated translation
table entry have completed might not hold true.

This behavior can result in memory corruption affecting translation tables and
other protected memory structures. Successful exploitation may lead to denial
of service or privilege escalation within the affected translation regime.

Practical exploitation has been demonstrated to the extent of page table
corruption. Feasible privilege escalation is considered possible, although full
exploitation has not yet been publicly demonstrated.

Conditions
----------

1. PE A executes a store that crosses a page boundary.
2. PE B executes a TLB invalidate instruction which affects at least one of
   pages targeted by PE A's store, followed by a DSB.
3. Complex micro-architectural conditions occur.
4. PE B's DSB may complete before the global observation of a portion of PE A's
   store which was affected by the TLB invalidation. The relevant TLB entries
   are invalidated correctly before the completion of the DSB.

Workaround
----------

This can be avoided by executing the TLBI and DSB instructions a 2nd time. For
code sequences that have multiple TLB invalidate instuctions followed by a
single DSB, only the last TLB invalidate and DSB need to be repeated.

Impact on TF-A
--------------

TF-a uses an affected instruction sequence only in the Xlat library, so the
workaround is inserted at the end of that function. Unlike most CVEs, this
workaround is disabled by default due to it being needed when a platform
contains any single CPU that is affected. So it must be explicitly enabled
using the flag WORKAROUND_CVE_2025_10263 by the platform when an affected
CPU is present.

For further information, affected CPUs, and detailed guidance, refer to the full
`Official Arm Advisory`_.

.. _CVE-2025-10263: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2025-10263
.. _Gerrit topic #gr/CVE-2025-10263: https://review.trustedfirmware.org/q/topic:gr/CVE-2025-10263
.. _Official Arm Advisory: https://developer.arm.com/documentation/112137
