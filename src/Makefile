CC = gcc
CFLAGS := -c -Wall -Werror
INCLUDES := -I.
LDFLAGS := -lpthread


.PHONY : all
all : threads


threads : worker_threads.o
	$(CC) $< -o $@ $(LDFLAGS)


.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $@


.PHONY : clean
clean :
	rm -f *.o threads
