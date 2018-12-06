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

#ifndef PTRARRAY_H
#define PTRARRAY_H

/** @file
 *  @brief Pointer array structure.
 *
 * This file implements a container implemented as a pointer array.
 *
 * To be type-safe, this functionality is fully defined by macros. The pointer array consists of a length member and
 * a data pointer.
 */

#include "utils.h" /* container_of() */
#include <stdbool.h>

/** @brief Declare an array of @a type.
 *
 * This is typically added as a struct member. It cannot be used directly as a function parameter, but it can be used
 * in a typedef.
 *
 * The pointer array must be initialised to 0 before using it, either with memset or by implicit initialisation of
 * static variables.
 *
 * To get the number of elements, access the length member directly.
 *
 * To iterate, use:
 * @code
    unsigned i;
    for (i = 0; i < ptrarray.length; i++)
    {
        ... ptrarray.data[i] ...
    }
 * @endcode
 *
 * Note that @a type may be any type (it must be a scalar type because other macros use assignment and comparison on
 * it). Normally it is a pointer type (hence the name pointer array).
 */
#define PTRARRAY(type) \
    struct { \
        unsigned length; \
        type *data; \
    }

/** @brief Add an element to a pointer array.
 *
 * The element is always added at the end.
 *
 * @param ptrarray A pointer array declared with PTRARRAY().
 * @param item A pointer to the element to by added. Must be a pointer to the same type as in the PTRARRAY declaration.
 */
#define PTRARRAY_ADD(ptrarray, item) \
    do { \
        (ptrarray).data = memrealloc((ptrarray).data, ((ptrarray).length + 1) * sizeof(*(ptrarray).data)); \
        (ptrarray).data[(ptrarray).length] = item; \
        (ptrarray).length++; \
    } while (0)

/** @brief Find a pointer in the pointer array.
 *
 * @return The index if found, @a ptrarray.length if not found.
 */
#define PTRARRAY_FIND(ptrarray, item) ({ \
        unsigned ptrarray_find_i; \
        for (ptrarray_find_i = 0; ptrarray_find_i < (ptrarray).length; ptrarray_find_i++) \
        { \
            if ((ptrarray).data[ptrarray_find_i] == (item)) \
                break; \
        } \
        ptrarray_find_i; \
    })

/** @brief Remove an item at a certain index from a pointer array.
 *
 * For convenience when used in combination with PTRARRAY_FIND, if @a index is equal to the length of the array, nothing
 * is removed.
 */
#define PTRARRAY_REMOVE(ptrarray, index) \
    do { \
        if (index >= (ptrarray).length) \
            break; \
        (ptrarray).length--; \
        if ((ptrarray).length > 0) \
        { \
            memmove((ptrarray).data + index, (ptrarray).data + (index) + 1, \
                    ((ptrarray).length - (index)) * sizeof(*(ptrarray).data)); \
            (ptrarray).data = memrealloc((ptrarray).data, ((ptrarray).length) * sizeof(*(ptrarray).data)); \
        } else { \
            free((ptrarray).data); \
            (ptrarray).data = NULL; \
        } \
    } while (0)

/** @brief Remove an item from a pointer array. */
#define PTRARRAY_REMOVE_ELEMENT(ptrarray, item) \
    PTRARRAY_REMOVE(ptrarray, PTRARRAY_FIND(ptrarray, item))


/** @brief Remove all elements from the pointer array. */
#define PTRARRAY_CLEAR(ptrarray) \
    do { \
        free((ptrarray).data); \
        (ptrarray).data = NULL; \
        (ptrarray).length = 0; \
    } while(0)

#endif // PTRARRAY_H
