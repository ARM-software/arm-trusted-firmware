Maximum Power Mitigation Mechanism (MPMM) Bindings
==================================================

|MPMM| support cannot be determined at runtime by the firmware. Instead, these
DTB bindings allow the platform to communicate per-core support for |MPMM| via
the ``HW_CONFIG`` device tree blob.

Bindings
^^^^^^^^

.. contents::
    :local:

``/cpus/cpus/cpu*`` node properties
"""""""""""""""""""""""""""""""""""

The ``cpu`` node has been augmented to allow the platform to indicate support
for |MPMM| on a given core.

+-------------------+-------+-------------+------------------------------------+
| Property name     | Usage | Value type  | Description                        |
+===================+=======+=============+====================================+
| ``supports-mpmm`` | O     | ``<empty>`` | If present, indicates that |MPMM|  |
|                   |       |             | is available on this core.         |
+-------------------+-------+-------------+------------------------------------+

Example
^^^^^^^

An example system offering two cores, one with support for |MPMM| and one
without, can be described as follows:

.. code-block::

    cpus {
        #address-cells = <2>;
        #size-cells = <0>;

        cpu0@00000 {
            ...

            supports-mpmm;
        };

        cpu1@00100 {
            ...
        };
    }
