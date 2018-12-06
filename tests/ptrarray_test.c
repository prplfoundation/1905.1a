/*
 *  prplMesh Wi-Fi Multi-AP
 *
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

#include <ptrarray.h>
#include "platform.h"
#include <stdarg.h>
#include <string.h>

static PTRARRAY(unsigned) ptrarray;

static int check_count(unsigned expected_count)
{
    if (ptrarray.length != expected_count)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("ptrarray length %u but expected %u\n", ptrarray.length, expected_count);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int check_values(const unsigned values[])
{
    unsigned i;
    for (i = 0; i < ptrarray.length; i++)
    {
        if (values[i] == 0)
        {
            PLATFORM_PRINTF_DEBUG_WARNING("ptrarray includes unexpected element %u\n", ptrarray.data[i]);
            return 1;
        }
        if (values[i] != ptrarray.data[i])
        {
            PLATFORM_PRINTF_DEBUG_WARNING("dlist includes unexpected element %u != %u\n", ptrarray.data[i], values[i]);
            return 1;
        }
    }
    return 0;
}

int main()
{
    int ret = 0;

    ret += check_count(0);

    PTRARRAY_ADD(ptrarray, 1);
    ret += check_count(1);
    ret += check_values((unsigned[]){1, 0});
    PTRARRAY_ADD(ptrarray, 2);
    ret += check_count(2);
    ret += check_values((unsigned[]){1, 2, 0});
    PTRARRAY_ADD(ptrarray, 3);
    ret += check_count(3);
    ret += check_values((unsigned[]){1, 2, 3, 0});

    if (PTRARRAY_FIND(ptrarray, 2) != 1)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("Element '2' not found in ptrarray");
        ret++;
    }
    if (PTRARRAY_FIND(ptrarray, 0) < 3)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("Element '4' found in ptrarray");
        ret++;
    }

    PTRARRAY_REMOVE(ptrarray, 0);
    ret += check_count(2);
    ret += check_values((unsigned[]){2, 3, 0});

    PTRARRAY_REMOVE_ELEMENT(ptrarray, 1);
    /* Element 1 not in list => nothing changed */
    ret += check_count(2);
    ret += check_values((unsigned[]){2, 3, 0});

    PTRARRAY_REMOVE_ELEMENT(ptrarray, 3);
    ret += check_count(1);
    ret += check_values((unsigned[]){2, 0});

    /* Same element can be added multiple times. */
    PTRARRAY_ADD(ptrarray, 2);
    ret += check_count(2);
    ret += check_values((unsigned[]){2, 2, 0});

    PTRARRAY_CLEAR(ptrarray);
    ret += check_count(0);
    PTRARRAY_ADD(ptrarray, 1);
    ret += check_count(1);
    ret += check_values((unsigned[]){1, 0});

    PTRARRAY_CLEAR(ptrarray);

    return ret;
}
