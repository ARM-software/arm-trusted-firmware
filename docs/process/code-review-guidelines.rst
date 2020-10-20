Code Review Guidelines
======================

This document provides TF-A specific details about the project's code review
process. It should be read in conjunction with the `Project Maintenance
Process`_, which it supplements.


Why do we do code reviews?
--------------------------

The main goal of code reviews is to improve the code quality. By reviewing each
other's code, we can help catch issues that were missed by the author
before they are integrated in the source tree. Different people bring different
perspectives, depending on their past work, experiences and their current use
cases of TF-A in their products.

Code reviews also play a key role in sharing knowledge within the
community. People with more expertise in one area of the code base can
help those that are less familiar with it.

Code reviews are meant to benefit everyone through team work. It is not about
unfairly criticizing or belittling the work of any contributor.


Good practices
--------------

To ensure the code review gives the greatest possible benefit, participants in
the project should:

-  Be considerate of other people and their needs. Participants may be working
   to different timescales, and have different priorities. Keep this in
   mind - be gracious while waiting for action from others, and timely in your
   actions when others are waiting for you.

-  Review other people's patches where possible. The more active reviewers there
   are, the more quickly new patches can be reviewed and merged. Contributing to
   code review helps everyone in the long run, as it creates a culture of
   participation which serves everyone's interests.


Guidelines for patch contributors
---------------------------------

In addition to the rules outlined in the :ref:`Contributor's Guide`, as a patch
contributor you are expected to:

-  Answer all comments from people who took the time to review your
   patches.

-  Be patient and resilient. It is quite common for patches to go through
   several rounds of reviews and rework before they get approved, especially
   for larger features.

   In the event that a code review takes longer than you would hope for, you
   may try the following actions to speed it up:

  -  Ping the reviewers on Gerrit or on the mailing list. If it is urgent,
     explain why. Please remain courteous and do not abuse this.

  -  If one code owner has become unresponsive, ask the other code owners for
     help progressing the patch.

  -  If there is only one code owner and they have become unresponsive, ask one
     of the project maintainers for help.

-  Do the right thing for the project, not the fastest thing to get code merged.

   For example, if some existing piece of code - say a driver - does not quite
   meet your exact needs, go the extra mile and extend the code with the missing
   functionality you require - as opposed to copying the code into some other
   directory to have the freedom to change it in any way. This way, your changes
   benefit everyone and will be maintained over time.


Guidelines for all reviewers
----------------------------

There are no good or bad review comments. If you have any doubt about a patch or
need some clarifications, it's better to ask rather than letting a potential
issue slip. Examples of review comments could be:

- Questions ("Why do you need to do this?", "What if X happens?")
- Bugs ("I think you need a logical \|\| rather than a bitwise \|.")
- Design issues ("This won't scale well when we introduce feature X.")
- Improvements ("Would it be better if we did Y instead?")


Guidelines for code owners
--------------------------

Code owners are listed on the :ref:`Project Maintenance<code owners>` page,
along with the module(s) they look after.

When reviewing a patch, code owners are expected to check the following:

-  The patch looks good from a technical point of view. For example:

  -  The structure of the code is clear.

  -  It complies with the relevant standards or technical documentation (where
     applicable).

  -  It leverages existing interfaces rather than introducing new ones
     unnecessarily.

  -  It fits well in the design of the module.

  -  It adheres to the security model of the project. In particular, it does not
     increase the attack surface (e.g. new SMCs) without justification.

-  The patch adheres to the TF-A :ref:`Coding Style`. The CI system should help
   catch coding style violations.

-  (Only applicable to generic code) The code is MISRA-compliant (see
   :ref:`misra-compliance`). The CI system should help catch violations.

-  Documentation is provided/updated (where applicable).

-  The patch has had an appropriate level of testing. Testing details are
   expected to be provided by the patch author. If they are not, do not hesitate
   to request this information.

-  All CI automated tests pass.

If a code owner is happy with a patch, they should give their approval
through the ``Code-Owner-Review+1`` label in Gerrit. If instead, they have
concerns, questions, or any other type of blocking comment, they should set
``Code-Owner-Review-1``.

Code owners are expected to behave professionally and responsibly. Here are some
guidelines for them:

-  Once you are engaged in a review, make sure you stay involved until the patch
   is merged. Rejecting a patch and going away is not very helpful. You are
   expected to monitor the patch author's answers to your review comments,
   answer back if needed and review new revisions of their patch.

-  Provide constructive feedback. Just saying, "This is wrong, you should do X
   instead." is usually not very helpful. The patch author is unlikely to
   understand why you are requesting this change and might feel personally
   attacked.

-  Be mindful when reviewing a patch. As a code owner, you are viewed as
   the expert for the relevant module. By approving a patch, you are partially
   responsible for its quality and the effects it has for all TF-A users. Make
   sure you fully understand what the implications of a patch might be.


Guidelines for maintainers
--------------------------

Maintainers are listed on the :ref:`Project Maintenance<maintainers>` page.

When reviewing a patch, maintainers are expected to check the following:

-  The general structure of the patch looks good. This covers things like:

   -  Code organization.

   -  Files and directories, names and locations.

      For example, platform code should be added under the ``plat/`` directory.

   -  Naming conventions.

      For example, platform identifiers should be properly namespaced to avoid
      name clashes with generic code.

   -  API design.

-  Interaction of the patch with other modules in the code base.

-  The patch aims at complying with any standard or technical documentation
   that applies.

-  New files must have the correct license and copyright headers. See :ref:`this
   paragraph<copyright-license-guidance>` for more information. The CI system
   should help catch files with incorrect or no copyright/license headers.

-  There is no third party code or binary blobs with potential IP concerns.
   Maintainers should look for copyright or license notices in code, and use
   their best judgement. If they are unsure about a patch, they should ask
   other maintainers for help.

-  Generally speaking, new driver code should be placed in the generic
   layer. There are cases where a driver has to stay into the platform layer but
   this should be the exception, rather than the rule.

-  Existing common drivers (in particular for Arm IPs like the GIC driver) should
   not be copied into the platform layer to cater for platform quirks. This
   type of code duplication hurts the maintainability of the project. The
   duplicate driver is less likely to benefit from bug fixes and future
   enhancements. In most cases, it is possible to rework a generic driver to
   make it more flexible and fit slightly different use cases. That way, these
   enhancements benefit everyone.

-  When a platform specific driver really is required, the burden lies with the
   patch author to prove the need for it. A detailed justification should be
   posted via the commit message or on the mailing list.

-  Before merging a patch, verify that all review comments have been addressed.
   If this is not the case, encourage the patch author and the relevant
   reviewers to resolve these together.

If a maintainer is happy with a patch, they should give their approval
through the ``Maintainer-Review+1`` label in Gerrit. If instead, they have
concerns, questions, or any other type of blocking comment, they should set
``Maintainer-Review-1``.

--------------

*Copyright (c) 2020, Arm Limited. All rights reserved.*

.. _Project Maintenance Process: https://developer.trustedfirmware.org/w/collaboration/project-maintenance-process/
