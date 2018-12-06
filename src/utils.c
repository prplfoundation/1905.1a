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

#include "platform.h"
#include "utils.h"

#include <string.h> // memcmp(), strncat()
#include <stdio.h> // snprintf()
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////////////
//

void copyLengthString(uint8_t *dest, uint8_t *length, const char *src, size_t size)
{
    size_t src_len = strlen(src);
    if (src_len > size)
        src_len = size;
    assert (size <= UINT8_MAX);
    *length = (uint8_t)src_len;
    memcpy(dest, src, src_len);
}

void print_callback(void (*write_function)(const char *fmt, ...), const char *prefix, uint8_t size, const char *name, const char *fmt, const void *p)
{

       if (0 == memcmp(fmt, "%s", 3))
       {
           // Strings are printed with triple quotes surrounding them
           //
           write_function("%s%s: \"\"\"%s\"\"\"\n", prefix, name, p);
           return;
       }
       else if (0 == memcmp(fmt, "%ipv4", 6))
       {
           // This is needed because "size == 4" on IPv4 addresses, but we don't
           // want them to be treated as 4 bytes integers, so we change the
           // "fmt" to "%d" and do *not* returns (so that the IPv4 ends up being
           // printed as a sequence of bytes.
           //
           fmt = "%d";
       }
       else
       {
           #define FMT_LINE_SIZE 20
           char fmt_line[FMT_LINE_SIZE];

           fmt_line[0] = 0x0;

           snprintf(fmt_line, FMT_LINE_SIZE-1, "%%s%%s: %s\n", fmt);
           fmt_line[FMT_LINE_SIZE-1] = 0x0;

           if (1 == size)
           {
               write_function(fmt_line, prefix, name, *(const uint8_t *)p);

               return;
           }
           else if (2 == size)
           {
               write_function(fmt_line, prefix, name, *(const uint16_t *)p);

               return;
           }
           else if (4 == size)
           {
               write_function(fmt_line, prefix, name, *(const uint32_t *)p);

               return;
           }
       }

       // If we get here, it's either an IPv4 address or a sequence of bytes
       //
       {
           #define AUX1_SIZE 200  // Store a whole output line
           #define AUX2_SIZE  20  // Store a fmt conversion

           uint16_t i, j;

           char aux1[AUX1_SIZE];
           char aux2[AUX2_SIZE];

           size_t aux1_offset = 0;
           size_t space_left = AUX1_SIZE;
           /* Take into account prefix length */
           space_left -= strlen(prefix);
           space_left -= strlen(name);
           space_left -= 2; // ": "

           aux1[0] = '\0';

           for (i=0; i<size; i++)
           {
               // Write one element to aux2
               //
               j = snprintf(aux2, AUX2_SIZE-1, fmt, *((const uint8_t *)p+i));
               if (j >= AUX2_SIZE)
               {
                   j = AUX2_SIZE - 1;
                   aux2[AUX2_SIZE-1] = '\0';
               }

               // Check if there is enough space left in "aux1"
               //
               //   NOTE: The "+2" is because we are going to append to "aux1"
               //         the contents of "aux2" plus a ", " string (which is
               //         three chars long)
               //         The second "+2" is because of the final "\n"
               //
               if (j+2+2 > space_left)
               {
                   // No more space left
                   // Since space_left starts as AUX1_SIZE - 2, and we keep space for 2 additional characters in the
                   // condition above, there is certainly space left for ...
                   strcpy(&aux1[aux1_offset], "...");
                   space_left -= 3;
                   aux1_offset += 3;
                   break;
               }
               // Because of the test above, there is certainly space for aux2.
               strcpy(&aux1[aux1_offset], aux2);
               space_left -= j;
               aux1_offset += j;

               strcpy(&aux1[aux1_offset], ", ");
               space_left -= 2;
               aux1_offset += 2;
           }

           write_function("%s%s: %s\n", prefix, name, aux1);
       }

       return;
}


