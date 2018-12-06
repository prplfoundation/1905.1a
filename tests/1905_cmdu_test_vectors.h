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

#ifndef _1905_CMDU_TEST_VECTORS_H_
#define _1905_CMDU_TEST_VECTORS_H_

#include "1905_cmdus.h"
#include "1905_tlvs.h"

extern struct CMDU   x1905_cmdu_structure_001;
extern uint8_t        *x1905_cmdu_streams_001[];
extern uint16_t        x1905_cmdu_streams_len_001[];

extern struct CMDU   x1905_cmdu_structure_002;
extern uint8_t        *x1905_cmdu_streams_002[];
extern uint16_t        x1905_cmdu_streams_len_002[];

extern struct CMDU   x1905_cmdu_structure_003;
extern uint8_t        *x1905_cmdu_streams_003[];
extern uint16_t        x1905_cmdu_streams_len_003[];

extern struct CMDU   x1905_cmdu_structure_004;
extern uint8_t        *x1905_cmdu_streams_004[];
extern uint16_t        x1905_cmdu_streams_len_004[];

extern struct CMDU   x1905_cmdu_structure_005;
extern uint8_t        *x1905_cmdu_streams_005[];
extern uint16_t        x1905_cmdu_streams_len_005[];

/** @defgroup tv_cmdu_header CMDU header parsing test vectors
 */

/** @defgroup tv_cmdu_header_001 CMDU header with last fragment indicator
 *
 * @ingroup tv_cmdu_header
 * @{
 */
extern struct CMDU_header x1905_cmdu_header_001;
extern uint8_t            x1905_cmdu_packet_001[];
extern size_t             x1905_cmdu_packet_len_001;
/** @} */

/** @defgroup tv_cmdu_header_002 CMDU header without last fragment indicator
 *
 * @ingroup tv_cmdu_header
 * @{
 */
extern struct CMDU_header x1905_cmdu_header_002;
extern uint8_t            x1905_cmdu_packet_002[];
extern size_t             x1905_cmdu_packet_len_002;
/** @} */

/** @defgroup tv_cmdu_header_003 CMDU header with wrong ether type
 *
 * @ingroup tv_cmdu_header
 * @{
 */
extern uint8_t            x1905_cmdu_packet_003[];
extern size_t             x1905_cmdu_packet_len_003;
/** @} */

/** @defgroup tv_cmdu_header_004 CMDU header is too short
 *
 * @ingroup tv_cmdu_header
 * @{
 */
extern uint8_t            x1905_cmdu_packet_004[];
extern size_t             x1905_cmdu_packet_len_004;
/** @} */

void init_1905_cmdu_test_vectors(void);

#endif
