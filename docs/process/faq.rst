Frequently-Asked Questions (FAQ)
================================

How do I update my changes?
---------------------------

Often it is necessary to update your patch set before it is merged. Refer to the
`Gerrit Upload Patch Set documentation`_ on how to do so.

If you need to modify an existing patch set with multiple commits, refer to the
`Gerrit Replace Changes documentation`_.

How long will my changes take to merge into ``integration``?
------------------------------------------------------------

This can vary a lot, depending on:

* How important the patch set is considered by the TF maintainers. Where
  possible, you should indicate the required timescales for merging the patch
  set and the impact of any delay. Feel free to add a comment to your patch set
  to get an estimate of when it will be merged.

* The quality of the patch set. Patches are likely to be merged more quickly if
  they follow the coding guidelines, have already had some code review, and have
  been appropriately tested.

* The impact of the patch set. For example, a patch that changes a key generic
  API is likely to receive much greater scrutiny than a local change to a
  specific platform port.

* How much opportunity for external review is required. For example, the TF
  maintainers may not wait for external review comments to merge trivial
  bug-fixes but may wait up to a week to merge major changes, or ones requiring
  feedback from specific parties.

* How many other patch sets are waiting to be integrated and the risk of
  conflict between the topics.

* If there is a code freeze in place in preparation for the release. Please
  refer the :ref:`Release Processes` document for more details.

* The workload of the TF maintainers.

How long will it take for my changes to go from ``integration`` to ``master``?
------------------------------------------------------------------------------

This depends on how many concurrent patches are being processed at the same
time. In simple cases where all potential regressions have already been tested,
the delay will be less than 1 day. If the TF maintainers are trying to merge
several things over the course of a few days, it might take up to a week.
Typically, it will be 1-2 days.

The worst case is if the TF maintainers are trying to make a release while also
receiving patches that will not be merged into the release. In this case, the
patches will be merged onto ``integration``, which will temporarily diverge from
the release branch. The ``integration`` branch will be rebased onto ``master``
after the release, and then ``master`` will be fast-forwarded to ``integration``
1-2 days later. This whole process could take up 4 weeks. Please refer to the
:ref:`Release Processes` document for code freeze dates. The TF maintainers
will inform the patch owner if this is going to happen.

It is OK to create a patch based on commits that are only available in
``integration`` or another patch set, rather than ``master``. There is a risk
that the dependency commits will change (for example due to patch set rework or
integration problems). If this happens, the dependent patch will need reworking.

What are these strange comments in my changes?
----------------------------------------------

All the comments from ``ci-bot-user`` are associated with Continuous Integration
infrastructure. The links published on the comment are not currently accessible,
but would be after the CI has been transitioned to `trustedfirmware.org`_.
Please refer to https://github.com/ARM-software/tf-issues/issues/681 for more
details on the timelines.

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

.. _Gerrit Upload Patch Set documentation: https://review.trustedfirmware.org/Documentation/intro-user.html#upload-patch-set
.. _Gerrit Replace Changes documentation: https://review.trustedfirmware.org/Documentation/user-upload.html#push_replace
.. _trustedfirmware.org: https://www.trustedfirmware.org/
