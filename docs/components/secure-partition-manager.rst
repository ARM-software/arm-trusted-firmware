Secure Partition Manager
************************

.. contents::

.. toctree::
  ffa-manifest-binding

Acronyms
========

+--------+--------------------------------------+
| DTS    | Device Tree Source                   |
+--------+--------------------------------------+
| FF-A   | Firmware Framework for Arm A-profile |
+--------+--------------------------------------+
| NWd    | Normal World                         |
+--------+--------------------------------------+
| SP     | Secure Partition                     |
+--------+--------------------------------------+
| SPD    | Secure Payload Dispatcher            |
+--------+--------------------------------------+
| SPM    | Secure Partition Manager             |
+--------+--------------------------------------+
| SPMC   | SPM Core                             |
+--------+--------------------------------------+
| SPMD   | SPM Dispatcher                       |
+--------+--------------------------------------+
| SWd    | Secure World                         |
+--------+--------------------------------------+

Foreword
========

Three implementations of a Secure Partition Manager co-exist in the TF-A
codebase:

#. S-EL2 SPMC based on the FF-A specification `[1]`_, enabling virtualization in
   the secure world, managing multiple S-EL1 or S-EL0 partitions `[5]`_.
#. EL3 SPMC based on the FF-A specification, managing a single S-EL1 partition
   without virtualization in the secure world `[6]`_.
#. EL3 SPM based on the MM specification, legacy implementation managing a
   single S-EL0 partition `[2]`_.

These implementations differ in their respective SW architecture and only one
can be selected at build time.

Support for legacy platforms
----------------------------

The SPM is split into a dispatcher and a core component (respectively SPMD and
SPMC) residing at different exception levels. To permit the FF-A specification
adoption and a smooth migration, the SPMD supports an SPMC residing either at
S-EL1 or S-EL2:

- The SPMD is located at EL3 and mainly relays the FF-A protocol from NWd
  (Hypervisor or OS kernel) to the SPMC.
- The same SPMD component is used for both S-EL1 and S-EL2 SPMC configurations.
- The SPMC exception level is a build time choice.

TF-A supports both cases:

- S-EL1 SPMC for platforms not supporting the FEAT_SEL2 architecture
  extension. The SPMD relays the FF-A protocol from EL3 to S-EL1.
- S-EL2 SPMC for platforms implementing the FEAT_SEL2 architecture
  extension. The SPMD relays the FF-A protocol from EL3 to S-EL2.

TF-A build options
==================

This section explains the TF-A build options involved in building with
support for an FF-A based SPM where the SPMD is located at EL3 and the
SPMC located at S-EL1, S-EL2 or EL3:

- **SPD=spmd**: this option selects the SPMD component to relay the FF-A
  protocol from NWd to SWd back and forth. It is not possible to
  enable another Secure Payload Dispatcher when this option is chosen.
- **SPMD_SPM_AT_SEL2**: this option adjusts the SPMC exception
  level to being at S-EL2. It defaults to enabled (value 1) when
  SPD=spmd is chosen.
- **SPMC_AT_EL3**: this option adjusts the SPMC exception level to being
  at EL3. If neither ``SPMD_SPM_AT_SEL2`` or ``SPMC_AT_EL3`` are enabled the
  SPMC exception level is set to S-EL1.
  ``SPMD_SPM_AT_SEL2`` is enabled. The context save/restore routine
  and exhaustive list of registers is visible at `[4]`_.
- **SPMC_AT_EL3_SEL0_SP**: this option enables the support to load SEL0 SP
  when SPMC at EL3 support is enabled.
- **SP_LAYOUT_FILE**: this option specifies a text description file
  providing paths to SP binary images and manifests in DTS format
  (see `[3]`_). It
  is required when ``SPMD_SPM_AT_SEL2`` is enabled hence when multiple
  secure partitions are to be loaded by BL2 on behalf of the SPMC.

+---------------+------------------+-------------+-------------------------+
|               | SPMD_SPM_AT_SEL2 | SPMC_AT_EL3 | CTX_INCLUDE_EL2_REGS(*) |
+---------------+------------------+-------------+-------------------------+
| SPMC at S-EL1 |        0         |      0      |             0           |
+---------------+------------------+-------------+-------------------------+
| SPMC at S-EL2 | 1 (default when  |      0      |             1           |
|               |    SPD=spmd)     |             |                         |
+---------------+------------------+-------------+-------------------------+
| SPMC at EL3   |        0         |      1      |             0           |
+---------------+------------------+-------------+-------------------------+

Other combinations of such build options either break the build or are not
supported.

Notes:

- Only Arm's FVP platform is supported to use with the TF-A reference software
  stack.
- When ``SPMD_SPM_AT_SEL2=1``, the reference software stack assumes enablement
  of FEAT_PAuth, FEAT_BTI and FEAT_MTE2 architecture extensions.
- ``(*) CTX_INCLUDE_EL2_REGS``, this flag is |TF-A| internal and informational
  in this table. When set, it provides the generic support for saving/restoring
  EL2 registers required when S-EL2 firmware is present.
- BL32 option is re-purposed to specify the SPMC image. It can specify either
  the Hafnium binary path (built for the secure world) or the path to a TEE
  binary implementing FF-A interfaces.
- BL33 option can specify the TFTF binary or a normal world loader
  such as U-Boot or the UEFI framework payload.

Sample TF-A build command line when the SPMC is located at S-EL1
(e.g. when the FEAT_SEL2 architecture extension is not implemented):

.. code:: shell

    make \
    CROSS_COMPILE=aarch64-none-elf- \
    SPD=spmd \
    SPMD_SPM_AT_SEL2=0 \
    BL32=<path-to-tee-binary> \
    BL33=<path-to-bl33-binary> \
    PLAT=fvp \
    all fip

Sample TF-A build command line when FEAT_SEL2 architecture extension is
implemented and the SPMC is located at S-EL2:

.. code:: shell

    make \
    CROSS_COMPILE=aarch64-none-elf- \
    PLAT=fvp \
    SPD=spmd \
    ARM_ARCH_MINOR=5 \
    BRANCH_PROTECTION=1 \
    CTX_INCLUDE_PAUTH_REGS=1 \
    ENABLE_FEAT_MTE2=1 \
    BL32=<path-to-hafnium-binary> \
    BL33=<path-to-bl33-binary> \
    SP_LAYOUT_FILE=sp_layout.json \
    all fip

Sample TF-A build command line when FEAT_SEL2 architecture extension is
implemented, the SPMC is located at S-EL2, and enabling secure boot:

.. code:: shell

    make \
    CROSS_COMPILE=aarch64-none-elf- \
    PLAT=fvp \
    SPD=spmd \
    ARM_ARCH_MINOR=5 \
    BRANCH_PROTECTION=1 \
    CTX_INCLUDE_PAUTH_REGS=1 \
    ENABLE_FEAT_MTE2=1 \
    BL32=<path-to-hafnium-binary> \
    BL33=<path-to-bl33-binary> \
    SP_LAYOUT_FILE=sp_layout.json \
    MBEDTLS_DIR=<path-to-mbedtls-lib> \
    TRUSTED_BOARD_BOOT=1 \
    COT=dualroot \
    ARM_ROTPK_LOCATION=devel_rsa \
    ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem \
    GENERATE_COT=1 \
    all fip

Sample TF-A build command line when the SPMC is located at EL3:

.. code:: shell

    make \
    CROSS_COMPILE=aarch64-none-elf- \
    SPD=spmd \
    SPMD_SPM_AT_SEL2=0 \
    SPMC_AT_EL3=1 \
    BL32=<path-to-tee-binary> \
    BL33=<path-to-bl33-binary> \
    PLAT=fvp \
    all fip

Sample TF-A build command line when the SPMC is located at EL3 and SEL0 SP is
enabled:

.. code:: shell

    make \
    CROSS_COMPILE=aarch64-none-elf- \
    SPD=spmd \
    SPMD_SPM_AT_SEL2=0 \
    SPMC_AT_EL3=1 \
    SPMC_AT_EL3_SEL0_SP=1 \
    BL32=<path-to-tee-binary> \
    BL33=<path-to-bl33-binary> \
    PLAT=fvp \
    all fip

Boot process
============

The boot process involving SPMC is highly dependent on the SPMC implementation.
It is recommended to refer to corresponding SPMC documentation for further
details. Some aspects of boot process are described here in the greater interest
of the project.

SPMC boot
---------

When SPMC resides at a lower EL i.e., S-EL1 or S-EL2, it is loaded by BL2 as the
BL32 image. The SPMC manifest is loaded by BL2 as the ``TOS_FW_CONFIG`` image `[7]`_.

BL2 passes the SPMC manifest address to BL31 through a register. At boot time,
the SPMD in BL31 runs from the primary core, initializes the core contexts and
launches the SPMC (BL32) passing the following information through registers:

- X0 holds the ``TOS_FW_CONFIG`` physical address (or SPMC manifest blob).
- X1 holds the ``HW_CONFIG`` physical address.
- X4 holds the currently running core linear id.


References
==========

.. _[1]:

[1] `Arm Firmware Framework for Arm A-profile <https://developer.arm.com/docs/den0077/latest>`__

.. _[2]:

[2] :ref:`Secure Partition Manager using MM interface<Secure Partition Manager (MM)>`

.. _[3]:

[3] https://hafnium.readthedocs.io/en/latest/secure-partition-manager/secure-partition-manager.html#secure-partitions-layout-file

.. _[4]:

[4] https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/lib/el3_runtime/aarch64/context.S#n45

.. _[5]:

[5] https://hafnium.readthedocs.io/en/latest/secure-partition-manager/index.html

.. _[6]:

[6] :ref:`EL3 Secure Partition Manager<EL3 Secure Partition Manager>`

.. _[7]:

[7] https://trustedfirmware-a.readthedocs.io/en/latest/design/firmware-design.html#dynamic-configuration-during-cold-boot

--------------

*Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.*
