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

#include <dlist.h>
#include "platform.h"
#include <stdarg.h>
#include <string.h>

struct dtest {
    dlist_item l;
    unsigned data;
};

static int check_count(dlist_head *list, size_t expected_count)
{
    size_t real_count = dlist_count(list);
    if (real_count != expected_count)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("dlist_count result %u but expected %u\n", (unsigned)real_count, (unsigned)expected_count);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int check_values(dlist_head *list, const unsigned values[])
{
    struct dtest *t;
    unsigned i = 0;
    dlist_for_each(t, *list, l)
    {
        if (values[i] == 0)
        {
            PLATFORM_PRINTF_DEBUG_WARNING("dlist includes unexpected element %u\n", t->data);
            return 1;
        }
        if (values[i] != t->data)
        {
            PLATFORM_PRINTF_DEBUG_WARNING("dlist includes unexpected element %u != %u\n", t->data, values[i]);
            return 1;
        }
        i++;
    }
    return 0;
}

int main()
{
    int ret = 0;
    dlist_head list1;
    DEFINE_DLIST_HEAD(list2);
    struct dtest t1;
    struct dtest t2;
    struct dtest t3;

    dlist_head_init(&list1);
    ret += check_count(&list1, 0);
    ret += check_count(&list2, 0);

    if (!dlist_empty(&list1))
    {
        PLATFORM_PRINTF_DEBUG_WARNING("dlist_empty() on empty list returns false\n");
        ret++;
    }

    t1.data = 1;
    t2.data = 2;
    t3.data = 3;

    dlist_add_head(&list1, &t1.l);
    ret += check_count(&list1, 1);
    ret += check_values(&list1, (unsigned[]){1, 0});
    dlist_add_head(&list1, &t2.l);
    ret += check_count(&list1, 2);
    ret += check_values(&list1, (unsigned[]){2, 1, 0});
    dlist_add_tail(&list1, &t3.l);
    ret += check_count(&list1, 3);
    ret += check_values(&list1, (unsigned[]){2, 1, 3, 0});

    dlist_remove(&t1.l);
    ret += check_count(&list1, 2);
    ret += check_values(&list1, (unsigned[]){2, 3, 0});

    if (dlist_empty(&list1))
    {
        PLATFORM_PRINTF_DEBUG_WARNING("dlist_empty() on non-empty list returns true\n");
        ret++;
    }

    dlist_add_head(&list2, &t1.l);
    ret += check_count(&list2, 1);
    ret += check_values(&list2, (unsigned[]){1, 0});
    /* list1 hasn't changed */
    ret += check_count(&list1, 2);
    ret += check_values(&list1, (unsigned[]){2, 3, 0});

    return ret;
}
