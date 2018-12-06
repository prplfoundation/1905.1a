#ifndef _NETLINK_FUNCS_H
#define _NETLINK_FUNCS_H
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

#include <stdbool.h>
#include <netlink/netlink.h> // struct nl_sock
#include <netlink/msg.h> // nl_msg

#include "datamodel.h" // struct alDevice / mac_address
#include "nl80211.h" // nl80211_commands

#define BIT(x) (1ULL<<(x))

/** @file
 *
 *  This file defines all the prototypes needed by the netlink functions
 */

struct nl80211_state {
    struct nl_sock *nl_sock;    /**< Netlink socket */
    int             nl80211_id; /**< Generic netlink family identifer */
};

/** @brief  Collect some infos from sysfs (mac, index, ...)
 *
 *  @param  basedir Directory to look for phy's attributes (ex. /sys/class/net/wlan0/phy80211)
 *  @param  name    Output name of the radio interface (phy{0..9})
 *  @param  mac     Output mac address (uid) of the radio
 *  @param  index   Output index of the PHY device
 *
 *  @return >0:success, 0:not found, <0:error
 */
extern int  phy_lookup(const char *basedir, char *name, mac_address *mac, int *index);

/** @brief  Add all the local radios found with their collected datas into global ::local_device
 *  @return >=0:Number of radios processed/found, <0:error
 */
extern int  netlink_collect_local_infos(void);

/** @brief  Open the netlink socket and prepare for commands
 *
 *  @param  out_nlstate Output structure
 *
 *  @return 0=success, <0=error
 */
extern int  netlink_open(struct nl80211_state *out_nlstate);

/** @brief  Prepare a new Netlink message to be sent
 *
 *  @param  nlsock  Netlink socket to use
 *  @param  cmd     Netlink Command to initiate (See nl80211.h)
 *  @param  flags   Optional command flags
 *
 *  @return 0=success, <0=error
 */
extern struct nl_msg*   netlink_prepare(const struct nl80211_state *nlsock,
                            enum nl80211_commands cmd, int flags);

/** @brief  Execute a netlink command
 *
 *  The @a cb callback is called when a valid data is received. Otherwise,
 *  internal handlers assume the error handling.
 *
 *  @param  nlstate Netlink state & socket infos
 *  @param  nlmsg   Netlink message to process
 *  @param  cb      Callback to process the valid datas returned by the nlmsg
 *  @param  cbdatas Callback's datas passed as second parameter to @a cb
 *
 *  @return 0:success, <0:error
 */
extern int  netlink_do(struct nl80211_state *nlstate, struct nl_msg *nlmsg,
                int (*cb)(struct nl_msg *, void *), void *cbdatas);

/** @brief  Close the netlink socket and free allocations
 *
 *  @param  nlstate Netlink socket state structure
 */
extern void netlink_close(struct nl80211_state *nlstate);

/** @brief  Get the frequency of the corresponding channel
 *
 *  @param  chan    Channel ID
 *  @param  band    Band this channel is from
 *
 *  @return Frequency (x100)
 */
extern int  ieee80211_channel_to_frequency(int chan, enum nl80211_band band);

/** @brief  Get the channel corresponding to this frequency
 *
 *  @param  freq    Frequency
 *
 *  @return Channel id
 */
extern int  ieee80211_frequency_to_channel(int freq);

#endif
