Contributor's Guide
===================

Getting Started
---------------

-  Make sure you have a Github account and you are logged on both
   `developer.trustedfirmware.org`_ and `review.trustedfirmware.org`_.

-  If you plan to contribute a major piece of work, it is usually a good idea to
   start a discussion around it on the mailing list. This gives everyone
   visibility of what is coming up, you might learn that somebody else is
   already working on something similar or the community might be able to
   provide some early input to help shaping the design of the feature.

   If you intend to include Third Party IP in your contribution, please mention
   it explicitly in the email thread and ensure that the changes that include
   Third Party IP are made in a separate patch (or patch series).

-  Clone `Trusted Firmware-A`_ on your own machine as described in
   :ref:`prerequisites_get_source`.

-  Create a local topic branch based on the `Trusted Firmware-A`_ ``master``
   branch.

Making Changes
--------------

-  Make commits of logical units. See these general `Git guidelines`_ for
   contributing to a project.

-  Keep the commits on topic. If you need to fix another bug or make another
   enhancement, please address it on a separate topic branch.

-  Split the patch in manageable units. Small patches are usually easier to
   review so this will speed up the review process.

-  Avoid long commit series. If you do have a long series, consider whether
   some commits should be squashed together or addressed in a separate topic.

-  Ensure that each commit in the series has at least one ``Signed-off-by:``
   line, using your real name and email address. The names in the
   ``Signed-off-by:`` and ``Commit:`` lines must match. By adding this line the
   contributor certifies the contribution is made under the terms of the
   :download:`Developer Certificate of Origin <../../dco.txt>`.

   There might be multiple ``Signed-off-by:`` lines, depending on the history
   of the patch.

   More details may be found in the `Gerrit Signed-off-by Lines guidelines`_.

-  Ensure that each commit also has a unique ``Change-Id:`` line. If you have
   cloned the repository with the "`Clone with commit-msg hook`" clone method
   (following the :ref:`Prerequisites` document), this should already be the
   case.

   More details may be found in the `Gerrit Change-Ids documentation`_.

-  Write informative and comprehensive commit messages. A good commit message
   provides all the background information needed for reviewers to understand
   the intent and rationale of the patch. This information is also useful for
   future reference.

   For example:

   -  What does the patch do?
   -  What motivated it?
   -  What impact does it have?
   -  How was it tested?
   -  Have alternatives been considered? Why did you choose this approach over
      another one?
   -  If it fixes an `issue`_, include a reference.

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
------------------

-  Submit your changes for review at https://review.trustedfirmware.org
   targeting the ``integration`` branch.

-  Add reviewers for your patch:

   -  At least one code owner for each module modified by the patch. See the list
      of modules and their :ref:`code owners`.

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
.. _review.trustedfirmware.org: https://review.trustedfirmware.org
.. _issue: https://developer.trustedfirmware.org/project/board/1/
.. _Trusted Firmware-A: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
.. _Git guidelines: http://git-scm.com/book/ch5-2.html
.. _Gerrit Uploading Changes documentation: https://review.trustedfirmware.org/Documentation/user-upload.html
.. _Gerrit Signed-off-by Lines guidelines: https://review.trustedfirmware.org/Documentation/user-signedoffby.html
.. _Gerrit Change-Ids documentation: https://review.trustedfirmware.org/Documentation/user-changeid.html
.. _TF-A Tests: https://trustedfirmware-a-tests.readthedocs.io
.. _Trusted Firmware binary repository: https://review.trustedfirmware.org/admin/repos/tf-binaries
.. _tf-binaries-readme: https://git.trustedfirmware.org/tf-binaries.git/tree/readme.rst
.. _TF-A mailing list: https://lists.trustedfirmware.org/mailman/listinfo/tf-a
