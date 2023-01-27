CC = gcc
CFLAGS = -Wall -g -std=gnu99
MAIN = sch
OBJS = schedule.o parse.o

all: $(MAIN)

$(MAIN) : $(OBJS) parse.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

parse.o : parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

schedule.o : schedule.c parse.h
	$(CC) $(CFLAGS) -c schedule.c

clean :
	rm *.o $(MAIN)
