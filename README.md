# Simple example of boss/worker pthreads


Simple example code that performs some 'blocking' function inside a worker
thread. The boss creates a number of tasks threads of different types which
will wait until the boss updates their parameters and then 'starts' them on
whatever particular type of task that have been given. The boss at some later
time will block on waiting for the worker threads to finish their tasks. Once
the worker has finished their task it will hold again until the boss re-starts
them.

The worker threads are created and destroyed at the start and end of the
applications life time. Usage its usage is very similar to 'create'/'join'
without the overhead of having to create a new thread each time work is needed.
