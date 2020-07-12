MAKEFLAGS += --silent

TARGET = snalmir
LIBS = -lpthread
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

SRC = $(shell find . -name *.c)
OBJECTS = $(patsubst %.c, %.o, $(SRC))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o bin/$@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
