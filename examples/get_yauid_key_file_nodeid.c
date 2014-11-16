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

#include <stdio.h>
#include <yauid.h>

int main(int argc, const char * argv[])
{
    yauid_status_t status;
    yauid* yaobj = yauid_init("lock.yauid", "node.id");
    
    if(yaobj == NULL)
    {
        printf("Can't create object\n");
    }
    else if((status = yauid_get_error_code(yaobj)) != YAUID_OK)
    {
        printf("%s\n", yauid_get_error_text_by_code(status));
    }
    else
    {
        printf("Max inc per second for a single node: %llu\n", yauid_get_max_inc());
        printf("Max node id: %llu\n", yauid_get_max_node_id());
        printf("Max timestamp: %llu\n", yauid_get_max_timestamp());

        size_t i;
        for(i = 0; i < 132769L; i++)
        {
            hkey_t key = yauid_get_key(yaobj);
            
            if(key == (hkey_t)(0))
            {
                printf("%s\n", yauid_get_error_text_by_code( yauid_get_error_code(yaobj) ));
                break;
            }
        
            printf("yauid: %"PRId64"; time: %ld; node id: %ld; inc: %ld \n", key,
                yauid_get_timestamp(key), yauid_get_node_id(key), yauid_get_inc_id(key));
        }
    }
    
    yauid_destroy(yaobj);
    
    return 0;
}

