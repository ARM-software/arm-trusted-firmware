Threat Model for RSE - AP interface
***********************************

************
Introduction
************
This document is an extension for the general TF-A threat-model. It considers
those platforms where a Runtime Security Engine (RSE) is included in the SoC
next to the Application Processor (AP).

********************
Target of Evaluation
********************
The scope of this threat model only includes the interface between the RSE and
AP. Otherwise, the TF-A :ref:`Generic Threat Model` document is applicable for
the AP core. The threat model for the RSE firmware will be provided by the RSE
firmware project in the future.


Data Flow Diagram
=================
This diagram is different only from the general TF-A data flow diagram in that
it includes the RSE and highlights the interface between the AP and the RSE
cores. The interface description only focuses on the AP-RSE interface the rest
is the same as in the general TF-A threat-model document.

.. uml:: ../../resources/diagrams/plantuml/tfa_rse_dfd.puml
  :caption: Figure 1: TF-A Data Flow Diagram including RSE

.. table:: Table 1: TF-A - RSE data flow diagram

  +-----------------+--------------------------------------------------------+
  | Diagram Element | Description                                            |
  +=================+========================================================+
  |       DF7       | | Boot images interact with RSE over a communication   |
  |                 |   channel to record boot measurements and get image    |
  |                 |   verification keys. At runtime, BL31 obtains the      |
  |                 |   realm world attestation signing key from RSE.        |
  +-----------------+--------------------------------------------------------+

Threat Assessment
=================
For this section, please reference the Threat Assessment under the general TF-A
threat-model document, :ref:`Generic Threat Model`. All the threats listed there
are applicable for the AP core, here only the differences are highlighted.

    - ID 11: The access to the communication interface between AP and RSE is
      allowed only for firmware running at EL3. Accidentally exposing this
      interface to NSCode can allow malicious code to interact with RSE and
      gain access to sensitive data.
    - ID 13: Relevant in the context of the realm attestation key, which can be
      retrieved by BL31 through DF7. The RSE communication protocol layer
      mitigates against this by clearing its internal buffer when reply is
      received. The caller of the API must do the same if data is not needed
      anymore.

--------------

*Copyright (c) 2022-2024, Arm Limited. All rights reserved.*
