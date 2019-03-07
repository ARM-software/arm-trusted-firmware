Advisory TFV-7 (CVE-2018-3639)
==============================

+----------------+-------------------------------------------------------------+
| Title          | Trusted Firmware-A exposure to cache speculation            |
|                | vulnerability Variant 4                                     |
+================+=============================================================+
| CVE ID         | `CVE-2018-3639`_                                            |
+----------------+-------------------------------------------------------------+
| Date           | 21 May 2018 (Updated 7 June 2018)                           |
+----------------+-------------------------------------------------------------+
| Versions       | All, up to and including v1.5                               |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Leakage of secure world data to normal world                |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #1392`_, `Pull Request #1397`_                |
+----------------+-------------------------------------------------------------+
| Credit         | Google                                                      |
+----------------+-------------------------------------------------------------+

This security advisory describes the current understanding of the Trusted
Firmware-A (TF-A) exposure to Variant 4 of the cache speculation vulnerabilities
identified by `Google Project Zero`_.  To understand the background and wider
impact of these vulnerabilities on Arm systems, please refer to the `Arm
Processor Security Update`_.

At the time of writing, the TF-A project is not aware of a Variant 4 exploit
that could be used against TF-A. It is likely to be very difficult to achieve an
exploit against current standard configurations of TF-A, due to the limited
interfaces into the secure world with attacker-controlled inputs. However, this
is becoming increasingly difficult to guarantee with the introduction of complex
new firmware interfaces, for example the `Software Delegated Exception Interface
(SDEI)`_.  Also, the TF-A project does not have visibility of all
vendor-supplied interfaces. Therefore, the TF-A project takes a conservative
approach by mitigating Variant 4 in hardware wherever possible during secure
world execution. The mitigation is enabled by setting an implementation defined
control bit to prevent the re-ordering of stores and loads.

For each affected CPU type, TF-A implements one of the two following mitigation
approaches in `Pull Request #1392`_ and `Pull Request #1397`_.  Both approaches
have a system performance impact, which varies for each CPU type and use-case.
The mitigation code is enabled by default, but can be disabled at compile time
for platforms that are unaffected or where the risk is deemed low enough.

Arm CPUs not mentioned below are unaffected.

Static mitigation
-----------------

For affected CPUs, this approach enables the mitigation during EL3
initialization, following every PE reset. No mechanism is provided to disable
the mitigation at runtime.

This approach permanently mitigates the entire software stack and no additional
mitigation code is required in other software components.

TF-A implements this approach for the following affected CPUs:

- Cortex-A57 and Cortex-A72, by setting bit 55 (Disable load pass store) of
  ``CPUACTLR_EL1`` (``S3_1_C15_C2_0``).

- Cortex-A73, by setting bit 3 of ``S3_0_C15_C0_0`` (not documented in the
  Technical Reference Manual (TRM)).

- Cortex-A75, by setting bit 35 (reserved in TRM) of ``CPUACTLR_EL1``
  (``S3_0_C15_C1_0``).

Dynamic mitigation
------------------

For affected CPUs, this approach also enables the mitigation during EL3
initialization, following every PE reset. In addition, this approach implements
``SMCCC_ARCH_WORKAROUND_2`` in the Arm architectural range to allow callers at
lower exception levels to temporarily disable the mitigation in their execution
context, where the risk is deemed low enough. This approach enables mitigation
on entry to EL3, and restores the mitigation state of the lower exception level
on exit from EL3. For more information on this approach, see `Firmware
interfaces for mitigating cache speculation vulnerabilities`_.

This approach may be complemented by additional mitigation code in other
software components, for example code that calls ``SMCCC_ARCH_WORKAROUND_2``.
However, even without any mitigation code in other software components, this
approach will effectively permanently mitigate the entire software stack, since
the default mitigation state for firmware-managed execution contexts is enabled.

Since the expectation in this approach is that more software executes with the
mitigation disabled, this may result in better system performance than the
static approach for some systems or use-cases.  However, for other systems or
use-cases, this performance saving may be outweighed by the additional overhead
of ``SMCCC_ARCH_WORKAROUND_2`` calls and TF-A exception handling.

TF-A implements this approach for the following affected CPU:

- Cortex-A76, by setting and clearing bit 16 (reserved in TRM) of
  ``CPUACTLR2_EL1`` (``S3_0_C15_C1_1``).

.. _Google Project Zero: https://bugs.chromium.org/p/project-zero/issues/detail?id=1528
.. _Arm Processor Security Update: http://www.arm.com/security-update
.. _CVE-2018-3639: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-3639
.. _Software Delegated Exception Interface (SDEI): http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _Firmware interfaces for mitigating cache speculation vulnerabilities: https://developer.arm.com/cache-speculation-vulnerability-firmware-specification
.. _Pull Request #1392: https://github.com/ARM-software/arm-trusted-firmware/pull/1392
.. _Pull Request #1397: https://github.com/ARM-software/arm-trusted-firmware/pull/1397
