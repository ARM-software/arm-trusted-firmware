License
=======

The software is provided under a BSD-3-Clause license (below). Contributions to
this project are accepted under the same license with developer sign-off as
described in the :ref:`Contributor's Guide`.

::

    Copyright (c) [XXXX-]YYYY, <OWNER>. All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    -  Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    -  Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    -  Neither the name of Arm nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SPDX Identifiers
----------------

Individual files contain the following tag instead of the full license text.

::

    SPDX-License-Identifier:    BSD-3-Clause

This enables machine processing of license information based on the SPDX
License Identifiers that are here available: http://spdx.org/licenses/


Other Projects
--------------

This project contains code from other projects as listed below. The original
license text is included in those source files.

-  The libc source code is derived from `FreeBSD`_ and `SCC`_. FreeBSD uses
   various BSD licenses, including BSD-3-Clause and BSD-2-Clause. The SCC code
   is used under the BSD-3-Clause license with the author's permission.

-  The libfdt source code is disjunctively dual licensed
   (GPL-2.0+ OR BSD-2-Clause). It is used by this project under the terms of
   the BSD-2-Clause license. Any contributions to this code must be made under
   the terms of both licenses.

-  The LLVM compiler-rt source code is disjunctively dual licensed
   (NCSA OR MIT). It is used by this project under the terms of the NCSA
   license (also known as the University of Illinois/NCSA Open Source License),
   which is a permissive license compatible with BSD-3-Clause. Any
   contributions to this code must be made under the terms of both licenses.

-  The zlib source code is licensed under the Zlib license, which is a
   permissive license compatible with BSD-3-Clause.

-  Some STMicroelectronics platform source code is disjunctively dual licensed
   (GPL-2.0+ OR BSD-3-Clause). It is used by this project under the terms of the
   BSD-3-Clause license. Any contributions to this code must be made under the
   terms of both licenses.

-  Some source files originating from the Linux source tree, which are
   disjunctively dual licensed (GPL-2.0 OR MIT), are redistributed under the
   terms of the MIT license. These files are:

   -  ``include/dt-bindings/interrupt-controller/arm-gic.h``
   -  ``include/dt-bindings/interrupt-controller/irq.h``

   See the original `Linux MIT license`_.

.. _FreeBSD: http://www.freebsd.org
.. _Linux MIT license: https://raw.githubusercontent.com/torvalds/linux/master/LICENSES/preferred/MIT
.. _SCC: http://www.simple-cc.org/
