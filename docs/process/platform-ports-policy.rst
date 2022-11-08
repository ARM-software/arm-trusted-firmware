Platform Ports Policy
=====================

This document clarifies a couple of policy points around platform ports
management.

Platform compatibility policy
-----------------------------

Platform compatibility is mainly affected by changes to Platform APIs (as
documented in the :ref:`Porting Guide`), driver APIs (like the GICv3 drivers) or
library interfaces (like xlat_table library). The project will try to maintain
compatibility for upstream platforms. Due to evolving requirements and
enhancements, there might be changes affecting platform compatibility which
means the previous interface needs to be deprecated and a new interface
introduced to replace it. In case the migration to the new interface is trivial,
the contributor of the change is expected to make good effort to migrate the
upstream platforms to the new interface.

The deprecated interfaces are listed inside :ref:`Release Processes` as well as
the release after which each one will be removed. When an interface is
deprecated, the page must be updated to indicate the release after which the
interface will be removed. This must be at least 1 full release cycle in future.
For non-trivial interface changes, an email should be sent out to the `TF-A
public mailing list`_ to notify platforms that they should migrate away from the
deprecated interfaces. Platforms are expected to migrate before the removal of
the deprecated interface.

Platform deprecation policy
---------------------------

If a platform is no longer maintained, it is best to deprecate it to keep the
projects' source tree clean and healthy. Deprecation can be a 1-stage or 2-stage
process (up to the platform maintainers).

 - *2-stage*: The platform's source code can be kept in the repository for a
   cooling off period before deleting it (typically 2 release cycles). In this
   case, we keep track ot the *Deprecated* version separately from the *Deleted*
   version.

 - *1-stage*: The platform's source code can be deleted straight away. In this
   case, both versions are the same.

The :ref:`Platform Ports` page provides a list of all deprecated/deleted
platform ports (or soon to be) to this day.

--------------

*Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.*

.. _TF-A public mailing list: https://lists.trustedfirmware.org/mailman3/lists/tf-a.lists.trustedfirmware.org/
