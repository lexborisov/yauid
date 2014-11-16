yauid
=====

A decentralized unique ID generator (int64)

## DESCRIPTION

ID generation at a node should not require coordination with other nodes. Ids should be roughly time-ordered when sorted lexicographically.

## BUILD

Build succeeded on FreeBSD, Mac OS X and Linux. Use GNU Make

##### FreeBSD

```sh
gmake
```

##### Other OS

```sh
make
```

## METHODS

See api/yauid.h file

## EXAMPLE

```c
#include <stdio.h>
#include <yauid.h>

int main(int argc, const char * argv[])
{
    yauid* yaobj = yauid_init("lock.yauid", NULL);
    
    if(yaobj == NULL)
    {
        printf("Can't create object\n");
        return 0;
    }
    
    yauid_set_node_id(yaobj, 12);
    
    hkey_t key = yauid_get_key(yaobj);
    
    printf("yauid: %"PRId64"; time: %ld; node id: %ld; inc: %ld \n", key,
                yauid_get_timestamp(key), yauid_get_node_id(key), yauid_get_inc_id(key));
    
    yauid_destroy(yaobj);
    
    return 0;
}
```

See more examples from examples directory

## AUTHOR

Alexander Borisov <lex.borisov@gmail.com>

## License

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

