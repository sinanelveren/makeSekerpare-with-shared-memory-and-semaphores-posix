CC = gcc
DB = gdb
CFLAGS = -o
DFLAGS = -g
MAIN = 111044074_main

PROGNAME = sekerpare


all:
	$(CC) -std=c11  -c $(MAIN).c -lrt -lpthread 
	$(CC) $(MAIN).o  -lrt -lpthread  $(CFLAGS) $(PROGNAME)

debug:
	$(CC) -std=c11 $(DFLAGS) $(MAIN).c $(CFLAGS) $(PROGNAME)
	$(DB) ./$(PROGNAME)


clean:
	rm -f $(PROGNAME) *.o *.log
