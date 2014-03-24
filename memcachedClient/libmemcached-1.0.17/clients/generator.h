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

/*
  Code to generate data to be pushed into memcached
*/

#pragma once

typedef struct pairs_st pairs_st;

struct pairs_st {
  char *key;
  size_t key_length;
  char *value;
  size_t value_length;
  void copy_from(pairs_st *from){
      key_length = from->key_length;
      value_length = from->value_length;
      key= (char *)calloc(100, sizeof(char));
      value= (char *)calloc(value_length, sizeof(char));
      memcpy(key, from->key, key_length);
      memcpy(value, from->value, value_length);
  }
};

#ifdef __cplusplus
extern "C" {
#endif

pairs_st *pairs_generate(uint64_t number_of, size_t value_length);
void pairs_free(pairs_st *pairs);
void get_random_string(char *buffer, size_t size);

#ifdef __cplusplus
} // extern "C"
#endif
