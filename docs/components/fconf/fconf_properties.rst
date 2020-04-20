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

- max-size [mandatory]
    - value type: <u32>
    - Maximum size of the configuration.

- id [mandatory]
    - value type: <u32>
    - Image ID of the configuration.

