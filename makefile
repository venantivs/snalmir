PROJ_NAME := snalmir

C_SOURCE := $(wildcard ./*.c)

H_SOURCE := $(wildcard ./*.h)

OBJ := $(subst .c, .o, $(subst source, objects, $(C_SOURCE)))

CC := gcc

CC_FLAGS := -Wall -Wextra -lpthread

build: $(PROJ_NAME) clean

$(PROJ_NAME): $(OBJ)
	$(CC) $^ $(CC_FLAGS) -o $@

clean: $(PROJ_NAME)
	rm $(PROJ_NAME) *.o
