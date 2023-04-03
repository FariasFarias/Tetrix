CC = gcc
CFLAGS = 
LDFLAGS =


ifeq ($(OS),Windows_NT)
    CFLAGS += -Wall -Wextra -pedantic -std=c11 -I./include/ncursesw -I./include -g
    LDFLAGS += -L./lib
else
    CFLAGS += -Wall -Wextra -pedantic -std=c11 -g
endif

LIBS = -lncursesw -lpthread


SRCS = tetrix.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: tetrix clean

tetrix: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(if $(filter $(OS),Windows_NT), del /f .\$(OBJS), rm -f $(OBJS))