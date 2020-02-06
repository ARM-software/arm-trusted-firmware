Contributor's Guide
===================

Getting Started
---------------

-  Make sure you have a Github account and you are logged on
   `developer.trustedfirmware.org`_.
-  Create an `issue`_ for your work if one does not already exist. This gives
   everyone visibility of whether others are working on something similar.

   -  If you intend to include Third Party IP in your contribution, please
      raise a separate `issue`_ for this and ensure that the changes that
      include Third Party IP are made on a separate topic branch.

-  Clone `Trusted Firmware-A`_ on your own machine as described in
   :ref:`prerequisites_get_source`.
-  Create a local topic branch based on the `Trusted Firmware-A`_ ``master``
   branch.

Making Changes
--------------

-  Make commits of logical units. See these general `Git guidelines`_ for
   contributing to a project.
-  Follow the :ref:`Coding Style` and :ref:`Coding Guidelines`.

   -  Use the checkpatch.pl script provided with the Linux source tree. A
      Makefile target is provided for convenience.

-  Keep the commits on topic. If you need to fix another bug or make another
   enhancement, please create a separate `issue`_ and address it on a separate
   topic branch.
-  Avoid long commit series. If you do have a long series, consider whether
   some commits should be squashed together or addressed in a separate topic.
-  Make sure your commit messages are in the proper format. If a commit fixes
   an `issue`_, include a reference.
-  Where appropriate, please update the documentation.

   -  Consider whether the :ref:`Porting Guide`,
      :ref:`Firmware Design` document or other in-source documentation needs
      updating.
   -  Ensure that each changed file has the correct copyright and license
      information. Files that entirely consist of contributions to this
      project should have a copyright notice and BSD-3-Clause SPDX license
      identifier of the form as shown in :ref:`license`. Files that contain
      changes to imported Third Party IP files should retain their original
      copyright and license notices. For significant contributions you may
      add your own copyright notice in following format:

      ::

          Portions copyright (c) [XXXX-]YYYY, <OWNER>. All rights reserved.

      where XXXX is the year of first contribution (if different to YYYY) and
      YYYY is the year of most recent contribution. <OWNER> is your name or
      your company name.
   -  If you are submitting new files that you intend to be the technical
      sub-maintainer for (for example, a new platform port), then also update
      the :ref:`maintainers` file.
   -  For topics with multiple commits, you should make all documentation
      changes (and nothing else) in the last commit of the series. Otherwise,
      include the documentation changes within the single commit.

-  Please test your changes. As a minimum, ensure that Linux boots on the
   Foundation FVP. See :ref:`Arm Fixed Virtual Platforms (FVP)` for more
   information. For more extensive testing, consider running the `TF-A Tests`_
   against your patches.

Submitting Changes
------------------

-  Ensure that each commit in the series has at least one ``Signed-off-by:``
   line, using your real name and email address. The names in the
   ``Signed-off-by:`` and ``Author:`` lines must match. If anyone else
   contributes to the commit, they must also add their own ``Signed-off-by:``
   line. By adding this line the contributor certifies the contribution is made
   under the terms of the
   :download:`Developer Certificate of Origin <../../dco.txt>`.

   More details may be found in the `Gerrit Signed-off-by Lines guidelines`_.

-  Ensure that each commit also has a unique ``Change-Id:`` line. If you have
   cloned the repository with the "`Clone with commit-msg hook`" clone method
   (following the :ref:`Prerequisites` document), this should already be the
   case.

   More details may be found in the `Gerrit Change-Ids documentation`_.

-  Submit your changes for review at https://review.trustedfirmware.org
   targeting the ``integration`` branch.

   -  The changes will then undergo further review and testing by the
      :ref:`maintainers`. Any review comments will be made directly on your
      patch. This may require you to do some rework.

   Refer to the `Gerrit Uploading Changes documentation`_ for more details.

-  When the changes are accepted, the :ref:`maintainers` will integrate them.

   -  Typically, the :ref:`maintainers` will merge the changes into the
      ``integration`` branch.
   -  If the changes are not based on a sufficiently-recent commit, or if they
      cannot be automatically rebased, then the :ref:`maintainers` may rebase it
      on the ``master`` branch or ask you to do so.
   -  After final integration testing, the changes will make their way into the
      ``master`` branch. If a problem is found during integration, the merge
      commit will be removed from the ``integration`` branch and the
      :ref:`maintainers` will ask you to create a new patch set to resolve the
      problem.

Binary Components
-----------------

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

*Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.*

.. _developer.trustedfirmware.org: https://developer.trustedfirmware.org
.. _issue: https://developer.trustedfirmware.org/project/board/1/
.. _Trusted Firmware-A: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
.. _Git guidelines: http://git-scm.com/book/ch5-2.html
.. _Gerrit Uploading Changes documentation: https://review.trustedfirmware.org/Documentation/user-upload.html
.. _Gerrit Signed-off-by Lines guidelines: https://review.trustedfirmware.org/Documentation/user-signedoffby.html
.. _Gerrit Change-Ids documentation: https://review.trustedfirmware.org/Documentation/user-changeid.html
.. _TF-A Tests: https://git.trustedfirmware.org/TF-A/tf-a-tests.git/about/
.. _Trusted Firmware binary repository: https://review.trustedfirmware.org/admin/repos/tf-binaries
.. _tf-binaries-readme: https://git.trustedfirmware.org/tf-binaries.git/tree/readme.rst
