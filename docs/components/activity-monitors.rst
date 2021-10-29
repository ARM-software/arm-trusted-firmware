Activity Monitors
=================

FEAT_AMUv1 of the Armv8-A architecture introduces the Activity Monitors
extension. This extension describes the architecture for the Activity Monitor
Unit (|AMU|), an optional non-invasive component for monitoring core events
through a set of 64-bit counters.

When the ``ENABLE_AMU=1`` build option is provided, Trusted Firmware-A sets up
the |AMU| prior to its exit from EL3, and will save and restore architected
|AMU| counters as necessary upon suspend and resume.

.. _Activity Monitor Auxiliary Counters:

Auxiliary counters
------------------

FEAT_AMUv1 describes a set of implementation-defined auxiliary counters (also
known as group 1 counters), controlled by the ``ENABLE_AMU_AUXILIARY_COUNTERS``
build option.

As a security precaution, Trusted Firmware-A does not enable these by default.
Instead, platforms may configure their auxiliary counters through one of two
possible mechanisms:

- |FCONF|, controlled by the ``ENABLE_AMU_FCONF`` build option.
- A platform implementation of the ``plat_amu_topology`` function (the default).

See :ref:`Activity Monitor Unit (AMU) Bindings` for documentation on the |FCONF|
device tree bindings.

--------------

*Copyright (c) 2021, Arm Limited. All rights reserved.*
