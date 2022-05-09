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
| v2.3            | 4th week of Apr '20       | 1st week of Apr '20          |
+-----------------+---------------------------+------------------------------+
| v2.4            | 2nd week of Nov '20       | 4th week of Oct '20          |
+-----------------+---------------------------+------------------------------+
| v2.5            | 3rd week of May '21       | 5th week of Apr '21          |
+-----------------+---------------------------+------------------------------+
| v2.6            | 4th week of Nov '21       | 2nd week of Nov '21          |
+-----------------+---------------------------+------------------------------+
| v2.7            | 5th week of May '22       | 3rd week of May '22          |
+-----------------+---------------------------+------------------------------+
| v2.8            | 5th week of Nov '22       | 3rd week of Nov '22          |
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
| STM32MP_USE_STM32IMAGE macro   |   Dec '21   |   2.7   | FIP is the recommended boot method for STM32MP          |
+--------------------------------+-------------+---------+---------------------------------------------------------+

--------------

*Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.*
