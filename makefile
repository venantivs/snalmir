MAKEFLAGS += --silent

TARGET = snalmir
LIBS = -lpthread
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET)
all: dir default

SOURCE = $(shell find . -name *.c)
HEADERS = $(shell find . -name *.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCE))

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o bin/$@

dir:
	mkdir -p bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f bin/$(TARGET)
