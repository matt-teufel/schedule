SCHEDULE = schedule
SCHEDULE_SRCS = schedule.c	parse.c
SCHEDULE_HDRS = parse.h
SCHEDULE_OBJS = $(SCHEDULE_SRCS:%.c%.o)

CC = gcc
CFLAGS = std=gnu99 -g

default:	$(SCHEDULE)
all: $(SCHEDULE)

$(SCHEDULE_OBJS):	$(SCHEDULE_HDRS)

$(SCHEDULE):	$(SCHEDULE_OBJS)
	$(CC)	-o	$(SCHEDULE)	$(CFLAGS)	$@ $(SCHEDULE_OBJS)

parse.o : parse.c
	$(CC)	$(CFLAGS)	-c	parse.c

#test 1: test parser for colons included in arg strings
#test 2: test parser for > 10 args
test: $(SCHEDULE)
	./$(SCHEDULE) 3000 prog1 a b c : prog2 :a :b : prog3
	./$(SCHEDULE) 3000 prog1 a b c d e f g h i j k : prog2 a b c

clean:
	rm -f $(SCHEDULE_OBJS)
	rm -f $(SCHEDULE)