Advisory TFV-14 (CVE-2026-34879)
================================

+----------------+-----------------------------------------------------------------+
| Title          | BL1 FWU range-check mismatch in FWU_SMC_IMAGE_COPY can cause    |
|                | out-of-range memory access.                                     |
+================+=================================================================+
| CVE ID         | `CVE-2026-34879`_                                               |
+----------------+-----------------------------------------------------------------+
| Date           | Reported on 16 Jan 2026                                         |
+----------------+-----------------------------------------------------------------+
| Versions       | TF-A releases with BL1 FWU mode enabled prior to the fixes      |
| Affected       | in `48739`_ and `48740`_.                                       |
+----------------+-----------------------------------------------------------------+
| Configurations | Platforms that enable BL1 FWU mode and expose                   |
| Affected       | FWU_SMC_IMAGE_COPY to the Normal World.                         |
+----------------+-----------------------------------------------------------------+
| Impact         | Denial of Service (secure world data abort / reboot loop).      |
+----------------+-----------------------------------------------------------------+
| Fix Version    | `48739`_ "fix(bl1): harden FWU copy/auth overflow checks"       |
|                | `48740`_ "fix(arm): harden FWU address range checks"            |
+----------------+-----------------------------------------------------------------+
| Credit         | Prasanth Sundararajan                                           |
|                | https://www.linkedin.com/in/prasanthsundararajan/               |
+----------------+-----------------------------------------------------------------+

Description
-----------

In BL1 Firmware Update (FWU) mode, the FWU_SMC_IMAGE_COPY path validated
image ranges using an overflow check based on (size - 1) but then consumed
the full block size in subsequent range checks and memcpy operations. This
created a mismatch where certain malformed requests could pass validation
and later trigger a memcpy from an invalid or unmapped address.

A compromised Normal World component can issue a malformed FWU_SMC_IMAGE_COPY
request while the device is in FWU mode, pass validation, and cause BL1 to
attempt to copy from an invalid address. In testing, this results in a BL1
data abort and can lead to a secure-side reboot loop (availability impact).

Exploitability Details
----------------------

This issue is exploitable only when the device is running in BL1 FWU mode and
the `FWU_SMC_IMAGE_COPY` interface is reachable from the Normal World. An
attacker needs control of a Normal World component capable of issuing SMC calls
in FWU mode (for example, a compromised bootloader, recovery image, or update
agent). No secure-world code execution is required; the attack is performed
entirely through crafted FWU requests.

The attacker submits a request with carefully chosen source address and size
parameters that pass the legacy range validation but produce a different
effective range when the copy occurs. Because the copy uses the full block size,
BL1 attempts to read from an invalid or unmapped address, resulting in a
secure-world data abort. In practice this yields a denial of service in FWU
mode, often observed as a secure-side reboot loop.

Mitigation and Recommendations
------------------------------

The issue is fixed by making overflow and range validation match the actual
block size used by bl1_plat_mem_check() and memcpy(), and by validating
end-exclusive ranges computed safely from (base + size).

Users should apply the fixes in the commits referenced below and ensure that
all TF-A builds enabling BL1 FWU mode include these changes.

.. _CVE-2026-34879: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2026-34879
.. _48739: https://review.trustedfirmware.org/c/TF-A/trusted-firmware-a/+/48739
.. _48740: https://review.trustedfirmware.org/c/TF-A/trusted-firmware-a/+/48740
