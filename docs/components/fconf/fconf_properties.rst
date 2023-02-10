DTB binding for FCONF properties
================================

This document describes the device tree format of |FCONF| properties. These
properties are not related to a specific platform and can be queried from
common code.

Dynamic configuration
~~~~~~~~~~~~~~~~~~~~~

The |FCONF| framework expects a *dtb-registry* node with the following field:

- compatible [mandatory]
   - value type: <string>
   - Must be the string "fconf,dyn_cfg-dtb_registry".

Then a list of subnodes representing a configuration |DTB|, which can be used
by |FCONF|. Each subnode should be named according to the information it
contains, and must be formed with the following fields:

- load-address [mandatory]
    - value type: <u64>
    - Physical loading base address of the configuration. 
      If secondary-load-address is also provided (see below), then this is the
      primary load address.

- max-size [mandatory]
    - value type: <u32>
    - Maximum size of the configuration.

- id [mandatory]
    - value type: <u32>
    - Image ID of the configuration.

- secondary-load-address [optional]
    - value type: <u64>
    - A platform uses this physical address to copy the configuration to
      another location during the boot-flow.

--------------

*Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.*
