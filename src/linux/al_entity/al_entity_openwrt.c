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

#include <hlist.h>

#include <platform.h>
#include "../../platform_interfaces.h"
#include "../platform_interfaces_priv.h"            // addInterface
#include "../platform_alme_server_priv.h"           // almeServerPortSet()
#include "../platform_uci.h"
#include "../../al.h"                                  // start1905AL

#include <datamodel.h>
#include "../../al_datamodel.h"

#include <arpa/inet.h>        // socket, AF_INTER, htons(), ...
#include <sys/ioctl.h>        // ioctl(), SIOCGIFHWADDR
#include <net/if.h>           // struct ifreq, IFNAZSIZE
#include <stdio.h>   // printf
#include <unistd.h>  // getopt
#include <stdlib.h>  // exit
#include <string.h>  // strtok

extern int  netlink_collect_local_infos(void);

#include <libubox/blobmsg.h>
#include <libubus.h>

////////////////////////////////////////////////////////////////////////////////
// Static (auxiliary) private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// Port number where the ALME server will be listening to by default
//
#define DEFAULT_ALME_SERVER_PORT 8888


/*
 * policies for network.wireless status
 */
enum {
    WIRELESS_UP,
    WIRELESS_PENDING,
    WIRELESS_AUTOSTART,
    WIRELESS_DISABLED,
    WIRELESS_RETRY_SETUP_FAILED,
    WIRELESS_CONFIG,
    WIRELESS_INTERFACES,
    __WIRELESS_MAX,
};

static const struct blobmsg_policy wireless_status_policy[__WIRELESS_MAX] = {
    [WIRELESS_UP] = { .name = "up", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_PENDING] = { .name = "pending", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_AUTOSTART] = { .name = "autostart", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_DISABLED] = { .name = "disabled", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_RETRY_SETUP_FAILED] = { .name = "retry_setup_failed", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_TABLE },
    [WIRELESS_INTERFACES] = { .name = "interfaces", .type = BLOBMSG_TYPE_ARRAY },
};

enum {
    WIRELESS_CONFIG_HWMODE,
    WIRELESS_CONFIG_PATH,
    WIRELESS_CONFIG_NOSCAN,
    WIRELESS_CONFIG_COUNTRY,
    WIRELESS_CONFIG_LEGACY_RATES,
    WIRELESS_CONFIG_DISTANCE,
    WIRELESS_CONFIG_CHANNEL,
    WIRELESS_CONFIG_HTMODE,
    WIRELESS_CONFIG_DISABLED,
    __WIRELESS_CONFIG_MAX,
};

static const struct blobmsg_policy wireless_config_policy[__WIRELESS_CONFIG_MAX] = {
    [WIRELESS_CONFIG_HWMODE] = { .name = "hwmode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_CONFIG_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_CONFIG_NOSCAN] = { .name = "noscan", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_CONFIG_COUNTRY] = { .name = "country", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_CONFIG_LEGACY_RATES] = { .name = "legacy_rates", .type = BLOBMSG_TYPE_INT8 },
    [WIRELESS_CONFIG_DISTANCE] = { .name = "distance", .type = BLOBMSG_TYPE_INT32 },
    [WIRELESS_CONFIG_CHANNEL] = { .name = "channel", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_CONFIG_HTMODE] = { .name = "htmode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_CONFIG_DISABLED] = { .name = "disabled", .type = BLOBMSG_TYPE_INT8 },
};

enum {
    WIRELESS_INTERFACE_SECTION,
    WIRELESS_INTERFACE_IFNAME,
    WIRELESS_INTERFACE_CONFIG,
    __WIRELESS_INTERFACE_MAX,
};

static const struct blobmsg_policy wireless_interface_policy[__WIRELESS_INTERFACE_MAX] = {
    [WIRELESS_INTERFACE_SECTION] = { .name = "section", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_INTERFACE_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_INTERFACE_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_TABLE },
};

enum {
    WIRELESS_IFCONFIG_MODE,
    WIRELESS_IFCONFIG_IFNAME,
    WIRELESS_IFCONFIG_SSID,
    WIRELESS_IFCONFIG_ENCRYPTION,
    WIRELESS_IFCONFIG_KEY,
    WIRELESS_IFCONFIG_MULTI_AP,
    WIRELESS_IFCONFIG_NETWORK,
    __WIRELESS_IFCONFIG_MAX,
};

static const struct blobmsg_policy wireless_ifconfig_policy[__WIRELESS_IFCONFIG_MAX] = {
    [WIRELESS_IFCONFIG_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFCONFIG_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFCONFIG_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFCONFIG_ENCRYPTION] = { .name = "encryption", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFCONFIG_KEY] = { .name = "key", .type = BLOBMSG_TYPE_STRING },
    [WIRELESS_IFCONFIG_MULTI_AP] = { .name = "multi_ap", .type = BLOBMSG_TYPE_INT32 },
    [WIRELESS_IFCONFIG_NETWORK] = { .name = "network", .type = BLOBMSG_TYPE_ARRAY },
};

/*
 * called by network.wireless status call with all radios and configured
 * interfaces. calls addInterface()
 */
static void wireless_status_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char **network = (char **)req->priv;
    struct blob_attr *tb[__WIRELESS_MAX];
    struct blob_attr *tbc[__WIRELESS_CONFIG_MAX];
    struct blob_attr *tbi[__WIRELESS_INTERFACE_MAX];
    struct blob_attr *tbic[__WIRELESS_IFCONFIG_MAX];

    struct blob_attr *cur, *curif, *curnt;
    int rem, remif, remnt;
    char *ucinet = NULL, *curucinet = NULL;

    blobmsg_for_each_attr(cur, msg, rem) {
        const char *radio = blobmsg_name(cur);
        char *section, *hwmode, *htmode;
        uint16_t media_type = INTERFACE_TYPE_UNKNOWN;
        bool is11a;

        if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE)
            continue;

        blobmsg_parse(wireless_status_policy, __WIRELESS_MAX, tb, blobmsg_data(cur), blobmsg_len(cur));
        if (!tb[WIRELESS_CONFIG])
            continue;
        blobmsg_parse(wireless_config_policy, __WIRELESS_CONFIG_MAX, tbc, blobmsg_data(tb[WIRELESS_CONFIG]), blobmsg_len(tb[WIRELESS_CONFIG]));

        hwmode = blobmsg_get_string(tbc[WIRELESS_CONFIG_HWMODE]);
        if (strchr(hwmode, 'a'))
            is11a = true;
        else
            is11a = false;

        htmode = blobmsg_get_string(tbc[WIRELESS_CONFIG_HTMODE]);
        if (htmode) {
            if (!strncmp(htmode, "VHT", 3))
                media_type = INTERFACE_TYPE_IEEE_802_11AC_5_GHZ;
            else if (!strncmp(htmode, "HT", 2))
                media_type = is11a?INTERFACE_TYPE_IEEE_802_11N_5_GHZ:INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ;
            else if (!strncmp(htmode, "NOHT", 4))
                media_type = is11a?INTERFACE_TYPE_IEEE_802_11A_5_GHZ:INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ;
            else
                fprintf(stderr, "unknown htmode %s\n", htmode);
        }

        /* fall-back to 802.11a/g if htmode is unset or didn't parse */
        if (media_type == INTERFACE_TYPE_UNKNOWN)
            media_type = is11a?INTERFACE_TYPE_IEEE_802_11A_5_GHZ:INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ;

        fprintf(stderr, "found netifd radio %s, hwmode %s, htmode %s\n", radio, hwmode, htmode);
        blobmsg_for_each_attr(curif, tb[WIRELESS_INTERFACES], remif) {
            blobmsg_parse(wireless_interface_policy, __WIRELESS_INTERFACE_MAX, tbi, blobmsg_data(curif), blobmsg_len(curif));
            section = blobmsg_get_string(tbi[WIRELESS_INTERFACE_SECTION]);
            if (!section)
                continue;
            blobmsg_parse(wireless_ifconfig_policy, __WIRELESS_IFCONFIG_MAX, tbic, blobmsg_data(tbi[WIRELESS_INTERFACE_CONFIG]), blobmsg_len(tbi[WIRELESS_INTERFACE_CONFIG]));
            fprintf(stderr, " found wifi-iface section %s\n", section);
            if(tbi[WIRELESS_INTERFACE_IFNAME] && tbic[WIRELESS_IFCONFIG_MULTI_AP] && blobmsg_get_u32(tbic[WIRELESS_IFCONFIG_MULTI_AP])) {
                struct interfaceWifi *interface_wifi;
                char *ifname = blobmsg_get_string(tbi[WIRELESS_INTERFACE_IFNAME]);

                blobmsg_for_each_attr(curnt, tbic[WIRELESS_IFCONFIG_NETWORK], remnt) {
                    curucinet = blobmsg_get_string(curnt);
                    if (!ucinet)
                        ucinet = curucinet;
                    else if (strcmp(ucinet, curucinet))
                        fprintf(stderr, "warning: wifi-iface assigned to different network %s != %s\n", ucinet, curucinet);
                }

                fprintf(stderr, " adding multi-ap iface %s type %d\n", ifname, media_type);

                int fd;
                struct ifreq s;
                mac_address addr;

                strcpy(s.ifr_name, ifname);
                fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
                if (0 != ioctl(fd, SIOCGIFHWADDR, &s))
                {
                    PLATFORM_PRINTF_DEBUG_ERROR("[PLATFORM] Could not obtain MAC address of interface %s\n", ifname);
                    close(fd);
                    continue;
                }
                close(fd);
                memcpy(addr, s.ifr_addr.sa_data, 6);

                char sys_path[100];

                snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/phy80211/name", ifname);

                FILE *fp = fopen(sys_path, "r");
                if(fp == NULL) {
                    PLATFORM_PRINTF_DEBUG_ERROR("No sysfs phy80211 name for wireless interface %s\n", ifname);
                    continue;
                }
                char phy[30];
                if (NULL == fgets(phy, sizeof(phy), fp))
                {
                    PLATFORM_PRINTF_DEBUG_ERROR("Failed to read phy name from %s\n", sys_path);
                    fclose(fp);
                    continue;
                }
                fclose(fp);

                /* Strip trailing newline */
                phy[strlen(phy) - 1] = '\0';
                struct radio *radio = findLocalRadio(phy);
                interface_wifi = interfaceWifiAlloc(addr, local_device);
                radioAddInterfaceWifi(radio, interface_wifi);

                interface_wifi->i.name = strdup(ifname);
                interface_wifi->i.power_state = interface_power_state_on;
                interface_wifi->i.media_type = media_type;

                const char* role = blobmsg_get_string(tbic[WIRELESS_IFCONFIG_MODE]);
                if (!strcmp(role, "ap")) {
                    interface_wifi->role = interface_wifi_role_ap;
                    switch (blobmsg_get_u32(tbic[WIRELESS_IFCONFIG_MULTI_AP])) {
                    case 1:
                        interface_wifi->bssInfo.backhaul = true;
                        interface_wifi->bssInfo.backhaul_only = true;
                        break;
                    case 2:
                        interface_wifi->bssInfo.backhaul = false;
                        break;
                    case 3:
                        interface_wifi->bssInfo.backhaul = true;
                        interface_wifi->bssInfo.backhaul_only = false;
                        break;
                    default:
                        PLATFORM_PRINTF_DEBUG_ERROR("Unkown multiap mode %d for interface %s\n", \
                                                    blobmsg_get_u32(tbic[WIRELESS_IFCONFIG_MULTI_AP]), ifname);
                        break;
                    }
                    memcpy(interface_wifi->bssInfo.bssid, addr, 6);
                } else if (!strcmp(role, "sta"))
                    interface_wifi->role = interface_wifi_role_sta;
                else
                    interface_wifi->role = interface_wifi_role_other;

                copyLengthString(interface_wifi->bssInfo.ssid.ssid, &interface_wifi->bssInfo.ssid.length,
                                 blobmsg_get_string(tbic[WIRELESS_IFCONFIG_SSID]),
                                 sizeof(interface_wifi->bssInfo.ssid.ssid));
                if (tbic[WIRELESS_IFCONFIG_KEY]) {
                    copyLengthString(interface_wifi->bssInfo.key, &interface_wifi->bssInfo.key_len,
                                     blobmsg_get_string(tbic[WIRELESS_IFCONFIG_KEY]),
                                     sizeof(interface_wifi->bssInfo.key));
                    interface_wifi->bssInfo.auth_mode = auth_mode_wpa2psk;
                } else {
                    interface_wifi->bssInfo.auth_mode = auth_mode_open;
                }

                addInterface(ifname);
            }
        }
    }
    if (!ucinet) {
        fprintf(stderr, " error: no network detected\n");
        return;
    }
    *network = strdup(ucinet);
}

/*
 * policy for network.interface.%s status
 */
enum {
    IFSTATUS_DEVICE,
    __IFSTATUS_MAX,
};

static const struct blobmsg_policy ifstatus_policy[__IFSTATUS_MAX] = {
    [IFSTATUS_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
};

/*
 * called by network.interface.%s status call with (among other things) device name
 */
static void network_status_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char **devname = (char **)req->priv;
    struct blob_attr *tb[__IFSTATUS_MAX];

    blobmsg_parse(ifstatus_policy, __IFSTATUS_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[IFSTATUS_DEVICE])
        return;

    *devname = strdup(blobmsg_get_string(tb[IFSTATUS_DEVICE]));
}

/*
 * policy for network.device status
 */
enum {
    DEVSTATUS_TYPE,
    DEVSTATUS_BRIDGE_MEMBERS,
    DEVSTATUS_MACADDR,
    __DEVSTATUS_MAX,
};

static const struct blobmsg_policy devstatus_policy[__DEVSTATUS_MAX] = {
    [DEVSTATUS_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [DEVSTATUS_BRIDGE_MEMBERS] = { .name = "bridge-members", .type = BLOBMSG_TYPE_ARRAY },
    [DEVSTATUS_MACADDR] = { .name = "macaddr", .type = BLOBMSG_TYPE_STRING },
};

/*
 * called by network.device status call with macaddr and bridge-members
 */
static void network_device_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char **macaddr = (char **)req->priv;
    struct blob_attr *tb[__DEVSTATUS_MAX];

    blobmsg_parse(devstatus_policy, __DEVSTATUS_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[DEVSTATUS_TYPE] || strcmp("bridge", blobmsg_get_string(tb[DEVSTATUS_TYPE]))) {
        fprintf(stderr, " error: not a bridge\n");
        return;
    }
    /* ToDo: add wired interfaces from bridge as well */
    *macaddr = strdup(blobmsg_get_string(tb[DEVSTATUS_MACADDR]));
}

static const char netobj_prefix[] = "network.interface.";

static void uci_get_interfaces()
{
    struct ubus_context *ctx = ubus_connect(NULL);
    uint32_t id;
    static struct blob_buf req;
    char *netobj = NULL, *network = NULL, *devname = NULL, *macstr = NULL;

    if (!ctx)
        return;

    if (!ubus_lookup_id(ctx, "network.wireless", &id))
        ubus_invoke(ctx, id, "status", NULL, wireless_status_cb, &network, 3000);

    if (!network)
        goto uci_get_interfaces_out;

    netobj = alloca(strlen(network) + strlen(netobj_prefix) + 1);
    sprintf(netobj, "%s%s", netobj_prefix, network);

    if (!ubus_lookup_id(ctx, netobj, &id))
        ubus_invoke(ctx, id, "status", NULL, network_status_cb, &devname, 3000);

    if (!devname)
        goto uci_get_interfaces_out;

    blob_buf_init(&req, 0);
    blobmsg_add_string(&req, "name", devname);

    if (!ubus_lookup_id(ctx, "network.device", &id))
        ubus_invoke(ctx, id, "status", req.head, network_device_cb, &macstr, 3000);

    fprintf(stderr, "got macaddr %s\n", macstr);

uci_get_interfaces_out:
    ubus_free(ctx);
}

/*
 * policy for uci get prplmesh
 */
enum {
    PRPLMESH_TYPE,
    PRPLMESH_PORT,
    PRPLMESH_AL_ADDR,
    PRPLMESH_WHOLE_NETWORK,
    PRPLMESH_SSID,
    PRPLMESH_KEY,
    PRPLMESH_BACKHAUL,
    PRPLMESH_BACKHAUL_ONLY,
    PRPLMESH_BAND,
    PRPLMESH_MAX,
};

static const struct blobmsg_policy prplmesh_policy[PRPLMESH_MAX] = {
    [PRPLMESH_TYPE] = { .name = ".type", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_AL_ADDR] = { .name = "al_address", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_WHOLE_NETWORK] = { .name = "whole_network", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_KEY] = { .name = "key", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_BACKHAUL] = { .name = "backhaul", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_BACKHAUL_ONLY] = { .name = "backhaul_only", .type = BLOBMSG_TYPE_STRING },
    [PRPLMESH_BAND] = { .name = "band", .type = BLOBMSG_TYPE_STRING },
};

static void prplmesh_config_parse(struct blob_attr *tb[PRPLMESH_MAX])
{
    char *al_mac_str;
    mac_address al_mac_address;
    bool map_whole_network = false;
    uint16_t alme_port_number = DEFAULT_ALME_SERVER_PORT;

    if (!tb[PRPLMESH_AL_ADDR]) {
        PLATFORM_PRINTF_DEBUG_ERROR("error: AL MAC address not set in UCI.\n");
        exit(1);
    }
    al_mac_str = blobmsg_get_string(tb[PRPLMESH_AL_ADDR]);
    asciiToMac(al_mac_str, al_mac_address);
    DMalMacSet(al_mac_address);

    if (tb[PRPLMESH_WHOLE_NETWORK]) {
        const char *value = blobmsg_get_string(tb[PRPLMESH_WHOLE_NETWORK]);
        if (!strcmp(value, "true") ||
                !strcmp(value, "yes") ||
                !strcmp(value, "on") ||
                !strcmp(value, "enabled") ||
                !strcmp(value, "1")) {
            map_whole_network = true;
        }
    }
    DMmapWholeNetworkSet(map_whole_network);

    if (tb[PRPLMESH_PORT]) {
        const char *value = blobmsg_get_string(tb[PRPLMESH_PORT]);
        char *endptr;
        unsigned long port;
        port = strtoul(value, &endptr, 0);
        if (endptr == NULL || *endptr != '\0') {
            PLATFORM_PRINTF_DEBUG_WARNING("prplmesh.prplmesh.port value `%s' is not a number\n", value);
        } else if (port == 0 || port > 0xffff) {
            PLATFORM_PRINTF_DEBUG_WARNING("Invalid port %lu\n", port);
        } else {
            alme_port_number = (uint16_t)port;
        }
    }
    almeServerPortSet(alme_port_number);

    PLATFORM_PRINTF_DEBUG_INFO("Starting AL entity (AL MAC = "MACSTR"). Port = %u. Map whole network = %d...\n",
                               MAC2STR(al_mac_address), alme_port_number, map_whole_network);
}

static void registrar_config_parse(struct blob_attr *tb[PRPLMESH_MAX])
{
    char *value;
    struct wscRegistrarInfo *wsc_info;

    if (!tb[PRPLMESH_SSID]) {
        PLATFORM_PRINTF_DEBUG_WARNING("Registrar section without SSID\n");
        return;
    }

    registrar.d = local_device;
    /* For now, it is always a MAP Controller. */
    registrar.is_map = true;

    wsc_info = zmemalloc(sizeof(struct wscRegistrarInfo));
    copyLengthString(wsc_info->bss_info.ssid.ssid, &wsc_info->bss_info.ssid.length,
                     blobmsg_get_string(tb[PRPLMESH_SSID]), sizeof(wsc_info->bss_info.ssid.ssid));
    if (tb[PRPLMESH_KEY]) {
        wsc_info->bss_info.auth_mode = auth_mode_wpa2psk;
        copyLengthString(wsc_info->bss_info.key, &wsc_info->bss_info.key_len,
                         blobmsg_get_string(tb[PRPLMESH_KEY]), sizeof(wsc_info->bss_info.key));
    } else {
        wsc_info->bss_info.auth_mode = auth_mode_open;
    }
    if (tb[PRPLMESH_BACKHAUL]) {
        wsc_info->bss_info.backhaul = atoi(blobmsg_get_string(tb[PRPLMESH_BACKHAUL]));
    }
    if (tb[PRPLMESH_BACKHAUL_ONLY]) {
        wsc_info->bss_info.backhaul = atoi(blobmsg_get_string(tb[PRPLMESH_BACKHAUL_ONLY]));
    }
    if (tb[PRPLMESH_BAND]) {
        wsc_info->rf_bands = atoi(blobmsg_get_string(tb[PRPLMESH_BAND]));
    } else {
        wsc_info->rf_bands = WPS_RF_24GHZ | WPS_RF_50GHZ;
    }
    strncpy(wsc_info->device_data.device_name, "prplMesh", sizeof(wsc_info->device_data.device_name) - 1);
    strncpy(wsc_info->device_data.manufacturer_name, "prpl", sizeof(wsc_info->device_data.manufacturer_name) - 1);
    strncpy(wsc_info->device_data.model_name, "prplMesh", sizeof(wsc_info->device_data.model_name) - 1);
    strncpy(wsc_info->device_data.model_number, "0.1", sizeof(wsc_info->device_data.model_number) - 1);
    strncpy(wsc_info->device_data.serial_number, "00000", sizeof(wsc_info->device_data.serial_number) - 1);
    registrarAddWsc(wsc_info);
}

/*
 * policy for UCI get
 */
enum {
    UCI_GET_VALUES,
    UCI_GET_MAX,
};

static const struct blobmsg_policy uciget_policy[UCI_GET_MAX] = {
    [UCI_GET_VALUES] = { .name = "values", .type = BLOBMSG_TYPE_TABLE },
};

/*
 * called by uci get prplmesh call
 */
static void prplmesh_config_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *tbv[UCI_GET_MAX];
    struct blob_attr *tb[PRPLMESH_MAX];
    struct blob_attr *cur;
    int rem;

    blobmsg_parse(uciget_policy, UCI_GET_MAX, tbv, blob_data(msg), blob_len(msg));
    if (!tbv[UCI_GET_VALUES] || blobmsg_type(tbv[UCI_GET_VALUES]) != BLOBMSG_TYPE_TABLE) {
        PLATFORM_PRINTF_DEBUG_ERROR("error: no UCI values for prplmesh\n");
        exit(1);
    }

    blobmsg_for_each_attr(cur, tbv[UCI_GET_VALUES], rem) {
        blobmsg_parse(prplmesh_policy, PRPLMESH_MAX, tb, blobmsg_data(cur), blobmsg_data_len(cur));
        if (!tb[PRPLMESH_TYPE]) {
            PLATFORM_PRINTF_DEBUG_WARNING("UCI section %s without type\n", blobmsg_name(cur));
            continue;
        }
        if (!strcmp(blobmsg_get_string(tb[PRPLMESH_TYPE]), "prplmesh")) {
            prplmesh_config_parse(tb);
        } else if (!strcmp(blobmsg_get_string(tb[PRPLMESH_TYPE]), "registrar")) {
                registrar_config_parse(tb);
        } else {
            PLATFORM_PRINTF_DEBUG_WARNING("Unexpected UCI section %s type %s\n", blobmsg_name(cur), blobmsg_get_string(tb[PRPLMESH_TYPE]));
        }
    }
}

static void uci_get_prplmesh_config()
{
    struct ubus_context *ctx = ubus_connect(NULL);
    uint32_t id;
    struct blob_buf req = {0,};

    if (!ctx) {
        fprintf(stderr, "failed to connect to ubus.\n");
        return;
    }

    blob_buf_init(&req, 0);
    blobmsg_add_string(&req, "config", "prplmesh");
    if (ubus_lookup_id(ctx, "uci", &id) ||
        ubus_invoke(ctx, id, "get", req.head, prplmesh_config_cb, NULL, 3000))
        goto err_out;

err_out:
    blob_buf_free(&req);
    ubus_free(ctx);
}

static void _printUsage(char *program_name)
{
    printf("AL entity (build %s)\n", _BUILD_NUMBER_);
    printf("\n");
    printf("Usage: %s [-v]\n", program_name);
    printf("\n");
    printf("  ...where:\n");
    printf("       '-v', if present, will increase the verbosity level. Can be present more than once,\n");
    printf("       making the AL entity even more verbose each time.\n");
    printf("\n");
}


////////////////////////////////////////////////////////////////////////////////
// External public functions
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{

    int   c;

    int verbosity_counter = 1; // Only ERROR and WARNING messages

    while ((c = getopt (argc, argv, "vh:")) != -1)
    {
        switch (c)
        {
            case 'v':
            {
                // Each time this flag appears, the verbosity counter is
                // incremented.
                //
                verbosity_counter++;
                break;
            }

            case 'h':
            {
                _printUsage(argv[0]);
                exit(0);
            }

        }
    }

    PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(verbosity_counter);

    // Initialize platform-specific code
    //
    if (0 == PLATFORM_INIT())
    {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to initialize platform\n");
        return AL_ERROR_OS;
    }

    // Insert the provided AL MAC address into the database
    //
    DMinit();

    uci_get_prplmesh_config();

    // Collect all the informations about local radios throught netlink
    //
    PLATFORM_PRINTF_DEBUG_DETAIL("Retrieving list of local radios throught netlink...\n");
    if (0 > netlink_collect_local_infos())
    {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to collect radios from netlink\n");
        return AL_ERROR_OS;
    }

    uci_get_interfaces();

    /* @todo HACK backhaul SSIDs will create a new VIF for each connected bSTA. Include these in the list of interfaces
     * so we can send/receive packets on them. Note that this will screw up the topology somewhat. To reduce the fallout,
     * we make it "other" type interfaces. */
    struct radio *radio;
    dlist_for_each(radio, local_device->radios, l) {
        unsigned i;
        for (i = 0; i < radio->configured_bsses.length; i++) {
            struct interfaceWifi *interface_wifi = radio->configured_bsses.data[i];
            if (interface_wifi->bssInfo.backhaul) {
                unsigned sta;
                for (sta = 0; sta < 10; sta++) {
                    size_t namelen = strlen(interface_wifi->i.name) + strlen(".staX") + 1;
                    char *name = memalloc(namelen);
                    struct interface *interface;

                    snprintf(name, namelen, "%s.sta%.1u", interface_wifi->i.name, sta);
                    interface = interfaceAlloc(interface_wifi->bssInfo.bssid, local_device);
                    interface->media_type = MEDIA_TYPE_UNKNOWN;
                    interface->name = name;
                    interface->power_state = interface_power_state_on;
                    interface->type = interface_type_ethernet; /* To make sure it is secured */
                    addInterface(name);
                }
            }
        }
    }

    uci_register_handlers();

    start1905AL();

    return 0;
}
