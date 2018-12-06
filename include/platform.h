/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, prpl Foundation
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdarg.h>   // va_list
#include <stddef.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Hardware stuff
////////////////////////////////////////////////////////////////////////////////

// The following preprocessor macros must be defined to a platform-dependent
// value:
//
//   _HOST_IS_LITTLE_ENDIAN_        |--> Set one (and only one!) of these macros
//   _HOST_IS_BIG_ENDIAN_ENDIAN_    |    to "1" to indicate your CPU endianness
//
//
//   MAX_NETWORK_SEGMENT_SIZE  --------> This is the maximum packet size that
//                                       is allowed in your platform. It is
//                                       used to 'fragment' CMDUs.  Note that
//                                       even if your platform supports packets
//                                       bigger than 1500 bytes, this macro
//                                       must never be bigger than that.  This
//                                       macro is only present in this file for
//                                       those special cases where, for some
//                                       platform related reason, packets must
//                                       be smaller than 1500.
//
//
// In the next few lines we are just going to check that these are defined,
// nothing else.
// In order to actually define them use the "root" Makefile where these MACROS
// are sent to the compiler using the "-D flag" (open the "root" Makefile and
// search for "CCFLAGS" to understand how to do this)

#if !defined(_HOST_IS_LITTLE_ENDIAN_) && !defined(_HOST_IS_BIG_ENDIAN_ENDIAN_)
#  error  "You must define either '_HOST_IS_LITTLE_ENDIAN_' or '_HOST_IS_BIG_ENDIAN_'"
#elif defined(_HOST_IS_LITTLE_ENDIAN_) && defined(_HOST_IS_BIG_ENDIAN_ENDIAN_)
#  error  "You cannot define both '_HOST_IS_LITTLE_ENDIAN_' and '_HOST_IS_BIG_ENDIAN_' at the same time"
#endif

#ifndef  MAX_NETWORK_SEGMENT_SIZE
#  error  "You must define 'MAX_NETWORK_SEGMENT_SIZE' to some value (for example, '1500')"
#endif


////////////////////////////////////////////////////////////////////////////////
// Typical libc stuff
////////////////////////////////////////////////////////////////////////////////

// Output the provided format string (see 'man 3 printf' on any Linux box)
//
void PLATFORM_PRINTF(const char *format, ...) __attribute__((format (printf, 1, 2)));

// Same as 'PLATFORM_PRINTF', but the message will only be processed if the
// platform has the pertaining debug level enabled
//
void PLATFORM_PRINTF_DEBUG_ERROR(const char *format, ...) __attribute__((format (printf, 1, 2)));
void PLATFORM_PRINTF_DEBUG_WARNING(const char *format, ...) __attribute__((format (printf, 1, 2)));
void PLATFORM_PRINTF_DEBUG_INFO(const char *format, ...) __attribute__((format (printf, 1, 2)));
void PLATFORM_PRINTF_DEBUG_DETAIL(const char *format, ...) __attribute__((format (printf, 1, 2)));

// Used to set the verbosity of the previous functions:
//
//   0 => Only print ERROR messages
//   1 => Print ERROR and WARNING messages
//   2 => Print ERROR, WARNING and INFO messages
//   3 => Print ERROR, WARNING, INFO and DETAIL messages
//
void PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(int level);

// Return the number of milliseconds ellapsed since the program started
//
uint32_t PLATFORM_GET_TIMESTAMP(void);


////////////////////////////////////////////////////////////////////////////////
// Misc stuff
////////////////////////////////////////////////////////////////////////////////

// [PLATFORM PORTING NOTE]
//   Depending on what other platform headers you have included up until this
//   point, 'NULL' might or might not be defined. If so, define it here
//
#ifndef NULL
#  define NULL (0x0)
#endif


////////////////////////////////////////////////////////////////////////////////
// Initialization functions
////////////////////////////////////////////////////////////////////////////////

// This function *must* be called before any other "PLATFORM_*()" API function
//
// Returns "0" if there was a problem. "1" otherwise.
//
// [PLATFORM PORTING NOTE]
//   Use this function to reserve memory, initialize semaphores, etc...
//
uint8_t PLATFORM_INIT(void);

#endif
