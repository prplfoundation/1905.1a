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

/** @file
 * @brief Driver interface for UCI
 *
 * This file provides driver functionality using UCI. It uses UCI calls to create access points.
 *
 * The register function must be called when the radios have already been discovered (e.g. with nl80211).
 */

#include <datamodel.h>
#include "platform_uci.h"

#include <stdio.h>      // printf(), popen()
#include <stdlib.h>     // malloc(), ssize_t
#include <string.h>     // strdup()
#include <libubox/blobmsg.h>
#include <libubus.h>

#include <platform.h>

/** Invoke method on UCI over ubus
 *
 * @a b is freed whether or not the method fails.
 *
 * The call is done synchronously.
 */
static bool uci_invoke(const char *method, struct blob_buf *b)
{
    struct ubus_context *ctx = ubus_connect(NULL);
    bool ret = true;
    uint32_t id;

    if (!ctx) {
        fprintf(stderr, "failed to connect to ubus.\n");
        return false;
    }

    if (ubus_lookup_id(ctx, "uci", &id) ||
        ubus_invoke(ctx, id, method, b->head, NULL, NULL, 3000)) {
        ret = false;
    }

    blob_buf_free(b);
    ubus_free(ctx);
    return ret;
}



static bool uci_commit_wireless()
{
    struct blob_buf b;

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", "wireless");
    return uci_invoke("commit", &b);
}

static bool uci_teardown_iface(struct interface *interface)
{
    struct interfaceWifi *interface_wifi = container_of(interface, struct interfaceWifi, i);
    char macstr[18];
    struct blob_buf b;
    void *match;

    if (interface->type != interface_type_wifi)
    {
        return false;
    }

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", "wireless");
    blobmsg_add_string(&b, "type", "wifi-iface");
    match = blobmsg_open_table(&b, "match");
    snprintf(macstr, sizeof(macstr), MACSTR, MAC2STR(interface_wifi->bssInfo.bssid));
    blobmsg_add_string(&b, "bssid", macstr);
    blobmsg_add_string(&b, "device", (char *)interface_wifi->radio->priv);
    blobmsg_close_table(&b, match);
    if (!uci_invoke("delete", &b)) {
        return false;
    }

    /* @todo The removal of the interface should be detected through netlink. For the time being, however, we update the data model
     * straight away. */
    interfaceWifiRemove(interface_wifi);
    return uci_commit_wireless();
}

static bool uci_create_iface(struct radio *radio, struct bssInfo bssInfo, bool ap)
{
    char macstr[18];
    struct blob_buf b;
    void *values;
    uint8_t multi_ap;

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", "wireless");
    blobmsg_add_string(&b, "type", "wifi-iface");
    values = blobmsg_open_table(&b, "values");
    blobmsg_add_string(&b, "device", (char *)radio->priv);
    blobmsg_add_string(&b, "mode", ap ? "ap" : "sta");
    blobmsg_add_string(&b, "network", "lan"); /* @todo set appropriate network */
    snprintf(macstr, sizeof(macstr), MACSTR, MAC2STR(bssInfo.bssid));
    blobmsg_add_string(&b, "bssid", macstr);
    blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "ssid", bssInfo.ssid.ssid, bssInfo.ssid.length);
    switch (bssInfo.auth_mode) {
    case auth_mode_open:
        blobmsg_add_string(&b, "encryption", "none");
        break;
    case auth_mode_wpa2:
        PLATFORM_PRINTF_DEBUG_ERROR("Encryption type WPA2-Enterprise not supported");
        blob_buf_free(&b);
        return false;
    case auth_mode_wpa2psk:
        blobmsg_add_string(&b, "encryption", "psk2");
        blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "key", bssInfo.key, bssInfo.key_len);
        break;
    }

    if (ap) {
        if (bssInfo.backhaul && bssInfo.backhaul_only) {
            multi_ap = 1;
        } else if (bssInfo.backhaul) {
            multi_ap = 3;
        } else {
            multi_ap = 2; /* Fronthaul only */
        }
    } else {
        multi_ap = 1; /* STA is always a backhaul STA */
    }
    blobmsg_add_u8(&b, "multi_ap", multi_ap);

    if (ap && !bssInfo.backhaul_only) {
        /* Fronthaul BSS must have WPS enabled */
        blobmsg_add_u8(&b, "wps_pushbutton", 1);
        if (local_device->backhaul_ssid.length > 0) {
            blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "multi_ap_backhaul_ssid",
                              local_device->backhaul_ssid.ssid, local_device->backhaul_ssid.length);
            if (local_device->backhaul_key_length > 0) {
                blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "multi_ap_backhaul_key",
                                  local_device->backhaul_key, local_device->backhaul_key_length);
            }
        }
    }

    blobmsg_close_table(&b, values);
    if (!uci_invoke("add", &b)) {
        return false;
    }

    /* @todo The presence of the new AP should be detected through netlink. For the time being, however, we update the data model
     * straight away. */
    struct interfaceWifi *iface = interfaceWifiAlloc(bssInfo.bssid, local_device);
    radioAddInterfaceWifi(radio, iface);
    iface->role = interface_wifi_role_ap;
    memcpy(&iface->bssInfo, &bssInfo, sizeof(bssInfo));
    iface->i.tearDown = uci_teardown_iface;

    return uci_commit_wireless();
}

static bool uci_create_ap(struct radio *radio, struct bssInfo bssInfo)
{
    return uci_create_iface(radio, bssInfo, true);
}

static bool uci_create_sta(struct radio *radio, struct bssInfo bssInfo)
{
    return uci_create_iface(radio, bssInfo, false);
}

static bool uci_set_backhaul_values(const struct ssid ssid, const uint8_t *key, size_t key_length, const char *multi_ap_val)
{
    struct blob_buf b;
    void *values;
    void *match;

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", "wireless");
    match = blobmsg_open_table(&b, "match");
    blobmsg_add_string(&b, "multi_ap", multi_ap_val);
    blobmsg_close_table(&b, match);

    if (ssid.length > 0) {
        values = blobmsg_open_table(&b, "values");
        blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "multi_ap_backhaul_ssid", ssid.ssid, ssid.length);
        if (key_length > 0) {
            blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "multi_ap_backhaul_key", key, key_length);
        }
        blobmsg_close_table(&b, values);
        return uci_invoke("set", &b);
    } else {
        values = blobmsg_open_array(&b, "options");
        blobmsg_add_string(&b, NULL, "multi_ap_backhaul_ssid");
        blobmsg_add_string(&b, NULL, "multi_ap_backhaul_key");
        blobmsg_close_array(&b, values);
        return uci_invoke("delete", &b);
    }
}

static bool uci_set_backhaul_ssid(__attribute__((unused)) struct radio *radio,
                                  const struct ssid ssid, const uint8_t *key, size_t key_length)
{
    bool result;

    /* Instead of iterating over configured_bsses, we just use the UCI match table to set all fronthaul aps.
     * We need to do that twice, for multi_ap = 2 and multi_ap = 3.
     *
     * Note that we do this for all radios, not just for the radio passed as an argument. The function will
     * anyway be called for all radios.
     */
    result = uci_set_backhaul_values(ssid, key, key_length, "2");
    result = result && uci_set_backhaul_values(ssid, key, key_length, "3");
    result = result && uci_commit_wireless();
    return result;
}

/*
 * policy for UCI get
 */
enum {
    UCI_GET_VALUES,
    __UCI_GET_MAX,
};

static const struct blobmsg_policy uciget_policy[__UCI_GET_MAX] = {
    [UCI_GET_VALUES] = { .name = "values", .type = BLOBMSG_TYPE_TABLE },
};

/* dlist to store uci wifi-device section names */
struct uciradiolist {
    dlist_item l;
    char *section;
    char *phyname;
};

/*
 * called by UCI get ubus call with all wifi-device sections
 * populates dlist with uci config names of wifi radios
 */
static void radiolist_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    dlist_head *list = (dlist_head *)req->priv;
    struct blob_attr *tb[__UCI_GET_MAX];
    struct blob_attr *cur;
    struct uciradiolist *item;
    int rem;

    blobmsg_parse(uciget_policy, __UCI_GET_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[UCI_GET_VALUES]) {
        fprintf(stderr, "No radios found\n");
        return;
    }

    blobmsg_for_each_attr(cur, tb[UCI_GET_VALUES], rem) {
        item = zmemalloc(sizeof(struct uciradiolist));
        item->section = strdup(blobmsg_name(cur));
        dlist_add_tail(list, &item->l);
    }
}

/*
 * policy for iwinfo phyname
 */
enum {
    IWINFO_PHYNAME_PHYNAME,
    __IWINFO_PHYNAME_MAX,
};

static const struct blobmsg_policy phyname_policy[__IWINFO_PHYNAME_MAX] = {
    [IWINFO_PHYNAME_PHYNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

/*
 * called by iwinfo phyname ubus call with resolved phyname
 * copies phyname to result pointer
 */
static void phyname_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char **phyname = (char **)req->priv;
    struct blob_attr *tb[__IWINFO_PHYNAME_MAX];
    blobmsg_parse(phyname_policy, __IWINFO_PHYNAME_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[IWINFO_PHYNAME_PHYNAME]) {
        fprintf(stderr, "No phyname returned\n");
        return;
    }

    *phyname = strdup(blobmsg_get_string(tb[IWINFO_PHYNAME_PHYNAME]));
}

void uci_register_handlers(void)
{
    struct ubus_context *ctx = ubus_connect(NULL);
    uint32_t id;
    static struct blob_buf req;
    static dlist_head uciradios;
    struct radio *radio;
    struct uciradiolist *uciphymatch;
    char *phyname;

    if (!ctx) {
        fprintf(stderr, "failed to connect to ubus.\n");
        return;
    }

    blob_buf_init(&req, 0);
    blobmsg_add_string(&req, "config", "wireless");
    blobmsg_add_string(&req, "type", "wifi-device");

    dlist_head_init(&uciradios);

    /* get radios from UCI */
    if (ubus_lookup_id(ctx, "uci", &id) ||
        ubus_invoke(ctx, id, "get", req.head, radiolist_cb, &uciradios, 3000))
        goto reghandlers_out;

    blob_buf_free(&req);

    /* populate phyname for each UCI wifi-device section */
    dlist_for_each(uciphymatch, uciradios, l)
    {
        blob_buf_init(&req, 0);
        blobmsg_add_string(&req, "section", uciphymatch->section);
        /* get phyname from iwinfo */
        if (ubus_lookup_id(ctx, "iwinfo", &id) ||
            ubus_invoke(ctx, id, "phyname", req.head, phyname_cb, &phyname, 3000))
            goto reghandlers_out;

        blob_buf_free(&req);
        uciphymatch->phyname = phyname;
    }

    /* register handlers for phy matching wifi-device UCI section */
    dlist_for_each(radio, local_device->radios, l)
    {
        dlist_for_each(uciphymatch, uciradios, l)
        {
            /*
             * register handlers if there is an UCI config section for
             * a discovered phy.
             */
            if(!strcmp(radio->name, uciphymatch->phyname)) {
                radio->addAP = uci_create_ap;
                radio->addSTA = uci_create_sta;
                radio->setBackhaulSsid = uci_set_backhaul_ssid;
                radio->priv = (void *)strdup(uciphymatch->section);
                PLATFORM_PRINTF_DEBUG_DETAIL("registered UCI wifi-device %s (%s)\n",
                                             uciphymatch->section, uciphymatch->phyname);
                break;
            }
        }
    }

reghandlers_out:
    while ((uciphymatch = container_of(dlist_get_first(&uciradios), struct uciradiolist, l)))
    {
        dlist_remove(&uciphymatch->l);
        free(uciphymatch->section);
        free(uciphymatch->phyname);
        free(uciphymatch);
    }

    ubus_free(ctx);
}
