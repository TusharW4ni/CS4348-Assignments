SOURCES = $(wildcard *.c)
EXECS = $(SOURCES:%.c=%)
CFLAGS = -std=c99 -g -lpthread
LDFLAGS = -lm
CC=gcc

all: $(EXECS)

$(EXECS): %: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm $(EXECS)
