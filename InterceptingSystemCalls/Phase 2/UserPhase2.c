/**

Yared Taye @ ymtaye@wpi.edu

**/



#include  <sys/syscall.h>
#include  <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "processinfo.h"




#define __NR_cs3013_syscall2 378



long cs3013_syscall2(struct processinfo* info);
void Printer(struct processinfo* info);

int main(){
	struct processinfo info;
	
	int PID = fork();
	if(PID == 0){
		printf("---------------------CHILD---------------%ld-----\n", (long) getpid());   

			

	}else if(PID > 0){
	cs3013_syscall2(&info); 
 		 Printer(&info);
		
		wait(0);
		printf("---------------------PARENT--------------------\n");   
	
	}


	
	
	return 0;
}
void Printer(struct processinfo* info){

	printf("------------------------Inside user space--------------------\n");   
	printf("--------------------------------------------------------------\n");
		//Printning the process information
	printf("---------------User ID: %d is running process.-----------------\n",  info->uid);
	printf("--------------------------------------------------------------\n");
	printf("Current state of process---------------------: %14d\n", info->state);
	printf("Process ID of current process----------------: %14d\n", info->pid);
	printf("Process ID of parent-------------------------: %14d\n", (int)info->parent_pid);
	printf("Process ID of Youngest Child-----------------: %14d\n", (int)info->youngest_child);
	printf("Process ID of Older Sibling------------------: %14d\n", info->older_sibling);
	printf("Process ID of Younger Sibling----------------: %14d\n", info->younger_sibling);
	printf("Process start time---------------------------: %20lld nanoseconds\n", info->start_time);
	printf("CPU time spent in User mode------------------: %14lld microseconds\n", (unsigned long) info->user_time);
	printf("CPU time spent in System mode----------------: %14lld microseconds\n", (unsigned long) info->sys_time);
	printf("CPU time spent in User mode for all Children-: %14lld microseconds\n", (unsigned long) info->cutime);
	printf("CPU time spent in System mode for all Children: %14lld microseconds\n", (unsigned long) info->cstime);

	}

long cs3013_syscall2(struct processinfo* info){
	return (long) syscall(__NR_cs3013_syscall2, info);
	}


