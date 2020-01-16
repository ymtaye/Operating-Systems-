YARED TAYE 
Project 2

Intercepting System Calls:

Sept, 13, 2019

PART 1

For this part, I intecepeted the sys_open and sys_close calls. I changed the pointer to the call to reference the new_sys_open and new_sys_close calls. 

Steps: First, make to build kernel module. Next, run sudo insmod Phase1.ko to insert the module. Access updated /var/log/syslog file to make sure users are being recorded in log.

Deliverable:
Text file (Log.txt) containing last 20 lines of var/log/syslog, which demonstrates the successful interception of sys_open and sys_close calls.

Phase1.c - source code containing implementation of the intercepted system calls

Makefile

PART 2

In Phase2, I was able to implement most functionalities. The processinfo struct from the Kernel successfully transfers information to user. 

the processinfo.h file contains the definition for the struct as directed in assignment.

Phase2.c - source code for kernel implementation

UserPhase2.c - User side implementation; 

Steps: First, make to build kernel module. Next, run sudo insmod Phase2.ko to insert the module. Compile the user space code and run the outputted executable. While I was able to transfer all the information from the kernel space to user space with all of them having same values. I wasn't able to print the cutime, cstime, sys_time, user_time fields without getting negative values. Also, sometimes the youngest_child field  gives negative values. All the IDs for current process, user, parent print correctly and are accurate figures. 
