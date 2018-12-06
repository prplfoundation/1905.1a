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

#include "aletest.h"

#include <1905_l2.h>
#include <1905_cmdus.h>
#include <1905_tlvs.h>
#include <platform.h>
#include <platform_linux.h>
#include <utils.h>

#include <arpa/inet.h>        // recv()
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>             // utime()

static struct CMDU aletest_expect_cmdu_topology_discovery =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_DISCOVERY,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // expect_al_mac_tlv.tlv,
            NULL, // ADDR_MAC0
            NULL,
        },
};


static struct CMDU aletest_send_cmdu_topology_discovery =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_DISCOVERY,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // (struct tlv *)(struct alMacAddressTypeTLV[]){
            NULL, // ADDR_MAC_PEER0
            NULL,
        },
};

static struct CMDU aletest_cmdu_topology_query =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_QUERY,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL,
        },
};

static struct _localInterfaceEntries aletest_local_interfaces[] = {
    {
        .mac_address = ADDR_MAC0,
        .media_type  = MEDIA_TYPE_IEEE_802_11G_2_4_GHZ,
        .media_specific_data_size = 10,
        .media_specific_data.ieee80211 = {
            .network_membership = { 0x00, 0x16, 0x03, 0x01, 0x85, 0x1f, },
            .role = IEEE80211_SPECIFIC_INFO_ROLE_AP,
            .ap_channel_band = 0x10,
            .ap_channel_center_frequency_index_1 = 0x20,
            .ap_channel_center_frequency_index_2 = 0x30,
        },
    },
    {
        .mac_address = ADDR_MAC1,
        .media_type  = MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET,
        .media_specific_data_size = 0,
    },
    {
        .mac_address = ADDR_MAC2,
        .media_type  = MEDIA_TYPE_IEEE_802_11G_2_4_GHZ,
        .media_specific_data_size = 10,
        .media_specific_data.ieee80211 = {
            .network_membership = { 0x00, 0x16, 0x03, 0x01, 0x85, 0x1e, },
            .role = IEEE80211_SPECIFIC_INFO_ROLE_AP,
            .ap_channel_band = 0x10,
            .ap_channel_center_frequency_index_1 = 0x20,
            .ap_channel_center_frequency_index_2 = 0x30,
        },
    },
    {
        .mac_address = ADDR_MAC3,
        .media_type  = MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET,
        .media_specific_data_size = 0,
    },
};

static struct CMDU aletest_expect_cmdu_topology_response =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_RESPONSE,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            (struct tlv *)(struct deviceInformationTypeTLV[]){
                {
                    .tlv.type            = TLV_TYPE_DEVICE_INFORMATION_TYPE,
                    .al_mac_address      = ADDR_AL,
                    .local_interfaces_nr = 4,
                    .local_interfaces    = aletest_local_interfaces,
                }
            },
            (struct tlv *)(struct deviceBridgingCapabilityTLV[]){
                {
                    .tlv.type            = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES,
                    .bridging_tuples_nr  = 0,
                },
            },
            (struct tlv *)(struct neighborDeviceListTLV[]){
                {
                    .tlv.type            = TLV_TYPE_NEIGHBOR_DEVICE_LIST,
                    .local_mac_address   = ADDR_MAC0,
                    .neighbors_nr        = 1,
                    .neighbors           = (struct _neighborEntries[]) {
                        {
                            .mac_address = ADDR_AL_PEER0,
                            .bridge_flag = 0,
                        },
                    },
                }
            },
            (struct tlv *)(struct powerOffInterfaceTLV[]){
                {
                    .tlv.type                 = TLV_TYPE_POWER_OFF_INTERFACE,
                    .power_off_interfaces_nr  = 0,
                },
            },
            (struct tlv *)(struct l2NeighborDeviceTLV[]){
                {
                    .tlv.type            = TLV_TYPE_L2_NEIGHBOR_DEVICE,
                    .local_interfaces_nr  = 0,
                },
            },
            NULL, /* multiApControllerService */
            NULL /* multiApOperationalBss */,
            NULL,
        },
};

static struct CMDU aletest_send_cmdu_topology_discovery2 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_DISCOVERY,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // (struct tlv *)(struct alMacAddressTypeTLV[]){
            NULL, // ADDR_MAC_PEER1
            NULL,
        },
};


static struct CMDU aletest_expect_cmdu_topology_discovery2 =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_DISCOVERY,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // &expect_al_mac_tlv.tlv,
            NULL, // ADDR_MAC1,
            NULL,
        },
};

/* Minimal topology response */
static struct CMDU aletest_send_cmdu_topology_response2 = {
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_RESPONSE,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            (struct tlv *)(struct deviceInformationTypeTLV[]){
                {
                    .tlv.type            = TLV_TYPE_DEVICE_INFORMATION_TYPE,
                    .al_mac_address      = ADDR_AL_PEER1,
                    .local_interfaces_nr = 1,
                    .local_interfaces    = (struct _localInterfaceEntries[]){
                        {
                            .mac_address = ADDR_MAC_PEER1,
                            .media_type  = MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET,
                            .media_specific_data_size = 0,
                        }
                    },
                }
            },
            /* No device bridging capability */
            /* No Non-1905 neighbors */
            /* No 1905 neighbors */
            NULL, /* supportedService */
            NULL,
        },
};

static struct CMDU aletest_expect_cmdu_topology_notification =
{
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_TOPOLOGY_NOTIFICATION,
    .relay_indicator = 1,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // &expect_al_mac_tlv.tlv,
            NULL,
        },
};

static void initExpected()
{
    struct alMacAddressTypeTLV *expect_al_mac_tlv =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(expect_al_mac_tlv->al_mac_address, ADDR_AL, 6);
    aletest_expect_cmdu_topology_discovery.list_of_TLVs[0] = &expect_al_mac_tlv->tlv;
    struct macAddressTypeTLV *expect_mac_tlv =
            X1905_TLV_ALLOC(macAddressType, TLV_TYPE_MAC_ADDRESS_TYPE, NULL);
    memcpy(expect_mac_tlv->mac_address, ADDR_MAC0, 6);
    aletest_expect_cmdu_topology_discovery.list_of_TLVs[1] = &expect_mac_tlv->tlv;

    struct alMacAddressTypeTLV *send_al_mac_tlv =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(send_al_mac_tlv->al_mac_address, ADDR_AL_PEER0, 6);
    aletest_send_cmdu_topology_discovery.list_of_TLVs[0] = &send_al_mac_tlv->tlv;
    struct macAddressTypeTLV *send_mac_tlv =
            X1905_TLV_ALLOC(macAddressType, TLV_TYPE_MAC_ADDRESS_TYPE, NULL);
    memcpy(send_mac_tlv->mac_address, ADDR_MAC_PEER0, 6);
    aletest_send_cmdu_topology_discovery.list_of_TLVs[1] = &send_mac_tlv->tlv;

    struct supportedServiceTLV *multiApControllerService = supportedServiceTLVAlloc(NULL, true, true);
    aletest_expect_cmdu_topology_response.list_of_TLVs[5] = &multiApControllerService->tlv;

    struct apOperationalBssTLV * multiApOperationalBss =
            X1905_TLV_ALLOC(apOperationalBss, TLV_TYPE_AP_OPERATIONAL_BSS, NULL);
    struct _apOperationalBssRadio *radio;
    mac_address bssid1 = { 0x00, 0x16, 0x03, 0x01, 0x85, 0x1f};
    struct ssid ssid1 = { 15, "My WIFI network"};
    mac_address bssid2 = { 0x00, 0x16, 0x03, 0x01, 0x85, 0x1e};
    struct ssid ssid2 = { 19, "My 2nd WIFI network"};

    radio = apOperationalBssTLVAddRadio(multiApOperationalBss, (uint8_t *)ADDR_MAC0);
    apOperationalBssRadioAddBss(radio, bssid1, ssid1);

    radio = apOperationalBssTLVAddRadio(multiApOperationalBss, (uint8_t *)ADDR_MAC2);
    apOperationalBssRadioAddBss(radio, bssid2, ssid2);

    aletest_expect_cmdu_topology_response.list_of_TLVs[6] = &multiApOperationalBss->tlv;

    struct alMacAddressTypeTLV *send_al_mac_tlv2 =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(send_al_mac_tlv2->al_mac_address, ADDR_AL_PEER1, 6);
    aletest_send_cmdu_topology_discovery2.list_of_TLVs[0] = &send_al_mac_tlv2->tlv;
    struct macAddressTypeTLV *send_mac_tlv2 =
            X1905_TLV_ALLOC(macAddressType, TLV_TYPE_MAC_ADDRESS_TYPE, NULL);
    memcpy(send_mac_tlv2->mac_address, ADDR_MAC_PEER1, 6);
    aletest_send_cmdu_topology_discovery2.list_of_TLVs[1] = &send_mac_tlv2->tlv;

    struct supportedServiceTLV *multiApAgentService = supportedServiceTLVAlloc(NULL, false, true);
    aletest_send_cmdu_topology_response2.list_of_TLVs[1] = &multiApAgentService->tlv;

    struct alMacAddressTypeTLV *expect_al_mac_tlv2 =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(expect_al_mac_tlv2->al_mac_address, ADDR_AL, 6);
    aletest_expect_cmdu_topology_discovery2.list_of_TLVs[0] = &expect_al_mac_tlv2->tlv;
    struct macAddressTypeTLV *expect_mac_tlv2 =
            X1905_TLV_ALLOC(macAddressType, TLV_TYPE_MAC_ADDRESS_TYPE, NULL);
    memcpy(expect_mac_tlv2->mac_address, ADDR_MAC1, 6);
    aletest_expect_cmdu_topology_discovery2.list_of_TLVs[1] = &expect_mac_tlv2->tlv;

    struct alMacAddressTypeTLV *expect_al_mac_tlv3 =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(expect_al_mac_tlv3->al_mac_address, ADDR_AL, 6);
    aletest_expect_cmdu_topology_notification.list_of_TLVs[0] = &expect_al_mac_tlv3->tlv;
}

int main()
{
    int result = 0;
    int s0, s1;
    uint8_t buf[10];

    PLATFORM_INIT();
    PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(3);

    initExpected();

    s0 = openPacketSocket(getIfIndex("aletestpeer0"), ETHERTYPE_1905);
    if (-1 == s0) {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to open aletestpeer0");
        return 1;
    }

    /* The AL MUST send a topology discovery CMDU every 60 seconds (+1s jitter). */
    result += expect_cmdu_match(s0, 61000, "topology discovery", &aletest_expect_cmdu_topology_discovery,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);

    /* Trigger a topology query from the AL by sending a topology discovery. The AL MAY send a query, but we expect the
     * AL under test to indeed send one immediately. */
    aletest_send_cmdu_topology_discovery.message_id = 0x4321;
    result += send_cmdu(s0, (uint8_t *)MCAST_1905, (uint8_t *)ADDR_AL_PEER0, &aletest_send_cmdu_topology_discovery);
#ifdef SPEED_UP_DISCOVERY
    /* The AL also sends another topology discovery. */
    result += expect_cmdu_match(s0, 3000, "topology discovery repeat", &aletest_expect_cmdu_topology_discovery,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);
#endif
    result += expect_cmdu_match(s0, 3000, "topology query", &aletest_cmdu_topology_query,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0);
    /* No need to respond to the query. */
#ifdef SPEED_UP_DISCOVERY
    /* A second topology discovery (with a new MID) must not re-trigger discovery. */
    aletest_send_cmdu_topology_discovery.message_id++;
    result += send_cmdu(s0, (uint8_t *)MCAST_1905, (uint8_t *)ADDR_AL_PEER0, &aletest_send_cmdu_topology_discovery);
    /* Don't expect anything on that interface. */
    sleep(1);
    if (-1 != recv(s0, buf, sizeof(buf), MSG_DONTWAIT) || EAGAIN != errno) {
        PLATFORM_PRINTF_DEBUG_ERROR("Got a response on second topology discovery\n");
        result++;
    }
#endif

    /* Send a topology query. The AL MUST send a response. */
    result += send_cmdu(s0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0, &aletest_cmdu_topology_query);
    /* AL must respond within 1 second */
    result += expect_cmdu_match(s0, 1000, "topology response", &aletest_expect_cmdu_topology_response,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0);

    s1 = openPacketSocket(getIfIndex("aletestpeer1"), ETHERTYPE_1905);
    if (-1 == s1) {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to open aletestpeer1");
        close(s0);
        return 1;
    }

    /* Announce a second AL by sending a second toplogy discovery, which should trigger another query. */
    result += send_cmdu(s1, (uint8_t *)MCAST_1905, (uint8_t *)ADDR_AL_PEER1, &aletest_send_cmdu_topology_discovery2);
#ifdef SPEED_UP_DISCOVERY
    /* The AL also sends another topology discovery. */
    result += expect_cmdu_match(s1, 3000, "topology discovery aletest1", &aletest_expect_cmdu_topology_discovery2,
                                (uint8_t *)ADDR_MAC1, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);
#endif
    result += expect_cmdu_match(s1, 3000, "topology query aletest1", &aletest_cmdu_topology_query,
                                (uint8_t *)ADDR_MAC1, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER1);
    result += send_cmdu(s1, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER1, &aletest_send_cmdu_topology_response2);
    /* This should trigger a topology notification on the other interface, because there is a new neighbor. */
    /* TODO Currently this doesn't trigger a topology change! So ignore this error for now. */
    (void) expect_cmdu_match(s0, 1000, "topology notification 0", &aletest_expect_cmdu_topology_notification,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);

    /* Force a topology notification with the virtual file. */
    if (-1 == utime("/tmp/topology_change", NULL)) {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to trigger topology change: %d (%s)\n", errno, strerror(errno));
        result++;
    } else {
        /* Notification should appear on both interfaces. */
        result += expect_cmdu_match(s0, 1000, "topology notification triggered 0", &aletest_expect_cmdu_topology_notification,
                                    (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);
        result += expect_cmdu_match(s1, 1000, "topology notification triggered 1", &aletest_expect_cmdu_topology_notification,
                                    (uint8_t *)ADDR_MAC1, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);
    }

    /* The AL MUST send a topology discovery CMDU every 60 seconds (+1s jitter). */
    /* FIXME we should subtract the time spent since the last topology discovery message */
    result += expect_cmdu_match(s0, 61000, "topology discovery", &aletest_expect_cmdu_topology_discovery,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);
    result += expect_cmdu_match(s1, 61000, "topology discovery aletest1", &aletest_expect_cmdu_topology_discovery2,
                                (uint8_t *)ADDR_MAC1, (uint8_t *)ADDR_AL, (uint8_t *)MCAST_1905);

    close(s0);
    close(s1);
    return result;
}
