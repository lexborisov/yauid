/*
 Copyright (c) 2014 Alexander Borisov
 
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

#ifndef yauid_yauid_h
#define yauid_yauid_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/file.h>
#include <unistd.h>

#if defined(__x86_64__) || defined(__ppc64__) || defined(_WIN64)
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif

/***********************************************************************************
 *
 * Base init
 *
 ***********************************************************************************/

#ifndef LIMIT_MIN_NODE_ID
#define LIMIT_MIN_NODE_ID    1L
#endif

#ifndef BIT_LIMIT
#define BIT_LIMIT           64L
#endif

#ifndef BIT_LIMIT_TIMESTAMP
#define BIT_LIMIT_TIMESTAMP 33L
#endif

#ifndef BIT_LIMIT_NODE
#define BIT_LIMIT_NODE      14L
#endif

#define BIT_LIMIT_INC       (BIT_LIMIT - (BIT_LIMIT_TIMESTAMP + BIT_LIMIT_NODE))

#define NUMBER_LIMIT           ((1L << BIT_LIMIT_INC) - 1)
#define NUMBER_LIMIT_NODE      ((1L << BIT_LIMIT_NODE) - 1)
#define NUMBER_LIMIT_TIMESTAMP ((1L << BIT_LIMIT_TIMESTAMP) - 1)

// 64 bit
typedef uint64_t hkey_t;

enum yauid_status {
    YAUID_OK = 0,
    YAUID_ERROR_CREATE_KEY_FILE,
    YAUID_ERROR_OPEN_LOCK_FILE,
    YAUID_ERROR_KEYS_ENDED,
    YAUID_ERROR_FILE_NODE_ID,
    YAUID_ERROR_FILE_NODE_MEM,
    YAUID_ERROR_FILE_NODE_EXT,
    YAUID_ERROR_FILE_LOCK,
    YAUID_ERROR_LONG_NODE_ID,
    YAUID_ERROR_SHORT_NODE_ID,
    YAUID_ERROR_READ_KEY,
    YAUID_ERROR_FILE_SEEK,
    YAUID_ERROR_WRITE_KEY,
    YAUID_ERROR_FLUSH_KEY,
    YAUID_ERROR_TRY_COUNT_KEY,
    YAUID_ERROR_CREATE_OBJECT,
    YAUID_ERROR_ALLOC_KEY_FILE
}
typedef yauid_status_t;

// base structure
struct yauid {
    int           i_lockfile;
    char*         c_lockfile;
    FILE*         h_lockfile;
    unsigned long node_id;
    
    unsigned int try_count;
    useconds_t sleep_usec;
    
    enum yauid_status error;
    void *ext_value;
}
typedef yauid;

struct yauid_period_key {
    hkey_t min;
    hkey_t max;
}
typedef yauid_period_key;

/***********************************************************************************
 *
 * YAUID
 *
 ***********************************************************************************/

/**
 * Create a new yauid
 *
 * @param[in] File path to lock file. Important! All yauid (on one node) link to this file
 * @param[in] NULL or file path to node id. See yauid_set_node_id
 * @return yauid structure
 */
yauid * yauid_init(const char *filepath_key, const char *filepath_node_id);

/**
 * Frees all allocated resources
 *
 * @param[in] yauid
 */
void yauid_destroy(yauid* yaobj);

/**
 * Get unique key by current node
 *
 * @param[in] yauid
 * @return unique key if successful or 0 if any error. See yauid_get_error_code
 */
hkey_t yauid_get_key(yauid* yaobj);

/**
 * Tries once get unique key by current node
 *
 * @param[in] yauid
 * @return unique key if successful or 0 if any error. See yauid_get_error_code
 */
hkey_t yauid_get_key_once(yauid* yaobj);

/**
 * Set current node id
 *
 * @param[in] yauid
 * @param[in] node id equal yauid_get_min_node_id() to yauid_get_max_node_id()
 */
void yauid_set_node_id(yauid* yaobj, unsigned long node_id);

/**
 * Set sleeping time to try get key (refers to yauid_get_key)
 *
 * @param[in] yauid
 * @param[in] microsecond intervals. Default: 35000 (35000 microsecond = 0.3 second)
 */
void yauid_set_sleep_usec(yauid* yaobj, useconds_t sleep_usec);

/**
 * Number of attempts before surrender (refers to yauid_get_key)
 *
 * @param[in] yauid
 * @param[in] try count. Default: 0 (0 is infinity)
 */
void yauid_set_try_count(yauid* yaobj, unsigned int try_count);

/**
 * Get timestamp from yauid key
 *
 * @param[in] yauid key
 * @return timestamp
 */
unsigned long yauid_get_timestamp(hkey_t key);

/**
 * Get node id from yauid key
 *
 * @param[in] yauid key
 * @return node id
 */
unsigned long yauid_get_node_id(hkey_t key);

/**
 * Get inc (unique id per second in the current key) id from yauid key
 *
 * @param[in] yauid key
 * @return inc id
 */
unsigned long yauid_get_inc_id(hkey_t key);

/**
 * Get the maximum possible inc per one second
 *
 * @return inc id
 */
unsigned long long int yauid_get_max_inc();

/**
 * Get the minimum possible node id
 *
 * @return node id
 */
unsigned long long int yauid_get_min_node_id();

/**
 * Get the maximum possible node id
 *
 * @return node id
 */
unsigned long long int yauid_get_max_node_id();

/**
 * Get the maximum possible timestamp
 *
 * @return timestamp
 */
unsigned long long int yauid_get_max_timestamp();

/**
 * Convert datetime to timestamp. 2014-07-12 04:23:12 => 1405124592
 *
 * @param[in] datetime (e.g. 2014-07-12 04:23:12)
 * @return timestamp
 */
time_t yauid_datetime_to_timestamp(const char *datetime);

/**
 * Get minimum and maximum key from date and node id by timestamp
 *
 * @param[in] from timestamp (e.g. 1405124592)
 * @param[in] to timestamp. NULL = from timestamp
 * @param[in] from node id
 * @param[in] to node id. NULL = from node
 * @param[out] min and max yauid key
 */
void yauid_get_period_key_by_timestamp(time_t from_timestamp,
                                       time_t to_timestamp,
                                       unsigned long long int from_node_id,
                                       unsigned long long int to_node_id,
                                       yauid_period_key *pkey);

/**
 * Get minimum and maximum key from date and node id by datetime
 *
 * @param[in] from datetime (e.g. "2014-07-12 04:23:12")
 * @param[in] to datetime. NULL = from datetime
 * @param[in] from node id
 * @param[in] to node id. NULL = from node
 * @param[out] min and max yauid key
 */
void yauid_get_period_key_by_datetime(const char *from_datetime,
                                      const char *to_datetime,
                                      unsigned long long int from_node_id,
                                      unsigned long long int to_node_id,
                                      yauid_period_key *pkey);

/**
 * Get error description by error code
 *
 * @param[in] error code. See yauid_get_error_code
 * @return pointer to description
 */
char * yauid_get_error_text_by_code(yauid_status_t error_code);

/**
 * Get current status of yauid
 *
 * @param[in] yauid
 * @return status code from enum yauid_status
 */
yauid_status_t yauid_get_error_code(yauid* yaobj);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

