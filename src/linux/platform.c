/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
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

#include <platform.h>
#include <platform_linux.h>

#include <stdlib.h>      // free(), malloc(), ...
#include <string.h>      // memcpy(), memcmp(), ...
#include <stdio.h>       // printf(), ...
#include <stdarg.h>      // va_list
#include <sys/time.h>    // gettimeofday()
#include <errno.h>       // errno

#include <arpa/inet.h>        // htons()
#include <linux/if_packet.h>  // sockaddr_ll
#include <net/if.h>           // struct ifreq, IFNAZSIZE
#include <netinet/ether.h>    // ETH_P_ALL, ETH_A_LEN
#include <sys/socket.h>       // socket()
#include <sys/ioctl.h>        // ioctl(), SIOCGIFINDEX
#include <unistd.h>           // close()

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
#    include <pthread.h> // mutexes, pthread_self()
#endif



////////////////////////////////////////////////////////////////////////////////
// Private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// *********** libc stuff ******************************************************

// We will use this variable to save the instant when "PLATFORM_INIT()" was
// called. This way we sill be able to get relative timestamps later when
// someone calls "PLATFORM_GET_TIMESTAMP()"
//
static struct timeval tv_begin;

// The following variable is used to set which "PLATFORM_PRINTF_DEBUG_*()"
// functions should be ignored:
//
//   0 => Only print ERROR messages
//   1 => Print ERROR and WARNING messages
//   2 => Print ERROR, WARNING and INFO messages
//   3 => Print ERROR, WARNING, INFO and DETAIL messages
//
static int verbosity_level = 2;

// Mutex to avoid STDOUT "overlaping" due to different threads writing at the
// same time.
//
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


////////////////////////////////////////////////////////////////////////////////
// Platform API: libc stuff
////////////////////////////////////////////////////////////////////////////////


void PLATFORM_PRINTF(const char *format, ...)
{
    va_list arglist;

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

void PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(int level)
{
    verbosity_level = level;
}

void PLATFORM_PRINTF_DEBUG_ERROR(const char *format, ...)
{
    va_list arglist;
    uint32_t ts;

    if (verbosity_level < 0)
    {
        return;
    }

    ts = PLATFORM_GET_TIMESTAMP();

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    printf("[%03d.%03d] ", ts/1000, ts%1000);
    printf("ERROR   : ");
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

void PLATFORM_PRINTF_DEBUG_WARNING(const char *format, ...)
{
    va_list arglist;
    uint32_t ts;

    if (verbosity_level < 1)
    {
        return;
    }

    ts = PLATFORM_GET_TIMESTAMP();

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    printf("[%03d.%03d] ", ts/1000, ts%1000);
    printf("WARNING : ");
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

void PLATFORM_PRINTF_DEBUG_INFO(const char *format, ...)
{
    va_list arglist;
    uint32_t ts;

    if (verbosity_level < 2)
    {
        return;
    }

    ts = PLATFORM_GET_TIMESTAMP();

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    printf("[%03d.%03d] ", ts/1000, ts%1000);
    printf("INFO    : ");
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

void PLATFORM_PRINTF_DEBUG_DETAIL(const char *format, ...)
{
    va_list arglist;
    uint32_t ts;

    if (verbosity_level < 3)
    {
        return;
    }

    ts = PLATFORM_GET_TIMESTAMP();

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    printf("[%03d.%03d] ", ts/1000, ts%1000);
    printf("DETAIL  : ");
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

uint32_t PLATFORM_GET_TIMESTAMP(void)
{
    struct timeval tv_end;
    uint32_t diff;

    gettimeofday(&tv_end, NULL);

    diff = (tv_end.tv_usec - tv_begin.tv_usec) / 1000 + (tv_end.tv_sec - tv_begin.tv_sec) * 1000;

    return diff;
}


////////////////////////////////////////////////////////////////////////////////
// Platform API: Initialization functions
////////////////////////////////////////////////////////////////////////////////

uint8_t PLATFORM_INIT(void)
{

    // Call "_timeval_print()" for the first time so that the initialization
    // time is saved for future reference.
    //
    gettimeofday(&tv_begin, NULL);

    return 1;
}

int getIfIndex(const char *interface_name)
{
    int                 s;
    struct ifreq        ifr;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (-1 == s)
    {
        PLATFORM_PRINTF_DEBUG_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a RAW socket\n",
                                    interface_name, errno, strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1)
    {
          PLATFORM_PRINTF_DEBUG_ERROR("[PLATFORM] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s) while opening a RAW socket\n",
                                      interface_name, errno, strerror(errno));
          close(s);
          return -1;
    }
    close(s);
    return ifr.ifr_ifindex;
}

int openPacketSocket(int ifindex, uint16_t eth_type)
{
    int                 s;
    struct sockaddr_ll  socket_address;

    s = socket(AF_PACKET, SOCK_RAW, htons(eth_type));
    if (-1 == s)
    {
        return -1;
    }

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_family   = AF_PACKET;
    socket_address.sll_ifindex  = ifindex;
    socket_address.sll_protocol = htons(eth_type);

    if (-1 == bind(s, (struct sockaddr*)&socket_address, sizeof(socket_address)))
    {
        close(s);
        return -1;
    }

    return s;
}
