Threat Model
============

Threat modeling is an important part of Secure Development Lifecycle (SDL)
that helps us identify potential threats and mitigations affecting a system.

As the TF-A codebase is highly configurable to allow tailoring it best for each
platform's needs, providing a holistic threat model covering all of its features
is not necessarily the best approach. Instead, we provide a collection of
documents which, together, form the project's threat model. These are
articulated around a core document, called the :ref:`Generic Threat Model`,
which focuses on the most common configuration we expect to see. The other
documents typically focus on specific features not covered in the core document.

As the TF-A codebase evolves and new features get added, these threat model
documents will be updated and extended in parallel to reflect at best the
current status of the code from a security standpoint.

   .. note::

      Although our aim is eventually to provide threat model material for all
      features within the project, we have not reached that point yet. We expect
      to gradually fill these gaps over time.

Each of these documents give a description of the target of evaluation using a
data flow diagram, as well as a list of threats we have identified using the
`STRIDE threat modeling technique`_ and corresponding mitigations.

.. toctree::
   :maxdepth: 1
   :caption: Contents

   threat_model
   threat_model_el3_spm
   threat_model_fvp_r
   threat_model_rss_interface
   threat_model_arm_cca

--------------

*Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.*

.. _STRIDE threat modeling technique: https://docs.microsoft.com/en-us/azure/security/develop/threat-modeling-tool-threats#stride-model
