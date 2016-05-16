/*
 Copyright (c) 2014-2016 Alexander Borisov
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <yauid.h>

#ifdef ENVIRONMENT32
#error 64 bit system only
#else

char error_text[][64] = {
    "OK",
    "Can't create key file",
    "Can't open key file",
    "All key in current sec done",
    "Can't read size for file node_id",
    "Can't allocate memory for node_id",
    "File node_id not exists",
    "Can't set lock",
    "Node_id is too long",
    "Node_id is too short",
    "Can't read key from file",
    "Can't seek to start file position",
    "Can't write key to file",
    "Can't flush key to file",
    "Number of attempts to get the key exhausted",
    "Can't create object"
};

unsigned long yauid_get_inc_id(hkey_t key)
{
    key <<= (BIT_LIMIT_TIMESTAMP + BIT_LIMIT_NODE);
    key >>= (BIT_LIMIT - BIT_LIMIT_INC);
    
    return (unsigned long)(key);
}

unsigned long yauid_get_node_id(hkey_t key)
{
    key <<= BIT_LIMIT_TIMESTAMP;
    key >>= (BIT_LIMIT - BIT_LIMIT_NODE);
    
    return (unsigned long)(key);
}

unsigned long yauid_get_timestamp(hkey_t key)
{
    key >>= (BIT_LIMIT_NODE + BIT_LIMIT_INC);
    
    return (unsigned long)(key);
}

unsigned long long int yauid_get_min_node_id()
{
    return LIMIT_MIN_NODE_ID;
}

unsigned long long int yauid_get_max_node_id()
{
    return NUMBER_LIMIT_NODE;
}

unsigned long long int yauid_get_max_inc()
{
    return NUMBER_LIMIT;
}

unsigned long long int yauid_get_max_timestamp()
{
    return NUMBER_LIMIT_TIMESTAMP;
}

hkey_t yauid_get_key(yauid* yaobj)
{
    hkey_t key = (hkey_t)(0);
    unsigned int count = 0;
    
    for(;;)
    {
        if((key = yauid_get_key_once(yaobj)) == (hkey_t)(0))
        {
            if(yaobj->error == YAUID_ERROR_KEYS_ENDED)
            {
                count++;
                
                if(yaobj->try_count && count >= yaobj->try_count)
                {
                    yaobj->error = YAUID_ERROR_TRY_COUNT_KEY;
                    break;
                }
                
                usleep(yaobj->sleep_usec);
                continue;
            }
        }
        
        break;
    }
    
    return key;
}

hkey_t yauid_get_key_once(yauid* yaobj)
{
    hkey_t key = (hkey_t)(0), tmp = (hkey_t)(1), ltime = (hkey_t)(0);
    
    if(yaobj->node_id < LIMIT_MIN_NODE_ID)
    {
        yaobj->error = YAUID_ERROR_SHORT_NODE_ID;
        return key;
    }
    else if(yaobj->node_id > NUMBER_LIMIT_NODE)
    {
        yaobj->error = YAUID_ERROR_LONG_NODE_ID;
        return key;
    }
    
    yaobj->error = YAUID_OK;
    
    if(flock(yaobj->i_lockfile, LOCK_EX) == -1)
    {
        yaobj->error = YAUID_ERROR_FILE_LOCK;
        return (hkey_t)(0);
    }
    
    if(fseek(yaobj->h_lockfile, 0, SEEK_SET) != 0)
    {
        yaobj->error = YAUID_ERROR_FILE_SEEK;
        return (hkey_t)(0);
    }
    
    if(fread((void *)(&key), sizeof(hkey_t), 1, yaobj->h_lockfile) != 1)
    {
        if(fseek(yaobj->h_lockfile, 0L, SEEK_END) != 0)
        {
            yaobj->error = YAUID_ERROR_FILE_SEEK;
            return (hkey_t)(0);
        }
        
        long h_size = ftell(yaobj->h_lockfile);
        if(h_size > 0)
        {
            yaobj->error = YAUID_ERROR_READ_KEY;
            return (hkey_t)(0);
        }
        
        if(fseek(yaobj->h_lockfile, 0, SEEK_SET) != 0)
        {
            yaobj->error = YAUID_ERROR_FILE_SEEK;
            return (hkey_t)(0);
        }
    }
    
    ltime = time(NULL);
    
    if(key)
    {
        tmp = key >> (BIT_LIMIT_NODE + BIT_LIMIT_INC);
        key <<= (BIT_LIMIT_TIMESTAMP + BIT_LIMIT_NODE);
        key >>= (BIT_LIMIT - BIT_LIMIT_INC);
        
        key++;
        
        if(tmp == ltime)
        {
            if(key > (hkey_t)(NUMBER_LIMIT))
            {
                flock(yaobj->i_lockfile, LOCK_UN);
                
                yaobj->error = YAUID_ERROR_KEYS_ENDED;
                return (hkey_t)(0);
            }
            
            tmp = key;
        }
        else
            tmp = (hkey_t)(1);
    }
    
    key = ltime;
    key <<= BIT_LIMIT_NODE;
    
    key |= yaobj->node_id;
    key <<= BIT_LIMIT_INC;
    
    key |= tmp;
    
    if(fseek(yaobj->h_lockfile, 0, SEEK_SET) != 0)
    {
        yaobj->error = YAUID_ERROR_FILE_SEEK;
        return (hkey_t)(0);
    }
    
    if(fwrite((const void *)(&key), sizeof(hkey_t), 1, yaobj->h_lockfile) != 1)
    {
        yaobj->error = YAUID_ERROR_WRITE_KEY;
        return (hkey_t)(0);
    }
    
    if(fflush(yaobj->h_lockfile) != 0)
    {
        yaobj->error = YAUID_ERROR_FLUSH_KEY;
        return (hkey_t)(0);
    }
    
    if(flock(yaobj->i_lockfile, LOCK_UN) == -1)
    {
        yaobj->error = YAUID_ERROR_FILE_LOCK;
        return (hkey_t)(0);
    }
    
    yaobj->error = YAUID_OK;
    
    return key;
}

yauid * yauid_init(const char *filepath_key, const char *filepath_node_id)
{
    yauid* yaobj = (yauid *)malloc(sizeof(yauid));
    
    if(yaobj)
    {
        yaobj->node_id    = 0;
        yaobj->error      = YAUID_OK;
        yaobj->i_lockfile = 0;
        yaobj->h_lockfile = NULL;
        yaobj->try_count  = 0;
        yaobj->sleep_usec = (useconds_t)(35000L);
        yaobj->ext_value  = 0;
        
        if(filepath_key == NULL)
        {
            yaobj->error = YAUID_ERROR_CREATE_KEY_FILE;
            return yaobj;
        }
        
        yaobj->c_lockfile = strdup(filepath_key);
        if(yaobj->c_lockfile == NULL)
        {
            yaobj->error = YAUID_ERROR_ALLOC_KEY_FILE;
            return yaobj;
        }
        
        if(filepath_node_id != NULL)
        {
            if(access( filepath_node_id, F_OK ) != -1)
            {
                FILE* h_node_id;
                if((h_node_id = fopen(filepath_node_id, "rb")))
                {
                    fseek(h_node_id, 0L, SEEK_END);
                    
                    long h_size = ftell(h_node_id);
                    if(h_size <= 0)
                    {
                        fclose(h_node_id);
                        yaobj->error = YAUID_ERROR_FILE_NODE_ID;
                        return yaobj;
                    }
                    
                    fseek(h_node_id, 0L, SEEK_SET);
                    
                    char *text = (char *)malloc(sizeof(char) * (h_size + 1));
                    if(text == NULL)
                    {
                        fclose(h_node_id);
                        yaobj->error = YAUID_ERROR_FILE_NODE_MEM;
                        return yaobj;
                    }
                    
                    fread(text, sizeof(char), h_size, h_node_id);
                    fclose(h_node_id);
                    
                    long i = 0;
                    for(i = 0; i < h_size; i++)
                    {
                        if(text[i] >= '0' && text[i] <= '9')
                            yaobj->node_id = (text[i] - '0') + (yaobj->node_id * 10);
                    }
                    
                    free(text);
                    
                    if(yaobj->node_id < LIMIT_MIN_NODE_ID)
                    {
                        yaobj->error = YAUID_ERROR_SHORT_NODE_ID;
                        return yaobj;
                    }
                    else if(yaobj->node_id > NUMBER_LIMIT_NODE)
                    {
                        yaobj->error = YAUID_ERROR_LONG_NODE_ID;
                        return yaobj;
                    }
                }
            }
            else {
                yaobj->error = YAUID_ERROR_FILE_NODE_EXT;
                return yaobj;
            }
        }
        
        if(access( yaobj->c_lockfile, F_OK ) == -1)
        {
            if((yaobj->h_lockfile = fopen(yaobj->c_lockfile, "ab")) == 0)
            {
                yaobj->error = YAUID_ERROR_CREATE_KEY_FILE;
                return yaobj;
            }
            
            fclose(yaobj->h_lockfile);
        }
        
        if((yaobj->h_lockfile = fopen(yaobj->c_lockfile, "rb+")) == 0)
        {
            yaobj->error = YAUID_ERROR_OPEN_LOCK_FILE;
            return yaobj;
        }
        
        setbuf(yaobj->h_lockfile, NULL);
        
        yaobj->i_lockfile = fileno(yaobj->h_lockfile);
    }
    
    return yaobj;
}

void yauid_destroy(yauid* yaobj)
{
    if(yaobj == NULL)
        return;
    
    if(yaobj->h_lockfile)
        fclose(yaobj->h_lockfile);
    if(yaobj->c_lockfile)
        free(yaobj->c_lockfile);
    
    free(yaobj);
}

char * yauid_get_error_text_by_code(yauid_status_t error)
{
    if((YAUID_ERROR_CREATE_OBJECT - YAUID_OK) < error)
        return NULL;
    
    return error_text[error];
}

void yauid_set_node_id(yauid* yaobj, unsigned long node_id)
{
    yaobj->error = YAUID_OK;
    
    if(node_id < LIMIT_MIN_NODE_ID)
        yaobj->error = YAUID_ERROR_SHORT_NODE_ID;
    else if(node_id > NUMBER_LIMIT_NODE)
        yaobj->error = YAUID_ERROR_LONG_NODE_ID;
    else
        yaobj->node_id = node_id;
}

void yauid_set_sleep_usec(yauid* yaobj, useconds_t sleep_usec)
{
    yaobj->error = YAUID_OK;
    yaobj->sleep_usec = sleep_usec;
}

void yauid_set_try_count(yauid* yaobj, unsigned int try_count)
{
    yaobj->error = YAUID_OK;
    yaobj->try_count = try_count;
}

time_t yauid_datetime_to_timestamp(const char *datetime)
{
    struct tm tm;
    time_t epoch;
    
    if(strptime(datetime, "%Y-%m-%d %H:%M:%S", &tm) != NULL)
        epoch = mktime(&tm);
    else
        epoch = (time_t)(0);
    
    return epoch;
}

hkey_t yauid_get_key_by_timestamp(time_t timestamp, size_t node_id, size_t counter)
{
    if(counter > NUMBER_LIMIT)
        return 0;
    if(node_id > NUMBER_LIMIT_NODE)
        return 0;
    if(timestamp > NUMBER_LIMIT_TIMESTAMP)
        return 0;
    
    hkey_t hkey = (size_t)timestamp;
    hkey <<= BIT_LIMIT_NODE;
    hkey |= node_id;
    hkey <<= BIT_LIMIT_INC;
    hkey |= counter;
    
    return hkey;
}

void yauid_get_period_key_by_datetime(const char *from_datetime,
                          const char *to_datetime,
                          unsigned long long int from_node_id,
                          unsigned long long int to_node_id,
                          yauid_period_key *pkey)
{
    if(to_datetime == NULL)
        to_datetime = from_datetime;
    
    yauid_get_period_key_by_timestamp(yauid_datetime_to_timestamp(from_datetime),
                                      yauid_datetime_to_timestamp(to_datetime),
                                      from_node_id, to_node_id,
                                      pkey);
}

void yauid_get_period_key_by_timestamp(time_t from_timestamp,
                          time_t to_timestamp,
                          unsigned long long int from_node_id,
                          unsigned long long int to_node_id,
                          yauid_period_key *pkey)
{
    if(pkey == NULL)
        return;
    
    pkey->max = 0;
    pkey->min = 0;
    
    if(to_timestamp == 0)
        to_timestamp = from_timestamp;
    
    if(from_node_id == 0)
        from_node_id = 1;
    
    if(to_node_id == 0)
        to_node_id = NUMBER_LIMIT_NODE;
    
    pkey->min = (hkey_t)(from_timestamp);
    if(pkey->min == 0)
        return;
    
    pkey->min <<= BIT_LIMIT_NODE;
    
    pkey->min |= (hkey_t)(from_node_id);
    pkey->min <<= BIT_LIMIT_INC;
    
    pkey->min |= (hkey_t)(1);
    
    pkey->max = (hkey_t)(to_timestamp);
    if(pkey->max == 0)
        return;
    
    pkey->max <<= BIT_LIMIT_NODE;
    
    pkey->max |= (hkey_t)(to_node_id);
    pkey->max <<= BIT_LIMIT_INC;
    
    pkey->max |= (hkey_t)(NUMBER_LIMIT);
}

yauid_status_t yauid_get_error_code(yauid* yaobj)
{
    return yaobj->error;
};

#endif


