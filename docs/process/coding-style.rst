Coding Style
============

The following sections outline the |TF-A| coding style for *C* code. The style
is based on the `Linux kernel coding style`_, with a few modifications.

The style should not be considered *set in stone*. Feel free to provide feedback
and suggestions.

.. note::
   You will almost certainly find code in the |TF-A| repository that does not
   follow the style. The intent is for all code to do so eventually.

File Encoding
-------------

The source code must use the **UTF-8** character encoding. Comments and
documentation may use non-ASCII characters when required (e.g. Greek letters
used for units) but code itself is still limited to ASCII characters.

Newlines must be in **Unix** style, which means that only the Line Feed (``LF``)
character is used to break a line and reset to the first column.

Language
--------

The primary language for comments and naming must be International English. In
cases where there is a conflict between the American English and British English
spellings of a word, the American English spelling is used.

Exceptions are made when referring directly to something that does not use
international style, such as the name of a company. In these cases the existing
name should be used as-is.

C Language Standard
-------------------

The C language mode used for TF-A is *GNU99*. This is the "GNU dialect of ISO
C99", which implies the *ISO C99* standard with GNU extensions.

Both GCC and Clang compiler toolchains have support for *GNU99* mode, though
Clang does lack support for a small number of GNU extensions. These
missing extensions are rarely used, however, and should not pose a problem.

.. _misra-compliance:

MISRA Compliance
----------------

TF-A attempts to comply with the `MISRA C:2012 Guidelines`_. Coverity
Static Analysis is used to regularly generate a report of current MISRA defects
and to prevent the addition of new ones.

It is not possible for the project to follow all MISRA guidelines. We maintain
`a spreadsheet`_ that lists all rules and directives and whether we aim to
comply with them or not. A rationale is given for each deviation.

.. note::
   Enforcing a rule does not mean that the codebase is free of defects
   of that rule, only that they would ideally be removed.

.. note::
   Third-party libraries are not considered in our MISRA analysis and we do not
   intend to modify them to make them MISRA compliant.

Indentation
-----------

Use **tabs** for indentation. The use of spaces for indentation is forbidden
except in the case where a term is being indented to a boundary that cannot be
achieved using tabs alone.

Tab spacing should be set to **8 characters**.

Trailing whitespace is not allowed and must be trimmed.

Spacing
-------

Single spacing should be used around most operators, including:

- Arithmetic operators (``+``, ``-``, ``/``, ``*``)
- Assignment operators (``=``, ``+=``, etc)
- Boolean operators (``&&``, ``||``)
- Comparison operators (``<``, ``>``, ``==``, etc)

A space should also be used to separate parentheses and braces when they are not
already separated by a newline, such as for the ``if`` statement in the
following example:

.. code:: c

  int function_foo(bool bar)
  {
      if (bar) {
          function_baz();
      }
  }

Note that there is no space between the name of a function and the following
parentheses.

Control statements (``if``, ``for``, ``switch``, ``while``, etc) must be
separated from the following open parenthesis by a single space. The previous
example illustrates this for an ``if`` statement.

Line Length
-----------

Line length *should* be at most **80 characters**. This limit does not include
non-printing characters such as the line feed.

This rule is a *should*, not a must, and it is acceptable to exceed the limit
**slightly** where the readability of the code would otherwise be significantly
reduced. Use your judgement in these cases.

Blank Lines
-----------

Functions are usually separated by a single blank line. In certain cases it is
acceptable to use additional blank lines for clarity, if required.

The file must end with a single newline character. Many editors have the option
to insert this automatically and to trim multiple blank lines at the end of the
file.

Braces
------

Opening Brace Placement
^^^^^^^^^^^^^^^^^^^^^^^

Braces follow the **Kernighan and Ritchie (K&R)** style, where the opening brace
is **not** placed on a new line.

Example for a ``while`` loop:

.. code:: c

  while (condition) {
      foo();
      bar();
  }

This style applies to all blocks except for functions which, following the Linux
style, **do** place the opening brace on a new line.

Example for a function:

.. code:: c

  int my_function(void)
  {
      int a;

      a = 1;
      return a;
  }

Conditional Statement Bodies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Where conditional statements (such as ``if``, ``for``, ``while`` and ``do``) are
used, braces must be placed around the statements that form the body of the
conditional. This is the case regardless of the number of statements in the
body.

.. note::
  This is a notable departure from the Linux coding style that has been
  adopted to follow MISRA guidelines more closely and to help prevent errors.

For example, use the following style:

.. code:: c

  if (condition) {
      foo++;
  }

instead of omitting the optional braces around a single statement:

.. code:: c

  /* This is violating MISRA C 2012: Rule 15.6 */
  if (condition)
      foo++;

The reason for this is to prevent accidental changes to control flow when
modifying the body of the conditional. For example, at a quick glance it is easy
to think that the value of ``bar`` is only incremented if ``condition``
evaluates to ``true`` but this is not the case - ``bar`` will always be
incremented regardless of the condition evaluation. If the developer forgets to
add braces around the conditional body when adding the ``bar++;`` statement then
the program execution will not proceed as intended.

.. code:: c

  /* This is violating MISRA C 2012: Rule 15.6 */
  if (condition)
      foo++;
      bar++;

Naming
------

Functions
^^^^^^^^^

Use lowercase for function names, separating multiple words with an underscore
character (``_``). This is sometimes referred to as *Snake Case*. An example is
given below:

.. code:: c

  void bl2_arch_setup(void)
  {
      ...
  }

Local Variables and Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Local variables and function parameters use the same format as function names:
lowercase with underscore separation between multiple words. An example is
given below:

.. code:: c

  static void set_scr_el3_from_rm(uint32_t type,
                                  uint32_t interrupt_type_flags,
                                  uint32_t security_state)
  {
      uint32_t flag, bit_pos;

      ...

  }

Preprocessor Macros
^^^^^^^^^^^^^^^^^^^

Identifiers that are defined using preprocessor macros are written in all
uppercase text.

.. code:: c

  #define BUFFER_SIZE_BYTES 64

Function Attributes
-------------------

Place any function attributes after the function type and before the function
name.

.. code:: c

   void __init plat_arm_interconnect_init(void);

Alignment
---------

Alignment should be performed primarily with tabs, adding spaces if required to
achieve a granularity that is smaller than the tab size. For example, with a tab
size of eight columns it would be necessary to use one tab character and two
spaces to indent text by ten columns.

Switch Statement Alignment
^^^^^^^^^^^^^^^^^^^^^^^^^^

When using ``switch`` statements, align each ``case`` statement with the
``switch`` so that they are in the same column.

.. code:: c

  switch (condition) {
  case A:
      foo();
  case B:
      bar();
  default:
      baz();
  }

Pointer Alignment
^^^^^^^^^^^^^^^^^

The reference and dereference operators (ampersand and *pointer star*) must be
aligned with the name of the object on which they are operating, as opposed to
the type of the object.

.. code:: c

  uint8_t *foo;

  foo = &bar;


Comments
--------

The general rule for comments is that the double-slash style of comment (``//``)
is not allowed. Examples of the allowed comment formats are shown below:

.. code:: c

  /*
   * This example illustrates the first allowed style for multi-line comments.
   *
   * Blank lines within multi-lines are allowed when they add clarity or when
   * they separate multiple contexts.
   *
   */

.. code:: c

  /**************************************************************************
   * This is the second allowed style for multi-line comments.
   *
   * In this style, the first and last lines use asterisks that run the full
   * width of the comment at its widest point.
   *
   * This style can be used for additional emphasis.
   *
   *************************************************************************/

.. code:: c

  /* Single line comments can use this format */

.. code:: c

  /***************************************************************************
   * This alternative single-line comment style can also be used for emphasis.
   **************************************************************************/

Headers and inclusion
---------------------

Header guards
^^^^^^^^^^^^^

For a header file called "some_driver.h" the style used by |TF-A| is:

.. code:: c

  #ifndef SOME_DRIVER_H
  #define SOME_DRIVER_H

  <header content>

  #endif /* SOME_DRIVER_H */

Include statement ordering
^^^^^^^^^^^^^^^^^^^^^^^^^^

All header files that are included by a source file must use the following,
grouped ordering. This is to improve readability (by making it easier to quickly
read through the list of headers) and maintainability.

#. *System* includes: Header files from the standard *C* library, such as
   ``stddef.h`` and ``string.h``.

#. *Project* includes: Header files under the ``include/`` directory within
   |TF-A| are *project* includes.

#. *Platform* includes: Header files relating to a single, specific platform,
   and which are located under the ``plat/<platform_name>`` directory within
   |TF-A|, are *platform* includes.

Within each group, ``#include`` statements must be in alphabetical order,
taking both the file and directory names into account.

Groups must be separated by a single blank line for clarity.

The example below illustrates the ordering rules using some contrived header
file names; this type of name reuse should be otherwise avoided.

.. code:: c

  #include <string.h>

  #include <a_dir/example/a_header.h>
  #include <a_dir/example/b_header.h>
  #include <a_dir/test/a_header.h>
  #include <b_dir/example/a_header.h>

  #include "a_header.h"

Include statement variants
^^^^^^^^^^^^^^^^^^^^^^^^^^

Two variants of the ``#include`` directive are acceptable in the |TF-A|
codebase. Correct use of the two styles improves readability by suggesting the
location of the included header and reducing ambiguity in cases where generic
and platform-specific headers share a name.

For header files that are in the same directory as the source file that is
including them, use the ``"..."`` variant.

For header files that are **not** in the same directory as the source file that
is including them, use the ``<...>`` variant.

Example (bl1_fwu.c):

.. code:: c

  #include <assert.h>
  #include <errno.h>
  #include <string.h>

  #include "bl1_private.h"

Typedefs
--------

Avoid anonymous typedefs of structs/enums in headers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For example, the following definition:

.. code:: c

  typedef struct {
          int arg1;
          int arg2;
  } my_struct_t;


is better written as:

.. code:: c

  struct my_struct {
          int arg1;
          int arg2;
  };

This allows function declarations in other header files that depend on the
struct/enum to forward declare the struct/enum instead of including the
entire header:

.. code:: c

  struct my_struct;
  void my_func(struct my_struct *arg);

instead of:

.. code:: c

  #include <my_struct.h>
  void my_func(my_struct_t *arg);

Some TF definitions use both a struct/enum name **and** a typedef name. This
is discouraged for new definitions as it makes it difficult for TF to comply
with MISRA rule 8.3, which states that "All declarations of an object or
function shall use the same names and type qualifiers".

The Linux coding standards also discourage new typedefs and checkpatch emits
a warning for this.

Existing typedefs will be retained for compatibility.

--------------

*Copyright (c) 2020, Arm Limited. All rights reserved.*

.. _`Linux kernel coding style`: https://www.kernel.org/doc/html/latest/process/coding-style.html
.. _`MISRA C:2012 Guidelines`: https://www.misra.org.uk/Activities/MISRAC/tabid/160/Default.aspx
.. _`a spreadsheet`: https://developer.trustedfirmware.org/file/download/lamajxif3w7c4mpjeoo5/PHID-FILE-fp7c7acszn6vliqomyhn/MISRA-and-TF-Analysis-v1.3.ods
