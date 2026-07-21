Experimental Nix flake
======================

The repository contains an experimental `Nix flake`_ for reproducibly building
the default |TF-A| configuration, which currently builds an AArch64 release for
the FVP platform, and supports AArch64 and x86-64 hosts running Linux or macOS.

.. note::
   The flake's interface is preliminary and is likely to change.

Installing Nix
--------------

On Linux and macOS, a multi-user installation can be performed with the
official installer:

.. code:: shell

   curl -L https://nixos.org/nix/install | sh -s -- --daemon

Start a new shell after the installer finishes, then confirm that Nix is
available:

.. code:: shell

   nix --version

See the `Nix download page`_ for other installation methods and platform-
specific instructions.

Enabling flakes
---------------

Enable the ``nix-command`` and ``flakes`` experimental features by adding the
following line to ``~/.config/nix/nix.conf``:

.. code:: text

   experimental-features = nix-command flakes

The same setting can instead be added to ``/etc/nix/nix.conf`` for all users.
To try the flake without changing either configuration file, pass the features
on the command line:

.. code:: shell

   nix build --extra-experimental-features "nix-command flakes"

Building TF-A
-------------

From the root of the |TF-A| repository, run:

.. code:: shell

   nix build

On success, the ``result`` symbolic link should contain:

.. code:: text

   result/bl1.bin
   result/bl2.bin
   result/bl31.bin

Useful commands
---------------

The following commands inspect and validate the flake or format its Nix files:

.. code:: shell

   nix flake show
   nix flake check
   nix fmt **/*.nix

Add ``--print-build-logs`` to ``nix build`` to display the complete build log.

Updating the pinned dependencies with ``nix flake update`` changes
``flake.lock`` and should be committed and reviewed like any other source
change.

Some Git worktree configurations cannot currently be read by Nix's Git
implementation. If Nix reports an unsupported Git repository extension, use an
explicit path flake reference:

.. code:: shell

   nix build "path:${PWD}"

--------------

*Copyright (c) 2026, Arm Limited. All rights reserved.*

.. _Nix flake: https://nix.dev/concepts/flakes.html
.. _Nix download page: https://nixos.org/download/
