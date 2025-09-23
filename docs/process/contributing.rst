Contributor's Guide
*******************

Getting Started
===============

-  Make sure you have a Github account and you are logged on to
   `review.trustedfirmware.org`_.

   Also make sure that you have registered your full name and email address in
   your `review.trustedfirmware.org`_ profile. Otherwise, the Gerrit server
   might reject patches you attempt to post for review.

-  If you plan to contribute a major piece of work, it is usually a good idea to
   start a discussion around it on the `TF-A mailing list`_. This gives everyone
   visibility of what is coming up, you might learn that somebody else is
   already working on something similar or the community might be able to
   provide some early input to help shaping the design of the feature.

   If you intend to include Third Party IP in your contribution, please mention
   it explicitly in the email thread and ensure that the changes that include
   Third Party IP are made in a separate patch (or patch series).

-  Clone the Trusted Firmware-A source code on your own machine as described in
   :ref:`prerequisites_get_source`.

-  Create a local topic branch based on the Trusted Firmware-A ``master``
   branch.

Making Changes
==============

-  Ensure commits adhere to the project's :ref:`Commit Style`.

-  Make commits of logical units. See these general `Git guidelines`_ for
   contributing to a project.

-  Keep the commits on topic. If you need to fix another bug or make another
   enhancement, please address it on a separate topic branch.

-  Split the patch in manageable units. Small patches are usually easier to
   review so this will speed up the review process.

-  Avoid long commit series. If you do have a long series, consider whether
   some commits should be squashed together or addressed in a separate topic.

-  Follow the :ref:`Coding Style` and :ref:`Coding Guidelines`.

   -  Use the checkpatch.pl script provided with the Linux source tree. A
      Makefile target is provided for convenience, see :ref:`this
      section<automatic-compliance-checking>` for more details.

-  Where appropriate, please update the documentation.

   -  Consider whether the :ref:`Porting Guide`, :ref:`Firmware Design` document
      or other in-source documentation needs updating.

   -  If you are submitting new files that you intend to be the code owner for
      (for example, a new platform port), then also update the
      :ref:`code owners` file.

   -  For topics with multiple commits, you should make all documentation changes
      (and nothing else) in the last commit of the series. Otherwise, include
      the documentation changes within the single commit.

.. _copyright-license-guidance:

-  Ensure that each changed file has the correct copyright and license
   information. Files that entirely consist of contributions to this project
   should have a copyright notice and BSD-3-Clause SPDX license identifier of
   the form as shown in :ref:`license`. Files that contain changes to imported
   Third Party IP files should retain their original copyright and license
   notices.

   For significant contributions you may add your own copyright notice in the
   following format:

   ::

       Portions copyright (c) [XXXX-]YYYY, <OWNER>. All rights reserved.

   where XXXX is the year of first contribution (if different to YYYY) and YYYY
   is the year of most recent contribution. <OWNER> is your name or your company
   name.

-  Ensure that each patch in the patch series compiles in all supported
   configurations. Patches which do not compile will not be merged.

-  Please test your changes. As a minimum, ensure that Linux boots on the
   Foundation FVP. See :ref:`Arm Fixed Virtual Platforms (FVP)` for more
   information. For more extensive testing, consider running the `TF-A Tests`_
   against your patches.

-  Ensure that all CI automated tests pass. Failures should be fixed. They might
   block a patch, depending on how critical they are.

Submitting Changes
==================

.. note::
   Please follow the `How to Contribute Code`_ section of the OpenCI
   documentation for general instructions on setting up Gerrit and posting
   patches there. The rest of this section provides details about patch
   submission rules specifically for the TF-A project.

-  Submit your changes for review using the ``git review`` command.

   This will automatically rebase them onto the upstream ``integration`` branch,
   as required by TF-A's patch submission process.

-  From the Gerrit web UI, add reviewers for your patch:

   -  At least one code owner for each module modified by the patch. See the
      list of modules and their :ref:`code owners`.

   -  At least one maintainer. See the list of :ref:`maintainers`.

   -  If some module has no code owner, try to identify a suitable (non-code
      owner) reviewer. Running ``git blame`` on the module's source code can
      help, as it shows who has been working the most recently on this area of
      the code.

      Alternatively, if it is impractical to identify such a reviewer, you might
      send an email to the `TF-A mailing list`_ to broadcast your review request
      to the community.

   Note that self-reviewing a patch is prohibited, even if the patch author is
   the only code owner of a module modified by the patch. Getting a second pair
   of eyes on the code is essential to keep up with the quality standards the
   project aspires to.

-  The changes will then undergo further review by the designated people. Any
   review comments will be made directly on your patch. This may require you to
   do some rework. For controversial changes, the discussion might be moved to
   the `TF-A mailing list`_ to involve more of the community.

   Refer to the `Gerrit Uploading Changes documentation`_ for more details.

-  The patch submission rules are the following. For a patch to be approved
   and merged in the tree, it must get:

   -  One ``Code-Owner-Review+1`` for each of the modules modified by the patch.
   -  A ``Maintainer-Review+1``.

   In the case where a code owner could not be found for a given module,
   ``Code-Owner-Review+1`` is substituted by ``Code-Review+1``.

   In addition to these various code review labels, the patch must also get a
   ``Verified+1``. This is usually set by the Continuous Integration (CI) bot
   when all automated tests passed on the patch. Sometimes, some of these
   automated tests may fail for reasons unrelated to the patch. In this case,
   the maintainers might (after analysis of the failures) override the CI bot
   score to certify that the patch has been correctly tested.

   In the event where the CI system lacks proper tests for a patch, the patch
   author or a reviewer might agree to perform additional manual tests
   in their review and the reviewer incorporates the review of the additional
   testing in the ``Code-Review+1`` or ``Code-Owner-Review+1`` as applicable to
   attest that the patch works as expected. Where possible additional tests should
   be added to the CI system as a follow up task. For example, for a
   platform-dependent patch where the said platform is not available in the CI
   system's board farm.

-  When the changes are accepted, the :ref:`maintainers` will integrate them.

   -  Typically, the :ref:`maintainers` will merge the changes into the
      ``integration`` branch.

   -  If the changes are not based on a sufficiently-recent commit, or if they
      cannot be automatically rebased, then the :ref:`maintainers` may rebase it
      on the ``integration`` branch or ask you to do so.

   -  After final integration testing, the changes will make their way into the
      ``master`` branch. If a problem is found during integration, the
      :ref:`maintainers` will request your help to solve the issue. They may
      revert your patches and ask you to resubmit a reworked version of them or
      they may ask you to provide a fix-up patch.

Add CI Configurations
=====================

TF-A uses Jenkins for Continuous Integration and testing activities. Various CI
jobs are deployed to run tests on every patch before being merged. Each of your
patches go through a series of checks before they get merged on to the master
branch. Kindly ensure that every time you add new files under your platform,
they are covered by the following two sections.

Coverity Scan
-------------

The TF-A project makes use of `Coverity Scan` for static analysis, a service
offered by Synopsys for open-source projects. This tool is able to find defects
and vulnerabilities in a code base, such as dereferences of NULL pointers, use
of uninitialized data, control flow issues and many other things.

The TF-A source code is submitted daily to this service for analysis. Results of
the latest and previous scans, as well as the complete list of defects it
detected, are accessible online from
https://scan.coverity.com/projects/arm-software-arm-trusted-firmware.

The `tf-a-ci-scripts repository`_ contains scripts to run the Coverity Scan
tools on the integration branch of the TF-A code base and make them available on
https://scan.coverity.com. These scripts get executed daily by the
`tf-a-coverity Jenkins job`_.

In order to maintain a high level of coverage, including on newly introduced
code, it is important to maintain the appropriate TF-A CI scripts. Details of
when to update these scripts and how to do so follow.

We maintain a build script - ``tf-cov-make`` - which contains the build
configurations of various platforms in order to cover the entire source code
being analysed by Coverity.

When you submit your patches for review, and if they contain new source files,
`TF-A CI static checks job`_ might report that these files are not covered. In
this case, the job's console output will show the following error message::

   ****** Newly added files detection check for Coverity Scan analysis on patch(es) ******

   Result : FAILURE

   New source files have been identified in your patch..
   some/dir/file.c

   please ensure to include them for the ``Coverity Scan analysis`` by adding
   the respective build configurations in the ``tf-cov-make`` build script.

In this section you find the details on how to append your new build
configurations for Coverity scan analysis illustrated with examples:

#. We maintain a separate repository named `tf-a-ci-scripts repository`_
   for placing all the test scripts which will be executed by the CI Jobs.

#. In this repository, ``tf-cov-make`` script is located at
   ``tf-a-ci-scripts/script/tf-coverity/tf-cov-make``

#. Edit the `tf-cov-make`_ script by appending all the possible build
   configurations with the specific build flags relevant to your platform, so
   that newly added source files get built and analysed by Coverity.

#. For better understanding follow the below specified examples listed in the
   ``tf-cov-make`` script.

.. code:: shell

    Example 1:
    #Intel
    make PLAT=stratix10 $(common_flags) all
    make PLAT=agilex $(common_flags) all

-  In the above example there are two different SoCs ``stratix`` and ``agilex``
   under the Intel platform and the build configurations has been added suitably
   to include most of their source files.

.. code:: shell

    Example 2:
    #Hikey
    make PLAT=hikey $(common_flags) ${TBB_OPTIONS} ENABLE_PMF=1 all
    make PLAT=hikey960 $(common_flags) ${TBB_OPTIONS} all
    make PLAT=poplar $(common_flags) all

-  In this case for ``Hikey`` boards additional build flags have been included
   along with the ``common_flags`` to cover most of the files relevant to it.

-  Similar to this you can still find many other different build configurations
   of various other platforms listed in the ``tf-cov-make`` script. Kindly refer
   them and append your build configurations respectively.

Test Build Configurations
-------------------------

We have CI jobs which run a set of test configurations on every TF-A patch
before they get merged upstream.

At the bare minimum, TF-A code should build without any errors for every
supported platform - and every feature of this platform. To make sure this is
the case, we maintain a set of build tests. ``tf-l1-build-plat`` is the test
group which holds all build tests for all platforms. So be kind enough to
verify that your newly added files are covered by such a build test.

If this is not the case, please follow the instructions below to add the
appropriate files. We will illustrate this with an example for the ``Hikey``
platform.

-  In the `tf-a-ci-scripts repository`_ we need to add a build configuration file
   ``hikey-default`` under ``tf_config/`` folder. ``tf_config/hikey-default``
   must list all the build parameters relevant to it.

.. code:: shell

   # Hikey Build Parameters
   CROSS_COMPILE=aarch64-none-elf-
   PLAT=hikey

-  Further another file, ``hikey-default:nil``, needs to be added under
   ``group/tf-l1-build-plat/`` folder to allow the platform to be built as part
   of this test group. ``group/tf-l1-build-plat/hikey-default:nil`` file just
   needs to exist but does not contain anything meaningful, apart from a
   mandatory copyright notice:

.. code:: shell

   #
   # Copyright (c) 2019-2022 Arm Limited. All rights reserved.
   #
   # SPDX-License-Identifier: BSD-3-Clause
   #

-  As illustrated above, you need to add similar files supporting your platform.

For a more elaborate explanation of the TF-A CI scripts internals, including how
to add more complex tests beyond a simple build test, please refer to the `TF-A
CI scripts overview`_ section of the OpenCI documentation.

Binary Components
=================

-  Platforms may depend on binary components submitted to the `Trusted Firmware
   binary repository`_ if they require code that the contributor is unable or
   unwilling to open-source. This should be used as a rare exception.
-  All binary components must follow the contribution guidelines (in particular
   licensing rules) outlined in the `readme.rst <tf-binaries-readme_>`_ file of
   the binary repository.
-  Binary components must be restricted to only the specific functionality that
   cannot be open-sourced and must be linked into a larger open-source platform
   port. The majority of the platform port must still be implemented in open
   source. Platform ports that are merely a thin wrapper around a binary
   component that contains all the actual code will not be accepted.
-  Only platform port code (i.e. in the ``plat/<vendor>`` directory) may rely on
   binary components. Generic code must always be fully open-source.

--------------

*Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.*

.. _review.trustedfirmware.org: https://review.trustedfirmware.org
.. _Git guidelines: https://git-scm.com/book/en/v2/Distributed-Git-Contributing-to-a-Project
.. _Gerrit Uploading Changes documentation: https://review.trustedfirmware.org/Documentation/user-upload.html
.. _TF-A Tests: https://trustedfirmware-a-tests.readthedocs.io
.. _Trusted Firmware binary repository: https://review.trustedfirmware.org/admin/repos/tf-binaries
.. _tf-binaries-readme: https://git.trustedfirmware.org/tf-binaries.git/tree/readme.rst
.. _TF-A mailing list: https://lists.trustedfirmware.org/mailman3/lists/tf-a.lists.trustedfirmware.org/
.. _tf-a-ci-scripts repository: https://git.trustedfirmware.org/ci/tf-a-ci-scripts.git/
.. _tf-cov-make: https://git.trustedfirmware.org/ci/tf-a-ci-scripts.git/tree/script/tf-coverity/tf-cov-make
.. _How to Contribute Code: https://tf-ci-users-guide.readthedocs.io/en/latest/#how-to-contribute-code
.. _TF-A CI scripts overview: https://tf-ci-users-guide.readthedocs.io/en/latest/#tf-a-ci-scripts-overview
.. _tf-a-coverity Jenkins job: https://ci.trustedfirmware.org/job/tf-a-coverity/
.. _TF-A CI static checks job: https://ci.trustedfirmware.org/job/tf-a-static-checks/
