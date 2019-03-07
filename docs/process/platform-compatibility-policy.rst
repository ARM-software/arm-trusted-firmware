Platform Compatibility Policy
=============================

Introduction
------------

This document clarifies the project's policy around compatibility for upstream
platforms.

Platform compatibility policy
-----------------------------

Platform compatibility is mainly affected by changes to Platform APIs (as
documented in the `Porting Guide`_), driver APIs (like the GICv3 drivers) or
library interfaces (like xlat_table library). The project will try to maintain
compatibility for upstream platforms. Due to evolving requirements and
enhancements, there might be changes affecting platform compatibility which
means the previous interface needs to be deprecated and a new interface
introduced to replace it. In case the migration to the new interface is trivial,
the contributor of the change is expected to make good effort to migrate the
upstream platforms to the new interface.

The `Release information`_ documents the deprecated interfaces and the intended
release after which it will be removed. When an interface is deprecated, the
page must be updated to indicate the release after which the interface will be
removed. This must be at least 1 full release cycle in future. For non-trivial
interface changes, a `tf-issue`_ should be posted to notify platforms that they
should migrate away from the deprecated interfaces. Platforms are expected to
migrate before the removal of the deprecated interface.

--------------

*Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.*

.. _Porting Guide: ../getting_started/porting-guide.rst
.. _Release information: https://github.com/ARM-software/arm-trusted-firmware/wiki/TF-A-Release-information#removal-of-deprecated-interfaces
.. _tf-issue: https://github.com/ARM-software/tf-issues/issues
