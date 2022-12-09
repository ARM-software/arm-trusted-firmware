Threat Model for RSS - AP interface
***********************************

************
Introduction
************
This document is an extension for the general TF-A threat-model. It considers
those platforms where a Runtime Security Subsystem (RSS) is included in the SoC
next to the Application Processor (AP).

********************
Target of Evaluation
********************
The scope of this threat model only includes the interface between the RSS and
AP. Otherwise, the TF-A :ref:`Generic Threat Model` document is applicable for
the AP core. The threat model for the RSS firmware will be provided by the RSS
firmware project in the future.


Data Flow Diagram
=================
This diagram is different only from the general TF-A data flow diagram in that
it includes the RSS and highlights the interface between the AP and the RSS
cores. The interface description only focuses on the AP-RSS interface the rest
is the same as in the general TF-A threat-model document.

.. uml:: ../resources/diagrams/plantuml/tfa_rss_dfd.puml
  :caption: Figure 1: TF-A Data Flow Diagram including RSS

.. table:: Table 1: TF-A - RSS data flow diagram

  +-----------------+--------------------------------------------------------+
  | Diagram Element | Description                                            |
  +=================+========================================================+
  |       DF7       | | Boot images interact with RSS over a communication   |
  |                 |   channel to record boot measurements and get image    |
  |                 |   verification keys. At runtime, BL31 obtains the      |
  |                 |   realm world attestation signing key from RSS.        |
  +-----------------+--------------------------------------------------------+

Threat Assessment
=================
For this section, please reference the Threat Assessment under the general TF-A
threat-model document, :ref:`Generic Threat Model`. All the threats listed there
are applicable for the AP core, here only the differences are highlighted.

    - ID 11: The access to the communication interface between AP and RSS is
      allowed only for firmware running at EL3. Accidentally exposing this
      interface to NSCode can allow malicious code to interact with RSS and
      gain access to sensitive data.
    - ID 13: Relevant in the context of the realm attestation key, which can be
      retrieved by BL31 through DF7. The RSS communication protocol layer
      mitigates against this by clearing its internal buffer when reply is
      received. The caller of the API must do the same if data is not needed
      anymore.

--------------

*Copyright (c) 2022, Arm Limited. All rights reserved.*