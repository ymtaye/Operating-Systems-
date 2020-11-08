/**

Yared Taye @ ymtaye@wpi.edu

**/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <asm/current.h>
#include <asm/cputime.h>
#include <asm/uaccess.h>
#include <asm/errno.h>



#include "processinfo.h"


unsigned long **sys_call_table;       //Pointer for syscall table 
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);
void TotalCPUTime(struct task_struct * Curr, struct processinfo * kinfo);
pid_t getYoungSibling(struct task_struct* Curr, struct task_struct* younger_sibling);
pid_t getOlderSibling(struct task_struct* Curr, struct task_struct* older_sibling);
pid_t getYoungestChild(struct task_struct* Curr, struct task_struct* youngest_child);




asmlinkage long new_sys_cs3013_syscall2(struct processinfo *info){
	
	struct processinfo kinfo;	//Process info struct in kernel	wth fields initialized
	kinfo.state = -1 ;		//
	kinfo.pid =  -1 ;		//
	kinfo.parent_pid  = -1 ;
	kinfo.youngest_child = -1;	//
	kinfo.younger_sibling = -1;	//
	kinfo.older_sibling  = -1;	//
	kinfo.uid = -1 ;		//
	kinfo.start_time = 0;		//	
	kinfo.user_time = 0;		//
	kinfo.sys_time = 0;		//
	kinfo.cutime = 0;		//
	kinfo.cstime = 0;		//
	struct task_struct * sibling = NULL;
	struct task_struct * youngest_child = NULL;
	struct task_struct * older_sibling = NULL;
	struct task_struct * younger_sibling = NULL;
	
	kinfo.state = current->state;
	kinfo.pid = current->pid;
	kinfo.uid = current_uid().val;
	kinfo.parent_pid = current->parent->pid;


	//Assign values for CPU and Start times for current process
	kinfo.start_time = timespec_to_ns(&current->start_time);
	kinfo.user_time = cputime_to_usecs(current->utime);
	kinfo.sys_time = cputime_to_usecs(current->stime);	
	
	TotalCPUTime(current, &kinfo);   //Accumlates total CPU time of children in user space and system space
	
	kinfo.younger_sibling = getYoungSibling(current, younger_sibling);   //Retrieves the younger sibling
	kinfo.older_sibling = getOlderSibling(current, older_sibling);	// Retrieves the older sibling
	kinfo.youngest_child = getYoungestChild(current, youngest_child);	//Retrieves youngest child 
	//Check Process info Structure before sending to User
	
	printk("------------------------Inside Kernel-------------------------\n");   
	printk("--------------------------------------------------------------\n");
		//Printning the process information
	printk("---------------User ID: %d is running process.-----------------\n",  kinfo.uid);
	printk("--------------------------------------------------------------\n");
	printk("Current state of process---------------------: %14d\n", kinfo.state);
	printk("Process ID of current process----------------: %14d\n", kinfo.pid);
	printk("Process ID of parent-------------------------: %14d\n", (int)kinfo.parent_pid);
	printk("Process ID of Youngest Child-----------------: %14d\n", (int)kinfo.youngest_child);
	printk("Process ID of Older Sibling------------------: %14d\n", kinfo.older_sibling);
	printk("Process ID of Younger Sibling----------------: %14d\n", kinfo.younger_sibling);
	printk("Process start time---------------------------: %20lld nanoseconds\n", kinfo.start_time);
	printk("CPU time spent in User mode------------------: %14lld microseconds\n", (unsigned long) kinfo.user_time);
	printk("CPU time spent in System mode----------------: %14lld microseconds\n", (unsigned long) kinfo.sys_time);
	printk("CPU time spent in User mode for all Children-: %14lld microseconds\n", (unsigned long) kinfo.cutime);
	printk("CPU time spent in System mode for all Children: %14lld microseconds\n", (unsigned long) kinfo.cstime);
	

	if(copy_to_user(info, &kinfo, sizeof kinfo))
		return EFAULT;
	
}

void TotalCPUTime(struct task_struct * Curr, struct processinfo * kinfo){
		struct task_struct * child = NULL;

	list_for_each_entry(child, &(Curr->children), children){
		kinfo->cutime = kinfo->cutime + cputime_to_usecs(child->utime);
		kinfo->cstime = kinfo->cstime + cputime_to_usecs(child->stime);
	}
}

pid_t getYoungestChild(struct task_struct* Curr, struct task_struct* youngest_child){
		struct task_struct * child;
	//Checking wether process has any siblings at all
	if(list_empty(&(Curr->children))){
    	printk("Not reading Children Table");
		return -1;
		}
	youngest_child = list_first_entry(&(Curr->children), struct task_struct, children);
	list_for_each_entry(child, &(Curr->children), children){
		if(!(timespec_compare(&(youngest_child->start_time),&(child->start_time)) > 0)){
			youngest_child = child;
	}
}
	return youngest_child->pid;	
}


pid_t getYoungSibling( struct task_struct* Curr, struct task_struct* younger_sibling){
		struct task_struct * child = NULL;
	//Checking wether process has any siblings at all
	if(list_empty(&(Curr->sibling))){
		return -1;}
	younger_sibling = list_first_entry(&(Curr->sibling), struct task_struct, sibling);
	list_for_each_entry(child, &(Curr->sibling), sibling){
		if(!(timespec_compare(&(younger_sibling->start_time),&(child->start_time)) > 0)){
			younger_sibling = child;
	}
}
	return younger_sibling->pid;	


}

pid_t getOlderSibling(struct task_struct* Curr, struct task_struct* older_sibling){
		struct task_struct * child = NULL;
	//Checking wether process has any siblings at all
	if(list_empty(&(Curr->sibling))){
		return -1;}
	older_sibling = list_first_entry(&(Curr->sibling), struct task_struct, sibling);
	list_for_each_entry(child, &(Curr->sibling), sibling){
		if(!(timespec_compare(&(older_sibling->start_time),&(child->start_time)) > 0)){
			older_sibling = child;
	}
}	return older_sibling->pid;	
}


static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO  "Interceptor: Found syscall table at address: 0x%02lX",
                (unsigned long) sct);

      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
   */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];



  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;  // Replacing the pointer to syscall1 with a pointer to a new function.

  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;      // Setting the system call open to operate as before

  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);

module_exit(interceptor_end);
