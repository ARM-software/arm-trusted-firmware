Release Processes
=================

Project Release Cadence
-----------------------

The project currently aims to do a release once every 6 months which will be
tagged on the master branch. There will be a code freeze (stop merging
non-essential changes) up to 4 weeks prior to the target release date. The release
candidates will start appearing after this and only bug fixes or updates
required for the release will be merged. The maintainers are free to use their
judgement on what changes are essential for the release. A release branch may be
created after code freeze if there are significant changes that need merging onto
the integration branch during the merge window.

The release testing will be performed on release candidates and depending on
issues found, additional release candidates may be created to fix the issues.

::

                            |<----------6 months---------->|
            |<---4 weeks--->|              |<---4 weeks--->|
       +-----------------------------------------------------------> time
            |               |              |               |
         code freeze       ver w.x       code freeze     ver y.z


Upcoming Releases
~~~~~~~~~~~~~~~~~

These are the estimated dates for the upcoming release. These may change
depending on project requirement and partner feedback.

+-----------------+---------------------------+------------------------------+
| Release Version |  Target Date              | Expected Code Freeze         |
+=================+===========================+==============================+
| v2.0            | 1st week of Oct '18       | 1st week of Sep '18          |
+-----------------+---------------------------+------------------------------+
| v2.1            | 5th week of Mar '19       | 1st week of Mar '19          |
+-----------------+---------------------------+------------------------------+
| v2.2            | 4th week of Oct '19       | 1st week of Oct '19          |
+-----------------+---------------------------+------------------------------+
| v2.3            | 4th week of Mar '20       | 1st week of Mar '20          |
+-----------------+---------------------------+------------------------------+

Removal of Deprecated Interfaces
--------------------------------

As mentioned in the :ref:`Platform Compatibility Policy`, this is a live
document cataloging all the deprecated interfaces in TF-A project and the
Release version after which it will be removed.

+--------------------------------+-------------+---------+---------------------------------------------------------+
| Interface                      | Deprecation | Removed | Comments                                                |
|                                | Date        | after   |                                                         |
|                                |             | Release |                                                         |
+================================+=============+=========+=========================================================+
| ``AARCH32``/``AARCH64`` macros | Oct '19     | v2.3    | Deprecated in favor of ``__aarch64__``                  |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| ``__ASSEMBLY__`` macro         | Oct '19     | v2.3    | Deprecated in favor of ``__ASSEMBLER__``                |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| Prototype SPCI-based SPM       | Oct '19     | v2.2    | Based on outdated Alpha 1 spec. Will be replaced with   |
| (services/std_svc/spm)         |             |         | alternative methods of secure partitioning support.     |
+--------------------------------+-------------+---------+---------------------------------------------------------+

--------------

*Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.*
