CC = gcc
CFLAGS := -c -Wall -Werror
INCLUDES := -I.
LDFLAGS := -lpthread

EXECUTABLE = \
	threads_opt_wait \
	threads


.PHONY : all
all : $(EXECUTABLE)


threads_opt_wait : worker_threads_optional_wait.o
	$(CC) $< -o $@ $(LDFLAGS)


threads : worker_threads.o
	$(CC) $< -o $@ $(LDFLAGS)


.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $@


.PHONY : clean
clean :
	rm -f *.o $(EXECUTABLE)
