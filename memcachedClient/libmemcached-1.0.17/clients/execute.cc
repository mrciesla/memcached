/* LibMemcached
 * Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
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
  Execute a memcached_set() a set of pairs.
  Return the number of rows set.
*/

#include <mem_config.h>
#include "clients/execute.h"

using namespace std;
static bool myPairs = false;
map<unsigned int, timeval> *inFlightG;
//Time in microseconds
map< long, long > *doneG;
map<unsigned int, timeval> *inFlightS;
map< long, long > *doneS;

void START(int id, map<unsigned int, timeval> *inFlight){
    struct timeval t1;
    gettimeofday(&t1, NULL);
    (*inFlight)[id] = t1;
}

void END(unsigned int id, map<unsigned int, timeval> *inFlight, map<long, long> *done){

    struct timeval end;
    gettimeofday(&end, NULL);
    struct timeval start = (*inFlight)[id];
    inFlight->erase(inFlight->find(id)); 
    long mtime, seconds, useconds;
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000000 + useconds) ;

    if(done->count(mtime) > 0){
        (*done)[mtime] = (*done)[mtime] + 1;
    }else{
        (*done)[mtime] = 1;
    }

}
unsigned int execute_set(memcached_st *memc, pairs_st *pairs, unsigned int number_of)
{
  unsigned int x;
  unsigned int pairs_sent;

  for (x= 0, pairs_sent= 0; x < number_of; x++)
  {
    memcached_return_t rc= memcached_set(memc, pairs[x].key, pairs[x].key_length,
                                         pairs[x].value, pairs[x].value_length,
                                         0, 0);
    if (memcached_failed(rc))
    {
      fprintf(stderr, "%s:%d Failure on insert (%s) of %.*s\n",
              __FILE__, __LINE__,
              memcached_last_error_message(memc),
              (unsigned int)pairs[x].key_length, pairs[x].key);
      
      // We will try to reconnect and see if that fixes the issue
      memcached_quit(memc);
    }
    else
    {
      pairs_sent++;
    }
  }

  return pairs_sent;
}

/*
  Execute a mix of operations on a set of pairs.
  Return the number of rows retrieved.
*/

void do_get(unsigned int &retrieved, unsigned int number_of, memcached_st *memc, pairs_st *pairs){
    size_t value_length;
    uint32_t flags;
    static unsigned int id = 0;

    START(id, inFlightG);
    unsigned int fetch_key= (unsigned int)((unsigned int)random() % number_of);

    memcached_return_t rc;
    char *value= memcached_get(memc, pairs[fetch_key].key, pairs[fetch_key].key_length,
                               &value_length, &flags, &rc);

    if (memcached_failed(rc))
    {
      fprintf(stderr, "%s:%d Failure on read(%s) of %.*s\n",
              __FILE__, __LINE__,
              memcached_last_error_message(memc),
              (unsigned int)pairs[fetch_key].key_length, pairs[fetch_key].key);
    }
    else
    {
      retrieved++;
    }

    ::free(value);

    END(id, inFlightG, doneG);
}


void do_set_new(unsigned int &pairs_sent, unsigned int &number_of, memcached_st *memc, pairs_st *pairs){

    //Make the new pair
    static int id = 0;
    START(id, inFlightS);
    int x = number_of;
    unsigned int value_length = 400;
    pairs[x].key= (char *)calloc(100, sizeof(char));

    if (pairs[x].key == NULL){
      fprintf(stderr, "%s:%d Failure on insert \n",__FILE__, __LINE__);
    }

    get_random_string(pairs[x].key, 100);
    pairs[x].key_length= 100;

    if (value_length)
    {
      pairs[x].value= (char *)calloc(value_length, sizeof(char));

      if (pairs[x].value == NULL)
        fprintf(stderr, "%s:%d Failure on insert \n",__FILE__, __LINE__);

      get_random_string(pairs[x].value, value_length);
      pairs[x].value_length= value_length;
    }
    else
    {
      pairs[x].value= NULL;
      pairs[x].value_length= 0;
    }
    number_of++;
    //send the new key
    memcached_return_t rc= memcached_set(memc, pairs[x].key, pairs[x].key_length,
                                         pairs[x].value, pairs[x].value_length,
                                         0, 0);
    if (memcached_failed(rc))
    {
      fprintf(stderr, "%s:%d Failure on insert (%s) of %.*s\n",
              __FILE__, __LINE__,
              memcached_last_error_message(memc),
              (unsigned int)pairs[x].key_length, pairs[x].key);
      
      // We will try to reconnect and see if that fixes the issue
      memcached_quit(memc);
    }
    else
    {
      pairs_sent++;
    }

    END(id++, inFlightS, doneS);

}

void free_old(pairs_st *pairs){
  if (pairs == NULL)
  {
    return;
  }

  /* We free until we hit the null pair we stores during creation */
  for (uint32_t x= 0; pairs[x].key; x++)
  {
    free(pairs[x].key);
    if (pairs[x].value)
    {
      free(pairs[x].value);
    }
  }

  delete []pairs;
}

pairs_st *increase_pairs(pairs_st *pairs, unsigned int &size){
    printf("Increasing pair size\n");
  pairs_st *newPairs = new pairs_st[size*2+1]; 
  for(unsigned int i =0; i < size; i++){
      newPairs[i].copy_from(&pairs[i]);
  }
  //free old
  if(myPairs){
      free_old(pairs);
  }
  size = size*2;
  myPairs = true;
  return newPairs;
}

void postProcess(map<long, long> *done, char *prefix){
    printf("Post process %ld\n", done->size());
    map<long, long>::iterator start = done->begin();
    map<long, long>::iterator end = done->end();
    long max =0;
    //Find max
    while(start!=end){
        if(start->first > max){
            max = start->first;
        }
        start++;
    }
    //Bucket sort!
    long *values = new long[max + 1];
    for(long i =0; i <=max; i++){
        values[i] = 0;
    }
    
    start = done->begin();
    while(start!=end){
        values[start->first] = start->second;
        start++;
    }
    for(long i =0; i <= max; i++){
        if(values[i] != 0){
            printf("%s,%ld,%ld\n", prefix, i, values[i]);
        }
    }

    delete []values;

}


unsigned int execute_mix(memcached_st *memr, pairs_st *pairs, unsigned int number_of, unsigned int num_ops, unsigned int write_percentage)
{
  unsigned int retrieved =0;
  unsigned int pairs_sent =0;

  unsigned int size_of_pairs = number_of;
  
  //reset_stats( memr, NULL, 0);
  //After a reset the connection is bonkers so basically create a new one
  memcached_st *memc= memcached_clone(NULL, memr);
 
  inFlightG = new map<unsigned int, timeval>();
  doneG = new map<long, long>();

  inFlightS = new map<unsigned int, timeval>();
  doneS = new map<long, long>();
  
  num_ops = num_ops == 0 ? 1000:num_ops;

  printf("Doing mix\n");
  //Create a copy of pairs that is 2x in order to have room for new sets
  pairs = increase_pairs(pairs, size_of_pairs);
  
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  do{
    unsigned int prob= (unsigned int)((unsigned int)random() % 100);

    if(prob > write_percentage){
        do_get(retrieved, number_of, memc, pairs);
    }else{
        if(number_of >= size_of_pairs -1){
            pairs = increase_pairs(pairs, size_of_pairs);
        }
        do_set_new(pairs_sent, number_of, memc, pairs);
    }
    gettimeofday(&t2, NULL);
  }while((t2.tv_sec  - t1.tv_sec) < 600);
    postProcess(doneS, "Set");
    postProcess(doneG, "Get");
  if(myPairs){
      free_old(pairs);
  }

  memcached_stat_st *memc_stat= memcached_stat(memc, NULL, NULL);
  printf("Get %s\n", memcached_stat_get_value(memc, memc_stat,"cmd_get", NULL));
  printf("Set %s\n", memcached_stat_get_value(memc, memc_stat,"cmd_set", NULL));

  memcached_free(memc);
  return retrieved;
}


/*
  Execute a memcached_get() on a set of pairs.
  Return the number of rows retrieved.
*/
unsigned int execute_get(memcached_st *memc, pairs_st *pairs, unsigned int number_of)
{
  unsigned int x;
  unsigned int retrieved;


  for (retrieved= 0,x= 0; x < number_of; x++)
  {
    size_t value_length;
    uint32_t flags;

    unsigned int fetch_key= (unsigned int)((unsigned int)random() % number_of);

    memcached_return_t rc;
    char *value= memcached_get(memc, pairs[fetch_key].key, pairs[fetch_key].key_length,
                               &value_length, &flags, &rc);

    if (memcached_failed(rc))
    {
      fprintf(stderr, "%s:%d Failure on read(%s) of %.*s\n",
              __FILE__, __LINE__,
              memcached_last_error_message(memc),
              (unsigned int)pairs[fetch_key].key_length, pairs[fetch_key].key);
    }
    else
    {
      retrieved++;
    }

    ::free(value);
  }

  return retrieved;
}

/**
 * Callback function to count the number of results
 */
static memcached_return_t callback_counter(const memcached_st *ptr,
                                           memcached_result_st *result,
                                           void *context)
{
  (void)ptr;
  (void)result;
  unsigned int *counter= (unsigned int *)context;
  *counter= *counter + 1;

  return MEMCACHED_SUCCESS;
}

/**
 * Try to run a large mget to get all of the keys
 * @param memc memcached handle
 * @param keys the keys to get
 * @param key_length the length of the keys
 * @param number_of the number of keys to try to get
 * @return the number of keys received
 */
unsigned int execute_mget(memcached_st *memc,
                          const char * const *keys,
                          size_t *key_length,
                          unsigned int number_of)
{
  unsigned int retrieved= 0;
  memcached_execute_fn callbacks[]= { callback_counter };
  memcached_return_t rc;
  rc= memcached_mget_execute(memc, keys, key_length,
                             (size_t)number_of, callbacks, &retrieved, 1);

  if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_NOTFOUND ||
          rc == MEMCACHED_BUFFERED || rc == MEMCACHED_END)
  {
    rc= memcached_fetch_execute(memc, callbacks, (void *)&retrieved, 1);
    if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_NOTFOUND && rc != MEMCACHED_END)
    {
      fprintf(stderr, "%s:%d Failed to execute mget: %s\n",
              __FILE__, __LINE__,
              memcached_strerror(memc, rc));
      memcached_quit(memc);
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "%s:%d Failed to execute mget: %s\n",
            __FILE__, __LINE__,
            memcached_strerror(memc, rc));
    memcached_quit(memc);
    return 0;
  }

  return retrieved;
}
