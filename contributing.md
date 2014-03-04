Contributing to ARM Trusted Firmware
====================================

Before you start contributing to this project you must sign the ARM
Contributor License Agreement (CLA).

Individuals who want to contribute their own work must sign and return an
Individual CLA. Companies that want to contribute must sign and return a
Corporate CLA if their employees' intellectual property has been assigned to
the employer. Copies of the CLAs are available from the [contributing page] of
the ARM website.

For this project, ARM also requires the GitHub account name(s) associated with
each individual contributor or the designated employees of corporate
contributors. Only contributions originating from these accounts will be
considered covered by the CLA. To avoid delay, you should provide the Github
account name(s) at the same time as the signed CLA.

ARM reserves the right to not accept a contribution. This may be for technical,
commercial or legal reasons.


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

*   Ensure we have your signed CLA.
*   Push your local changes to your fork of the repository.
*   Submit a [pull request] to arm-trusted-firmware.
    *   The changes in the pull request will then undergo further review and
        testing. Any review comments will be made as comments on the [pull
        request]. This may require you to do some rework.
*   When the changes are accepted, ARM will integrate them.
    *   To ensure a linear commit history, ARM will typically rebase the commits
        locally before merging to the [arm-trusted-firmware] `master` branch. In
        this case, the [pull request] will be closed rather than directly merged
        on GitHub. If the rebase is not trivial, you may be asked to rebase the
        commits yourself.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[User Guide]:                           ./docs/user-guide.md
[Running the software]:                 ./docs/user-guide.md#6--running-the-software
[Porting Guide]:                        ./docs/porting-guide.md
[Firmware Design]:                      ./docs/firmware-design.md
[Acknowledgements]:                     ./acknowledgements.md "Contributor acknowledgements"

[contributing page]:            http://www.arm.com/community/open-source-contributing.php
[GitHub account]:               https://github.com/signup/free
[Fork]:                         https://help.github.com/articles/fork-a-repo
[issue tracking repository]:    https://github.com/ARM-software/tf-issues
[issue]:                        https://github.com/ARM-software/tf-issues/issues
[pull request]:                 https://help.github.com/articles/using-pull-requests
[automatically closed]:         https://help.github.com/articles/closing-issues-via-commit-messages
[Git guidelines]:               http://git-scm.com/book/ch5-2.html
[Linux coding style]:           https://www.kernel.org/doc/Documentation/CodingStyle
[arm-trusted-firmware]:         https://github.com/ARM-software/arm-trusted-firmware
