Contributing to ARM Trusted Firmware
====================================

Getting Started
---------------

-  Make sure you have a `GitHub account`_.
-  Create an `issue`_ for your work if one does not already exist. This gives
   everyone visibility of whether others are working on something similar. ARM
   licensees may contact ARM directly via their partner managers instead if
   they prefer.

   -  Note that the `issue`_ tracker for this project is in a separate
      `issue tracking repository`_. Please follow the guidelines in that
      repository.
   -  If you intend to include Third Party IP in your contribution, please
      raise a separate `issue`_ for this and ensure that the changes that
      include Third Party IP are made on a separate topic branch.

-  `Fork`_ `arm-trusted-firmware`_ on GitHub.
-  Clone the fork to your own machine.
-  Create a local topic branch based on the `arm-trusted-firmware`_ ``master``
   branch.

Making Changes
--------------

-  Make commits of logical units. See these general `Git guidelines`_ for
   contributing to a project.
-  Follow the `Linux coding style`_; this style is enforced for the ARM Trusted
   Firmware project (style errors only, not warnings).

   -  Use the checkpatch.pl script provided with the Linux source tree. A
      Makefile target is provided for convenience (see section 2 in the
      `User Guide`_).

-  Keep the commits on topic. If you need to fix another bug or make another
   enhancement, please create a separate `issue`_ and address it on a separate
   topic branch.
-  Avoid long commit series. If you do have a long series, consider whether
   some commits should be squashed together or addressed in a separate topic.
-  Make sure your commit messages are in the proper format. If a commit fixes
   a GitHub `issue`_, include a reference (e.g.
   "fixes arm-software/tf-issues#45"); this ensures the `issue`_ is
   `automatically closed`_ when merged into the `arm-trusted-firmware`_ ``master``
   branch.
-  Where appropriate, please update the documentation.

   -  Consider whether the `User Guide`_, `Porting Guide`_, `Firmware Design`_ or
      other in-source documentation needs updating.
   -  Ensure that each changed file has the correct copyright and license
      information. Files that entirely consist of contributions to this
      project should have the copyright notice and BSD-3-Clause SPDX license
      identifier as shown in `license.rst`_. Files that contain
      changes to imported Third Party IP should contain a notice as follows,
      with the original copyright and license text retained:

      ::

          Portions copyright (c) [XXXX-]YYYY, ARM Limited and Contributors. All rights reserved.

      where XXXX is the year of first contribution (if different to YYYY) and
      YYYY is the year of most recent contribution.
   -  If not done previously, you may add your name or your company name to
      the `Acknowledgements`_ file.
   -  If you are submitting new files that you intend to be the technical
      sub-maintainer for (for example, a new platform port), then also update
      the `Maintainers`_ file.
   -  For topics with multiple commits, you should make all documentation
      changes (and nothing else) in the last commit of the series. Otherwise,
      include the documentation changes within the single commit.

-  Please test your changes. As a minimum, ensure UEFI boots to the shell on
   the Foundation FVP. See `Running the software on FVP`_ for more information.

Submitting Changes
------------------

-  Ensure that each commit in the series has at least one ``Signed-off-by:``
   line, using your real name and email address. The names in the
   ``Signed-off-by:`` and ``Author:`` lines must match. If anyone else contributes
   to the commit, they must also add their own ``Signed-off-by:`` line.
   By adding this line the contributor certifies the contribution is made under
   the terms of the `Developer Certificate of Origin (DCO)`_.
-  Push your local changes to your fork of the repository.
-  Submit a `pull request`_ to the `arm-trusted-firmware`_ ``integration`` branch.

   -  The changes in the `pull request`_ will then undergo further review and
      testing by the `Maintainers`_. Any review comments will be made as
      comments on the `pull request`_. This may require you to do some rework.

-  When the changes are accepted, the `Maintainers`_ will integrate them.

   -  Typically, the `Maintainers`_ will merge the `pull request`_ into the
      ``integration`` branch within the GitHub UI, creating a merge commit.
   -  Please avoid creating merge commits in the `pull request`_ itself.
   -  If the `pull request`_ is not based on a recent commit, the `Maintainers`_
      may rebase it onto the ``master`` branch first, or ask you to do this.
   -  If the `pull request`_ cannot be automatically merged, the `Maintainers`_
      will ask you to rebase it onto the ``master`` branch.
   -  After final integration testing, the `Maintainers`_ will push your merge
      commit to the ``master`` branch. If a problem is found during integration,
      the merge commit will be removed from the ``integration`` branch and the
      `Maintainers`_ will ask you to create a new pull request to resolve the
      problem.
   -  Please do not delete your topic branch until it is safely merged into
      the ``master`` branch.

--------------

*Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.*

.. _GitHub account: https://github.com/signup/free
.. _issue: https://github.com/ARM-software/tf-issues/issues
.. _issue tracking repository: https://github.com/ARM-software/tf-issues
.. _Fork: https://help.github.com/articles/fork-a-repo
.. _arm-trusted-firmware: https://github.com/ARM-software/arm-trusted-firmware
.. _Git guidelines: http://git-scm.com/book/ch5-2.html
.. _Linux coding style: https://www.kernel.org/doc/Documentation/CodingStyle
.. _User Guide: ./docs/user-guide.rst
.. _automatically closed: https://help.github.com/articles/closing-issues-via-commit-messages
.. _Porting Guide: ./docs/porting-guide.rst
.. _Firmware Design: ./docs/firmware-design.rst
.. _license.rst: ./license.rst
.. _Acknowledgements: ./acknowledgements.rst
.. _Maintainers: ./maintainers.rst
.. _Running the software on FVP: ./docs/user-guide.rst#user-content-running-the-software-on-fvp
.. _Developer Certificate of Origin (DCO): ./dco.txt
.. _pull request: https://help.github.com/articles/using-pull-requests
