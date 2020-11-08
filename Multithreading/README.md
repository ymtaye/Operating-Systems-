# Multithreading and Synchronization

## Description
The number given on the command line will determine the #No of threads to spawn from main thread.
After threads are all created, program prompts user for inputs (2 numbers per line representing the message value and ID respectively).
WARNING: Program will seg fault if message is sent to non existing thread. Program stops waiting for input only when EOF is detected; a Termination message will be queued for each thread generated.
For negative values program will simply print error message and proceed to next line input.
Now threads can begin receiving messages and terminate after reading all their corresponding messages. A thread has to be in RUNNING QUEUE to do this and only one thread can receive messages and send a message back to main upon completion at a time.
Synchronization is done mainly through semaphores. Child threads exist and run within the adder() routine. They are given an ID beginning  from 1 as an argument on creation order rank. They use this to access their specific mailbox which can contain only 1 msg. Once a thread receives a msg from it's mailbox it frees it, hence its able to get more messages. 

msg sending handled by RecvMsg() and SendMsg(), each have a producer and consumer type semaphores. Sending being producer in this case.
NOTE: Current running can switch at any time so a mutex was implemented within their routine so that no thread can interrupt while a thread receives or sends a message. Exception to this would be the main thread which could send the next msg to that thread's mailbox if vacant. 
Once all threads receive their stdin messages and exit, the main thread prints their total msg value, count of operations(RecvMsg() and SendMsg(), total time run in secs. 

## COMPILE AND GENERATE EXECUTABLE
```shell script
gcc -pthread -o mt Multithreader.c
```

RUN 
./mt {optional argument values [1-10]: default = 10}

After Being prompted with Value and IDs tabs
enter values and IDs: 2 numbers per line, upto 100 Lines
Use (Ctrl+D || EOF) to signal end of inputs to program

Input? and Output? contain results for different stdin given to programs. They match their corresponding number; eg. Output1 would be result of running Input1.






