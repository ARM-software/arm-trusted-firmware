DTB binding for Event Log properties
====================================

This document describes the device tree format of Event Log properties.
These properties are not related to a specific platform and can be queried
from common code.

Dynamic configuration for Event Log
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Measured Boot driver expects a *tpm_event_log* node with the following field
in 'tb_fw_config', 'nt_fw_config' and 'tsp_fw_config' DTS files:

- compatible [mandatory]
   - value type: <string>
   - Must be the string "arm,tpm_event_log".

Then a list of properties representing Event Log configuration, which
can be used by Measured Boot driver. Each property is named according
to the information it contains:

- tpm_event_log_sm_addr [fvp_nt_fw_config.dts with OP-TEE]
    - value type: <u64>
    - Event Log base address in secure memory.

Note. Currently OP-TEE does not support reading DTBs from Secure memory
and this property should be removed when this feature is supported.

- tpm_event_log_addr [mandatory]
    - value type: <u64>
    - Event Log base address in non-secure memory.

- tpm_event_log_size [mandatory]
    - value type: <u32>
    - Event Log size.
