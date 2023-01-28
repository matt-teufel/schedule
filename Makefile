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

#test 1: test parser for colons included in arg strings
#test 2: test parser for > 10 args
#./$(SCHEDULE) 10000 ./prog1 a b c d e f g h i j k : ./prog2 a b c
test: $(SCHEDULE)
	./$(MAIN) 10000 two 1 : two 2 : two 3
	./$(MAIN) 500 two 1 : two 2 : two 3
	./$(MAIN) 100 two 1 : two 2 : two 3
	./$(MAIN) 1000 prog1 a b c : prog2 :a :b : prog3
