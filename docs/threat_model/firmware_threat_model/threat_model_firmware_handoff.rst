Threat Model for Firmware Handoff
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Introduction
************

This document provides a threat model of TF-A firmware for platforms that make
use of the `Firmware Handoff` framework, which is implemented via the Transfer
List library (libTL).

Target of Evaluation
********************

The target of evaluation is the `Transfer List library` implementation of the
Firmware Handoff specification. This library is a generic implementation shared
by A class firmware components. It is used to transfer state/metadata (e.g.,
Device Trees, entrypoint information) between boot stages in TF-A. This includes
the boot ROM (BL1), trusted boot firmware (BL2), and runtime EL3 firmware
(BL31).

Assumptions
===========

The following assumptions are made about the TF-A Firmware Handoff framework
implementation in TF-A:

- Within the context of Trusted Board Boot (|TBB|), the Transfer List (TL) producer is
  considered integrity checked.
- The shared buffer containing the TL is retained until it's consumed, and all
  necessary cache/coherency operations are performed to ensure the TL is visible to
  the receiver.
- Handoff between stages is atomic; that is, there aren't incremental updates or
  partial reads.

Data Flow Diagram
=================

From a system perspective, we re-use the :ref:`Generic TF-A threat model
<tfa_dfd>` Data Flow Diagram (DFD). A generalized DFD showing information flow
between a TL producer and receiver is presented below. The dataflow DF7 in this
diagram is representative of dataflows in TF-A where Firmware Handoff might be
used (e.g., ``DF4``, and ``DF5`` in the generic model).

.. uml:: ../../resources/diagrams/plantuml/firmware_handoff_dfd.puml
  :caption: Figure 1: Generalized Firmware Handoff Data Flow Diagram

.. table:: Table 1: TF-A Data Flow Diagram Description

  +-----------------+--------------------------------------------------------+
  | Diagram Element | Description                                            |
  +=================+========================================================+
  |       DF7       | | Information produced during any stage of the AP boot |
  |                 |   process must be stored in or referenced by the       |
  |                 |   TL so that later stages can access                   |
  |                 |   and use it.                                          |
  +-----------------+--------------------------------------------------------+

Threat Assessment
=================

This analysis follows the same methodology as the :ref:`Generic TF-A threat
model <threat_analysis>`, adopting the STRIDE framework to categorize and assess
threats.

From the library’s perspective, the primary asset is the sensitive data conveyed
through the TL — including platform configuration data, firmware metadata, and
potentially security-critical values exchanged between boot stages or execution
environments.

The library’s main security responsibility is to manage the TL safely
and correctly, ensuring that:

- Sensitive data is not corrupted, tampered with, or misrouted during processing.
- Confidential information within the TL is not exposed to unauthorized entities
  or domains.
- All data handling operations preserve integrity, confidentiality, and
  availability across trust boundaries (e.g., secure vs. non-secure world).

This includes protection against both malicious manipulation and accidental
faults, as well as validation of inputs and outputs to prevent memory corruption
or data leakage.

The following table analyses the :ref:`General Threats` in the context of the
Firmware Handoff library.

  +----+--------------------------------------------------------+
  | ID | Comments                                               |
  +====+========================================================+
  | 05 | | As in the general case, it’s common to use logging   |
  |    |   to print information about a TL to the console.      |
  |    |   Because a TL can include sensitive data, the         |
  |    |   implementation must prevent said content from being  |
  |    |   written to the console.                              |
  |    |                                                        |
  |    | | Do not log the contents of Transfer Entries when     |
  |    |   logging is enabled. In production releases only      |
  |    |   print diagnostic error messages and ensure they do   |
  |    |   not contain sensitive information.                   |
  |    |                                                        |
  |    | | Disable logging by default, and ensure users         |
  |    |   explicitly opt in and configure their preferred      |
  |    |   logging setup.                                       |
  +----+--------------------------------------------------------+
  | 08 | | A producer typically performs various out-of-band    |
  |    |   memory operations on the TL. Common scenarios        |
  |    |   include authentication and loading of the platform   |
  |    |   |FDT|, and relocation of the TL from one buffer to   |
  |    |   another.                                             |
  |    |                                                        |
  |    | | Any of these operations may corrupt the TL due to    |
  |    |   copy-related bugs  (e.g., errors in ``memcpy`` or    |
  |    |   similar  memory manipulation functions).             |
  |    |                                                        |
  |    | | Verify the checksum before consuming a TL.           |
  |    |                                                        |
  |    | | Note that this mitigation does not stop an           |
  |    |   attacker from altering the data and                  |
  |    |   and recalculating the checksum; checksums            |
  |    |   only help detect data corruption.                    |
  +----+--------------------------------------------------------+
  | 13 | | A producer may use a TL to convey sensitive data     |
  |    |   between stages. If the same TL is used in successive |
  |    |   stages (e.g., between TF-A  boot stages), sensitive  |
  |    |   data might be  leaked to later stages.               |
  |    |                                                        |
  |    | | This can occur when information is not properly      |
  |    |   scrubbed during remove operations; or, when one      |
  |    |   stage relocates a  TL from one location to another   |
  |    |   (e.g., on-chip SRAM to non-secure DRAM) without      |
  |    |   removing sensitive data.                             |
  |    |                                                        |
  |    | | On TE remove, provide a mechanism for scrubbing an   |
  |    |   entire transfer entry.                               |
  +----+--------------------------------------------------------+

*Copyright (c) 2025, Arm Limited. All rights reserved.*

.. _Firmware Handoff: https://firmwarehandoff.github.io/firmware_handoff/main/index.html
.. _Transfer List library: https://review.trustedfirmware.org/plugins/gitiles/shared/transfer-list-library
