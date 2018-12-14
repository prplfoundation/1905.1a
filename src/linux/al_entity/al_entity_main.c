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
#include "../platform_interfaces_priv.h"            // addInterface
#include "../platform_interfaces_ghnspirit_priv.h"  // registerGhnSpiritInterfaceType
#include "../platform_interfaces_simulated_priv.h"  // registerSimulatedInterfaceType
#include "../platform_alme_server_priv.h"           // almeServerPortSet()
#include "../../al.h"                                  // start1905AL

#include <stdio.h>   // printf
#include <unistd.h>  // getopt
#include <stdlib.h>  // exit
#include <string.h>  // strtok

#ifdef OPENWRT
#include <libubox/blobmsg.h>
#include <libubus.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Static (auxiliary) private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// Port number where the ALME server will be listening to by default
//
#define DEFAULT_ALME_SERVER_PORT 8888

#ifndef OPENWRT
// This function receives a comma separated list of interface names (example:
// "eth0,eth1,wlan0") and, for each of them, calls "addInterface()" (example:
// addInterface("eth0") + addInterface("eth1") + addInterface("wlan0"))
//
static void _parseInterfacesList(const char *str)
{
    char *aux;
    char *interface_name;
    char *save_ptr;

    if (NULL == str)
    {
        return;
    }

    aux = strdup(str);

    interface_name = strtok_r(aux, ",", &save_ptr);
    if (NULL != interface_name)
    {
        addInterface(interface_name);

        while (NULL != (interface_name = strtok_r(NULL, ",", &save_ptr)))
        {
            addInterface(interface_name);
        }
    }

    free(aux);
    return;
}
#else
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
        if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE)
            continue;

        blobmsg_parse(wireless_status_policy, __WIRELESS_MAX, tb, blobmsg_data(cur), blobmsg_len(cur));
        if (!tb[WIRELESS_CONFIG])
            continue;
        blobmsg_parse(wireless_config_policy, __WIRELESS_CONFIG_MAX, tbc, blobmsg_data(tb[WIRELESS_CONFIG]), blobmsg_len(tb[WIRELESS_CONFIG]));
        fprintf(stderr, "found netifd radio %s, hwmode %s, htmode %s\n", blobmsg_name(cur), blobmsg_get_string(tbc[WIRELESS_CONFIG_HWMODE]), blobmsg_get_string(tbc[WIRELESS_CONFIG_HTMODE]));
        blobmsg_for_each_attr(curif, tb[WIRELESS_INTERFACES], remif) {
            blobmsg_parse(wireless_interface_policy, __WIRELESS_INTERFACE_MAX, tbi, blobmsg_data(curif), blobmsg_len(curif));
            blobmsg_parse(wireless_ifconfig_policy, __WIRELESS_IFCONFIG_MAX, tbic, blobmsg_data(tbi[WIRELESS_INTERFACE_CONFIG]), blobmsg_len(tbi[WIRELESS_INTERFACE_CONFIG]));
            fprintf(stderr, " found wifi-iface section %s\n", blobmsg_get_string(tbi[WIRELESS_INTERFACE_SECTION]));
            if(tbi[WIRELESS_INTERFACE_IFNAME] && tbic[WIRELESS_IFCONFIG_MULTI_AP] && blobmsg_get_u32(tbic[WIRELESS_IFCONFIG_MULTI_AP])) {
                blobmsg_for_each_attr(curnt, tbic[WIRELESS_IFCONFIG_NETWORK], remnt) {
                    curucinet = blobmsg_get_string(curnt);
                    if (!ucinet)
                        ucinet = curucinet;
                    else if (strcmp(ucinet, curucinet))
                        fprintf(stderr, "warning: wifi-iface assigned to different network %s != %s\n", ucinet, curucinet);
                }
                fprintf(stderr, " adding multi-ap iface %s\n", blobmsg_get_string(tbi[WIRELESS_INTERFACE_IFNAME]));
                addInterface(blobmsg_get_string(tbi[WIRELESS_INTERFACE_IFNAME]));
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
static void network_status_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
    char **devname = (char **)req->priv;
    struct blob_attr *tb[__IFSTATUS_MAX];

    blobmsg_parse(ifstatus_policy, __IFSTATUS_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[IFSTATUS_DEVICE])
        return;

    *devname = strdup(blobmsg_get_string(tb[IFSTATUS_DEVICE]));
};

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
static void network_device_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
    char **macaddr = (char **)req->priv;
    struct blob_attr *tb[__DEVSTATUS_MAX];

    blobmsg_parse(devstatus_policy, __DEVSTATUS_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[DEVSTATUS_TYPE] || strcmp("bridge", blobmsg_get_string(tb[DEVSTATUS_TYPE]))) {
        fprintf(stderr, " error: not a bridge\n");
        return;
    }
    /* ToDo: add wired interfaces from bridge as well */
    *macaddr = strdup(blobmsg_get_string(tb[DEVSTATUS_MACADDR]));
};

static const char netobj_prefix[] = "network.interface.";

static void uci_get_interfaces(mac_address *al_mac_address) {
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
    asciiToMac(macstr, al_mac_address);

uci_get_interfaces_out:
    ubus_free(ctx);
}
#endif

static void _printUsage(char *program_name)
{
    printf("AL entity (build %s)\n", _BUILD_NUMBER_);
    printf("\n");
#ifdef OPENWRT
    printf("Usage: %s [-w] [-r <registrar_interface>] [-v] [-p <alme_port_number>]\n", program_name);
#else
    printf("Usage: %s -m <al_mac_address> -i <interfaces_list> [-w] [-r <registrar_interface>] [-v] [-p <alme_port_number>]\n", program_name);
#endif
    printf("\n");
    printf("  ...where:\n");
#ifndef OPENWRT
    printf("       '<al_mac_address>' is the AL MAC address that this AL entity will receive\n");
    printf("       (ex: '00:4f:21:03:ab:0c'\n");
    printf("\n");
    printf("       '<interfaces_list>' is a comma sepparated list of local interfaces that will be\n");
    printf("        managed by the AL entity (ex: 'eth0,eth1,wlan0')\n");
    printf("\n");
#endif
    printf("       '-w', if present, will instruct the AL entity to map the whole network (instead of\n");
    printf("       just its local neighbors)\n");
    printf("\n");
    printf("       '-r', if present, will tell the AL entity that '<registrar_interface>' is the name\n");
    printf("       of the local interface that will act as the *unique* wifi registrar in the whole\n");
    printf("       network.\n");
    printf("\n");
    printf("       '-v', if present, will increase the verbosity level. Can be present more than once,\n");
    printf("       making the AL entity even more verbose each time.\n");
    printf("\n");
    printf("       '<alme_port_number>', is the port number where a TCP socket will be opened to receive\n");
    printf("       ALME messages. If this argument is not given, a default value of '8888' is used.\n");
    printf("\n");

    return;
}


////////////////////////////////////////////////////////////////////////////////
// External public functions
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    mac_address al_mac_address = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t map_whole_network = 0;

    int   c;
#ifndef OPENWRT
    char *al_mac              = NULL;
    char *al_interfaces       = NULL;
#endif
    int  alme_port_number     = 0;
    char *registrar_interface = NULL;

    int verbosity_counter = 1; // Only ERROR and WARNING messages

    registerGhnSpiritInterfaceType();
    registerSimulatedInterfaceType();

#ifdef OPENWRT
    while ((c = getopt (argc, argv, "wr:vh:p:")) != -1)
#else
    while ((c = getopt (argc, argv, "m:i:wr:vh:p:")) != -1)
#endif
    {
        switch (c)
        {
#ifndef OPENWRT
            case 'm':
            {
                // AL MAC address in "xx:xx:..:xx" format
                //
                al_mac = optarg;
                break;
            }

            case 'i':
            {
                // Comma sepparated list of interfaces: 'eth0,eth1,wlan0'
                //
                al_interfaces = optarg;
                break;
            }

#endif
            case 'w':
            {
                // If set to '1', the AL entity will not only query its direct
                // neighbors, but also its neighbors's neighbors and so on...
                // taking much more memory but obtaining a whole network map.
                //
                map_whole_network = 1;
                break;
            }

            case 'r':
            {
                // This is the interface that acts as Wifi registrar in the
                // network.
                // Remember that only one interface in the whole network should
                // act as a registrar.
                //
                registrar_interface = optarg;
                break;
            }

            case 'v':
            {
                // Each time this flag appears, the verbosity counter is
                // incremented.
                //
                verbosity_counter++;
                break;
            }

            case 'p':
            {
                // Alme server port number
                //
                alme_port_number = atoi(optarg);
                break;
            }

            case 'h':
            {
                _printUsage(argv[0]);
                exit(0);
            }

        }
    }

#ifndef OPENWRT
    if (NULL == al_mac || NULL == al_interfaces)
    {
        _printUsage(argv[0]);
        exit(1);
    }
#endif

    if (0 == alme_port_number)
    {
        alme_port_number = DEFAULT_ALME_SERVER_PORT;
    }

    PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(verbosity_counter);

#ifdef OPENWRT
    uci_get_interfaces(&al_mac_address);
#else
    _parseInterfacesList(al_interfaces);
    asciiToMac(al_mac, &al_mac_address);
#endif

    almeServerPortSet(alme_port_number);

    start1905AL(al_mac_address, map_whole_network, registrar_interface);

    return 0;
}
