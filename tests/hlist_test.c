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

#include <hlist.h>
#include "platform.h"
#include <stdarg.h>
#include <string.h>

struct htest1 {
    hlist_item h;
    unsigned data;
};

struct htest2 {
    hlist_item h;
    char data;
    char data2[15]; /* The rest of data, used to test the various print formats. */
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

int main()
{
    int ret = 0;
    dlist_head list;
    struct htest1 *ht1;

    dlist_head_init(&list);
    ret += check_count(&list, 0);
    ht1 = HLIST_ALLOC(struct htest1, h, &list);
    ht1->data = 242;
    HLIST_ALLOC(struct htest2, h, &ht1->h.children[0])->data = 42;
    HLIST_ALLOC(struct htest2, h, &ht1->h.children[0])->data = 43;

    ret += check_count(&list, 1);
    ret += check_count(&ht1->h.children[0], 2);

    dlist_remove(&ht1->h);
    ret += check_count(&list, 0);
    hlist_delete_item(&ht1->h);
    /* This also deletes the two htest2 children, but there's no way to check that. */

    /* Deleting an empty list works. */
    hlist_delete(&list);
    ret += check_count(&list, 0);

    return ret;
}
