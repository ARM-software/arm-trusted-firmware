Advisory TFV-12 (CVE-2024-5660)
================================

+----------------+--------------------------------------------------------------+
| Title          | When Hardware Page Aggregation (HPA) is enabled memory       |
|                | accesses may be translated incorrectly.                      |
+================+==============================================================+
| CVE ID         | `CVE-2024-5660`_                                             |
+----------------+--------------------------------------------------------------+
| Date           | Reported on 26 Jan 2024                                      |
+----------------+--------------------------------------------------------------+
| Versions       | TF-A version from v2.2 to v2.12                              |
| Affected       | LTS releases lts-v2.8.0 to lts-v2.8.26                       |
|                | LTS releases lts-v2.10.0 to lts-v2.10.10                     |
+----------------+--------------------------------------------------------------+
| Configurations | Arm CPUs with Hardware Page Aggregation (HPA) running in     |
| Affected       | environments where a modified, untrusted guest OS may        |
|                | operate, especially with specific hypervisors.               |
+----------------+--------------------------------------------------------------+
| Impact         | Potential for a compromised guest OS to attack the host via  |
|                | HPA mechanism, resulting in possible information disclosure. |
+----------------+--------------------------------------------------------------+
| Fix Version    | `Gerrit-Patches`_                                            |
+----------------+--------------------------------------------------------------+
| Credit         | Arm                                                          |
+----------------+--------------------------------------------------------------+

Description
-----------

A vulnerability has been identified in certain Arm CPUs implementing the
Hardware Page Aggregation (HPA) feature. In environments utilizing virtualization,
a specially crafted or compromised guest operating system could exploit this
vulnerability to affect the host system. This could potentially lead to information
disclosure depending on the deployment scenario and hypervisor configuration.

The below table lists the CPUs that mitigate against this vulnerability in TF-A.

+---------------+
| **Core**      |
+---------------+
| Cortex-A77    |
+---------------+
| Cortex-A78    |
+---------------+
| Cortex-A78C   |
+---------------+
| Cortex-A78AE  |
+---------------+
| Cortex-A710   |
+---------------+
| Cortex-X1     |
+---------------+
| Cortex-X2     |
+---------------+
| Cortex-X3     |
+---------------+
| Cortex-X4     |
+---------------+
| Cortex-X925   |
+---------------+
| Neoverse-V1   |
+---------------+
| Neoverse-V2   |
+---------------+
| Neoverse-V3   |
+---------------+
| Neoverse-N2   |
+---------------+

Mitigation and Recommendations
------------------------------

Arm recommends following the mitigation steps and configuration changes described in the
official advisory. The issue is avoided by setting CPUECTLR_EL1[46] to 1 which will
disable hardware page aggregation.

Users should refer to the latest firmware updates as provided by vendors
and ensure that HPA-related security mitigations are enabled where applicable.

For further technical information, affected CPUs, and detailed guidance, refer to the
full `Official Arm Advisory`_.

.. _CVE-2024-5660: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2024-5660
.. _Gerrit-Patches: https://review.trustedfirmware.org/q/topic:%22sm/fix_erratum%22
.. _Official Arm Advisory: https://developer.arm.com/documentation/110324/latest
