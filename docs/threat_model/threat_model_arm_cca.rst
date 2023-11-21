Threat Model for TF-A with Arm CCA support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Introduction
************

This document provides a threat model of TF-A firmware for platforms with Arm
Realm Management Extension (RME) support which implement Arm Confidential
Compute Architecture (Arm CCA).

Although it is a separate document, it references the :ref:`Generic Threat
Model` in a number of places, as some of the contents is commonly applicable to
TF-A with or without Arm CCA support.

Target of Evaluation
********************

In this threat model, the target of evaluation is the Trusted Firmware for
A-class Processors (TF-A) with RME support and Arm CCA support. This includes
the boot ROM (BL1), the trusted boot firmware (BL2) and the runtime EL3 firmware
(BL31).

Assumptions
===========

We make the following assumptions:

- :ref:`Realm Management Extension (RME)` is enabled on the platform.

- Arm CCA Hardware Enforced Security (HES) is available on the platform, as
  recommended by `Arm CCA security model`_:

    *[R0004] Arm strongly recommends that all implementations of CCA utilize*
    *hardware enforced security (CCA HES).*

- All TF-A images run from on-chip memory. Data used by these images also live
  in on-chip memory. This means TF-A is not vulnerable to an attacker that can
  probe or tamper with off-chip memory.

  These are requirements of the `Arm CCA security model`_:

    *[R0147] Monitor code executes entirely from on-chip memory.*

    *[R0149] Any monitor data that may affect the CCA security guarantee, other*
    *than GPT, is either held in on-chip memory, or in external memory but with*
    *additional integrity protection.*

  Note that this threat model hardens *[R0149]* requirement by forbidding to
  hold data in external memory, even if it is integrity-protected - except for
  GPT data.

- TF-A BL1 image is immutable and thus implicitly trusted. It runs from
  read-only memory or write-protected memory. This could be on-chip ROM, on-chip
  OTP, locked on-chip flash, or write-protected on-chip RAM for example.

  This is a requirement of the `Arm CCA security model`_:

    *[R0158] Arm recommends that all initial boot code is immutable on a*
    *secured system.*

    *[R0050] If all or part of initial boot code is instantiated in on-chip*
    *memory then other trusted subsystems or application PE cannot modify that*
    *code before it has been executed.*

- Trusted boot and measured boot are enabled. This means an attacker can't boot
  arbitrary images that are not approved by platform providers.

  These are requirements of the `Arm CCA security model`_:

    *[R0048] A secured system can only load authorized CCA firmware.*

    *[R0079] All Monitor firmware loaded by PE initial boot is measured and*
    *verified as outlined in Verified boot.*

- No experimental features are enabled. These are typically incomplete features,
  which need more time to stabilize. Thus, we do not consider threats that may
  come from them. It is not recommended to use these features in production
  builds.

Data Flow Diagram
=================

Figure 1 shows a high-level data flow diagram for TF-A. The diagram shows a
model of the different components of a TF-A-based system and their interactions
with TF-A. A description of each diagram element is given on Table 1. On the
diagram, the red broken lines indicate trust boundaries. Components outside of
the broken lines are considered untrusted by TF-A.

.. uml:: ../resources/diagrams/plantuml/tfa_arm_cca_dfd.puml
  :caption: Figure 1: Data Flow Diagram

.. table:: Table 1: Data Flow Diagram Description

  +-----------------+--------------------------------------------------------+
  | Diagram Element | Description                                            |
  +=================+========================================================+
  |       DF1       | | Refer to DF1 description in the                      |
  |                 |   :ref:`Generic Threat Model`. Additionally TF-A       |
  |                 |   loads realm images.                                  |
  +-----------------+--------------------------------------------------------+
  |     DF2-DF6     | | Refer to DF2-DF6 descriptions in the                 |
  |                 |   :ref:`Generic Threat Model`.                         |
  +-----------------+--------------------------------------------------------+
  |       DF7       | | Boot images interact with Arm CCA HES to record boot |
  |                 |   measurements and retrieve data used for AP images    |
  |                 |   authentication.                                      |
  |                 |                                                        |
  |                 | | The runtime firmware interacts with Arm CCA HES to   |
  |                 |   obtain sensitive attestation data for the realm      |
  |                 |   world.                                               |
  +-----------------+--------------------------------------------------------+
  |       DF8       | | Realm world software (e.g. TF-RMM) interact with     |
  |                 |   TF-A through SMC call interface and/or shared        |
  |                 |   memory.                                              |
  +-----------------+--------------------------------------------------------+

Threat Analysis
***************

In this threat model, we use the same method to analyse threats as in the
:ref:`Generic Threat Model`. This section only points out differences where
applicable.

- There is an additional threat agent: *RealmCode*. It takes the form of
  malicious or faulty code running in the realm world, including R-EL2, R-EL1
  and R-EL0 levels.

- At this time we only consider the ``Server`` target environment. New threats
  identified in this threat model will only be given a risk rating for this
  environment. Other environments may be added in a future revision

Threat Assessment
=================

General Threats for All Firmware Images
---------------------------------------

The following table analyses the :ref:`General Threats` in the context of this
threat model. Only deltas are pointed out.

  +----+-------------+-------------------------------------------------------+
  | ID | Applicable? | Comments                                              |
  +====+=============+=======================================================+
  | 05 |     Yes     |                                                       |
  +----+-------------+-------------------------------------------------------+
  | 06 |     Yes     |                                                       |
  +----+-------------+-------------------------------------------------------+
  | 08 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 11 |     Yes     | | Misconfiguration of the Memory Management Unit      |
  |    |             |   (MMU) may allow a **normal/secure/realm** world     |
  |    |             |   software to access sensitive data, execute arbitrary|
  |    |             |   code or access otherwise restricted HW interface.   |
  |    |             |                                                       |
  |    |             | | **Note that on RME systems, MMU configuration also  |
  |    |             |   includes Granule Protection Tables (GPT) setup.**   |
  |    |             |                                                       |
  |    |             | | Additional diagram elements: DF4, DF7, DF8.         |
  |    |             |                                                       |
  |    |             | | Additional threat agents: SecCode, RealmCode.       |
  +----+-------------+-------------------------------------------------------+
  | 13 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 15 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

The following table analyses the :ref:`Boot Firmware Threats` in the context of
this threat model. Only deltas are pointed out.

  +----+-------------+-------------------------------------------------------+
  | ID | Applicable? | Comments                                              |
  +====+=============+=======================================================+
  | 01 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 02 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 03 |     Yes     |                                                       |
  +----+-------------+-------------------------------------------------------+
  | 04 |     Yes     |                                                       |
  +----+-------------+-------------------------------------------------------+

Threats to be Mitigated by the Runtime EL3 Firmware
---------------------------------------------------

The following table analyses the :ref:`Runtime Firmware Threats` in the context
of this threat model. Only deltas are pointed out.

  +----+-------------+-------------------------------------------------------+
  | ID | Applicable? | Comments                                              |
  +====+=============+=======================================================+
  | 07 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 09 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 10 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 12 |     Yes     | Additional diagram element: DF8.                      |
  |    |             |                                                       |
  |    |             | Additional threat agent: RealmCode.                   |
  +----+-------------+-------------------------------------------------------+
  | 14 |     Yes     |                                                       |
  +----+-------------+-------------------------------------------------------+

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _Arm CCA Security Model: https://developer.arm.com/documentation/DEN0096/A_a
