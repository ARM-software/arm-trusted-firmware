Experimental Nix flake
======================

The |TF-A| repository includes an experimental `Nix flake`_ that you can use to
build known-good configurations of |TF-A|.

.. note::
   The flake's interface is preliminary and is likely to change.

Prerequisites
-------------

Before using the flake, follow the instructions on the `Nix download page`_ to
install Nix for your system. You will also need to enable the ``nix-command``
and ``flakes`` experimental features; `Nix's flake documentation`_ explains how
to do this for individual commands, and its `configuration reference`_ covers
persistent settings.

Discovering configurations
--------------------------

A good place to start is ``nix flake show``, which lists the outputs that the
flake provides. Run it from the root of the |TF-A| repository:

.. code:: shell

   nix flake show

Look under ``packages`` for your host system to choose a firmware configuration
to build. The host system is the machine on which you will run the build tools;
the firmware's target platform and architecture are selected separately by each
configuration.

The examples below assume that you are working from the repository root.
Wherever you see ``<package>``, replace it with a package name from the listing
for your host system.

Building TF-A
-------------

Once you have chosen a package, pass its name to ``nix build`` to build that
firmware configuration:

.. code:: shell

   nix build '.#<package>'

Here, ``.`` refers to your current checkout, and the name after ``#`` selects
the package.

When it is asked to build a package, Nix takes a snapshot of the source tree
and builds it in a separate directory. When you are working in a local Git
checkout, this snapshot includes your changes to tracked files, but does not
include untracked files. If you add new files, make sure to use ``git add`` to
include them in the snapshot too; you can build your changes without committing
them first.

By convention, TF-A packages provide an ``out`` output containing the artifacts
that all consumers of |TF-A| need to build a complete software stack, and a
``debug`` output containing artifacts useful for debugging the firmware.

Nix selects the ``out`` output by default. When the build succeeds, you can
access these artifacts through a symbolic link called ``result``.

If you need a package's debugging artifacts, you can explicitly request them
by building its ``debug`` output:

.. code:: shell

   nix build '.#<package>^debug'

You can then access these artifacts through the ``result-debug`` symbolic
link. If you need both the firmware and its debugging artifacts, you can select
all outputs together:

.. code:: shell

   nix build '.#<package>^*'

Both outputs come from the same firmware build. Selecting ``debug`` makes the
debugging artifacts available, but it does not enable the TF-A ``DEBUG`` build
option - that is dictated by the package's firmware configuration.

If you want to follow the build in more detail, add ``--print-build-logs`` when
running ``nix build`` to show log output while it runs.

--------------

*Copyright (c) 2026, Arm Limited. All rights reserved.*

.. _Nix flake: https://nix.dev/concepts/flakes.html
.. _Nix download page: https://nixos.org/download/
.. _Nix's flake documentation: https://nix.dev/concepts/flakes.html#running-commands
.. _configuration reference: https://nix.dev/manual/nix/stable/command-ref/conf-file.html#conf-experimental-features
