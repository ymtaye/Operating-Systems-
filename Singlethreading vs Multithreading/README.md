#Project 4

Yared Taye 

This project was implementation of file reading system using single and multi threads.

first complile and make executables running make. This will generate executable proj4, which can be run with 2 additional arguments(argv[1] = thread, argv[2] = {numThreads}) this is the multi threaded version. User can simply call ./proj4 to run serial version of this project.

This program takes input from STDIN
ls -1d * | ./proj4  ----------works!

OUTPUTS
csv file contain 100 iterations of program run with No addition threads, 5, 10, 15.... They output NumThreads,SYStime, USER time,
SERIAL VERSION
./proj4

For this part, user simply calls executable as directed above, a check for argc will be done with main function to make sure only one argument is passed. If passed, program will continue to fgets call which is implemented in a while loop in order to detect a EOF input.
getStats() and getUserStats are run on the got file, these generate the Byte size counts and File type counts respectively. Next, after EOF is detected function exits loop and prints acquired stats. Finally, it will print systime and user time for task.

MULTITHREADED VERSION

./proj4 thread {#numThreads} 

-thread tells program which version to run, numThreads will determine how many maximum no of threads user wants program to run at a given time

In this version, main will check for keyword thread to make sure of the version being run. Next will check numThreads, if none defaults to 15. Also, won't let user input > 15 value which also defaults. Now this part runs similary as serial version as while loops for every file name recieved, however, within this number of current threads active will be checked against MAX(users input). When threads active is less than MAX a pthread_create will be called, this will invoke the reader() starter routine and will feed it current filename being iterated. When pthreads running reach maximum, a join will be invoked for all active threads. However, there will be a pthread_create for that specific file being read in while loop. Once EOF is detected, the while loop will exit and a 2nd join will be made for all remaining threads.
Printer() is then called to retrieve stats.
getStats is called within the reader() routine; this is done while processing thread has set a lock on the region and only unlocks once it completes getting stats.

