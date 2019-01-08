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

#include <datamodel.h>
#include <platform.h>

#include <assert.h>
#include <string.h> // memcpy

#define EMPTY_MAC_ADDRESS {0, 0, 0, 0, 0, 0}

struct alDevice *local_device = NULL;

struct registrar registrar = {
    .d = NULL,
    .is_map = false,
    .wsc = {&registrar.wsc, &registrar.wsc},
};

DEFINE_DLIST_HEAD(network);

void datamodelInit(void)
{
}

/* 'alDevice' related functions
 */
struct alDevice *alDeviceAlloc(const mac_address al_mac_addr)
{
    struct alDevice *ret = zmemalloc(sizeof(struct alDevice));
    dlist_add_tail(&network, &ret->l);
    memcpy(ret->al_mac_addr, al_mac_addr, sizeof(mac_address));
    dlist_head_init(&ret->interfaces);
    dlist_head_init(&ret->radios);
    ret->is_map_agent = false;
    ret->is_map_controller = false;
    return ret;
}

void alDeviceDelete(struct alDevice *alDevice)
{
    while (!dlist_empty(&alDevice->interfaces)) {
        struct interface *interface = container_of(dlist_get_first(&alDevice->interfaces), struct interface, l);
        interfaceDelete(interface);
    }
    while (!dlist_empty(&alDevice->radios)) {
        struct radio *radio = container_of(dlist_get_first(&alDevice->radios), struct radio, l);
        radioDelete(radio);
    }
    dlist_remove(&alDevice->l);
    free(alDevice);
}

/* 'radio' related functions
 */
struct radio*   radioAlloc(struct alDevice *dev, const mac_address mac)
{
    struct radio *r = zmemalloc(sizeof(struct radio));
    memcpy(&r->uid, &mac, sizeof(mac_address));
    r->index = -1;
    dlist_add_tail(&dev->radios, &r->l);
    return r;
}

struct radio*   radioAllocLocal(const mac_address mac, const char *name, int index)
{
    struct radio *r = radioAlloc(local_device, mac);
    strcpy(r->name, name);
    r->index = index;
    r->configured = local_device->configured;
    return r;
}

void    radioDelete(struct radio *radio)
{
    unsigned i;
    dlist_remove(&radio->l);
    for (i = 0; i < radio->configured_bsses.length; i++)
    {
        /* The interfaceWifi is deleted automatically when we delete the interface itself. */
        interfaceDelete(&radio->configured_bsses.data[i]->i);
    }
    PTRARRAY_CLEAR(radio->configured_bsses);
    for ( i=0 ; i < radio->bands.length ; i++ ) {
        PTRARRAY_CLEAR(radio->bands.data[i]->channels);
        free(radio->bands.data[i]);
    }
    PTRARRAY_CLEAR(radio->bands);
    free(radio);
}

struct radio *findDeviceRadio(const struct alDevice *device, const mac_address uid)
{
    struct radio *radio;
    dlist_for_each(radio, device->radios, l)
    {
        if (memcmp(radio->uid, uid, 6) == 0)
        {
            return radio;
        }
    }
    return NULL;
}

struct radio *findLocalRadio(const char *name)
{
    struct radio *radio;
    dlist_for_each(radio, local_device->radios, l)
    {
        if (strncmp(radio->name, name, sizeof(radio->name)) == 0)
        {
            return radio;
        }
    }
    return NULL;
}

int radioAddInterfaceWifi(struct radio *radio, struct interfaceWifi *ifw)
{
    PTRARRAY_ADD(radio->configured_bsses, ifw);
    ifw->radio = radio;
    return 0;
}

void radioAddAp(struct radio *radio, struct bssInfo bss_info)
{
    if (radio->addAP == NULL)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("No addAP callback for radio " MACSTR " to be configured with ssid %.*s\n",
                                      MAC2STR(radio->uid), bss_info.ssid.length, bss_info.ssid.ssid);
        return;
    }
    radio->addAP(radio, bss_info);
}

void radioAddSta(struct radio *radio, struct bssInfo bss_info)
{
    if (radio->addSTA == NULL)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("No addSTA callback for radio " MACSTR " to be configured with ssid %.*s\n",
                                      MAC2STR(radio->uid), bss_info.ssid.length, bss_info.ssid.ssid);
        return;
    }
    radio->addSTA(radio, bss_info);
}

void interfaceTearDown(struct interface *iface)
{
    if (iface->tearDown == NULL)
    {
        PLATFORM_PRINTF_DEBUG_WARNING("No tearDown callback for interface %s\n", iface->name);
        /* Just destroy the interface instead. */
        if (iface->type == interface_type_wifi)
        {
            interfaceWifiRemove(container_of(iface, struct interfaceWifi, i));
        }
        else
        {
            interfaceDelete(iface);
        }
        return;
    }
    iface->tearDown(iface);
}


/* 'interface' related functions
 */
static struct interface * interfaceInit(struct interface *i, const mac_address addr, struct alDevice *owner)
{
    i->type = interface_type_unknown;
    memcpy(i->addr, addr, 6);
    if (owner != NULL) {
        alDeviceAddInterface(owner, i);
    }
    return i;
}

struct interface *interfaceAlloc(const mac_address addr, struct alDevice *owner)
{
    return interfaceInit(zmemalloc(sizeof(struct interface)), addr, owner);
}

void interfaceDelete(struct interface *interface)
{
    unsigned i;
    for (i = 0; i < interface->neighbors.length; i++)
    {
        interfaceRemoveNeighbor(interface, interface->neighbors.data[i]);
    }
    /* Even if the interface doesn't have an owner, removing it from the empty list doesn't hurt. */
    dlist_remove(&interface->l);
    free(interface);
}

void interfaceAddNeighbor(struct interface *interface, struct interface *neighbor)
{
    PTRARRAY_ADD(interface->neighbors, neighbor);
    PTRARRAY_ADD(neighbor->neighbors, interface);
}

void interfaceRemoveNeighbor(struct interface *interface, struct interface *neighbor)
{
    PTRARRAY_REMOVE_ELEMENT(interface->neighbors, neighbor);
    PTRARRAY_REMOVE_ELEMENT(neighbor->neighbors, interface);
    if (neighbor->owner == NULL && neighbor->neighbors.length == 0)
    {
        /* No more references to the neighbor interface. */
        free(neighbor);
    }
}

/* 'interfaceWifi' related functions
 */
struct interfaceWifi *interfaceWifiAlloc(const mac_address addr, struct alDevice *owner)
{
    struct interfaceWifi *ifw = zmemalloc(sizeof(*ifw));
    interfaceInit(&ifw->i, addr, owner);
    ifw->i.type = interface_type_wifi;
    return ifw;
}

void    interfaceWifiRemove(struct interfaceWifi *ifw)
{
    PTRARRAY_REMOVE_ELEMENT(ifw->radio->configured_bsses, ifw);
    /* Clients don't need to be deleted; they are also in the interface neighbour list, so they will be deleted or unlinked
     * together with the interface. */
    interfaceDelete(&ifw->i); /* This also frees interfaceWifi itself. */
}

/* 'alDevice' related functions
 */
void alDeviceAddInterface(struct alDevice *device, struct interface *interface)
{
    assert(interface->owner == NULL);
    dlist_add_tail(&device->interfaces, &interface->l);
    interface->owner = device;
}

struct alDevice *alDeviceFind(const mac_address al_mac_addr)
{
    struct alDevice *ret;
    dlist_for_each(ret, network, l)
    {
        if (memcmp(ret->al_mac_addr, al_mac_addr, 6) == 0)
            return ret;
    }
    return NULL;
}

struct alDevice *alDeviceFindFromAnyAddress(const mac_address sender_addr)
{
    struct alDevice *sender_device = alDeviceFind(sender_addr);
    if (sender_device == NULL)
    {
        struct interface *sender_interface = findDeviceInterface(sender_addr);
        if (sender_interface != NULL)
        {
            sender_device = sender_interface->owner;
        }
    }
    return sender_device;
}

struct interface *alDeviceFindInterface(const struct alDevice *device, const mac_address addr)
{
    struct interface *ret;
    dlist_for_each(ret, device->interfaces, l)
    {
        if (memcmp(ret->addr, addr, 6) == 0)
        {
            return ret;
        }
    }
    return NULL;
}

static bool localDeviceBackhaulSsidChanged(const struct ssid ssid, const uint8_t *key, size_t key_length)
{
    if (ssid.length != local_device->backhaul_ssid.length) {
        return true;
    }
    if (memcmp(ssid.ssid, local_device->backhaul_ssid.ssid, ssid.length) != 0) {
        return true;
    }
    if (key_length != local_device->backhaul_key_length) {
        return true;
    }
    if (memcmp(key, local_device->backhaul_key, key_length) != 0) {
        return true;
    }
    return false;
}

void localDeviceUpdateBackhaulSsid(const struct ssid ssid, const uint8_t *key, size_t key_length)
{
    if (localDeviceBackhaulSsidChanged(ssid, key, key_length))
    {
        struct radio *radio;
        dlist_for_each(radio, local_device->radios, l)
        {
            if (radio->setBackhaulSsid != NULL) {
                radio->setBackhaulSsid(radio, ssid, key, key_length);
            }
        }

        memcpy(&local_device->backhaul_ssid, &ssid, sizeof(ssid));
        memcpy(local_device->backhaul_key, key, key_length);
        local_device->backhaul_key_length = key_length;
    }
}

void localDeviceSetConfigured(bool configured)
{
    local_device->configured = true;
    if (local_device->setConfigured)
    {
        local_device->setConfigured(true);
    } else {
        PLATFORM_PRINTF_DEBUG_WARNING("Configured state can't be peristed.\n");
    }
    /* When unconfiguring, also mark all radios as unconfigured. */
    if (!configured)
    {
        struct radio *radio;
        dlist_for_each(radio, local_device->radios, l)
        {
            radio->configured = false;
        }
    }
}

struct interface *findDeviceInterface(const mac_address addr)
{
    struct alDevice *alDevice;
    struct interface *ret = NULL;

    dlist_for_each(alDevice, network, l)
    {
        ret = alDeviceFindInterface(alDevice, addr);
        if (ret != NULL)
        {
            return ret;
        }
    }
    return NULL;
}

struct interface *findLocalInterface(const char *name)
{
    struct interface *ret;
    if (local_device == NULL)
    {
        return NULL;
    }
    dlist_for_each(ret, local_device->interfaces, l)
    {
        if (ret->name && strcmp(ret->name, name) == 0)
        {
            return ret;
        }
    }
    return NULL;
}

void registrarAddWsc(struct wscRegistrarInfo *wsc)
{
    dlist_add_head(&registrar.wsc, &wsc->l);
}
