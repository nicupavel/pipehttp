src = $(wildcard *.c)
obj = $(src:.c=.o)

ROOT_PREFIX ?= /usr/local/

CFLAGS = -Wall -I.
OPTFLAGS = -s -O3
LDFLAGS = 

all: ph

debug: CFLAGS += -g -DDEBUG
debug: OPTFLAGS = -O0
debug: clean
debug: ph

ph: $(obj)
	$(CC) $(OPTFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: install
install:
	install -m 557 ph $(ROOT_PREFIX)/bin

.PHONY: clean
clean:
	rm -f $(obj) ph