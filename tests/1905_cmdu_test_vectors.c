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

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_cmdu_test_vectors.h"

#include <utils.h> // ARRAY_SIZE

// This file contains test vectors than can be used to check the
// "parse_1905_CMDU_from_packets()" and "forge_1905_CMDU_from_structure()"
// functions
//
// Each test vector is made up of three variables:
//
//   - A CMDU structure
//   - An arrray of streams
//   - An array with the lengths of each stream
//
// Note that some test vectors can be used to test both functions, while others
// can only be used to test "forge_1905_CMDU_from_structure()" or
// "parse_1905_CMDU_from_packets()":
//
//   - Test vectors marked with "CMDU --> packet" can only be used to test the
//     "forge_1905_TLV_from_structure()" function.
//
//   - Test vectors marked with "CMDU <-- packet" can only be used to test the
//     "parse_1905_CMDU_from_packets()" function.
//
//   - All the other test vectors are marked with "CMDU <--> packet", meaning
//     they can be used to test  both functions.
//
// The reason this is happening is that, according to the standard, sometimes
// bits are ignored/changed/forced-to-a-value when forging a packet. Thus, not
// all test vectors are "inversible" (ie. forge(parse(stream)) != x)
//
// This is how you use these test vectors:
//
//   A) stream = forge_1905_CMDU_from_structure(tlv_xxx, &stream_len);
//
//   B) tlv = parse_1905_CMDU_from_packets(stream_xxx);
//


////////////////////////////////////////////////////////////////////////////////
//// Test vector 001 (CMDU <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct CMDU x1905_cmdu_structure_001 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_LINK_METRIC_QUERY,
    .message_id      = 7,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv* []){ NULL, NULL, },
};

uint8_t *x1905_cmdu_streams_001[] =
{
    (uint8_t []){
        0x00,
        0x00,
        0x00, 0x05,
        0x00, 0x07,
        0x00,
        0x80,

        0x08,
        0x00, 0x08,
        0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02,

        0x00,
        0x00, 0x00,
    },
    NULL
};

uint16_t x1905_cmdu_streams_len_001[] = {22, 0};


////////////////////////////////////////////////////////////////////////////////
//// Test vector 002 (CMDU <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct CMDU x1905_cmdu_structure_002 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type   = CMDU_TYPE_LINK_METRIC_QUERY,
    .message_id      = 2348,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv* []){ NULL, NULL, },
};

uint8_t *x1905_cmdu_streams_002[] =
{
    (uint8_t []){
        0x00,
        0x00,
        0x00, 0x05,
        0x09, 0x2c,
        0x00,
        0x80,

        0x08,
        0x00, 0x08,
        0x01,
        0x01, 0x02, 0x02, 0x03, 0x04, 0x05,
        0x02,

        0x00,
        0x00, 0x00,
    },
    NULL
};

uint16_t x1905_cmdu_streams_len_002[] = {22, 0};


////////////////////////////////////////////////////////////////////////////////
//// Test vector 003 (CMDU --> packet)
////////////////////////////////////////////////////////////////////////////////

struct CMDU x1905_cmdu_structure_003 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_LINK_METRIC_QUERY,
    .message_id      = 2348,
    .relay_indicator = 1,
    .list_of_TLVs    =
        (struct tlv* []){ NULL, NULL, },
};

uint8_t *x1905_cmdu_streams_003[] =
{
    (uint8_t []){
        0x00,
        0x00,
        0x00, 0x05,
        0x09, 0x2c,
        0x00,
        0x80, // Note that 'relay_indicator' is *not* set because for
              // this type of message ("CMDU_TYPE_LINK_METRIC_QUERY")
              // it must always be set to '0'

        0x08,
        0x00, 0x08,
        0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02,

        0x00,
        0x00, 0x00,
    },
    NULL
};

uint16_t x1905_cmdu_streams_len_003[] = {22, 0};


////////////////////////////////////////////////////////////////////////////////
//// Test vector 004 (CMDU <-- packet)
////////////////////////////////////////////////////////////////////////////////

struct CMDU x1905_cmdu_structure_004 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_LINK_METRIC_QUERY,
    .message_id      = 2348,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv* []){ NULL, NULL, },
};

uint8_t *x1905_cmdu_streams_004[] =
{
    (uint8_t []){
        0x00,
        0x00,
        0x00, 0x05,
        0x09, 0x2c,
        0x00,
        0x80,

        0x08,
        0x00, 0x08,
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02,

        0x00,
        0x00, 0x00,
    },
    NULL
};

uint16_t x1905_cmdu_streams_len_004[] = {22, 0};


////////////////////////////////////////////////////////////////////////////////
//// Test vector 005 (CMDU <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct CMDU x1905_cmdu_structure_005 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_QUERY,
    .message_id      = 9,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv* []){
            NULL
        },
};

uint8_t *x1905_cmdu_streams_005[] =
{
    (uint8_t []){
        0x00,
        0x00,
        0x00, 0x02,
        0x00, 0x09,
        0x00,
        0x80,

        0x00,
        0x00, 0x00,
    },
    NULL
};

uint16_t x1905_cmdu_streams_len_005[] = {11, 0};


// TODO: More tests for all types of CMDUs


struct CMDU_header x1905_cmdu_header_001 =
{
    .dst_addr                = "\x00\xb2\xc3\xd4\xe5\xf6",
    .src_addr                = "\x02\x22\x33\x44\x55\x66",
    .message_type            = 0x0002,
    .mid                     = 0x4321,
    .fragment_id             = 0x00,
    .last_fragment_indicator = true,
};

uint8_t x1905_cmdu_packet_001[] =
{
    0x00, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6,
    0x02, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x89, 0x3a,
    0x00,
    0x00,
    0x00, 0x02,
    0x43, 0x21,
    0x00,
    0x80,

    0x00,
    0x00, 0x00,
};
size_t  x1905_cmdu_packet_len_001 = ARRAY_SIZE(x1905_cmdu_packet_001);

struct CMDU_header x1905_cmdu_header_002 =
{
    .dst_addr                = "\x00\xb2\xc3\xd4\xe5\xf6",
    .src_addr                = "\x02\x22\x33\x44\x55\x66",
    .message_type            = 0x0002,
    .mid                     = 0x4321,
    .fragment_id             = 0x01,
    .last_fragment_indicator = false,
};

uint8_t x1905_cmdu_packet_002[] =
{
    0x00, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6,
    0x02, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x89, 0x3a,
    0x00,
    0x00,
    0x00, 0x02,
    0x43, 0x21,
    0x01,
    0x40,

    0x00,
    0x00, 0x00,
};
size_t  x1905_cmdu_packet_len_002 = ARRAY_SIZE(x1905_cmdu_packet_002);

uint8_t x1905_cmdu_packet_003[] =
{
    0x00, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6,
    0x02, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x89, 0x3b,
    0x00,
    0x00,
    0x00, 0x02,
    0x43, 0x21,
    0x02,
    0x80,

    0x00,
    0x00, 0x00,
};
size_t  x1905_cmdu_packet_len_003 = ARRAY_SIZE(x1905_cmdu_packet_003);

uint8_t x1905_cmdu_packet_004[] =
{
    0x00, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6,
    0x02, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x89, 0x3a,
    0x00,
    0x00,
    0x00, 0x02,
    0x43, 0x21,
    0x01,
};
size_t  x1905_cmdu_packet_len_004 = ARRAY_SIZE(x1905_cmdu_packet_004);

void init_1905_cmdu_test_vectors(void)
{
    x1905_cmdu_structure_001.list_of_TLVs[0] =
            &linkMetricQueryTLVAllocAll(NULL, LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS)->tlv;

    mac_address specific_neighbor = {0x01, 0x02, 0x02, 0x03, 0x04, 0x05};
    x1905_cmdu_structure_002.list_of_TLVs[0] =
            &linkMetricQueryTLVAllocSpecific(NULL, specific_neighbor,
                                             LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS)->tlv;

    struct linkMetricQueryTLV *link_metric_query =
            linkMetricQueryTLVAllocAll(NULL, LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS);
    memcpy(link_metric_query->specific_neighbor, specific_neighbor, 6);
    x1905_cmdu_structure_003.list_of_TLVs[0] = &link_metric_query->tlv;

    x1905_cmdu_structure_004.list_of_TLVs[0] =
            &linkMetricQueryTLVAllocAll(NULL, LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS)->tlv;
}
