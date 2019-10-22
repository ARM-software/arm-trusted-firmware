Release Processes
=================

Project Release Cadence
-----------------------

The project currently aims to do a release once every 6 months which will be
tagged on the master branch. There will be a code freeze (stop merging
non-essential PRs) up to 4 weeks prior to the target release date. The release
candidates will start appearing after this and only bug fixes or updates
required for the release will be merged. The maintainers are free to use their
judgement on what PRs are essential for the release. A release branch may be
created after code freeze if there are significant PRs that need merging onto
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
| Legacy Console API             | Jan '18     | v2.1    | Deprecated in favour of ``MULTI_CONSOLE_API``           |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| Weak default                   | Oct '18     | v2.1    | The default implementations are defined in              |
| ``plat_crash_console_*``       |             |         | ``crash_console_helpers.S``. The platforms have to      |
| APIs                           |             |         | define ``plat_crash_console_*``.                        |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| ``finish_console_register``    | Oct '18     | v2.1    | The old version of the macro is deprecated. See commit  |
| macro in                       |             |         | cc5859c_ for more details.                              |
| ``MULTI_CONSOLE_API``          |             |         |                                                         |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| Types ``tzc_action_t`` and     | Oct '18     | v2.1    | Using logical operations such as OR in enumerations     |
| ``tzc_region_attributes_t``    |             |         | goes against the MISRA guidelines.                      |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| Macro ``EL_IMPLEMENTED()``     | Oct '18     | v2.1    | Deprecated in favour of ``el_implemented()``.           |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| ``get_afflvl_shift()``,        | Dec '18     | v2.1    | Removed.                                                |
| ``mpidr_mask_lower_afflvls()``,|             |         |                                                         |
| and ``eret()``.                |             |         |                                                         |
+--------------------------------+-------------+---------+---------------------------------------------------------+
| Extra include paths in the     | Jan '18     | v2.1    | Now it is needed to use the full path of the common     |
| Makefile in ``INCLUDES``.      |             |         | header files. More information in commit 09d40e0e0828_. |
+--------------------------------+-------------+---------+---------------------------------------------------------+

--------------

*Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.*

.. _cc5859c: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=cc5859ca19ff546c35eb0331000dae090b6eabcf
.. _09d40e0e0828: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=09d40e0e08283a249e7dce0e106c07c5141f9b7e
