Commit Style
============

When writing commit messages, please think carefully about the purpose and scope
of the change you are making: describe briefly what the change does, and
describe in detail why it does it. This helps to ensure that changes to the
code-base are transparent and approachable to reviewers, and it allows us to
keep a more accurate changelog. You may use Markdown in commit messages.

A good commit message provides all the background information needed for
reviewers to understand the intent and rationale of the patch. This information
is also useful for future reference.

For example:

- What does the patch do?
- What motivated it?
- What impact does it have?
- How was it tested?
- Have alternatives been considered? Why did you choose this approach over
  another one?
- If it fixes an `issue`_, include a reference.

|TF-A| follows the `Conventional Commits`_ specification. All commits to the
main repository are expected to adhere to these guidelines, so it is
**strongly** recommended that you read at least the `quick summary`_ of the
specification.

To briefly summarize, commit messages are expected to be of the form:

.. code::

    <type>[optional scope]: <description>

    [optional body]

    [optional footer(s)]

The following example commit message demonstrates the use of the
``refactor`` type and the ``amu`` scope:

.. code::

    refactor(amu): factor out register accesses

    This change introduces a small set of register getters and setters to
    avoid having to repeatedly mask and shift in complex code.

    Change-Id: Ia372f60c5efb924cd6eeceb75112e635ad13d942
    Signed-off-by: Chris Kay <chris.kay@arm.com>

The following `types` are permissible and are strictly enforced:

+--------------+---------------------------------------------------------------+
| Scope        | Description                                                   |
+==============+===============================================================+
| ``feat``     | A new feature                                                 |
+--------------+---------------------------------------------------------------+
| ``fix``      | A bug fix                                                     |
+--------------+---------------------------------------------------------------+
| ``build``    | Changes that affect the build system or external dependencies |
+--------------+---------------------------------------------------------------+
| ``ci``       | Changes to our CI configuration files and scripts             |
+--------------+---------------------------------------------------------------+
| ``docs``     | Documentation-only changes                                    |
+--------------+---------------------------------------------------------------+
| ``perf``     | A code change that improves performance                       |
+--------------+---------------------------------------------------------------+
| ``refactor`` | A code change that neither fixes a bug nor adds a feature     |
+--------------+---------------------------------------------------------------+
| ``revert``   | Changes that revert a previous change                         |
+--------------+---------------------------------------------------------------+
| ``style``    | Changes that do not affect the meaning of the code            |
|              | (white-space, formatting, missing semi-colons, etc.)          |
+--------------+---------------------------------------------------------------+
| ``test``     | Adding missing tests or correcting existing tests             |
+--------------+---------------------------------------------------------------+
| ``chore``    | Any other change                                              |
+--------------+---------------------------------------------------------------+

The permissible `scopes` are more flexible, and we maintain a list of them in
our :download:`Commitizen configuration file <../../.cz.json>`. Scopes in this
file are organized by their changelog section, each of which may have one or
more accepted scopes, but only the first of which is considered to be "blessed".
Scopes that are not blessed exist for changes submitted before scope enforcement
came into effect, and are considered deprecated.

While we don't enforce scopes strictly, we do ask that commits use these if they
can, or add their own if no appropriate one exists (see :ref:`Adding Scopes`).

It's highly recommended that you use the tooling installed by the optional steps
in the :ref:`prerequisites <Prerequisites>` guide to validate commit messages
locally, as commitlint reports a live list of the acceptable scopes.

.. _Adding Scopes:

Adding Scopes
-------------

Scopes that are either a) unblessed in the configuration file, or b) do not
exist in the configuration file at all are considered to be deprecated. If you
are adding a new component that does not yet have a designated scope, please
feel free to add one.

For example, if you are adding or making modifications to `Foo`'s latest and
greatest new platform `Bar`, you would add it to the `Platforms` changelog
section, and the hierarchy should look something like this:

.. code:: json

    {
        "sections": [
            {
                "title": "Platforms",
                "sections": [
                    {
                        "title": "Foo",
                        "scopes": ["foo"],
                        "sections": [
                            {
                                "title": "Bar",
                                "scopes": ["bar"]
                            }
                        ]
                    }
                ]
            }
        ]
    }

When creating new scopes, try to keep them short and succinct, and use kebab
case (``this-is-kebab-case``). Components with a product name (i.e. most
platforms and some drivers) should use that name (e.g. ``gic600ae``,
``flexspi``, ``stpmic1``), otherwise use a name that uniquely represents the
component (e.g. ``marvell-comphy-3700``, ``rcar3-drivers``, ``a3720-uart``).

Mandated Trailers
-----------------

Commits are expected to be signed off with the ``Signed-off-by:`` trailer using
your real name and email address. You can do this automatically by committing
with Git's ``-s`` flag.

There may be multiple ``Signed-off-by:`` lines depending on the history of the
patch, but one **must** be the committer. More details may be found in the
`Gerrit Signed-off-by Lines guidelines`_.

Ensure that each commit also has a unique ``Change-Id:`` line. If you have
followed optional steps in the prerequisites to either install the Node.js tools
or clone the repository using the "`Clone with commit-msg hook`" clone method,
then this should be done automatically for you.

More details may be found in the `Gerrit Change-Ids documentation`_.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

.. _Conventional Commits: https://www.conventionalcommits.org/en/v1.0.0
.. _Gerrit Change-Ids documentation: https://review.trustedfirmware.org/Documentation/user-changeid.html
.. _Gerrit Signed-off-by Lines guidelines: https://review.trustedfirmware.org/Documentation/user-signedoffby.html
.. _issue: https://developer.trustedfirmware.org/project/board/1/
.. _quick summary: https://www.conventionalcommits.org/en/v1.0.0/#summary
