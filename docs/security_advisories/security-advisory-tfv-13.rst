Advisory TFV-13 (CVE-2024-7881)
================================

+----------------+-----------------------------------------------------------------+
| Title          | An unprivileged context can trigger a data memory-dependent     |
|                | prefetch engine to fetch the contents of a privileged location  |
|                | and consume those contents as an address that is                |
|                | also dereferenced.                                              |
|                |                                                                 |
+================+=================================================================+
| CVE ID         | `CVE-2024-7881`_                                                |
+----------------+-----------------------------------------------------------------+
| Date           | Reported on 16 August 2024                                      |
+----------------+-----------------------------------------------------------------+
| Versions       | TF-A version from v2.2 to v2.12                                 |
| Affected       | LTS releases lts-v2.8.0 to lts-v2.8.28                          |
|                | LTS releases lts-v2.10.0 to lts-v2.10.12                        |
+----------------+-----------------------------------------------------------------+
| Configurations | All                                                             |
| Affected       |                                                                 |
+----------------+-----------------------------------------------------------------+
| Impact         | Potential leakage of secure world data to normal world.         |
+----------------+-----------------------------------------------------------------+
| Fix Version    | `Gerrit topic #ar/smccc_arch_wa_4`_                             |
|                | Also see mitigation guidance in the `Official Arm Advisory`_    |
+----------------+-----------------------------------------------------------------+
| Credit         | Arm                                                             |
+----------------+-----------------------------------------------------------------+

Description
-----------

An issue has been identified in some Arm-based CPUs that may allow
an unprivileged context to trigger a data memory-dependent prefetch engine
to fetch the contents of a privileged location (for which it
does not have read permission) and consume those contents as an address
that is also dereferenced.

The below table lists all the CPUs impacted by this vulnerability and have
mitigation in TF-A.

+----------------------+
| Core                 |
+----------------------+
| Cortex-X3            |
+----------------------+
| Cortex-X4            |
+----------------------+
| Cortex-X925          |
+----------------------+
| Neoverse-V2          |
+----------------------+
| Neoverse-V3          |
+----------------------+
| Neoverse-V3AE        |
+----------------------+

Mitigation and Recommendations
------------------------------

Arm recommends following the mitigation steps and configuration changes
described in the official advisory. The mitigation for CVE-2024-7881 is
implemented at EL3 and addresses vulnerabilities caused by memory-dependant
speculative prefetching. This issue is avoided by setting CPUACTLR6_EL1[41]
to 1, this disables the affected prefetcher.

Arm has updated the SMC Calling Convention spec so that privileged normal world
software can identify when the issue has been mitigated in
firmware (SMCCC_ARCH_WORKAROUND_4). Refer to the `SMC Calling Convention
Specification`_ for more details.

The above workaround is enabled by default (on vulnerable CPUs only).
Platforms can choose to disable them at compile time if
they do not require them.

For further technical information, affected CPUs, and detailed guidance,
refer to the full `Official Arm Advisory`_.

.. _CVE-2024-7881: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2024-7881
.. _Gerrit topic #ar/smccc_arch_wa_4: https://review.trustedfirmware.org/q/topic:%22ar/smccc_arch_wa_4%22
.. _SMC Calling Convention specification: https://developer.arm.com/documentation/den0028/latest
.. _Official Arm Advisory: https://developer.arm.com/documentation/110326/latest
