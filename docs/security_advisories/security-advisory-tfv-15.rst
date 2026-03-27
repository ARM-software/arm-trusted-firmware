Advisory TFV-15 (CVE-2026-34878)
================================

+----------------+-----------------------------------------------------------------+
| Title          | Insufficient validation of FIP ToC offsets in BL1/BL2 can cause |
|                | unintended reads and secure memory disclosure at boot.          |
+================+=================================================================+
| CVE ID         | `CVE-2026-34878`_                                               |
+----------------+-----------------------------------------------------------------+
| Date           | Reported on 22 Jan 2026                                         |
+----------------+-----------------------------------------------------------------+
| Versions       | TF-A releases prior to the fixes in `48351`_, `49485`_.         |
| Affected       |                                                                 |
+----------------+-----------------------------------------------------------------+
| Configurations | Platforms that load BL1/BL2 from a FIP and allow firmware       |
| Affected       | update or boot from a modified FIP.                             |
+----------------+-----------------------------------------------------------------+
| Impact         | Secure memory disclosure during boot and potential DoS          |
|                | (boot halts on failed authentication).                          |
+----------------+-----------------------------------------------------------------+
| Fix Version    | `48351`_ "fix(io): validate FIP ToC bounds and catch short      |
|                | reads"                                                          |
|                | `49485`_ "feat(lib): add u64 overflow helper"                   |
|                | checks"                                                         |
+----------------+-----------------------------------------------------------------+
| Credit         | Kevin Chen                                                      |
+----------------+-----------------------------------------------------------------+

Description
-----------

Even when Trusted Boot is enabled, the FIP metadata (header and ToC) is not
cryptographically signed by design and must be treated as untrusted input.
BL1/BL2 parse the FIP ToC to locate images, calculate offsets and lengths, and
perform reads and copies before authentication is performed. The existing
logic did not sufficiently validate FIP metadata fields (including ToC
offsets, sizes, and read lengths) and did not consistently detect short reads.
A crafted or corrupted FIP can therefore cause BL1/BL2 to read and copy
unintended data-including secure memory mapped into the EL3 translation
regime-into non-secure memory regions, potentially disclosing sensitive
contents.

This issue affects boot-time images only (BL1/BL2), not runtime firmware
(BL31). The exposure exists only during early boot while BL1/BL2 are parsing
the FIP and copying images; once authentication fails, boot halts. In most
deployments, useful secrets are not resident in memory during that early
boot interval. In configurations where TF-A decrypts confidential images into
Trusted SRAM, the disclosure can expose plaintext at boot. Note that BL33 is
typically loaded into non-secure memory, so not all image copies target secure
buffers during authentication. In practice, the same malformed metadata
typically leads to authentication failure and halts the boot process,
resulting in a DoS.

Exploitability Details
----------------------

An attacker must be able to supply or modify the FIP that BL1/BL2 will parse,
for example via a firmware update mechanism, removable storage, or a system
configuration that allows boot from an untrusted FIP source. Because the FIP
metadata is unsigned, Trusted Boot does not protect the ToC from tampering.
By crafting offsets and sizes that point outside the actual FIP payload, or
that trigger integer overflow, the attacker can force BL1/BL2 to issue reads
that extend beyond the FIP and into adjacent regions mapped in secure address
space. The incorrect or short reads can then be copied into the destination
buffers for authentication, which fails, halting the boot process.

Mitigation and Recommendations
------------------------------

Apply the fixes referenced below. They add strict bounds checks, overflow-safe
validation, and short-read detection for FIP metadata parsing and associated
copy operations.

Where feasible, reduce exposure by ensuring the FIP is stored in a protected
location and by restricting or disabling boot paths that accept externally
modifiable FIP images.

.. _CVE-2026-34878: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2026-34878
.. _48351: https://review.trustedfirmware.org/c/TF-A/trusted-firmware-a/+/48351
.. _49485: https://review.trustedfirmware.org/c/TF-A/trusted-firmware-a/+/49485
