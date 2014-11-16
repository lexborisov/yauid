CC      = gcc
INC_DIR = ../api
CFLAGS  = -fPIC -Wall -I$(INC_DIR)
LIB_INC = ../libyauid_static.a


all: yauid_simple get_yauid_period_key_datetime get_yauid_key_set_nodeid get_yauid_key_file_nodeid clean_o

clean:
	rm -f get_yauid_key_file_nodeid
	rm -f get_yauid_key_set_nodeid
	rm -f get_yauid_period_key_datetime
	rm -f yauid_simple

clean_o:
	rm -f get_yauid_key_file_nodeid.o
	rm -f get_yauid_key_set_nodeid.o
	rm -f get_yauid_period_key_datetime.o
	rm -f yauid_simple.o

get_yauid_key_file_nodeid : get_yauid_key_file_nodeid.o
	$(CC) $(CFLAGS) -o $@ get_yauid_key_file_nodeid.o $(LIB_INC)

get_yauid_key_file_nodeid.o : get_yauid_key_file_nodeid.c 
	$(CC) $(CFLAGS) -c get_yauid_key_file_nodeid.c -o $@

get_yauid_key_set_nodeid : get_yauid_key_set_nodeid.o
	$(CC) $(CFLAGS) -o $@ get_yauid_key_set_nodeid.o $(LIB_INC)

get_yauid_key_set_nodeid.o : get_yauid_key_set_nodeid.c 
	$(CC) $(CFLAGS) -c get_yauid_key_set_nodeid.c -o $@

get_yauid_period_key_datetime : get_yauid_period_key_datetime.o
	$(CC) $(CFLAGS) -o $@ get_yauid_period_key_datetime.o $(LIB_INC)

get_yauid_period_key_datetime.o : get_yauid_period_key_datetime.c 
	$(CC) $(CFLAGS) -c get_yauid_period_key_datetime.c -o $@

yauid_simple : yauid_simple.o
	$(CC) $(CFLAGS) -o $@ yauid_simple.o $(LIB_INC)

yauid_simple.o : yauid_simple.c 
	$(CC) $(CFLAGS) -c yauid_simple.c -o $@

