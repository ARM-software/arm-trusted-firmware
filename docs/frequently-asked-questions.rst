Frequently-Asked Questions (FAQ)
================================

How do I update a Pull Request?
-------------------------------

Often it is necessary to update a Pull Request (PR) before it is merged.  When
you push to the source topic branch of an open PR, the PR is automatically
updated with the new commits.

If you need to modify existing commits in the PR (for example following review
comments), then use the ``--force`` option when pushing. Any comments that apply
to previous versions of the PR are retained in the PR. Sometimes it may be
confusing whether comments apply to the current or a previous version of the PR,
especially if there are several rounds of rework. In this case, you may be asked
to close the PR and create a new one with the latest commits. The new PR should
have a version appended to the name (e.g. "My topic v2") and you should create a
link to the old PR so that reviewers can easily find previous versions.

When the PR is finally merged, you will be given the option of deleting your
topic branch. It is recommended you delete this (and any previous topic branch
versions) to avoid polluting your fork with obsolete branches.

How long will my Pull Request take to merge?
--------------------------------------------

This can vary a lot, depending on:

* How important the Pull Request (PR) is considered by the TF maintainers. Where
  possible, you should indicate the required timescales for merging the PR and
  the impact of any delay.

* The quality of the PR. PRs are likely to be merged quicker if they follow the
  coding guidelines, have already had some code review, and have been
  appropriately tested. Note that PRs from Arm engineers go through an internal
  review process before appearing on GitHub, therefore may appear to be merged
  more quickly.

* The impact of the PR. For example, a PR that changes a key generic API is
  likely to receive much greater scrutiny than a local change to a specific
  platform port.

* How much opportunity for external review is required. For example, the TF
  maintainers may not wait for external review comments to merge trivial
  bug-fixes but may wait up to a week to merge major changes, or ones requiring
  feedback from specific parties.

* How many other topics need to be integrated and the risk of conflict between
  the topics.

* Is there a code freeze in place in preparation for the release. Please refer
  the `release information`_ for more details.

* The workload of the TF maintainers.

Feel free to add a comment to your PR to get an estimate of when it will
be merged.

How long will it take for my merged Pull Request to go from ``integration`` to ``master``?
------------------------------------------------------------------------------------------

This depends on how many concurrent Pull Requests (PRs) are being processed at
the same time. In simple cases where all potential regressions have already been
tested, the delay will be less than 1 day.  If the TF maintainers are trying to
merge several things over the course of a few days, it might take up to a week.
Typically, it will be 1-2 days.

The worst case is if the TF maintainers are trying to make a release while also
receiving PRs that will not be merged into the release. In this case, the PRs
will be merged onto ``integration``, which will temporarily diverge from the
release branch. The ``integration`` branch will be rebased onto ``master`` after
the release, and then ``master`` will be fast-forwarded to ``integration`` 1-2
days later. This whole process could take up 4 weeks. Please refer the `release
information`_ for code freeze dates. The TF maintainers will inform the PR owner
if this is going to happen.

It is OK to create a PR based on commits that are only available in
``integration`` or another PR, rather than ``master``. There is a risk that the
dependency commits will change (for example due to PR rework or integration
problems). If this happens, the dependent PR will need reworking.

What are these strange comments in my Pull Request?
---------------------------------------------------

For example, comments like "Can one of the admins verify this patch?" or "test
this please". These are associated with Arm's Continuous Integration
infrastructure and can be safely ignored. Those who are curious can see the
documentation for `this Jenkins plugin`_ for more details.

.. _release information: release-information.rst
.. _this Jenkins plugin: https://wiki.jenkins-ci.org/display/JENKINS/GitHub+pull+request+builder+plugin
