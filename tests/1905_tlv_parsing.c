/*
 *  prplMesh Wi-Fi Multi-AP
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

//
// This file tests the "parse_1905_TLV_from_packet()" function by providing
// some test input streams and checking the generated output structure.
//

#include "platform.h"
#include "utils.h"

#include "1905_tlvs.h"
#include "1905_tlv_test_vectors.h"

static uint8_t _check(const char *test_description, const uint8_t *input, struct tlv *expected_output)
{
    uint8_t  result;
    struct tlv *real_output;

    real_output = parse_1905_TLV_from_packet(input);

    if (real_output == NULL)
    {
        result = 1;
        PLATFORM_PRINTF("Parse %-100s: KO !!!\n", test_description);
        PLATFORM_PRINTF("  Parse failure\n");
    }
    else if (0 == compare_1905_TLV_structures(real_output, expected_output))
    {
        result = 0;
        PLATFORM_PRINTF("Parse %-100s: OK\n", test_description);
    }
    else
    {
        result = 1;
        PLATFORM_PRINTF("Parse %-100s: KO !!!\n", test_description);
        PLATFORM_PRINTF("  Expected output:\n");
        visit_1905_TLV_structure(expected_output, print_callback, PLATFORM_PRINTF, "");
        PLATFORM_PRINTF("  Real output    :\n");
        visit_1905_TLV_structure(real_output, print_callback, PLATFORM_PRINTF, "");
    }
    free_1905_TLV_structure(real_output);

    return result;
}

int main(void)
{
    int result = 0;
    struct x1905_tlv_test_vector *t;
    dlist_head test_vectors;

    dlist_head_init(&test_vectors);
    get_1905_tlv_test_vectors(&test_vectors);

    hlist_for_each(t, test_vectors, struct x1905_tlv_test_vector, h)
    {
        if (t->parse)
            result += _check(t->description, t->stream, container_of(t->h.children[0].next, struct tlv, s.h.l));
    }
    // @todo currently the test vectors still point to statically allocated TLVs
    // hlist_delete(&test_vectors);

    // Return the number of test cases that failed
    //
    return result;
}
