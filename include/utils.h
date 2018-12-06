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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdlib.h> // malloc(), NULL
#include <string.h> // memset()
#include <stdio.h> // fprintf

/** @brief Get the number of elements in an array.
 *
 * Note that this simple macro may evaluate its argument 0, 1 or 2 times, and that it doesn't check at all if the
 * parameter is indeed an array. Calling it with a pointer parameter will lead to incorrect results.
 */
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))

/** @brief Compile-time check that two objects are of compatible type.
 *
 * The first object is returned. Each argument is evaluated only once.
 *
 * Inspired on https://en.wikipedia.org/wiki/Offsetof#Usage
 */
#define check_compatible_types(object, other_object) \
    ((long)"ERROR types are incompatible" ? (object) : (other_object))

/** @brief Get the parent structure from a pointer.
 *
 * @param ptr Pointer to the sub-structure (member).
 * @param type Type of the super-structure (container).
 * @param member Name of the struct member of @a type that corresponds to @a ptr.
 *
 * Inspired on https://en.wikipedia.org/wiki/Offsetof#Usage
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)check_compatible_types(ptr, &((type*)ptr)->member) - offsetof(type, member)))


/** @ brief Allocate a chunk of 'n' bytes and return a pointer to it.
 *
 * If no memory can be allocated, this function exits immediately.
 */
static inline void *memalloc(size_t size)
{
    void *p;

    p = malloc(size);

    if (NULL == p)
    {
        fprintf(stderr, "ERROR: Out of memory!\n");
        exit(1);
    }

    return p;
}

static inline void *zmemalloc(size_t size) {
    return memset(memalloc(size), 0, size);
}

/** @brief Redimension a memory area previously obtained with memalloc().
 *
 * If no memory can be allocated, this function exits immediately.
 */
static inline void *memrealloc(void *ptr, size_t size)
{
    void *p;

    p = realloc(ptr, size);

    if (NULL == p)
    {
        fprintf(stderr, "ERROR: Out of memory!\n");
        exit(1);
    }

    return p;
}

/** @brief Copy a 0-terminated string to a max-sized string.
 *
 * Some strings are represented by a length and value field in the internal model, but are initialized from 0-terminated
 * strings (e.g. coming from a config file). This function copies in such a string. Note that the destination will NOT
 * be 0-terminated.
 *
 * @param dest Pointer to the value field in which to copy.
 * @param length Pointer to the (1-byte) length field.
 * @param src Source string to copy.
 * @param size Allocated size of @a dest, typically sizeof(dest). Must be <=255.
 */
void copyLengthString(uint8_t *dest, uint8_t *length, const char *src, size_t size);


typedef void (*visitor_callback) (void (*write_function)(const char *fmt, ...), const char *prefix, uint8_t size, const char *name, const char *fmt, const void *p);

// This is an auxiliary function which is used when calling the "visit_*()"
// family of functions so that the contents of the memory structures can be
// printed on screen for debugging/logging purposes.
//
// The documentation for any of the "visit_*()" function explains what this
// functions should do and look like.
//
void print_callback (void (*write_function)(const char *fmt, ...), const char *prefix, uint8_t size, const char *name, const char *fmt, const void *p);

#endif
