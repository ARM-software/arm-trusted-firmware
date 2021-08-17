Activity Monitors
=================

FEAT_AMUv1 of the Armv8-A architecture introduces the Activity Monitors
extension. This extension describes the architecture for the Activity Monitor
Unit (|AMU|), an optional non-invasive component for monitoring core events
through a set of 64-bit counters.

When the ``ENABLE_AMU=1`` build option is provided, Trusted Firmware-A sets up
the |AMU| prior to its exit from EL3, and will save and restore architected
|AMU| counters as necessary upon suspend and resume.

--------------

*Copyright (c) 2021, Arm Limited. All rights reserved.*
