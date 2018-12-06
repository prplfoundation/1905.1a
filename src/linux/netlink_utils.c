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

#include <stdio.h> // snprintf()
#include <stdlib.h> // atoi()

#include <sys/types.h> // open()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // close()/read()

#include "netlink_funcs.h"
#include "nl80211.h"
#include "hlist.h"

int phy_lookup(const char *basedir, char *name, mac_address *mac, int *index)
{
    char    buf[128];
    int     fd, n;

    #define _GET_FILE_CONTENT(b) do { \
        if ( (fd = open((b), O_RDONLY)) < 0 ) \
            return  0; \
        if ( (n = read(fd, (b), sizeof(b)-1)) <= 0 ) { \
            close(fd); \
            return -1; \
        } \
        close(fd); \
        (b)[n] = 0; \
        /* strip trailing newline, if present */ \
        if ((b)[n-1] == '\n') { \
            (b)[n-1] = 0; \
        } \
    } while (0)

    snprintf(buf, sizeof(buf), "%s/index", basedir);
    _GET_FILE_CONTENT(buf);
    *index = atoi(buf);

    snprintf(buf, sizeof(buf), "%s/macaddress", basedir);
    _GET_FILE_CONTENT(buf);
    asciiToMac(buf, mac);

    snprintf(buf, sizeof(buf), "%s/name", basedir);
    _GET_FILE_CONTENT(buf);
    strncpy(name, buf, T_RADIO_NAME_SZ-1);
    name[T_RADIO_NAME_SZ] = 0;

    #undef _GET_FILE_CONTENT

    return 1;
}

int ieee80211_channel_to_frequency(int chan, enum nl80211_band band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (chan <= 0)
		return 0; /* not supported */
	switch (band) {
	case NL80211_BAND_2GHZ:
		if (chan == 14)
			return 2484;
		else if (chan < 14)
			return 2407 + chan * 5;
		break;
	case NL80211_BAND_5GHZ:
		if (chan >= 182 && chan <= 196)
			return 4000 + chan * 5;
		else
			return 5000 + chan * 5;
		break;
	case NL80211_BAND_60GHZ:
		if (chan < 5)
			return 56160 + chan * 2160;
		break;
	default:
		;
	}
	return 0; /* not supported */
}

int ieee80211_frequency_to_channel(int freq)
{
	/* see 802.11-2007 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq <= 45000) /* DMG band lower limit */
		return (freq - 5000) / 5;
	else if (freq >= 58320 && freq <= 64800)
		return (freq - 56160) / 2160;
	else
		return 0;
}
