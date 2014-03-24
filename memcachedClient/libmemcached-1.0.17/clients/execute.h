/* LibMemcached
 * Copyright (C) 2006-2009 Brian Aker
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 *
 * Summary:
 *
 */

#pragma once 

#include <stdio.h>

#include <libmemcached-1.0/memcached.h>
#include <libmemcached/common.h>
#include "clients/generator.h"
#include <mem_config.h>

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <map>
#include <sys/time.h>



#ifdef __cplusplus
extern "C" {
#endif

unsigned int execute_set(memcached_st *memc, pairs_st *pairs, unsigned int number_of);
unsigned int execute_get(memcached_st *memc, pairs_st *pairs, unsigned int number_of);
unsigned int execute_mget(memcached_st *memr, const char * const *keys, size_t *key_length,
                          unsigned int number_of);
unsigned int execute_mix(memcached_st *memc, pairs_st *pairs, unsigned int number_of,
        unsigned int num_ops, unsigned int write_percentage);
#ifdef __cplusplus
} // extern "C"
#endif
