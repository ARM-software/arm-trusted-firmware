EL3 SPMC Threat Model
*********************

************
Introduction
************
This document provides a threat model for the TF-A `EL3 Secure Partition Manager`_
(EL3 SPM) implementation. The EL3 SPM implementation is based on the
`Arm Firmware Framework for Arm A-profile`_ specification.

********************
Target of Evaluation
********************
In this threat model, the target of evaluation is the ``Secure Partition Manager Core``
component (SPMC) within the EL3 firmware.
The monitor and SPMD at EL3 are covered by the `Generic TF-A threat model`_.

The scope for this threat model is:

- The TF-A implementation for the EL3 SPMC
- The implementation complies with the FF-A v1.1 specification.
- Secure partition is statically provisioned at boot time.
- Focus on the run-time part of the life-cycle (no specific emphasis on boot
  time, factory firmware provisioning, firmware udpate etc.)
- Not covering advanced or invasive physical attacks such as decapsulation,
  FIB etc.

Data Flow Diagram
=================
Figure 1 shows a high-level data flow diagram for the SPM split into an SPMD
and SPMC component at EL3. The SPMD mostly acts as a relayer/pass-through between
the normal world and the secure world. It is assumed to expose small attack surface.

A description of each diagram element is given in Table 1. In the diagram, the
red broken lines indicate trust boundaries.

Components outside of the broken lines are considered untrusted.

.. uml:: ../resources/diagrams/plantuml/el3_spm_dfd.puml
  :caption: Figure 1: EL3 SPMC Data Flow Diagram

.. table:: Table 1: EL3 SPMC Data Flow Diagram Description

  +---------------------+--------------------------------------------------------+
  | Diagram Element     | Description                                            |
  +=====================+========================================================+
  | DF1                 | SP to SPMC communication. FF-A function invocation or  |
  |                     | implementation-defined Hypervisor call.                |
  |                     |                                                        |
  |                     | Note:- To communicate with LSP, SP1 performs a direct  |
  |                     | message request to SPMC targeting LSP as destination.  |
  +---------------------+--------------------------------------------------------+
  | DF2                 | SPMC to SPMD communication.                            |
  +---------------------+--------------------------------------------------------+
  | DF3                 | SPMD to NS forwarding.                                 |
  +---------------------+--------------------------------------------------------+
  | DF4                 | SPMC to LSP communication.                             |
  |                     | NWd to LSP communication happens through SPMC.         |
  |                     | LSP can send direct response SP1 or NWd through SPMC.  |
  +---------------------+--------------------------------------------------------+
  | DF5                 | HW control.                                            |
  +---------------------+--------------------------------------------------------+
  | DF6                 | Bootloader image loading.                              |
  +---------------------+--------------------------------------------------------+
  | DF7                 | External memory access.                                |
  +---------------------+--------------------------------------------------------+


***************
Threat Analysis
***************

This threat model follows a similar methodology to the `Generic TF-A threat model`_.
The following sections define:

- Trust boundaries
- Assets
- Theat agents
- Threat types

Trust boundaries
================

- Normal world is untrusted.
- Secure world and normal world are separate trust boundaries.
- EL3 monitor, SPMD and SPMC are trusted.
- Bootloaders (in particular BL1/BL2 if using TF-A) and run-time BL31 are
  implicitely trusted by the usage of trusted boot.
- EL3 monitor, SPMD, SPMC do not trust SPs.

Assets
======

The following assets are identified:

- SPMC state.
- SP state.
- Information exchange between endpoints (partition messages).
- SPMC secrets (e.g. pointer authentication key when enabled)
- SP secrets (e.g. application keys).
- Scheduling cycles.
- Shared memory.

Threat Agents
=============

The following threat agents are identified:

- Non-secure endpoint (referred NS-Endpoint later): normal world client at
  NS-EL2 (Hypervisor) or NS-EL1 (VM or OS kernel).
- Secure endpoint (referred as S-Endpoint later): typically a secure partition.
- Hardware attacks (non-invasive) requiring a physical access to the device,
  such as bus probing or DRAM stress.

Threat types
============

The following threat categories as exposed in the `Generic TF-A threat model`_
are re-used:

- Spoofing
- Tampering
- Repudiation
- Information disclosure
- Denial of service
- Elevation of privileges

Similarly this threat model re-uses the same threat risk ratings. The risk
analysis is evaluated based on the environment being ``Server`` or ``Mobile``.
IOT is not evaluated as the EL3 SPMC is primarily meant for use in Client.

Threat Assessment
=================

The following threats are identified by applying STRIDE analysis on each diagram
element of the data flow diagram.

+------------------------+----------------------------------------------------+
| ID                     | 01                                                 |
+========================+====================================================+
| Threat                 | **An endpoint impersonates the sender              |
|                        | FF-A ID in a direct request/response invocation.** |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3, DF4                                 |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMD, SPMC                                         |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP state                                           |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Spoofing                                           |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |  Mobile                 |
+------------------------+--------------------------++------------------------+
| Impact                 | Critical(5)              | Critical(5)             |
+------------------------+--------------------------++------------------------+
| Likelihood             | Critical(5)              | Critical(5)             |
+------------------------+--------------------------++------------------------+
| Total Risk Rating      | Critical(25)             | Critical(25)            |
+------------------------+--------------------------+-------------------------+
| Mitigations            | SPMC must be able to correctly identify an         |
|                        | endpoint and enforce checks to disallow spoofing.  |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes.                                               |
| implemented?           | The SPMC enforces checks in the direct message     |
|                        | request/response interfaces such an endpoint cannot|
|                        | spoof the origin and destination worlds (e.g. a NWd|
|                        | originated message directed to the SWd cannot use a|
|                        | SWd ID as the sender ID).                          |
|                        | Also enforces check for direct response being sent |
|                        | only to originator of request.                     |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 02                                                 |
+========================+====================================================+
| Threat                 | **An endpoint impersonates the receiver            |
|                        | FF-A ID in a direct request/response invocation.** |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3, DF4                                 |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMD, SPMC                                         |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP state                                           |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Spoofing, Denial of Service                        |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |  Mobile                 |
+------------------------+--------------------------++------------------------+
| Impact                 | Critical(5)              | Critical(5)             |
+------------------------+--------------------------++------------------------+
| Likelihood             | Critical(5)              | Critical(5)             |
+------------------------+--------------------------++------------------------+
| Total Risk Rating      | Critical(25)             | Critical(25)            |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Validate if endpoind has permission to send        |
|                        | request to other endpoint by implementation        |
|                        | defined means.                                     |
+------------------------+----------------------------------------------------+
| Mitigations            | Platform specific.                                 |
| implemented?           |                                                    |
|                        | The guidance below is left for a system integrator |
|                        | to implement as necessary.                         |
|                        |                                                    |
|                        | Additionally a software component residing in the  |
|                        | SPMC can be added for the purpose of direct        |
|                        | request/response filtering.                        |
|                        |                                                    |
|                        | It can be configured with the list of known IDs    |
|                        | and about which interaction can occur between one  |
|                        | and another endpoint (e.g. which NWd endpoint ID   |
|                        | sends a direct request to which SWd endpoint ID).  |
|                        |                                                    |
|                        | This component checks the sender/receiver fields   |
|                        | for a legitimate communication between endpoints.  |
|                        |                                                    |
|                        | A similar component can exist in the OS kernel     |
|                        | driver, or Hypervisor although it remains untrusted|
|                        | by the SPMD/SPMC.                                  |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 03                                                 |
+========================+====================================================+
| Threat                 | **Tampering with memory shared between an endpoint |
|                        | and the SPMC.**                                    |
|                        |                                                    |
|                        | A malicious endpoint may attempt tampering with its|
|                        | RX/TX buffer contents while the SPMC is processing |
|                        | it (TOCTOU).                                       |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF3, DF7                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Shared memory, Information exchange                |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Tampering                                          |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |    Mobile               |
+------------------------+--------------------------+-------------------------+
| Impact                 | High (4)                 | High (4)                |
+------------------------+--------------------------+-------------------------+
| Likelihood             | High (4)                 | High (4)                |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | High (16)                | High (16)               |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Validate all inputs, copy before use.              |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes. In context of FF-A v1.1 this is the case of   |
| implemented?           | sharing the RX/TX buffer pair and usage in the     |
|                        | PARTITION_INFO_GET or memory sharing primitives.   |
|                        |                                                    |
|                        | The SPMC copies the contents of the TX buffer      |
|                        | to an internal temporary buffer before processing  |
|                        | its contents. The SPMC implements hardened input   |
|                        | validation on data transmitted through the TX      |
|                        | buffer by an untrusted endpoint.                   |
|                        |                                                    |
|                        | The TF-A SPMC enforces                             |
|                        | checks on data transmitted through RX/TX buffers.  |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 04                                                 |
+========================+====================================================+
| Threat                 | **An endpoint may tamper with its own state or the |
|                        | state of another endpoint.**                       |
|                        |                                                    |
|                        | A malicious endpoint may attempt violating:        |
|                        |                                                    |
|                        | - its own or another SP state by using an unusual  |
|                        |   combination (or out-of-order) FF-A function      |
|                        |   invocations.                                     |
|                        |   This can also be an endpoint emitting FF-A       |
|                        |   function invocations to another endpoint while   |
|                        |   the latter in not in a state to receive it (e.g. |
|                        |   SP sends a direct request to the normal world    |
|                        |   early while the normal world is not booted yet). |
|                        | - the SPMC state itself by employing unexpected    |
|                        |   transitions in FF-A memory sharing, direct       |
|                        |   requests and responses, or handling of interrupts|
|                        |   This can be led by random stimuli injection or   |
|                        |   fuzzing.                                         |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMD, SPMC                                         |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP state, SPMC state                               |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Tampering                                          |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | High (4)                 | High (4)                |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | High (12)                | High (12)               |
+------------------------+------------------+-----------------+---------------+
| Mitigations            | Follow guidelines in FF-A v1.1 specification on    |
|                        | state transitions (run-time model).                |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes. The TF-A SPMC is hardened to follow this      |
| implemented?           | guidance.                                          |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 05                                                 |
+========================+====================================================+
| Threat                 | **Replay fragments of past communication between   |
|                        | endpoints.**                                       |
|                        |                                                    |
|                        | A malicious endpoint may replay a message exchange |
|                        | that occurred between two legitimate endpoints as  |
|                        | a matter of triggering a malfunction or extracting |
|                        | secrets from the receiving endpoint. In particular |
|                        | the memory sharing operation with fragmented       |
|                        | messages between an endpoint and the SPMC may be   |
|                        | replayed by a malicious agent as a matter of       |
|                        | getting access or gaining permissions to a memory  |
|                        | region which does not belong to this agent.        |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF2, DF3                                           |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Information exchange                               |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Repudiation                                        |
+------------------------+--------------------------+-------------------------+
| Application            |     Server               |    Mobile               |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | High (4)                 | High (4)	              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | High (12)                | High (12)               |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Strict input validation and state tracking.        |
+------------------------+----------------------------------------------------+
| Mitigations            | Platform specific.                                 |
| implemented?           |                                                    |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 06                                                 |
+========================+====================================================+
| Threat                 | **A malicious endpoint may attempt to extract data |
|                        | or state information by the use of invalid or      |
|                        | incorrect input arguments.**                       |
|                        |                                                    |
|                        | Lack of input parameter validation or side effects |
|                        | of maliciously forged input parameters might affect|
|                        | the SPMC.                                          |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMD, SPMC                                         |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP secrets, SPMC secrets, SP state, SPMC state     |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Information discolure                              |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |  Mobile                 |
+------------------------+--------------------------+-------------------------+
| Impact                 | High (4)                 | High (4)                |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | High (12)                | High (12)               |
+------------------------+--------------------------+-------------------------+
| Mitigations            | SPMC must be prepared to receive incorrect input   |
|                        | data from secure partitions and reject them        |
|                        | appropriately.                                     |
|                        | The use of software (canaries) or hardware         |
|                        | hardening techniques (XN, WXN, pointer             |
|                        | authentication) helps detecting and stopping       |
|                        | an exploitation early.                             |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes. The TF-A SPMC mitigates this threat by        |
| implemented?           | implementing stack protector, pointer              |
|                        | authentication, XN, WXN, security hardening        |
|                        | techniques.                                        |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 07                                                 |
+========================+====================================================+
| Threat                 | **A malicious endpoint may forge a direct message  |
|                        | request such that it reveals the internal state of |
|                        | another endpoint through the direct message        |
|                        | response.**                                        |
|                        |                                                    |
|                        | The secure partition or SPMC replies to a partition|
|                        | message by a direct message response with          |
|                        | information which may reveal its internal state    |
|                        | (e.g. partition message response outside of        |
|                        | allowed bounds).                                   |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SPMC or SP state                                   |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Information discolure                              |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |  Mobile                 |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Low (2)                  | Low (2)	              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | Medium (6)               | Medium (6)              |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Follow FF-A specification about state transitions, |
|                        | run time model, do input validation.               |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes. For the specific case of direct requests      |
| implemented?           | targeting the SPMC, the latter is hardened to      |
|                        | prevent its internal state or the state of an SP   |
|                        | to be revealed through a direct message response.  |
|                        | Further FF-A v1.1 guidance about run time models   |
|                        | and partition states is followed.                  |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 08                                                 |
+========================+====================================================+
| Threat                 | **Probing the FF-A communication between           |
|                        | endpoints.**                                       |
|                        |                                                    |
|                        | SPMC and SPs are typically loaded to external      |
|                        | memory (protected by a TrustZone memory            |
|                        | controller). A malicious agent may use non invasive|
|                        | methods to probe the external memory bus and       |
|                        | extract the traffic between an SP and the SPMC or  |
|                        | among SPs when shared buffers are held in external |
|                        | memory.                                            |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF7                                                |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP/SPMC state, SP/SPMC secrets                     |
+------------------------+----------------------------------------------------+
| Threat Agent           | Hardware attack                                    |
+------------------------+----------------------------------------------------+
| Threat Type            | Information disclosure                             |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Low (2)                  | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | Medium (6)               | Medium (9)              |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Implement DRAM protection techniques using         |
|                        | hardware countermeasures at platform or chip level.|
+------------------------+--------------------------+-------------------------+
| Mitigations            | Platform specific.                                 |
| implemented?           |                                                    |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 09                                                 |
+========================+====================================================+
| Threat                 | **A malicious agent may attempt revealing the SPMC |
|                        | state or secrets by the use of software-based cache|
|                        | side-channel attack techniques.**                  |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF7                                                |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SP or SPMC state                                   |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Information disclosure                             |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Low (2)                  | Low (2)                 |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | Medium (6)               | Medium (6)              |
+------------------------+--------------------------+-------------------------+
| Mitigations            | The SPMC may be hardened further with SW           |
|                        | mitigations (e.g. speculation barriers) for the    |
|                        | cases not covered in HW. Usage of hardened         |
|                        | compilers and appropriate options, code inspection |
|                        | are recommended ways to mitigate Spectre types of  |
|                        | attacks.                                           |
+------------------------+----------------------------------------------------+
| Mitigations            | No.                                                |
| implemented?           |                                                    |
+------------------------+----------------------------------------------------+


+------------------------+----------------------------------------------------+
| ID                     | 10                                                 |
+========================+====================================================+
| Threat                 | **A malicious endpoint may attempt flooding the    |
|                        | SPMC with requests targeting a service within an   |
|                        | endpoint such that it denies another endpoint to   |
|                        | access this service.**                             |
|                        |                                                    |
|                        | Similarly, the malicious endpoint may target a     |
|                        | a service within an endpoint such that the latter  |
|                        | is unable to request services from another         |
|                        | endpoint.                                          |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | SPMC state, Scheduling cycles                      |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Denial of service                                  |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | Medium (9)               | Medium (9)              |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Bounding the time for operations to complete can   |
|                        | be achieved by the usage of a trusted watchdog.    |
|                        | Other quality of service monitoring can be achieved|
|                        | in the SPMC such as counting a number of operations|
|                        | in a limited timeframe.                            |
+------------------------+----------------------------------------------------+
| Mitigations            | Platform specific.                                 |
| implemented?           |                                                    |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 11                                                 |
+========================+====================================================+
| Threat                 | **Denying a lender endpoint to make progress if    |
|                        | borrower endpoint encountered a fatal exception.   |
|                        | Denying a new sender endpoint to make progress     |
|                        | if receiver encountered a fatal exception.**       |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Shared resources, Scheduling cycles.               |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Denial of service                                  |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | Medium (3)               | Medium (3)              |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | Medium (9)               | Medium (9)              |
+------------------------+--------------------------+-------------------------+
| Mitigations            | SPMC must be able to detect fatal error in SP and  |
|                        | take ownership of shared resources. It should      |
|                        | be able to relinquish the access to shared memory  |
|                        | regions to allow lender to proceed.                |
|                        | SPMC must return ABORTED if new direct requests are|
|                        | targeted to SP which has had a fatal error.        |
+------------------------+----------------------------------------------------+
| Mitigations            | Platform specific.                                 |
| implemented?           |                                                    |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 12                                                 |
+========================+====================================================+
| Threat                 | **A malicious endpoint may attempt to donate,      |
|                        | share, lend, relinquish or reclaim unauthorized    |
|                        | memory region.**                                   |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF2, DF3                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | SPMC                                               |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 |  SP secrets, SPMC secrets, SP state, SPMC state    |
+------------------------+----------------------------------------------------+
| Threat Agent           | NS-Endpoint, S-Endpoint                            |
+------------------------+----------------------------------------------------+
| Threat Type            | Elevation of Privilege                             |
+------------------------+--------------------------+-------------------------+
| Application            |   Server                 |   Mobile                |
+------------------------+--------------------------+-------------------------+
| Impact                 | High (4)                 | High   (4)              |
+------------------------+--------------------------+-------------------------+
| Likelihood             | High (4)                 | High (4)                |
+------------------------+--------------------------+-------------------------+
| Total Risk Rating      | High (16)                | High (16)               |
+------------------------+--------------------------+-------------------------+
| Mitigations            | Follow FF-A specification guidelines               |
|                        | on Memory management transactions.                 |
+------------------------+----------------------------------------------------+
| Mitigations            | Yes. The SPMC tracks ownership and access state    |
| implemented?           | for memory transactions appropriately, and         |
|                        | validating the same for all operations.            |
|                        | SPMC follows FF-A v1.1                             |
|                        | guidance for memory transaction lifecycle.         |
+------------------------+----------------------------------------------------+

---------------

*Copyright (c) 2022, Arm Limited. All rights reserved.*

.. _Arm Firmware Framework for Arm A-profile: https://developer.arm.com/docs/den0077/latest
.. _EL3 Secure Partition Manager: ../components/el3-spmc.html
.. _Generic TF-A threat model: ./threat_model.html#threat-analysis
.. _FF-A ACS: https://github.com/ARM-software/ff-a-acs/releases
