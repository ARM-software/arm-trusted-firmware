Unit testing framework
======================

|TF-A| includes a method of unit testing the repository under
``tests/unit-tests``. It is based on CppUTest.

Prerequisites
-------------

On top of the regular TF-A dependencies, to build the unit tests you will need
required tools that can be installed on Ubuntu with the following commands:

::

  sudo apt-get install cmake python3 python3-pip
  poetry install --no-root --with unit-tests

Building unit tests
-------------------


The unit tests can be built from the top level TF-A directory like this:

::

  export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)
  make unit-tests


Running unit tests
------------------

CMake provides a built-in tool called ctest for running all the tests using a
single command. It is also able to filter tests or run them in parallel for
speeding up the tests process. Run all the tests using the following command:

::

  ctest

Each unit test suite has its own executable. The easiest way of running single
test suite is running it as a simple executable.

::

  ./bl1_fwu

Measuring code coverage
-----------------------

Inspecting code coverage is a useful method for detecting parts of the code
which is not exercised by tests. The build system includes an option for
generating code coverage report of the unit tests. The coverage is processed by
``lcov`` which needs to be installed for this feature. Also the coverage
measurement in only available when GCC is used as a compiler.

The ``COVERAGE`` option has to be set to be set before building.

This makes CMake to build the binaries with coverage information included. The
rest of the build process works the same way as before.

Before collecting coverage info and generating reports the tests must be run as
the coverage is a runtime measurement. See section `Running unit tests`_ for
more information about running unit tests.

The ``COVERAGE`` option adds two new build targets called ``coverage`` and
``coverage_report``. They can be used simply by running the following commands
if ``make`` is used as a build system.

::

  make coverage
  make coverage_report

The ``coverage`` target generates lcov info files for further processing. If
there are coverage files available from different sources (i.e. coverages of
other tests) they can be merged with the unit test coverage file and evaluated
together. Currently two coverage info files are generated during the build. One
of them contains the coverage of code under test (i.e. Trusted Firmware-A) and
the other one has the coverage of the unit tests themselves.

The ``coverage_report`` target generates a HTML report from the coverage info
files. The coverage reports can be found in the build directory's subdirectories
having ``-coverage`` suffix in their names. The report shows the directory
structure of the code and each file can be inspected individually. Line,
function and branch coverage is included.

Implementing tests
==================

Concept of unit testing
-----------------------

First of all unit tests exercise the C code on a function level. The tests
should call functions directly from the code under tests and verify if their
return values are matching the expected ones and the functions are behaving
according to the specification.

Because of the function level testing the dependencies of the tested functions
should be detached. This is done by mocking the underlying layer. This provides
an additional advantage of controlling and verifying all the call to the lower
layer.


Adding new unit test suite
--------------------------

The first step is to define a new unit test suite. If a completely new module is
being test the test suite definition should be created in a separate ``.cmake``
file which is placed in the test files' directory. Otherwise the test
definition can be added to an existing ``.cmake`` file. These files should be
included in the root ``CMakeLists.txt``.

The ``UnitTest`` CMake module defines the ``unit_test_add_suite`` function so
before using this function the module must be included in the ``.cmake`` file.
The function first requires a unique test name which will be test binary's name.
The test sources, include directories and macro definition are passed to the
function in the matching arguments. CMake variables can be used to reference
files relative to common directories:

- ``CMAKE_CURRENT_LIST_DIR`` - Relative to the ``.cmake`` file
- ``UNIT_TEST_PROJECT_PATH`` - Relative to the Trusted Firmware-A root directory
- ``PROJECT_SOURCE_DIR`` - Relative to the unit test root directory

.. code-block:: cmake

  # tests/new_module/new_test_suite.cmake
  include(UnitTest)

  unit_test_add_suite(
  	NAME [unique test name]
  	SOURCES
  		[source files]
  	INCLUDE_DIRECTORIES
  		[include directories]
  	COMPILE_DEFINITIONS
  		[defines]
  )

.. code-block:: cmake

  # Root CMakeLists.txt
  include(tests/new_module/new_test_suite.cmake)

Example test definition
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  unit_test_add_suite(
  	NAME memcmp
  	SOURCES
  		${CMAKE_CURRENT_LIST_DIR}/test_memcmp.cpp
  		${CMAKE_CURRENT_LIST_DIR}/memcmp.yml
  	INCLUDE_DIRECTORIES
  		${UNIT_TEST_PROJECT_PATH}/include
  		${UNIT_TEST_PROJECT_PATH}/include/lib/libc/aarch64/
  )


Using c-picker
--------------

c-picker is a simple tool used for detaching dependencies of the code under
test. It can copy elements (i.e. functions, variables, etc.) from the original
source code into generated files. This way the developer can pick functions from
compilation units and surround them with a mocked environment.

If a ``.yml`` file listed among source files the build system invokes c-picker
and the generated ``.c`` file is implicitly added to the source file list.

Example .yml file
^^^^^^^^^^^^^^^^^

In this simple example c-picker is instructed to copy the include directives and
the ``memcmp`` function from the ``lib/libc/memcmp.c`` file. The root directory
of the source files referenced by c-picker is the Trusted Firmware-A root
directory.

.. code-block:: yaml

  elements:
  - file: lib/libc/memcmp.c
    type: include
  - file: lib/libc/memcmp.c
    type: function
    name: memcmp


Writing unit tests
------------------

Unit test code should be placed in ``.cpp`` files.

Four-phase test pattern
^^^^^^^^^^^^^^^^^^^^^^^

All tests cases should follow the four-phase test pattern. This consists of four
simple steps that altogether ensure the isolation between test cases. These
steps follows below.

- Setup
- Exercise
- Verify
- Teardown

After the teardown step all global states should be the same as they were at the
beginning of the setup step.

Analyzing code coverage
-----------------------

The code coverage reports can be easily used for finding untested parts of the
code. The two main parts of the coverage report are the line coverage and the
branch coverage. Line coverage shows that how many times the tests ran the given
line of the source code. It is beneficial to increase the line coverage however
100% line coverage is still not enough to consider the code fully tested.

Let's have a look on the following example.

.. code-block:: C++

  void set_pointer_value(unsigned int id, unsigned int value) {
  	unsigned int *pointer;

  	if (id < MAX_ID) {
  		pointer = get_pointer(id);
  	}

  	*pointer = value;
  }

The 100% line coverage is achievable by testing the function with an ``id``
value smaller than ``MAX_ID``. However if an ``id`` larger than or equal to
``MAX_ID`` is used as a parameter of this function it will try to write to a
memory address pointed by an uninitialized variable. To catch untested
conditions like this the branch coverage comes handy. It will show that only one
branch of the  ``if`` statement has been tested as the condition was always true
in the tests.


--------------

*Copyright (c) 2026, Arm Limited. All rights reserved.*
