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

#include <datamodel.h>
#include "../platform_uci.h"
#include "../../al_datamodel.h"

#include <stdio.h>   // printf
#include <unistd.h>  // getopt
#include <stdlib.h>  // exit
#include <string.h>  // strtok

extern int  netlink_collect_local_infos(void);

////////////////////////////////////////////////////////////////////////////////
// Static (auxiliary) private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// Port number where the ALME server will be listening to by default
//
#define DEFAULT_ALME_SERVER_PORT 8888

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

static void _printUsage(char *program_name)
{
    printf("AL entity (build %s)\n", _BUILD_NUMBER_);
    printf("\n");
    printf("Usage: %s -m <al_mac_address> -i <interfaces_list> [-w] [-r <registrar_interface>] [-v] [-p <alme_port_number>]\n", program_name);
    printf("\n");
    printf("  ...where:\n");
    printf("       '<al_mac_address>' is the AL MAC address that this AL entity will receive\n");
    printf("       (ex: '00:4f:21:03:ab:0c'\n");
    printf("\n");
    printf("       '<interfaces_list>' is a comma sepparated list of local interfaces that will be\n");
    printf("        managed by the AL entity (ex: 'eth0,eth1,wlan0')\n");
    printf("\n");
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
    char *al_mac              = NULL;
    char *al_interfaces       = NULL;
    int  alme_port_number     = 0;
    char *registrar_interface = NULL;

    int verbosity_counter = 1; // Only ERROR and WARNING messages

    registerGhnSpiritInterfaceType();
    registerSimulatedInterfaceType();

    while ((c = getopt (argc, argv, "m:i:wr:vh:p:")) != -1)
    {
        switch (c)
        {
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

    if (NULL == al_mac || NULL == al_interfaces)
    {
        _printUsage(argv[0]);
        exit(1);
    }

    if (0 == alme_port_number)
    {
        alme_port_number = DEFAULT_ALME_SERVER_PORT;
    }

    PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(verbosity_counter);

    _parseInterfacesList(al_interfaces);
    asciiToMac(al_mac, &al_mac_address);

    almeServerPortSet(alme_port_number);

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
    DMalMacSet(al_mac_address);
    DMmapWholeNetworkSet(map_whole_network);
    PLATFORM_PRINTF_DEBUG_DETAIL("Starting AL entity (AL MAC = %02x:%02x:%02x:%02x:%02x:%02x). Map whole network = %d...\n",
                                al_mac_address[0],
                                al_mac_address[1],
                                al_mac_address[2],
                                al_mac_address[3],
                                al_mac_address[4],
                                al_mac_address[5],
                                map_whole_network);

    // Collect all the informations about local radios throught netlink
    //
    PLATFORM_PRINTF_DEBUG_DETAIL("Retrieving list of local radios throught netlink...\n");
    if (0 > netlink_collect_local_infos())
    {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to collect radios from netlink\n");
        return AL_ERROR_OS;
    }

#ifdef OPENWRT
    // Register UCI as the driver for local radios.
    uci_register_handlers();
#endif

    // Collect interfaces
    PLATFORM_PRINTF_DEBUG_DETAIL("Retrieving list of local interfaces...\n");
    createLocalInterfaces();

    // If an interface is the designated 1905 network registrar
    // interface, save its MAC address to the database
    //
    if (NULL != registrar_interface)
    {
        struct interface *interface;
        struct interfaceWifi *interface_wifi;

        interface = findLocalInterface(registrar_interface);

        if (interface == NULL)
        {
            PLATFORM_PRINTF_DEBUG_ERROR("Could not find registrar interface %s\n", registrar_interface);
        }
        else if (interface->type != interface_type_wifi)
        {
            PLATFORM_PRINTF_DEBUG_ERROR("Registrar interface %s is not a Wifi interface\n", registrar_interface);
        }
        else
        {
            struct interfaceInfo *x;

            interface_wifi = container_of(interface, struct interfaceWifi, i);
            x = PLATFORM_GET_1905_INTERFACE_INFO(interface->name);
            /* x cannot be NULL because the interface exists. */

            registrar.d = local_device;
            /* For now, it is always a MAP Controller. */
            registrar.is_map = true;

            /* Copy interface info into WSC info.
             * @todo this should come from a config file.
             * @todo Support multiple bands.
             */
            struct wscRegistrarInfo *wsc_info = zmemalloc(sizeof(struct wscRegistrarInfo));
            memcpy(&wsc_info->bss_info, &interface_wifi->bssInfo, sizeof(wsc_info->bss_info));
            strncpy(wsc_info->device_data.device_name, x->device_name, sizeof(wsc_info->device_data.device_name) - 1);
            strncpy(wsc_info->device_data.manufacturer_name, x->manufacturer_name, sizeof(wsc_info->device_data.manufacturer_name) - 1);
            strncpy(wsc_info->device_data.model_name, x->model_name, sizeof(wsc_info->device_data.model_name) - 1);
            strncpy(wsc_info->device_data.model_number, x->model_number, sizeof(wsc_info->device_data.model_number) - 1);
            strncpy(wsc_info->device_data.serial_number, x->serial_number, sizeof(wsc_info->device_data.serial_number) - 1);
            /* @todo support UUID; for now its 0. */
            switch(x->interface_type)
            {
                case INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ:
                case INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ:
                case INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ:
                    wsc_info->rf_bands = WPS_RF_24GHZ;
                    break;

                case INTERFACE_TYPE_IEEE_802_11A_5_GHZ:
                case INTERFACE_TYPE_IEEE_802_11N_5_GHZ:
                case INTERFACE_TYPE_IEEE_802_11AC_5_GHZ:
                    wsc_info->rf_bands = WPS_RF_50GHZ;
                    break;

                case INTERFACE_TYPE_IEEE_802_11AD_60_GHZ:
                    wsc_info->rf_bands = WPS_RF_60GHZ;
                    break;

                case INTERFACE_TYPE_IEEE_802_11AF_GHZ:
                    PLATFORM_PRINTF_DEBUG_ERROR("Interface %s is 802.11af which is not supported by WSC!\n",x->name);

                    free_1905_INTERFACE_INFO(x);
                    return AL_ERROR_INTERFACE_ERROR;

                default:
                    PLATFORM_PRINTF_DEBUG_ERROR("Interface %s is not a 802.11 interface and thus cannot act as a registrar!\n",x->name);

                    free(wsc_info);
                    free_1905_INTERFACE_INFO(x);
                    return AL_ERROR_INTERFACE_ERROR;

            }

            registrarAddWsc(wsc_info);
            free_1905_INTERFACE_INFO(x);
        }
    }

    start1905AL();

    return 0;
}
