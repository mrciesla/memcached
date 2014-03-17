#include <stdio.h>                                                                                                                      
#include <string.h>                                                                                                                    
#include <unistd.h>                                                                                                                    
#include <libmemcached/memcached.h>                                                                                                    
                                                                                                                                       
int main(int argc, char *argv[])                                                                                                        
{                                                                                                                                      
    memcached_server_st *servers = NULL;                                                                                                  
    memcached_st *memc;                                                                                                                  
    memcached_return rc;                                                                                                                  
    char *key = "keyString";                                                                                                              
    char *value = "ValueString";                                                                                                        
    char *retvalue = NULL;                                                                                                                
    size_t retlength;                                                                                                                    
    uint32_t flags;                                                                                                                      
    int i =0;
                                                                               
    /* Create an empty memcached interface */                                                                                            
    memc = memcached_create(NULL);                                                                                                        
                                                                                 
    /* Append a server to the list */                                                                                                    
    servers = memcached_server_list_append(servers, "127.0.0.1", 11211, &rc);                                                          
                                                                                   
    /* Update the memcached structure with the updated server list */                                                                    
    rc = memcached_server_push(memc, servers);                                                                                            
                                                                                     
    if (rc == MEMCACHED_SUCCESS)                                                                                                          
        fprintf(stderr,"Successfully added server\n");                                                                                      
    else                                                                                                                                  
        fprintf(stderr,"Couldn't add server: %s\n",memcached_strerror(memc, rc));                                                          
                                                                                         
    for(i =0; i< 10; i++){
       
        /* Store a value, without a timelimit */                                                                                              
        fprintf(stderr,"*****setting key: %s\n", key);                                                            
        rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);                                            
        if (rc == MEMCACHED_SUCCESS)                                                                                                          
            fprintf(stderr,"Value stored successfully\n");                                                                                      
        else                                                                                                                                  
            fprintf(stderr,"Couldn't store key: %s\n",memcached_strerror(memc, rc));                                                            
        
     /* Retrieve the Stored value */                                                                                                      
        fprintf(stderr,"*****getting key: %s\n", key);                                                            
        retvalue = memcached_get(memc, key, strlen(key), &retlength, &flags, &rc);                                                            
                                                                                                     
        if (rc == MEMCACHED_SUCCESS)                                                                                                          
            fprintf(stderr,"Value retrieved successfully: %s\n",retvalue);                                                                      
        else                                                                                                                                  
            fprintf(stderr,"Couldn't store key: %s\n",memcached_strerror(memc, rc));                                                            
     
    }
    char *tkey = "test";
    retvalue = memcached_get(memc, tkey, strlen(tkey), &retlength, &flags, &rc);                                                            
                                                                                                 
    if (rc == MEMCACHED_SUCCESS)                                                                                                          
        fprintf(stderr,"Value retrieved successfully: %s\n",retvalue);                                                                      
    else                                                                                                                                  
        fprintf(stderr,"Couldn't store key: %s\n",memcached_strerror(memc, rc));                                                            
   
                                                                                  
                                                                                               
                                                                                                    
    return 0;                                                                                                                            
}
