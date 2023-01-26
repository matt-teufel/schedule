SCHEDULE = schedule
SCHEDULE_SRCS = schedule.c	parse.c
SCHEDULE_HDRS = parse.h
SCHEDULE_OBJS = $(SCHEDULE_SRCS:%.c%.o)

CC = gcc
CFLAGS = std=gnu99 -g

default:	$(SCHEDULE)

$(SCHEDULE_OBJS):	$(SCHEDULE_HDRS)

$(SCHEDULE):	$(SCHEDULE_OBJS)
	$(CC)	$(CFLAGS)	-o	$@ $(SCHEDULE_OBJS)
