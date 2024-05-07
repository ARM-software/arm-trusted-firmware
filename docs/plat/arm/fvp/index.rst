Arm Fixed Virtual Platforms (FVP)
=================================

Arm |FVP|\s are complete simulations of an Arm system, including processor,
memory and peripherals. They enable software development without the need for
real hardware.

There exists many types of FVPs. This page provides details on how to build and
run TF-A on some of these FVPs.

Please also refer to the TF-A CI scripts under the `model/`_ directory for an
exhaustive list of |FVP|\s which TF-A is regularly tested on as part of our
continuous integration strategy.

.. toctree::
  :maxdepth: 1
  :caption: Contents

  fvp-support
  fvp-build-options
  fvp-foundation
  fvp-aemv8-base
  fvp-cortex-a57-a53
  fvp-cortex-a32
  fvp-specific-configs

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*

.. _model/: https://git.trustedfirmware.org/ci/tf-a-ci-scripts.git/tree/model
