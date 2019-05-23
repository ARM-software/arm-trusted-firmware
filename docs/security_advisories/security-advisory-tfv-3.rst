Advisory TFV-3 (CVE-2017-7563)
==============================

+----------------+-------------------------------------------------------------+
| Title          | RO memory is always executable at AArch64 Secure EL1        |
+================+=============================================================+
| CVE ID         | `CVE-2017-7563`_                                            |
+----------------+-------------------------------------------------------------+
| Date           | 06 Apr 2017                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | v1.3 (since `Pull Request #662`_)                           |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | AArch64 BL2, TSP or other users of xlat_tables library      |
| Affected       | executing at AArch64 Secure EL1                             |
+----------------+-------------------------------------------------------------+
| Impact         | Unexpected Privilege Escalation                             |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #924`_                                        |
+----------------+-------------------------------------------------------------+
| Credit         | ARM                                                         |
+----------------+-------------------------------------------------------------+

The translation table library in ARM Trusted Firmware (TF) (under
``lib/xlat_tables`` and ``lib/xlat_tables_v2``) provides APIs to help program
translation tables in the MMU. The xlat\_tables client specifies its required
memory mappings in the form of ``mmap_region`` structures.  Each ``mmap_region``
has memory attributes represented by the ``mmap_attr_t`` enumeration type. This
contains flags to control data access permissions (``MT_RO``/``MT_RW``) and
instruction execution permissions (``MT_EXECUTE``/``MT_EXECUTE_NEVER``). Thus a
mapping specifying both ``MT_RO`` and ``MT_EXECUTE_NEVER`` should result in a
Read-Only (RO), non-executable memory region.

This feature does not work correctly for AArch64 images executing at Secure EL1.
Any memory region mapped as RO will always be executable, regardless of whether
the client specified ``MT_EXECUTE`` or ``MT_EXECUTE_NEVER``.

The vulnerability is known to affect the BL2 and Test Secure Payload (TSP)
images on platforms that enable the ``SEPARATE_CODE_AND_RODATA`` build option,
which includes all ARM standard platforms, and the upstream Xilinx and NVidia
platforms. The RO data section for these images on these platforms is
unexpectedly executable instead of non-executable. Other platforms or
``xlat_tables`` clients may also be affected.

The vulnerability primarily manifests itself after `Pull Request #662`_.  Before
that, ``xlat_tables`` clients could not specify instruction execution
permissions separately to data access permissions. All RO normal memory regions
were implicitly executable. Before `Pull Request #662`_.  the vulnerability
would only manifest itself for device memory mapped as RO; use of this mapping
is considered rare, although the upstream QEMU platform uses this mapping when
the ``DEVICE2_BASE`` build option is used.

Note that one or more separate vulnerabilities are also required to exploit this
vulnerability.

The vulnerability is due to incorrect handling of the execute-never bits in the
translation tables. The EL3 translation regime uses a single ``XN`` bit to
determine whether a region is executable. The Secure EL1&0 translation regime
handles 2 Virtual Address (VA) ranges and so uses 2 bits, ``UXN`` and ``PXN``.
The ``xlat_tables`` library only handles the ``XN`` bit, which maps to ``UXN``
in the Secure EL1&0 regime. As a result, this programs the Secure EL0 execution
permissions but always leaves the memory as executable at Secure EL1.

The vulnerability is mitigated by the following factors:

- The xlat\_tables library ensures that all Read-Write (RW) memory regions are
  non-executable by setting the ``SCTLR_ELx.WXN`` bit. This overrides any value
  of the ``XN``, ``UXN`` or ``PXN`` bits in the translation tables. See the
  ``enable_mmu()`` function:

  ::

      sctlr = read_sctlr_el##_el();               \
      sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;       \

- AArch32 configurations are unaffected. Here the ``XN`` bit controls execution
  privileges of the currently executing translation regime, which is the desired
  behaviour.

- ARM TF EL3 code (for example BL1 and BL31) ensures that all non-secure memory
  mapped into the secure world is non-executable by setting the ``SCR_EL3.SIF``
  bit. See the ``el3_arch_init_common`` macro in ``el3_common_macros.S``.

.. _CVE-2017-7563: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-7563
.. _Pull Request #662: https://github.com/ARM-software/arm-trusted-firmware/pull/662
.. _Pull Request #924: https://github.com/ARM-software/arm-trusted-firmware/pull/924
