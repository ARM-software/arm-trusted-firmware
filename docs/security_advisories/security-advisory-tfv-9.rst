Advisory TFV-9 (CVE-2022-23960)
============================================================

+----------------+-------------------------------------------------------------+
| Title          | Trusted Firmware-A exposure to speculative processor        |
|                | vulnerabilities with branch prediction target reuse         |
+================+=============================================================+
| CVE ID         | `CVE-2022-23960`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | 08 Mar 2022                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v2.6                               |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Potential leakage of secure world data to normal world      |
|                | if an attacker is able to find a TF-A exfiltration primitive|
|                | that can be predicted as a valid branch target, and somehow |
|                | induce misprediction onto that primitive. There are         |
|                | currently no known exploits.                                |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Gerrit topic #spectre_bhb`_                                |
+----------------+-------------------------------------------------------------+
| Credit         | Systems and Network Security Group at Vrije Universiteit    |
|                | Amsterdam for CVE-2022-23960, Arm for patches               |
+----------------+-------------------------------------------------------------+

This security advisory describes the current understanding of the Trusted
Firmware-A exposure to the new speculative processor vulnerability.
To understand the background and wider impact of these vulnerabilities on Arm
systems, please refer to the `Arm Processor Security Update`_. The whitepaper
referred to below describes the Spectre attack and mitigation in more detail
including implementation specific mitigation details for all impacted Arm CPUs.


`CVE-2022-23960`_
-----------------

Where possible on vulnerable CPUs that implement FEAT_CSV2, Arm recommends
inserting a loop workaround with implementation specific number of iterations
that will discard the branch history on exception entry to a higher exception
level for the given CPU. This is done as early as possible on entry into EL3,
before any branch instruction is executed. This is sufficient to mitigate
Spectre-BHB on behalf of all secure world code, assuming that no secure world
code is under attacker control.

The below table lists the CPUs that mitigate against this vulnerability in
TF-A using the loop workaround(all cores that implement FEAT_CSV2 except the
revisions of Cortex-A73 and Cortex-A75 that implements FEAT_CSV2).

+----------------------+
| Core                 |
+----------------------+
| Cortex-A72(from r1p0)|
+----------------------+
| Cortex-A76           |
+----------------------+
| Cortex-A77           |
+----------------------+
| Cortex-A78           |
+----------------------+
| Cortex-X2            |
+----------------------+
| Cortex-A710          |
+----------------------+
| Neoverse-N1          |
+----------------------+
| Neoverse-N2          |
+----------------------+
| Neoverse-V1          |
+----------------------+

For all other cores impacted by Spectre-BHB, some of which that do not implement
FEAT_CSV2 and some that do e.g. Cortex-A73, the recommended mitigation is to
flush all branch predictions via an implementation specific route.

In case local workaround is not feasible, the Rich OS can invoke the SMC
(``SMCCC_ARCH_WORKAROUND_3``) to apply the workaround. Refer to `SMCCC Calling
Convention specification`_ for more details.

`Gerrit topic #spectre_bhb`_ This patchset implements the Spectre-BHB loop
workaround for CPUs mentioned in the above table. It also mitigates against
this vulnerability for Cortex-A72 CPU versions that support the CSV2 feature
(from r1p0). The patch stack also includes an implementation for a specified
`CVE-2022-23960`_ workaround SMC(``SMCCC_ARCH_WORKAROUND_3``) for use by normal
world privileged software. Details of ``SMCCC_ARCH_WORKAROUND_3`` can be found
in the `SMCCC Calling Convention specification`_. The specification and
implementation also enables the normal world to discover the presence of this
firmware service. This patch also implements ``SMCCC_ARCH_WORKAROUND_3`` for
Cortex-A57, Coxtex-A72, Cortex-A73 and Cortex-A75 using the existing workaround.
for CVE-2017-5715.

The above workaround is enabled by default (on vulnerable CPUs only). Platforms
can choose to disable them at compile time if they do not require them.

For more information about non-Arm CPUs, please contact the CPU vendor.

.. _Arm Processor Security Update: http://www.arm.com/security-update
.. _CVE-2022-23960: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2022-23960
.. _Gerrit topic #spectre_bhb: https://review.trustedfirmware.org/q/topic:"spectre_bhb"+(status:open%20OR%20status:merged)
.. _CVE-2022-23960 mitigation specification: https://developer.arm.com/support/arm-security-updates/speculative-processor-vulnerability
.. _SMCCC Calling Convention specification: https://developer.arm.com/documentation/den0028/latest
