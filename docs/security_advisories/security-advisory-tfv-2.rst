Advisory TFV-2 (CVE-2017-7564)
==============================

+----------------+-------------------------------------------------------------+
| Title          | Enabled secure self-hosted invasive debug interface can     |
|                | allow normal world to panic secure world                    |
+================+=============================================================+
| CVE ID         | `CVE-2017-7564`_                                            |
+----------------+-------------------------------------------------------------+
| Date           | 02 Feb 2017                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | All versions up to v1.3                                     |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Denial of Service (secure world panic)                      |
+----------------+-------------------------------------------------------------+
| Fix Version    | 15 Feb 2017 `Pull Request #841`_                            |
+----------------+-------------------------------------------------------------+
| Credit         | ARM                                                         |
+----------------+-------------------------------------------------------------+

The ``MDCR_EL3.SDD`` bit controls AArch64 secure self-hosted invasive debug
enablement. By default, the BL1 and BL31 images of the current version of ARM
Trusted Firmware (TF) unconditionally assign this bit to ``0`` in the early
entrypoint code, which enables debug exceptions from the secure world. This can
be seen in the implementation of the ``el3_arch_init_common`` `AArch64 macro`_ .
Given that TF does not currently contain support for this feature (for example,
by saving and restoring the appropriate debug registers), this may allow a
normal world attacker to induce a panic in the secure world.

The ``MDCR_EL3.SDD`` bit should be assigned to ``1`` to disable debug exceptions
from the secure world.

Earlier versions of TF (prior to `commit 495f3d3`_) did not assign this bit.
Since the bit has an architecturally ``UNKNOWN`` reset value, earlier versions
may or may not have the same problem, depending on the platform.

A similar issue applies to the ``MDCR_EL3.SPD32`` bits, which control AArch32
secure self-hosted invasive debug enablement. TF assigns these bits to ``00``
meaning that debug exceptions from Secure EL1 are enabled by the authentication
interface. Therefore this issue only exists for AArch32 Secure EL1 code when
secure privileged invasive debug is enabled by the authentication interface, at
which point the device is vulnerable to other, more serious attacks anyway.

However, given that TF contains no support for handling debug exceptions, the
``MDCR_EL3.SPD32`` bits should be assigned to ``10`` to disable debug exceptions
from AArch32 Secure EL1.

Finally, this also issue applies to AArch32 platforms that use the TF SP_MIN
image or integrate the `AArch32 equivalent`_ of the ``el3_arch_init_common``
macro. Here the affected bits are ``SDCR.SPD``, which should also be assigned to
``10`` instead of ``00``

.. _CVE-2017-7564: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-7564
.. _commit 495f3d3: https://github.com/ARM-software/arm-trusted-firmware/commit/495f3d3
.. _AArch64 macro: https://github.com/ARM-software/arm-trusted-firmware/blob/bcc2bf0/include/common/aarch64/el3_common_macros.S#L85
.. _AArch32 equivalent: https://github.com/ARM-software/arm-trusted-firmware/blob/bcc2bf0/include/common/aarch32/el3_common_macros.S#L41
.. _Pull Request #841: https://github.com/ARM-software/arm-trusted-firmware/pull/841
