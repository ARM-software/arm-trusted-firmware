Threat Model for TF-A with PSA FWU or TBBR FWU support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Introduction
************

This document provides a threat model of TF-A firmware for platforms with
the feature PSA firmware update or TBBR firmware update or both enabled.
To understand the design of the firmware update refer
:ref:`Firmware Update (FWU)`.

Although it is a separate document, it references the :ref:`Generic Threat
Model` in a number of places, as some of the contents are applicable to this
threat model.

Target of Evaluation
********************

In this threat model, the target of evaluation is the Trusted Firmware for
A-class Processors (TF-A) when PSA FWU support is enabled or TBBR FWU mode
is enabled. This includes the boot ROM (BL1), the trusted boot firmware (BL2).

Threat Assessment
*****************

For this section, please reference the Threat Assessment under the
:ref:`Generic Threat Model`. Here only the differences are highlighted.

PSA FWU
*******

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

The following table analyses the :ref:`Boot Firmware Threats` in the context
of this threat model. Only additional details are pointed out.

+----+-------------+-------------------------------------------------------+
| ID | Applicable? | Comments                                              |
+====+=============+=======================================================+
| 01 |     Yes     | | Attacker can use arbitrary images to update the     |
|    |             |   system.                                             |
+----+-------------+-------------------------------------------------------+
| 02 |     Yes     | | Attacker tries to update the system with the        |
|    |             |   vulnerable/older firmware.                          |
+----+-------------+-------------------------------------------------------+
| 03 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 04 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+


Threats to be mitigated by platform design
------------------------------------------

PSA FWU is driven by metadata stored in non-volatile storage. This metadata
is not cryptographically signed. Also, depending on the hardware design,
it may be stored in untrusted storage, which makes it possible for software
outside of TF-A security boundary or for a physical attacker to modify it
in order to change the behaviour of the FWU process.

Below we provide some possible FWU metadata corruption scenarios:

1. The FWU metadata includes the firmware bank for booting; the attacker
   tries to modify it to prevent the execution of the updated firmware.
2. The FWU metadata features a field indicating the firmware's status, either
   in trial run or accepted run. The attacker tries to manipulate this field,
   ensuring the updated firmware consistently runs in trial mode, with the
   intention of preventing the anti-rollback update.

By design, no software mitigations exist to prevent this. The safeguarding
of FWU metadata relies on the platform's hardware design to mitigate potential
attacks on it, if this is a concern in the platform's threat model.
For example, FWU metadata may be stored in secure storage under exclusive
access from secure software, protecting it from physical, unauthenticated
accesses and from non-secure software accesses.

TBBR FWU - Firmware Recovery
****************************

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

The following table analyses the :ref:`Boot Firmware Threats` in the context
of this threat model. Only additional details are pointed out.

+----+-------------+-------------------------------------------------------+
| ID | Applicable? | Comments                                              |
+====+=============+=======================================================+
| 01 |     Yes     | | Attacker can use arbitrary images to recover the    |
|    |             |   system.                                             |
+----+-------------+-------------------------------------------------------+
| 02 |     Yes     | | Attacker tries to recover the system with the       |
|    |             |   vulnerable/older firmware.                          |
+----+-------------+-------------------------------------------------------+
| 03 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 04 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+

--------------

*Copyright (c) 2024, Arm Limited. All rights reserved.*
