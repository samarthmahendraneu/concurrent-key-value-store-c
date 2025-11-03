LDLIBS=-lz -lpthread
CFLAGS=-ggdb3 -Wall

# Add argp-standalone support for macOS
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    ARGP_PREFIX := $(shell brew --prefix argp-standalone 2>/dev/null)
    ifneq ($(ARGP_PREFIX),)
        CFLAGS += -I$(ARGP_PREFIX)/include
        LDLIBS += -L$(ARGP_PREFIX)/lib -largp
    endif
endif

EXES = dbserver dbclient

all: $(EXES)

dbclient: dbclient.o

dbserver: dbserver.o stats.o queue.o kvstore.o worker.o

stats.o: stats.h stats.c

queue.o: queue.h queue.c

kvstore.o: kvstore.h kvstore.c

worker.o: worker.h worker.c

clean:
	rm -f $(EXES) *.o data.[0-9]*
