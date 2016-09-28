Contributing to ARM Trusted Firmware
====================================

Getting Started
---------------

*   Make sure you have a [GitHub account].
*   Create an [issue] for your work if one does not already exist. This gives
    everyone visibility of whether others are working on something similar. ARM
    licensees may contact ARM directly via their partner managers instead if
    they prefer.
    *   Note that the [issue] tracker for this project is in a separate
        [issue tracking repository]. Please follow the guidelines in that
        repository.
    *   If you intend to include Third Party IP in your contribution, please
        raise a separate [issue] for this and ensure that the changes that
        include Third Party IP are made on a separate topic branch.
*   [Fork][] [arm-trusted-firmware][] on GitHub.
*   Clone the fork to your own machine.
*   Create a local topic branch based on the [arm-trusted-firmware][] `master`
    branch.


Making Changes
--------------

*   Make commits of logical units. See these general [Git guidelines] for
    contributing to a project.
*   Follow the [Linux coding style]; this style is enforced for the ARM Trusted
    Firmware project (style errors only, not warnings).
    *   Use the checkpatch.pl script provided with the Linux source tree. A
        Makefile target is provided for convenience (see section 2 in the
        [User Guide]).
*   Keep the commits on topic. If you need to fix another bug or make another
    enhancement, please create a separate [issue] and address it on a separate
    topic branch.
*   Avoid long commit series. If you do have a long series, consider whether
    some commits should be squashed together or addressed in a separate topic.
*   Make sure your commit messages are in the proper format. If a commit fixes
    a GitHub [issue], include a reference (e.g.
    "fixes arm-software/tf-issues#45"); this ensures the [issue] is
    [automatically closed] when merged into the [arm-trusted-firmware] `master`
    branch.
*   Where appropriate, please update the documentation.
    *   Consider whether the [User Guide], [Porting Guide], [Firmware Design] or
        other in-source documentation needs updating.
    *   If this is your first contribution, you may add your name or your
        company name to the [Acknowledgements] file.
    *   For topics with multiple commits, you should make all documentation
        changes (and nothing else) in the last commit of the series. Otherwise,
        include the documentation changes within the single commit.
*   Please test your changes. As a minimum, ensure UEFI boots to the shell on
    the Foundation FVP. See the "[Running the software]" section of the
    [User Guide] for more information.


Submitting Changes
------------------

*   Ensure that each commit in the series has at least one `Signed-off-by:`
    line, using your real name and email address. The names in the
    `Signed-off-by:` and `Author:` lines must match. If anyone else contributes
    to the commit, they must also add their own `Signed-off-by:` line.
    By adding this line the contributor certifies the contribution is made under
    the terms of the [Developer Certificate of Origin (DCO)][DCO].
*   Push your local changes to your fork of the repository.
*   Submit a [pull request] to the [arm-trusted-firmware] `integration` branch.
    *   The changes in the [pull request] will then undergo further review and
        testing. Any review comments will be made as comments on the [pull
        request]. This may require you to do some rework.
*   When the changes are accepted, ARM will integrate them.
    *   Typically, ARM will merge the [pull request] into the `integration`
        branch within the GitHub UI, creating a merge commit.
    *   Please avoid creating merge commits in the [pull request] itself.
    *   If the [pull request] is not based on a recent commit, ARM may rebase
        it onto the `master` branch first, or ask you to do this.
    *   If the [pull request] cannot be automatically merged, ARM will ask you
        to rebase it onto the `master` branch.
    *   After final integration testing, ARM will push your merge commit to the
        `master` branch. If a problem is found at this stage, the merge commit
        will be removed from the `integration` branch and ARM will ask you to
        create a new pull request to resolve the problem.
    *   Please do not delete your topic branch until it is safely merged into
        the `master` branch.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved._


[User Guide]:                           ./docs/user-guide.md
[Running the software]:                 ./docs/user-guide.md#6--running-the-software
[Porting Guide]:                        ./docs/porting-guide.md
[Firmware Design]:                      ./docs/firmware-design.md
[Acknowledgements]:                     ./acknowledgements.md "Contributor acknowledgements"
[DCO]:                                  ./dco.txt

[GitHub account]:               https://github.com/signup/free
[Fork]:                         https://help.github.com/articles/fork-a-repo
[issue tracking repository]:    https://github.com/ARM-software/tf-issues
[issue]:                        https://github.com/ARM-software/tf-issues/issues
[pull request]:                 https://help.github.com/articles/using-pull-requests
[automatically closed]:         https://help.github.com/articles/closing-issues-via-commit-messages
[Git guidelines]:               http://git-scm.com/book/ch5-2.html
[Linux coding style]:           https://www.kernel.org/doc/Documentation/CodingStyle
[arm-trusted-firmware]:         https://github.com/ARM-software/arm-trusted-firmware
