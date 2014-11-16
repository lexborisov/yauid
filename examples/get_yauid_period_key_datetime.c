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
    unsigned long node_id = 12;
    yauid_period_key pkey = {0,0};

    yauid_get_period_key_by_datetime("2014-11-16 00:00:00", "2014-11-16 15:00:00",
                                     node_id, 0,
                                     &pkey);
    if(pkey.min)
        printf("min yauid: %"PRId64"; max yauid: %"PRId64"\n", pkey.min, pkey.max);
    else
        printf("Something wrong\n");
    
    return 0;
}

