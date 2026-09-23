/*
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2019-2026 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2019      Ben Wagner <bungeman@chromium.org>
   Copyright (c) 2019      Vadim Zeitlin <vadim@zeitlins.org>
   Copyright (c) 2026      Matthew Fernandez <matthew.fernandez@gmail.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.

   SPDX-License-Identifier: MIT
*/

#if defined(_WIN32)

#include "random_rand_s.h"

/* force stdlib to define rand_s() */
#if ! defined(_CRT_RAND_S)
#  define _CRT_RAND_S
#endif
#include "wxconfig.h"

// Workaround MinGW GCC trouble with recognizing `rand_s`, likely related
// to return type `error_t`; the symptom was:
// > error: implicit declaration of function ‘rand_s’
#if defined(__MINGW32__)
#  include <errno.h>
#endif

#include <stdlib.h> // for rand_s
#include <string.h> // for memcpy

// Help clang-tidy out with prototype of function `rand_s`
#if defined(XML_CLANG_TIDY)
int rand_s(unsigned int *);
#endif

/* All supported non-MinGW-32 compilers, including MinGW-64, have both rand_s()
   definition and declaration, but the situation is more complicated for
   MinGW-32, which only provides it since version 5.3.0 of its runtime package
   (mingwrt, containing stdlib.h), see the details about the upstream fix at
   https://osdn.net/projects/mingw/ticket/39658. Until the version 3.22.0, it
   didn't provide even the definition of this function in its libraries, and so
   it can't be used at all in this case. And for the intermediate versions,
   between 3.22 and 5.3, it didn't provide the declaration of the function in
   its headers -- that we can work around ourselves by providing it here.
*/
#if defined(__MINGW32__) && defined(__MINGW32_VERSION)                         \
      && ! defined(__MINGW64_VERSION_MAJOR)
#    if __MINGW32_MAJOR_VERSION < 3                                            \
        || (__MINGW32_MAJOR_VERSION == 3 && __MINGW32_MINOR_VERSION < 22)
#      define EXPAT_DISABLE_RAND_S
#    elif __MINGW32_VERSION < 5003000L
__declspec(dllimport) int rand_s(unsigned int *);
 #endif
#endif

#ifdef EXPAT_DISABLE_RAND_S

static int
writeRandomBytes_rand_s(void *target, size_t count) {
  return 0; /* unconditional failure */
}

#else /* ! EXPAT_DISABLE_RAND_S */

/* Obtain entropy on Windows using the rand_s() function which
 * generates cryptographically secure random numbers.  Internally it
 * uses RtlGenRandom API which is present in Windows XP and later.
 */
bool
writeRandomBytes_rand_s(void *target, size_t count) {
  size_t bytesWrittenTotal = 0;

  while (bytesWrittenTotal < count) {
    unsigned int random32 = 0;

    if (rand_s(&random32))
      return false; /* failure */

    size_t toUse = count - bytesWrittenTotal;
    if (toUse > sizeof(random32))
      toUse = sizeof(random32);
    memcpy((char *)target + bytesWrittenTotal, &random32, toUse);
    bytesWrittenTotal += toUse;
  }
  return true; /* success */
}

#endif /* EXPAT_DISABLE_RAND_S / ! EXPAT_DISABLE_RAND_S */

#endif // defined(_WIN32)
