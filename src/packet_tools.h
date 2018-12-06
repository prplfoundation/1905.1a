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

#ifndef _PACKET_TOOLS_H_
#define _PACKET_TOOLS_H_

#include <tlv.h> // mac_address

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <stdint.h>
#include <string.h> // memcpy()

// Auxiliary functions to:
//
//   A) Extract 1, 2 or 4 bytes from a stream received from the network.
//
//   B) Insert  1, 2 or 4 bytes into a stream which is going to be sent into
//      the network.
//
// These functions do three things:
//
//   1. Avoid unaligned memory accesses (which might cause slowdowns or even
//      exceptions on some architectures)
//
//   2. Convert from network order to host order (and the other way)
//
//   3. Advance the packet pointer as many bytes as those which have just
//      been extracted/inserted.

// Extract/insert 1 byte
//
static inline void _E1B(const uint8_t **packet_ppointer, uint8_t *memory_pointer)
{
    *memory_pointer     = **packet_ppointer;
    (*packet_ppointer) += 1;
}

static inline void _I1B(const uint8_t *memory_pointer, uint8_t **packet_ppointer)
{
    **packet_ppointer   = *memory_pointer;
    (*packet_ppointer) += 1;
}

static inline bool _E1BL(const uint8_t **packet_ppointer, uint8_t *memory_pointer, size_t *length)
{
    if (*length < 1)
    {
        return false;
    }
    else
    {
        _E1B(packet_ppointer, memory_pointer);
        (*length) -= 1;
        return true;
    }
}

static inline bool _I1BL(const uint8_t *memory_pointer, uint8_t **packet_ppointer, size_t *length)
{
    if (*length < 1)
    {
        return false;
    }
    else
    {
        _I1B(memory_pointer, packet_ppointer);
        (*length) -= 1;
        return true;
    }
}


// Extract/insert 2 bytes
//
static inline void _E2B(const uint8_t **packet_ppointer, uint16_t *memory_pointer)
{
#if _HOST_IS_BIG_ENDIAN_ == 1
    *(((uint8_t *)memory_pointer)+0)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+1)  = **packet_ppointer; (*packet_ppointer)++;
#elif _HOST_IS_LITTLE_ENDIAN_ == 1
    *(((uint8_t *)memory_pointer)+1)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+0)  = **packet_ppointer; (*packet_ppointer)++;
#else
#error You must specify your architecture endianess
#endif
}

static inline void _I2B(const uint16_t *memory_pointer, uint8_t **packet_ppointer)
{
#if _HOST_IS_BIG_ENDIAN_ == 1
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+0); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+1); (*packet_ppointer)++;
#elif _HOST_IS_LITTLE_ENDIAN_ == 1
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+1); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+0); (*packet_ppointer)++;
#else
#error You must specify your architecture endianess
#endif
}

static inline bool _E2BL(const uint8_t **packet_ppointer, uint16_t *memory_pointer, size_t *length)
{
    if (*length < 2)
    {
        return false;
    }
    else
    {
        _E2B(packet_ppointer, memory_pointer);
        (*length) -= 2;
        return true;
    }
}

static inline bool _I2BL(const uint16_t *memory_pointer, uint8_t **packet_ppointer, size_t *length)
{
    if (*length < 2)
    {
        return false;
    }
    else
    {
        _I2B(memory_pointer, packet_ppointer);
        (*length) -= 2;
        return true;
    }
}


// Extract/insert 4 bytes
//
static inline void _E4B(const uint8_t **packet_ppointer, uint32_t *memory_pointer)
{
#if _HOST_IS_BIG_ENDIAN_ == 1
    *(((uint8_t *)memory_pointer)+0)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+1)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+2)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+3)  = **packet_ppointer; (*packet_ppointer)++;
#elif _HOST_IS_LITTLE_ENDIAN_ == 1
    *(((uint8_t *)memory_pointer)+3)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+2)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+1)  = **packet_ppointer; (*packet_ppointer)++;
    *(((uint8_t *)memory_pointer)+0)  = **packet_ppointer; (*packet_ppointer)++;
#else
#error You must specify your architecture endianess
#endif
}

static inline void _I4B(const uint32_t *memory_pointer, uint8_t **packet_ppointer)
{
#if _HOST_IS_BIG_ENDIAN_ == 1
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+0); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+1); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+2); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+3); (*packet_ppointer)++;
#elif _HOST_IS_LITTLE_ENDIAN_ == 1
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+3); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+2); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+1); (*packet_ppointer)++;
    **packet_ppointer = *(((const uint8_t *)memory_pointer)+0); (*packet_ppointer)++;
#else
#error You must specify your architecture endianess
#endif
}

static inline bool _E4BL(const uint8_t **packet_ppointer, uint32_t *memory_pointer, size_t *length)
{
    if (*length < 4)
    {
        return false;
    }
    else
    {
        _E4B(packet_ppointer, memory_pointer);
        (*length) -= 4;
        return true;
    }
}

static inline bool _I4BL(const uint32_t *memory_pointer, uint8_t **packet_ppointer, size_t *length)
{
    if (*length < 4)
    {
        return false;
    }
    else
    {
        _I4B(memory_pointer, packet_ppointer);
        (*length) -= 4;
        return true;
    }
}



// Extract/insert N bytes (ignore endianess)
//
static inline void _EnB(const uint8_t **packet_ppointer, void *memory_pointer, uint32_t n)
{
    memcpy(memory_pointer, *packet_ppointer, n);
    (*packet_ppointer) += n;
}

static inline void _InB(const void *memory_pointer, uint8_t **packet_ppointer, uint32_t n)
{
    memcpy(*packet_ppointer, memory_pointer, n);
    (*packet_ppointer) += n;
}

static inline bool _EnBL(const uint8_t **packet_ppointer, void *memory_pointer, size_t n, size_t *length)
{
    if (*length < n)
    {
        return false;
    }
    else
    {
        _EnB(packet_ppointer, memory_pointer, n);
        (*length) -= n;
        return true;
    }
}

static inline bool _InBL(const void *memory_pointer, uint8_t **packet_ppointer, size_t n, size_t *length)
{
    if (*length < n)
    {
        return false;
    }
    else
    {
        _InB(memory_pointer, packet_ppointer, n);
        (*length) -= n;
        return true;
    }
}

// Specific instances of _EnBL/_InBL for mac_addresses.
static inline bool _EmBL(const uint8_t **packet_ppointer, mac_address addr, size_t *length)
{
    return _EnBL(packet_ppointer, addr, 6, length);
}

static inline bool _ImBL(const mac_address addr, uint8_t **packet_ppointer, size_t *length)
{
    return _InBL(addr, packet_ppointer, 6, length);
}

#endif
