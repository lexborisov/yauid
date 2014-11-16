AR      = ar
CC      = gcc
LDFLAGS = -shared
INC_DIR = api
SRC_DIR = src
CFLAGS  = -fPIC -Wall -I$(INC_DIR) 
OSNAME  = darwin
UNAMES := $(shell uname -s)
SO      = so
FLAGS   = -Iapi
SOURCES = $(SRC_DIR)/yauid.c
OBJECTS = $(SRC_DIR)/yauid.o

#ifeq ($(OS),Windows_NT)
ifeq ($(UNAMES),Darwin)
	SO = dylib
endif

TARGET        = libyauid.$(SO)
TARGET_STATIC = libyauid_static.a

all: $(OBJECTS) $(TARGET) $(TARGET_STATIC)
	@(cd examples; $(MAKE))

clean:
	rm -f $(TARGET)
	rm -f $(TARGET_STATIC)
	rm -f $(OBJECTS)
	@(cd examples; $(MAKE clean))
	
$(TARGET_STATIC) : $(OBJECTS)
	$(AR) crus $(TARGET_STATIC) $(OBJECTS)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) -o $(OBJECTS)
