LDLIBS=-lz -lpthread
CFLAGS=-ggdb3 -Wall

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
