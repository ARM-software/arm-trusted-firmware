Advisory TFV-6 (CVE-2017-5753, CVE-2017-5715, CVE-2017-5754)
============================================================

+----------------+-------------------------------------------------------------+
| Title          | Trusted Firmware-A exposure to speculative processor        |
|                | vulnerabilities using cache timing side-channels            |
+================+=============================================================+
| CVE ID         | `CVE-2017-5753`_ / `CVE-2017-5715`_ / `CVE-2017-5754`_      |
+----------------+-------------------------------------------------------------+
| Date           | 03 Jan 2018 (Updated 11 Jan, 18 Jan, 26 Jan, 30 Jan and 07  |
|                | June 2018)                                                  |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v1.4                               |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Leakage of secure world data to normal world                |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #1214`_, `Pull Request #1228`_,               |
|                | `Pull Request #1240`_ and `Pull Request #1405`_             |
+----------------+-------------------------------------------------------------+
| Credit         | Google / Arm                                                |
+----------------+-------------------------------------------------------------+

This security advisory describes the current understanding of the Trusted
Firmware-A exposure to the speculative processor vulnerabilities identified by
`Google Project Zero`_.  To understand the background and wider impact of these
vulnerabilities on Arm systems, please refer to the `Arm Processor Security
Update`_.

Variant 1 (`CVE-2017-5753`_)
----------------------------

At the time of writing, no vulnerable patterns have been observed in upstream TF
code, therefore no workarounds have been applied or are planned.

Variant 2 (`CVE-2017-5715`_)
----------------------------

Where possible on vulnerable CPUs, Arm recommends invalidating the branch
predictor as early as possible on entry into the secure world, before any branch
instruction is executed. There are a number of implementation defined ways to
achieve this.

For Cortex-A57 and Cortex-A72 CPUs, the Pull Requests (PRs) in this advisory
invalidate the branch predictor when entering EL3 by disabling and re-enabling
the MMU.

For Cortex-A73 and Cortex-A75 CPUs, the PRs in this advisory invalidate the
branch predictor when entering EL3 by temporarily dropping into AArch32
Secure-EL1 and executing the ``BPIALL`` instruction. This workaround is
significantly more complex than the "MMU disable/enable" workaround. The latter
is not effective at invalidating the branch predictor on Cortex-A73/Cortex-A75.

Note that if other privileged software, for example a Rich OS kernel, implements
its own branch predictor invalidation during context switch by issuing an SMC
(to execute firmware branch predictor invalidation), then there is a dependency
on the PRs in this advisory being deployed in order for those workarounds to
work. If that other privileged software is able to workaround the vulnerability
locally (for example by implementing "MMU disable/enable" itself), there is no
such dependency.

`Pull Request #1240`_ and `Pull Request #1405`_ optimise the earlier fixes by
implementing a specified `CVE-2017-5715`_ workaround SMC
(``SMCCC_ARCH_WORKAROUND_1``) for use by normal world privileged software. This
is more efficient than calling an arbitrary SMC (for example ``PSCI_VERSION``).
Details of ``SMCCC_ARCH_WORKAROUND_1`` can be found in the `CVE-2017-5715
mitigation specification`_.  The specification and implementation also enable
the normal world to discover the presence of this firmware service.

On Juno R1 we measured the round trip latency for both the ``PSCI_VERSION`` and
``SMCCC_ARCH_WORKAROUND_1`` SMCs on Cortex-A57, using both the "MMU
disable/enable" and "BPIALL at AArch32 Secure-EL1" workarounds described above.
This includes the time spent in test code conforming to the SMC Calling
Convention (SMCCC) from AArch64. For the ``SMCCC_ARCH_WORKAROUND_1`` cases, the
test code uses SMCCC v1.1, which reduces the number of general purpose registers
it needs to save/restore. Although the ``BPIALL`` instruction is not effective
at invalidating the branch predictor on Cortex-A57, the drop into Secure-EL1
with MMU disabled that this workaround entails effectively does invalidate the
branch predictor. Hence this is a reasonable comparison.

The results were as follows:

+------------------------------------------------------------------+-----------+
| Test                                                             | Time (ns) |
+==================================================================+===========+
| ``PSCI_VERSION`` baseline (without PRs in this advisory)         | 515       |
+------------------------------------------------------------------+-----------+
| ``PSCI_VERSION`` baseline (with PRs in this advisory)            | 527       |
+------------------------------------------------------------------+-----------+
| ``PSCI_VERSION`` with "MMU disable/enable"                       | 930       |
+------------------------------------------------------------------+-----------+
| ``SMCCC_ARCH_WORKAROUND_1`` with "MMU disable/enable"            | 386       |
+------------------------------------------------------------------+-----------+
| ``PSCI_VERSION`` with "BPIALL at AArch32 Secure-EL1"             | 1276      |
+------------------------------------------------------------------+-----------+
| ``SMCCC_ARCH_WORKAROUND_1`` with "BPIALL at AArch32 Secure-EL1"  | 770       |
+------------------------------------------------------------------+-----------+

Due to the high severity and wide applicability of this issue, the above
workarounds are enabled by default (on vulnerable CPUs only), despite some
performance and code size overhead. Platforms can choose to disable them at
compile time if they do not require them. `Pull Request #1240`_ disables the
workarounds for unaffected upstream platforms.

For vulnerable AArch32-only CPUs (for example Cortex-A8, Cortex-A9 and
Cortex-A17), the ``BPIALL`` instruction should be used as early as possible on
entry into the secure world. For Cortex-A8, also set ``ACTLR[6]`` to 1 during
early processor initialization. Note that the ``BPIALL`` instruction is not
effective at invalidating the branch predictor on Cortex-A15. For that CPU, set
``ACTLR[0]`` to 1 during early processor initialization, and invalidate the
branch predictor by performing an ``ICIALLU`` instruction.

On AArch32 EL3 systems, the monitor and secure-SVC code is typically tightly
integrated, for example as part of a Trusted OS. Therefore any Variant 2
workaround should be provided by vendors of that software and is outside the
scope of TF. However, an example implementation in the minimal AArch32 Secure
Payload, ``SP_MIN`` is provided in `Pull Request #1228`_.

Other Arm CPUs are not vulnerable to this or other variants. This includes
Cortex-A76, Cortex-A53, Cortex-A55, Cortex-A32, Cortex-A7 and Cortex-A5.

For more information about non-Arm CPUs, please contact the CPU vendor.

Variant 3 (`CVE-2017-5754`_)
----------------------------

This variant is only exploitable between Exception Levels within the same
translation regime, for example between EL0 and EL1, therefore this variant
cannot be used to access secure memory from the non-secure world, and is not
applicable for TF. However, Secure Payloads (for example, Trusted OS) should
provide mitigations on vulnerable CPUs to protect themselves from exploited
Secure-EL0 applications.

The only Arm CPU vulnerable to this variant is Cortex-A75.

.. _Google Project Zero: https://googleprojectzero.blogspot.co.uk/2018/01/reading-privileged-memory-with-side.html
.. _Arm Processor Security Update: http://www.arm.com/security-update
.. _CVE-2017-5753: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5753
.. _CVE-2017-5715: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5715
.. _CVE-2017-5754: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5754
.. _Pull Request #1214: https://github.com/ARM-software/arm-trusted-firmware/pull/1214
.. _Pull Request #1228: https://github.com/ARM-software/arm-trusted-firmware/pull/1228
.. _Pull Request #1240: https://github.com/ARM-software/arm-trusted-firmware/pull/1240
.. _Pull Request #1405: https://github.com/ARM-software/arm-trusted-firmware/pull/1405
.. _CVE-2017-5715 mitigation specification: https://developer.arm.com/cache-speculation-vulnerability-firmware-specification
