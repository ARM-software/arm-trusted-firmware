Trusty Dispatcher
=================

Trusty is a a set of software components, supporting a Trusted Execution
Environment (TEE) on mobile devices, published and maintained by Google.

Detailed information and build instructions can be found on the Android
Open Source Project (AOSP) webpage for Trusty hosted at
https://source.android.com/security/trusty

Boot parameters
---------------

Custom boot parameters can be passed to Trusty by providing a platform
specific function:

.. code:: c

    void plat_trusty_set_boot_args(aapcs64_params_t *args)

If this function is provided ``args->arg0`` must be set to the memory
size allocated to trusty. If the platform does not provide this
function, but defines ``TSP_SEC_MEM_SIZE``, a default implementation
will pass the memory size from ``TSP_SEC_MEM_SIZE``. ``args->arg1``
can be set to a platform specific parameter block, and ``args->arg2``
should then be set to the size of that block.

Supported platforms
-------------------

Out of all the platforms supported by Trusted Firmware-A, Trusty is only
verified and supported by NVIDIA's Tegra SoCs.
