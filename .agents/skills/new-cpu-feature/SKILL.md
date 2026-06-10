---
name: new-cpu-feature
description: Add support for a new A-profile architectural CPU feature that adds functionality at EL3.
license: BSD-3-Clause
compatibility: requires the machine readable A class repository
---

# TF-A CPU Feature

Start by reading:

- `docs/architecture_features.rst`
- The Arm ARM entry for the feature

Treat `docs/architecture_features.rst` as the status ledger and the generic
upstream checklist. Adapt it to the current tree before editing anything.

Keep in mind that ``FEAT_ABC`` stands for the feature you're implementing.

# Before you begin

Ask the user for the path to a local copy of the XML of the Architecture. Also
ask for the PDF of the Arm ARM. Use them for searching the architecture.

# Adding tests to TFTF

Ask the user for the path to the local copy of TFTF. Add a test for any
functionality that has EL3 involvement like a trap to an instruction. Add the
test like for other CPU features in the cpu-extensions test group. If the
feature requires it, register it with
tftf/tests/runtime_services/arm_arch_svc/smccc_feature_availability.c.

# Enabling the feature in CI

Ask the user for the path to the local copy of the CI scripts. If the feature is
always optional, add it to the list in model/base-aemva-common.sh. To determine
if a feature is always optional you can assume that EL2 and EL3 and AArch64 are
always implemented. If the feature has a dependency on other features that are
already enabled do not enable it. Use the command `FVP_Base_RevC-2xAEMvA -l` to
determine what flag to give to the model.
