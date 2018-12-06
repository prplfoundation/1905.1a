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
// This file tests the "forge_1905_TLV_from_structure()" function by providing
// some test input structures and checking the generated output stream.
//

#include "platform.h"
#include "1905_tlvs.h"
#include "1905_tlv_test_vectors.h"

#include <string.h> // memcmp(), memcpy(), ...

static char printBuf[1000] = "";

/** @brief Helper to test print functions. */
static void test_write_function(const char *fmt, ...)
{
    va_list arglist;
    size_t printBufLen = strlen(printBuf);

    va_start(arglist, fmt);
    vsnprintf(printBuf + printBufLen, sizeof(printBuf) - printBufLen - 1, fmt, arglist);
    va_end(arglist);
}

static int check_print_compare(const char *expected, const char *prefix, const char *function)
{
    if (strncmp(printBuf, expected, sizeof(printBuf)) != 0)
    {
        PLATFORM_PRINTF("Print %4s %-95s: KO !!!\n", prefix, function);
        PLATFORM_PRINTF("  Expected >>>\n%s<<<\n", expected);
        PLATFORM_PRINTF("  Got >>>\n%s<<<\n", printBuf);
        return 1;
    }
    else
    {
        PLATFORM_PRINTF("Print %4s %-95s: OK\n", prefix, function);
        return 0;
    }
}

static int check_print(struct tlv_struct *s, const char *prefix, const char *expected)
{
    int ret = 0;
    char new_prefix[100];
    /* The parent list contains exactly one element, so we can test both tlv_struct_print_list and tlv_struct_print. */
    printBuf[0] = '\0';
    tlv_struct_print_list(s->h.l.prev, false, test_write_function, prefix);
    ret += check_print_compare(expected, prefix, "tlv_struct_print_list");

    snprintf(new_prefix, sizeof(new_prefix)-1, "%s%s", prefix, s->desc->name);
    printBuf[0] = '\0';
    tlv_struct_print(s, test_write_function, new_prefix);
    ret += check_print_compare(expected, prefix, "tlv_struct_print");
    return ret;
}

/** @brief Test tlv_struct_compare_list and TLV_STRUCT_COMPARE on @a s1 and @a s2. */
static int check_compare(struct tlv_struct *s1, struct tlv_struct *s2, int expected_result, const char *reason)
{
    int ret = 0;
    int result;
    result = tlv_struct_compare(s1, s2);
    result = (result > 0 ? 1 : (result < 0 ? -1 : 0));
    if (result != expected_result)
    {
        PLATFORM_PRINTF("Compare %-98s: KO !!!\n", reason);
        PLATFORM_PRINTF("tlv_struct_compare result %d but expected %d\n", result, expected_result);
        ret++;
    }
    result = tlv_struct_compare_list(s1->h.l.prev, s2->h.l.prev);
    result = (result > 0 ? 1 : (result < 0 ? -1 : 0));
    if (result != expected_result)
    {
        PLATFORM_PRINTF("Compare %-98s: KO !!!\n", reason);
        PLATFORM_PRINTF("  tlv_struct_compare_list result %d but expected %d\n", result, expected_result);
        ret++;
    }
    if (ret == 0)
    {
        PLATFORM_PRINTF("Compare %-98s: OK\n", reason);
    }
    return ret;
}

static int _check(const char *test_description, const struct tlv *input, const uint8_t *expected_output,
               uint16_t expected_output_len)
{
    int      result;
    uint8_t *real_output;
    uint16_t real_output_len;

    real_output = forge_1905_TLV_from_structure(input, &real_output_len);

    if (NULL == real_output)
    {
        PLATFORM_PRINTF("Forge %-100s: KO !!!\n", test_description);
        PLATFORM_PRINTF("  forge_1905_TLV_from_structure() returned a NULL pointer\n");

        return 1;
    }

    if ((expected_output_len == real_output_len) && (0 == memcmp(expected_output, real_output, real_output_len)))
    {
        result = 0;
        PLATFORM_PRINTF("Forge %-100s: OK\n", test_description);
    }
    else
    {
        uint16_t i;

        result = 1;
        PLATFORM_PRINTF("Forge %-100s: KO !!!\n", test_description);
        PLATFORM_PRINTF("  Expected output: ");
        for (i=0; i<expected_output_len; i++)
        {
            PLATFORM_PRINTF("%02x ",expected_output[i]);
        }
        PLATFORM_PRINTF("\n");
        PLATFORM_PRINTF("  Real output    : ");
        for (i=0; i<real_output_len; i++)
        {
            PLATFORM_PRINTF("%02x ",real_output[i]);
        }
        PLATFORM_PRINTF("\n");
    }
    free_1905_TLV_packet(real_output);

    return result;
}


int main(void)
{
    int result = 0;
    struct x1905_tlv_test_vector *t;
    dlist_head test_vectors;
    DEFINE_DLIST_HEAD(list1);
    DEFINE_DLIST_HEAD(list2);
    struct associatedClientsTLV *tlv1;
    struct associatedClientsTLV *tlv2;
    struct _associatedClientsBssInfo *bss_info;
    mac_address addr = { 1, 2, 3, 4, 5, 6, };
    struct tlv_struct_description bssid_desc;
    const struct tlv_struct_description *bssid_desc_orig;

    dlist_head_init(&test_vectors);
    get_1905_tlv_test_vectors(&test_vectors);

    hlist_for_each(t, test_vectors, struct x1905_tlv_test_vector, h)
    {
        if (t->forge)
            result += _check(t->description, container_of(t->h.children[0].next, struct tlv, s.h.l), t->stream, t->stream_len);
    }
    // @todo currently the test vectors still point to statically allocated TLVs
    // hlist_delete(&test_vectors);

    /* The TLV print and comparison functions are not sufficiently covered by the parse/forge tests, so they are tested
     * here separately on an associatedClientsTLV. */
    tlv1 = X1905_TLV_ALLOC(associatedClients, TLV_TYPE_ASSOCIATED_CLIENTS, &list1);
    tlv2 = X1905_TLV_ALLOC(associatedClients, TLV_TYPE_ASSOCIATED_CLIENTS, &list2);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, 0, "Empty associatedClientsTLV");
    bss_info = associatedClientsTLVAddBssInfo(tlv1, addr);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, 1, "Longer associatedClientsTLV is larger");
    result += check_print(&tlv1->tlv.s, "%s%s", "%s%sassociatedClients->bss[0]->bssid: 01:02:03:04:05:06\n");

    /* Test the print format functions by varying the field description of bssid. */
    bssid_desc_orig = bss_info->s.desc;
    bssid_desc = *bssid_desc_orig;
    bss_info->s.desc = &bssid_desc;

    bssid_desc.fields[0].size = 5;
    bssid_desc.fields[0].format = tlv_struct_print_format_hex;
    result += check_print(&tlv1->tlv.s, "5x: ", "5x: associatedClients->bss[0]->bssid: 01 02 03 04 05 \n");

    bssid_desc.fields[0].size = 4;
    bssid_desc.fields[0].format = tlv_struct_print_format_ipv4;
    result += check_print(&tlv1->tlv.s, "4i: ", "4i: associatedClients->bss[0]->bssid: 1.2.3.4\n");

    *(int32_t*)&bss_info->bssid = -1778272308;
    bssid_desc.fields[0].format = tlv_struct_print_format_dec;
    result += check_print(&tlv1->tlv.s, "4d: ", "4d: associatedClients->bss[0]->bssid: -1778272308\n");

    *(uint32_t*)&bss_info->bssid = 0xa5991234;
    bssid_desc.fields[0].format = tlv_struct_print_format_unsigned;
    result += check_print(&tlv1->tlv.s, "4u: ", "4u: associatedClients->bss[0]->bssid: 2778272308\n");

    bssid_desc.fields[0].format = tlv_struct_print_format_hex;
    result += check_print(&tlv1->tlv.s, "4x: ", "4x: associatedClients->bss[0]->bssid: 0xa5991234\n");

    bssid_desc.fields[0].size = 2;
    *(uint16_t*)&bss_info->bssid = 0xa599;
    result += check_print(&tlv1->tlv.s, "2x: ", "2x: associatedClients->bss[0]->bssid: 0xa599\n");

    /* @todo bssid is not big enough to store an IPv6 address so that is not tested. */

    /* Restore the original situation */
    memcpy(bss_info->bssid, addr, 6);
    bss_info->s.desc = bssid_desc_orig;

    addr[0] = 0x10;
    associatedClientsTLVAddClientInfo(bss_info, addr, 42);
    result += check_print(&tlv1->tlv.s, "",
                          "associatedClients->bss[0]->bssid: 01:02:03:04:05:06\n"
                          "associatedClients->bss[0]->client[0]->addr: 10:02:03:04:05:06\n"
                          "associatedClients->bss[0]->client[0]->age: 42\n");

    addr[0] = 0x01;
    bss_info = associatedClientsTLVAddBssInfo(tlv2, addr);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, 1, "Longer associatedClientsTLV children is larger");
    addr[0] = 0x10;
    associatedClientsTLVAddClientInfo(bss_info, addr, 44);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, -1, "Smaller age value is smaller");
    hlist_delete(&bss_info->s.h.children[0]);
    addr[0] = 0x09;
    associatedClientsTLVAddClientInfo(bss_info, addr, 42);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, 1, "Larger addr is larger");

    hlist_delete(&bss_info->s.h.children[0]);
    addr[0] = 0x10;
    associatedClientsTLVAddClientInfo(bss_info, addr, 42);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, 0, "Recursively same structure");
    addr[0] = 0x00;
    associatedClientsTLVAddClientInfo(bss_info, addr, 42);
    result += check_compare(&tlv1->tlv.s, &tlv2->tlv.s, -1, "Shorter clients list is smaller");

    hlist_delete(&list1);
    hlist_delete(&list2);

    // Return the number of test cases that failed
    //
    return result;
}
