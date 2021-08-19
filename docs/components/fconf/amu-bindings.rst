Activity Monitor Unit (AMU) Bindings
====================================

To support platform-defined Activity Monitor Unit (|AMU|) auxiliary counters
through FCONF, the ``HW_CONFIG`` device tree accepts several |AMU|-specific
nodes and properties.

Bindings
^^^^^^^^

.. contents::
    :local:

``/cpus/cpus/cpu*`` node properties
"""""""""""""""""""""""""""""""""""

The ``cpu`` node has been augmented to support a handle to an associated |AMU|
view, which should describe the counters offered by the core.

+---------------+-------+---------------+-------------------------------------+
| Property name | Usage | Value type    | Description                         |
+===============+=======+===============+=====================================+
| ``amu``       | O     | ``<phandle>`` | If present, indicates that an |AMU| |
|               |       |               | is available and its counters are   |
|               |       |               | described by the node provided.     |
+---------------+-------+---------------+-------------------------------------+

``/cpus/amus`` node properties
""""""""""""""""""""""""""""""

The ``amus`` node describes the |AMUs| implemented by the cores in the system.
This node does not have any properties.

``/cpus/amus/amu*`` node properties
"""""""""""""""""""""""""""""""""""

An ``amu`` node describes the layout and meaning of the auxiliary counter
registers of one or more |AMUs|, and may be shared by multiple cores.

+--------------------+-------+------------+------------------------------------+
| Property name      | Usage | Value type | Description                        |
+====================+=======+============+====================================+
| ``#address-cells`` | R     | ``<u32>``  | Value shall be 1. Specifies that   |
|                    |       |            | the ``reg`` property array of      |
|                    |       |            | children of this node uses a       |
|                    |       |            | single cell.                       |
+--------------------+-------+------------+------------------------------------+
| ``#size-cells``    | R     | ``<u32>``  | Value shall be 0. Specifies that   |
|                    |       |            | no size is required in the ``reg`` |
|                    |       |            | property in children of this node. |
+--------------------+-------+------------+------------------------------------+

``/cpus/amus/amu*/counter*`` node properties
""""""""""""""""""""""""""""""""""""""""""""

A ``counter`` node describes an auxiliary counter belonging to the parent |AMU|
view.

+-------------------+-------+-------------+------------------------------------+
| Property name     | Usage | Value type  | Description                        |
+===================+=======+=============+====================================+
| ``reg``           | R     | array       | Represents the counter register    |
|                   |       |             | index, and must be a single cell.  |
+-------------------+-------+-------------+------------------------------------+
| ``enable-at-el3`` | O     | ``<empty>`` | The presence of this property      |
|                   |       |             | indicates that this counter should |
|                   |       |             | be enabled prior to EL3 exit.      |
+-------------------+-------+-------------+------------------------------------+

Example
^^^^^^^

An example system offering four cores made up of two clusters, where the cores
of each cluster share different |AMUs|, may use something like the following:

.. code-block::

    cpus {
        #address-cells = <2>;
        #size-cells = <0>;

        amus {
            amu0: amu-0 {
                #address-cells = <1>;
                #size-cells = <0>;

                counterX: counter@0 {
                    reg = <0>;

                    enable-at-el3;
                };

                counterY: counter@1 {
                    reg = <1>;

                    enable-at-el3;
                };
            };

            amu1: amu-1 {
                #address-cells = <1>;
                #size-cells = <0>;

                counterZ: counter@0 {
                    reg = <0>;

                    enable-at-el3;
                };
            };
        };

        cpu0@00000 {
            ...

            amu = <&amu0>;
        };

        cpu1@00100 {
            ...

            amu = <&amu0>;
        };

        cpu2@10000 {
            ...

            amu = <&amu1>;
        };

        cpu3@10100 {
            ...

            amu = <&amu1>;
        };
    }

In this situation, ``cpu0`` and ``cpu1`` (the two cores in the first cluster),
share the view of their AMUs defined by ``amu0``. Likewise, ``cpu2`` and
``cpu3`` (the two cores in the second cluster), share the view of their |AMUs|
defined by ``amu1``. This will cause ``counterX`` and ``counterY`` to be enabled
for both ``cpu0`` and ``cpu1``, and ``counterZ`` to be enabled for both ``cpu2``
and ``cpu3``.
