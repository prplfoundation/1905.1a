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
/*
 *  Code based on the 'iw' project from http://git.sipsolutions.net/iw.git/
 *
 *  Copyright (c) 2007, 2008        Johannes Berg
 *  Copyright (c) 2007              Andy Lutomirski
 *  Copyright (c) 2007              Mike Kershaw
 *  Copyright (c) 2008-2009         Luis R. Rodriguez
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/errno.h>
#include <netlink/utils.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "ieee80211.h"
#include "platform.h"
#include "netlink_funcs.h"

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
    PLATFORM_PRINTF_DEBUG_INFO("** error_handler() called **\n");

    *(int *)arg = err->error;

#ifdef NLM_F_ACK_TLVS
    {
        struct nlmsghdr *nlh = (struct nlmsghdr *)err - 1;
        unsigned         len = nlh->nlmsg_len;
        struct nlattr   *attrs;
        struct nlattr   *tb[NLMSGERR_ATTR_MAX + 1];
        unsigned         ack_len = sizeof(*nlh) + sizeof(int) + sizeof(*nlh);

        /* netlink: extended ACK reporting
         * Only since kernel 4.11.0-rc5
         */
        if ( ! (nlh->nlmsg_flags & NLM_F_ACK_TLVS) )
            return NL_STOP;

        if ( ! (nlh->nlmsg_flags & NLM_F_CAPPED) )
            ack_len += err->msg.nlmsg_len - sizeof(*nlh);

        if ( len <= ack_len )
            return NL_STOP;

        attrs = (void *)((unsigned char *)nlh + ack_len);
        len  -= ack_len;

        nla_parse(tb, NLMSGERR_ATTR_MAX, attrs, len, NULL);

        if ( tb[NLMSGERR_ATTR_MSG] ) {
            len = strnlen((char *)nla_data(tb[NLMSGERR_ATTR_MSG]), nla_len(tb[NLMSGERR_ATTR_MSG]));
            PLATFORM_PRINTF_DEBUG_ERROR("kernel reports: %*s\n", len, (char *)nla_data(tb[NLMSGERR_ATTR_MSG]));
        }
    }
#endif
    return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	PLATFORM_PRINTF_DEBUG_INFO("** finish_handler() called **\n");
	*(int *)arg = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	PLATFORM_PRINTF_DEBUG_INFO("** ack_handler() called **\n");
	*(int *)arg = 0;
	return NL_STOP;
}

int netlink_open(struct nl80211_state *s)
{
    int err;

    if ( ! (s->nl_sock = nl_socket_alloc()) ) {
        PLATFORM_PRINTF("ERROR! Failed to allocate netlink socket !\n");
        return -ENOMEM;
    }
    if ( genl_connect(s->nl_sock) ) {
        PLATFORM_PRINTF("ERROR! Failed to connect to generic netlink !\n");
        err = -ENOLINK;
        goto out_handle_destroy;
    }
    nl_socket_set_buffer_size(s->nl_sock, 8192, 8192);
#ifdef NETLINK_EXT_ACK
    /* try to set NETLINK_EXT_ACK to 1, ignoring errors */
    err = 1;
    setsockopt(nl_socket_get_fd(s->nl_sock), SOL_NETLINK, NETLINK_EXT_ACK, &err, sizeof(err));
#endif
    s->nl80211_id = genl_ctrl_resolve(s->nl_sock, "nl80211");
    if ( s->nl80211_id < 0 ) {
        PLATFORM_PRINTF("ERROR! nl80211 not found !\n");
        err = -ENOENT;
        goto out_handle_destroy;
    }
    return 0;

 out_handle_destroy:
    nl_socket_free(s->nl_sock);
    return err;
}

void netlink_close(struct nl80211_state *s)
{
    nl_socket_free(s->nl_sock);
}

struct nl_msg* netlink_prepare(
        const struct nl80211_state  *s,
        enum nl80211_commands        cmd,
        int                          flags)
{
    struct nl_msg *m;

    if ( ! (m = nlmsg_alloc()) )
        return NULL;

    if ( genlmsg_put(m, NL_AUTO_PORT, NL_AUTO_SEQ, s->nl80211_id, 0, flags, cmd, 0) < 0 ) {
        nlmsg_free(m);
        return NULL;
    }
    return m;
}

#define CB_DEFAULT      NL_CB_DEFAULT
//#define CB_DEFAULT      NL_CB_VERBOSE
//#define CB_DEFAULT      NL_CB_DEBUG

int netlink_do(
        struct nl80211_state *s,
        struct nl_msg        *m,
        int (*process)(struct nl_msg *, void *), void *process_datas)
{
    struct nl_cb    *cb, *s_cb;
    int              err;

    cb   = nl_cb_alloc(CB_DEFAULT);
    s_cb = nl_cb_alloc(CB_DEFAULT);

    if ( ! (cb && s_cb) ) {
        PLATFORM_PRINTF("ERROR ! Failed to allocate netlink callbacks\n");
        return -1;
    }
    nl_socket_set_cb(s->nl_sock, s_cb);

    if ( (err = nl_send_auto_complete(s->nl_sock, m)) < 0 )
        goto out;

    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);

    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler,  &err);
    nl_cb_set(cb, NL_CB_ACK,    NL_CB_CUSTOM, ack_handler,     &err);
    nl_cb_set(cb, NL_CB_VALID,  NL_CB_CUSTOM, process,         process_datas);

    while ( err > 0 )
        nl_recvmsgs(s->nl_sock, cb);

 out:
    nl_cb_put(cb);
    nl_cb_put(s_cb);
    nlmsg_free(m);
    return err;
}
