MAKEFLAGS += --silent

TARGET = snalmir
LIBS = -lpthread -lm -lrt
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET)
all: dir default

SOURCE = $(shell find . -name *.c)
HEADERS = $(shell find . -name *.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCE))

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ -g

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o bin/$@
	-rm -f $(OBJECTS)

dir:
	mkdir -p bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f bin/$(TARGET)
