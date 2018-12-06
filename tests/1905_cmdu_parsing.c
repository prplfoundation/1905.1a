/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
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
// This file tests the "parse_1905_CMDU_from_packets()" function by providing
// some test input streams and checking the generated output structure.
//

#include "platform.h"
#include "utils.h"

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_cmdu_test_vectors.h"

#include <string.h> // memcmp

static int check_parse_1905_cmdu(const char *test_description, uint8_t **input, struct CMDU *expected_output)
{
    int result;
    struct CMDU *real_output;

    real_output = parse_1905_CMDU_from_packets(input);

    if (0 == compare_1905_CMDU_structures(real_output, expected_output))
    {
        result = 0;
        PLATFORM_PRINTF("%-100s: OK\n", test_description);
    }
    else
    {
        result = 1;
        PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
        PLATFORM_PRINTF("  Expected output:\n");
        visit_1905_CMDU_structure(expected_output, print_callback, PLATFORM_PRINTF, "");
        PLATFORM_PRINTF("  Real output    :\n");
        visit_1905_CMDU_structure(real_output, print_callback, PLATFORM_PRINTF, "");
    }

    return result;
}

static int check_parse_1905_cmdu_header(const char *test_description, uint8_t *input, size_t input_len,
                                        struct CMDU_header *expected_output)
{
    int result = 1;
    struct CMDU_header real_output;

    memset(&real_output, 0x42, sizeof(real_output));

    if (parse_1905_CMDU_header_from_packet(input, input_len, &real_output))
    {
        if (NULL != expected_output)
        {
            if (0 == memcmp(expected_output, &real_output, sizeof(real_output)))
            {
                result = 0;
            }
        }
        // Else failed because we expected parse to fail
    }
    else
    {
        if (NULL == expected_output)
        {
            result = 0;
        }
    }

    if (0 == result)
    {
        PLATFORM_PRINTF("%-100s: OK\n", test_description);
    }
    else
    {
        PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
        if (NULL != expected_output)
        {
            PLATFORM_PRINTF("  Expected output:\n    dst_addr: " MACSTR "\n    src_addr: " MACSTR "\n"
                            "    MID: 0x%04x FID: 0x%02x Last fragment: %d\n",
                            MAC2STR(expected_output->dst_addr), MAC2STR(expected_output->src_addr),
                            expected_output->mid, expected_output->fragment_id, expected_output->last_fragment_indicator);
        }
        PLATFORM_PRINTF("  Real output:\n    dst_addr: " MACSTR "\n    src_addr: " MACSTR "\n"
                        "    MID: 0x%04x FID: 0x%02x Last fragment: %d\n",
                        MAC2STR(real_output.dst_addr), MAC2STR(real_output.src_addr),
                        real_output.mid, real_output.fragment_id, real_output.last_fragment_indicator);
    }

    return result;
}

int main(void)
{
    int result = 0;

    init_1905_cmdu_test_vectors();

    #define x1905CMDUPARSE001 "x1905CMDUPARSE001 - Parse link metric query CMDU (x1905_cmdu_streams_001)"
    result += check_parse_1905_cmdu(x1905CMDUPARSE001, x1905_cmdu_streams_001, &x1905_cmdu_structure_001);

    #define x1905CMDUPARSE002 "x1905CMDUPARSE002 - Parse link metric query CMDU (x1905_cmdu_streams_002)"
    result += check_parse_1905_cmdu(x1905CMDUPARSE002, x1905_cmdu_streams_002, &x1905_cmdu_structure_002);

    #define x1905CMDUPARSE003 "x1905CMDUPARSE003 - Parse link metric query CMDU (x1905_cmdu_streams_004)"
    result += check_parse_1905_cmdu(x1905CMDUPARSE003, x1905_cmdu_streams_004, &x1905_cmdu_structure_004);

    #define x1905CMDUPARSE004 "x1905CMDUPARSE004 - Parse topology query CMDU (x1905_cmdu_streams_005)"
    result += check_parse_1905_cmdu(x1905CMDUPARSE004, x1905_cmdu_streams_005, &x1905_cmdu_structure_005);

    result += check_parse_1905_cmdu_header("x1905CMDUPARSEHDR001 - Parse CMDU packet last fragment",
                                           x1905_cmdu_packet_001, x1905_cmdu_packet_len_001, &x1905_cmdu_header_001);

    result += check_parse_1905_cmdu_header("x1905CMDUPARSEHDR002 - Parse CMDU packet not last fragment",
                                           x1905_cmdu_packet_002, x1905_cmdu_packet_len_002, &x1905_cmdu_header_002);

    result += check_parse_1905_cmdu_header("x1905CMDUPARSEHDR003 - Parse CMDU packet wrong ethertype",
                                           x1905_cmdu_packet_003, x1905_cmdu_packet_len_003, NULL);

    result += check_parse_1905_cmdu_header("x1905CMDUPARSEHDR004 - Parse CMDU packet too short",
                                           x1905_cmdu_packet_004, x1905_cmdu_packet_len_004, NULL);

    // Return the number of test cases that failed
    //
    return result;
}







