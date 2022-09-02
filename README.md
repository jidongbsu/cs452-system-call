# Overview

In this assignment, we will write a Linux kernel module called tesla. Note that, you will only be able to test this assignment on a Linux machine where you have the root privilege. A VMware-based CentOS 7 (64 bit) VM image is therefore provided - you should be able to use this image no matter you use VMware or Virtualbox, they both recognize this image format. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with *sudo* to gain root privilege). 

## Learning Objectives

- Gain a better understanding of system calls and how they are implemented in a Linux system.
- Get familiar with some of the most frequently used system call functions.
- Understand why system calls should be protected and the system call table should be marked as read-only.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64) - which is the version of the default kernel installed on the cs452 VM. You will need to use the root privilege in this project is to load and unload the kernel module.

You are not allowed to use the function *memmove*() in this assignment - because a solution to this assignment is available on the internet, and that solution uses the *memmove*() function. Using *memmove*() in your code therefore is considered as cheating.

Also note that for all kernel level projects, your VM gets frozen is completely expected, just reboot the VM. When you have a bug in your kernel module, it easily gets the VM frozen, you just reboot your VM and debug your code and try to fix your bug. Throughout the entire semester, you likely will reboot your VM more than 100 times...

## Book References

- Operating Systems: Three Easy Pieces: [Chapter 6: Direct Execution](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf)  

The chapter tells you what system calls are, why system calls are needed, and how system calls in general work; but the chapter does not tell you how to intercept system calls. In other words the chapter talks about theory, in this assignment, we tackle the practical side of system calls.

Two system call functions are mentioned in this assignment description: *getdents*(), *kill*(). I use *kill*() and *sys_kill*() interchangeably, *kill*() is the system call function that applications can call, whereas *sys_kill*() is the function defined in the kernel which does the work on behalf of *kill*(). So they are literally the same thing. When users call *kill*() in applications, eventually *sys_kill*() will be called in the kernel. Using the terminology from the textbook chapter, anytime a *kill*() call occurs, a user-mode to kernel-mode transition will happen, and this is what the textbook chapter refers to as a "**trap**". Once the execution is trapped in the kernel mode, the kernel calls *sys_kill*(). When *sys_kill*() returns, the execution returns to user mode, and the user-called *kill*() function also returns - this is what the textbook chapter refers to as "**return-from-trap**". In the textbook terminology, *getdents*(), *kill*() are system calls, and *sys_getdents*(), and *sys_kill*() are "**syscall handlers**" or "**trap handlers**". The data structure which stores the addresses of these syscall handlers, is what most people call the **system call table**, and the book chapter refers to it as the "**trap table**".

## What Are Kernel Modules?

Kernel modules are binary code you can install and remove at runtime. "Install" means insert the code into memory. In Linux, we use the command "*sudo insmod xxx.ko*" to install a kernel module - which usually has "\*.ko" at the end of its file name. "Remove" means remove it from memory. In Linux, we use the command "*sudo rmmod xxx.ko*" to remove a kernel module. Once the code is inserted into memory, functions defined in the module will be available/accessible to other parts of the kernel, and will be executed if the PC counter is pointing to the address of these functions. Once the module is removed, these functions will disappear and other parts of the kernel will no longer have access to these functions.

The starter code is already a kernel module, which means if you compile - by running "*make*", you will produce a .ko file, and you should already be able to install it and remove it. However, the functionality of this module is not complete, and that is what you need add to the module. In the starter code, *tesla_init*() is the function that will be executed when you install the module, and *tesla_exit*() is the function that will be executed when you remove the module. In a Linux kernel module, the following two lines of code is how you tell the kernel, which function you want to run when the module is installed, and which function you want to run when the module is removed.

```c
module_init(tesla_init);
module_exit(tesla_exit);
```

# Specification

Your module will intercept a systems call, so as to achieve the goal of hiding files whose name contains the string "tesla".

## The Starter Code

The starter code looks like this.

```console
[cs452@localhost cs452-system-call]$ ls
Makefile  README.md  README.template  tesla.c  tesla.h  test1  test2  test3  test4
```

You should only modify the file *tesla.c*. The four directories: test1, test2, test3, and test4, will be used to test your code.

The starter code shows an example of how you can intercept a system call. It intercepts the kill() system call, so as to prevent users from killing a process whose name contains the string "ssh".

```console
[cs452@localhost system-call]$ make
make -C /lib/modules/`uname -r`/build M=`pwd` modules
make[1]: Entering directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
  CC [M]  /home/cs452/system-call-start/tesla.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/cs452/system-call-start/tesla.mod.o
  LD [M]  /home/cs452/system-call-start/tesla.ko
make[1]: Leaving directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
[cs452@localhost system-call]$ sudo insmod tesla.ko
[cs452@localhost system-call]$ whoami
cs452
[cs452@localhost system-call]$ ps -ef | grep ssh
root      3343     1  0 03:22 ?        00:00:00 /usr/sbin/sshd -D
root      3876  3343  0 03:22 ?        00:00:00 sshd: cs452 [priv]
cs452     4031  3876  0 03:22 ?        00:00:00 sshd: cs452@pts/0
cs452     4954  4036  0 03:38 pts/0    00:00:00 grep --color=auto ssh
[cs452@localhost system-call]$ kill -9 4031
-bash: kill: (4031) - Permission denied
[cs452@localhost system-call]$ sudo kill -9 4031
kill: sending signal to 4031 failed: Permission denied
[cs452@localhost system-call]$ 
```

As you can see, once the module is installed, the user can not kill the ssh process, whose pid is 4031, and even using the *sudo* command is still not enough.

```console
[cs452@localhost system-call]$ sudo rmmod tesla.ko
[cs452@localhost system-call-start]$ kill -9 4031
Connection to 192.168.56.103 closed by remote host.
Connection to 192.168.56.103 closed.
```

And then as you can see, once the module is removed, the user can now kill the ssh process - the ssh connection is then lost.

## Hiding Files

This is what you should achieve:

```console
[cs452@localhost cs452-system-call]$ ls
Makefile  README.md  README.template  tesla.c  tesla.h  test1  test2  test3  test4
[cs452@localhost cs452-system-call]$ make clean
/bin/rm --force .tesla* tesla.o tesla.mod.c tesla.mod.o tesla.ko Module.symvers modules.order
/bin/rm -fr .tmp_versions/
[cs452@localhost cs452-system-call]$ make
make -C /lib/modules/`uname -r`/build M=`pwd` modules
make[1]: Entering directory `/usr/src/kernels/3.10.0-1160.el7.x86_64'
  CC [M]  /home/cs452/cs452-system-call/tesla.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/cs452/cs452-system-call/tesla.mod.o
  LD [M]  /home/cs452/cs452-system-call/tesla.ko
make[1]: Leaving directory `/usr/src/kernels/3.10.0-1160.el7.x86_64'
[cs452@localhost cs452-system-call]$ ls
Makefile  modules.order  Module.symvers  README.md  README.template  tesla.c  tesla.h  tesla.ko  tesla.mod.c  tesla.mod.o  tesla.o  test1  test2  test3  test4
[cs452@localhost cs452-system-call]$ ls -l
total 844
-rw-rw-r-- 1 cs452 cs452    241 Aug 23 14:18 Makefile
-rw-rw-r-- 1 cs452 cs452     51 Aug 23 17:13 modules.order
-rw-rw-r-- 1 cs452 cs452      0 Aug 23 17:13 Module.symvers
-rw-rw-r-- 1 cs452 cs452  30195 Aug 23 14:18 README.md
-rw-rw-r-- 1 cs452 cs452   1899 Aug 23 14:18 README.template
-rw-rw-r-- 1 cs452 cs452   7677 Aug 23 16:29 tesla.c
-rw-rw-r-- 1 cs452 cs452   1275 Aug 23 16:26 tesla.h
-rw-rw-r-- 1 cs452 cs452 278992 Aug 23 17:13 tesla.ko
-rw-rw-r-- 1 cs452 cs452   1548 Aug 23 17:13 tesla.mod.c
-rw-rw-r-- 1 cs452 cs452  59672 Aug 23 17:13 tesla.mod.o
-rw-rw-r-- 1 cs452 cs452 222840 Aug 23 17:13 tesla.o
drwxrwxr-x 2 cs452 cs452     66 Aug 23 15:53 test1
drwxrwxr-x 2 cs452 cs452     54 Aug 23 15:53 test2
drwxrwxr-x 2 cs452 cs452     50 Aug 23 15:54 test3
drwxrwxr-x 2 cs452 cs452     73 Aug 23 15:54 test4
[cs452@localhost cs452-system-call]$ ls -la
total 928
drwxrwxr-x   8 cs452 cs452   4096 Aug 23 17:13 .
drwx------. 38 cs452 cs452   4096 Aug 23 17:12 ..
drwxrwxr-x   8 cs452 cs452    163 Aug 23 14:18 .git
-rw-rw-r--   1 cs452 cs452    241 Aug 23 14:18 Makefile
-rw-rw-r--   1 cs452 cs452     51 Aug 23 17:13 modules.order
-rw-rw-r--   1 cs452 cs452      0 Aug 23 17:13 Module.symvers
-rw-rw-r--   1 cs452 cs452  30195 Aug 23 14:18 README.md
-rw-rw-r--   1 cs452 cs452   1899 Aug 23 14:18 README.template
-rw-rw-r--   1 cs452 cs452   7677 Aug 23 16:29 tesla.c
-rw-rw-r--   1 cs452 cs452   1275 Aug 23 16:26 tesla.h
-rw-rw-r--   1 cs452 cs452 278992 Aug 23 17:13 tesla.ko
-rw-rw-r--   1 cs452 cs452    251 Aug 23 17:13 .tesla.ko.cmd
-rw-rw-r--   1 cs452 cs452   1548 Aug 23 17:13 tesla.mod.c
-rw-rw-r--   1 cs452 cs452  59672 Aug 23 17:13 tesla.mod.o
-rw-rw-r--   1 cs452 cs452  27419 Aug 23 17:13 .tesla.mod.o.cmd
-rw-rw-r--   1 cs452 cs452 222840 Aug 23 17:13 tesla.o
-rw-rw-r--   1 cs452 cs452  42751 Aug 23 17:13 .tesla.o.cmd
drwxrwxr-x   2 cs452 cs452     66 Aug 23 15:53 test1
drwxrwxr-x   2 cs452 cs452     54 Aug 23 15:53 test2
drwxrwxr-x   2 cs452 cs452     50 Aug 23 15:54 test3
drwxrwxr-x   2 cs452 cs452     73 Aug 23 15:54 test4
drwxrwxr-x   2 cs452 cs452     23 Aug 23 17:13 .tmp_versions
[cs452@localhost cs452-system-call]$ sudo insmod tesla.ko
[sudo] password for cs452: 
[cs452@localhost cs452-system-call]$ ls
Makefile  modules.order  Module.symvers  README.md  README.template  test1  test2  test3  test4
[cs452@localhost cs452-system-call]$ ls -l
total 44
-rw-rw-r-- 1 cs452 cs452   241 Aug 23 14:18 Makefile
-rw-rw-r-- 1 cs452 cs452    51 Aug 23 17:13 modules.order
-rw-rw-r-- 1 cs452 cs452     0 Aug 23 17:13 Module.symvers
-rw-rw-r-- 1 cs452 cs452 30195 Aug 23 14:18 README.md
-rw-rw-r-- 1 cs452 cs452  1899 Aug 23 14:18 README.template
drwxrwxr-x 2 cs452 cs452    66 Aug 23 15:53 test1
drwxrwxr-x 2 cs452 cs452    54 Aug 23 15:53 test2
drwxrwxr-x 2 cs452 cs452    50 Aug 23 15:54 test3
drwxrwxr-x 2 cs452 cs452    73 Aug 23 15:54 test4
[cs452@localhost cs452-system-call]$ ls -la
total 52
drwxrwxr-x   8 cs452 cs452  4096 Aug 23 17:13 .
drwx------. 38 cs452 cs452  4096 Aug 23 17:12 ..
drwxrwxr-x   8 cs452 cs452   163 Aug 23 14:18 .git
-rw-rw-r--   1 cs452 cs452   241 Aug 23 14:18 Makefile
-rw-rw-r--   1 cs452 cs452    51 Aug 23 17:13 modules.order
-rw-rw-r--   1 cs452 cs452     0 Aug 23 17:13 Module.symvers
-rw-rw-r--   1 cs452 cs452 30195 Aug 23 14:18 README.md
-rw-rw-r--   1 cs452 cs452  1899 Aug 23 14:18 README.template
drwxrwxr-x   2 cs452 cs452    66 Aug 23 15:53 test1
drwxrwxr-x   2 cs452 cs452    54 Aug 23 15:53 test2
drwxrwxr-x   2 cs452 cs452    50 Aug 23 15:54 test3
drwxrwxr-x   2 cs452 cs452    73 Aug 23 15:54 test4
drwxrwxr-x   2 cs452 cs452    23 Aug 23 17:13 .tmp_versions
[cs452@localhost cs452-system-call]$ sudo rmmod tesla
```
As you can see, after running make, you will have several files, some of the file names contain "tesla", and others do not contain. After installing your kernel module, all files whose name contains the string of "tesla" are now hidden, and you won't see these files no matter you run "ls", "ls -l", or "ls -la".

## What Does Intercept Mean?

When you run *ls* or *kill*, these commands will call some system call functions. These system call functions are stored in the kernel memory and there are pointers pointing to these functions. If you can change these pointers, and make them point to your functions, then the default functions will not be called. Instead, your functions will be called. Look at the starter code,

```c
    orig_kill= (void *)sys_call_table[__NR_kill];
    sys_call_table[__NR_kill] = (long *)tesla_kill;
```

**sys_call_table**[] - this array stores the system call table, which stores all the pointers for all the system call functions. **sys_call_table[__NR_kill]** - this element of the array stores the address of the *sys_kill*() system call function. So what the above two lines are doing are: first save the default system call's address into a function pointer called *orig_kill*(), and then assign a new function pointer, whose name is *tesla_kill*, to the array element.

After these two lines, when applications call kill, they won't be calling the default kill, rather, it is the *tesla_kill*() function which will be called. And then, if the goal of *tesla_kill*() is to prevent ssh from being killed, then we can use *tesla_kill*() as a wrapper function of the original *sys_kill*() function. Below is the main code of the *tesla_kill*() function.

```c
    target = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
    if(target){
        if(strstr(target->comm, "ssh")){
            return -EACCES;
        }
    }

    ret=orig_kill(pid, sig);
```

"pid" is the first parameter they pass to *sys_kill*(), which tells us which process the user wants to kill. Now, because *tesla_kill*() is the wrapper of *sys_kill*(), that means all parameters which are supposed to be passed to *sys_kill*() are now passed to *tesla_kill*(). So the first line of the above code snippet uses this pid to search in a list and find out the process name, if the process name contains "ssh", then we just return -EACCES, an error code in the kernel level, indicating permission denied. Thus the process will not be killed. But the last line here is also important, for regular processes, we still want them to be killed - otherwise, the kill command is becoming completely useless - if it can't kill anything. That's why here we call *orig_kill*(pid, sig), which basically is calling the original *sys_kill*() function - remember we just saved its address above, into the function pointer *orig_kill*(). We saved it before, and now it's time to use it, and we just pass the same parameters to *orig_kill*(), i.e., whatever passed to *tesla_kill*() will now be passed to *orig_kill*().

This above example, shows how you can intercept a system call. You basically want the kernel to call your wrapper function first, and then in your wrapper function, you call the original system call function only when it's needed.

## Kernel APIs and Global Variables

I used the following APIs. When unsure how an API should be called, you are encouraged to search in the Linux kernel source code: https://elixir.bootlin.com/linux/v3.10/source/. You should be able to find use examples in the kernel source code. Below, when I say in "include/linux/syscalls.h", it means a path within the Linux kernel source code tree, in other words, "include/linux/syscalls.h" means "https://elixir.bootlin.com/linux/v3.10/source/include/linux/syscalls.h". 

- kmalloc():

prototype: 

```c
void *kmalloc(size_t size, gfp_t flags);
```

in kernel space, when allocating memory, you can't use malloc() anymore. malloc() is not available. Instead, you use kmalloc(). This function takes two parameters. The first parameter specifies the size (i.e., the number of bytes) you want to allocate, the second parameter is called flags, in this course, you always use GFP\_KERNEL - you can search online to find out what other flags are available and what are the meanings, but throughout this semester, in all your kernel project, this flag is sufficient for you. In other words, in all your kernel projects for this course, whenever you call kmalloc(), the 2nd parameter is always GFP\_KERNEL. kmalloc() returns a void\* pointer, you may want to cast this pointer to other types - just like what you do when calling malloc().

- kfree():

prototype:

```c
void kfree(const void * objp);
```

this function frees previously allocated memory. whatever pointer returned by kmalloc(), should then be passed to kfree(), so the memory will be released. In this aspect, kmalloc()/kfree() is similar to malloc()/free(). kfree() does not return anything.

- copy\_from\_user():

prototype:

```c
unsigned long copy_from_user (void * to, const void __user * from, unsigned long n);
```

copy a block of data from user space to kernel space. This function returns number of bytes that could not be copied. On success, this will be zero. You don't really need to use the return value of this function if it's not successful, but you do need to check the return value of this function to make sure it is successful, otherwise you should return -EFAULT, an error code which says "bad address", indicating "an invalid user space address was specified for an argument".

The system call functions you are going to intercept, take parameters from applications, which are programs running in user space; when these parameters are pointers, they point to a user-space address, such addresses are not accessible to kernel code. Or at least it is not safe for kernel to directly access a user-space address. In order to access the data pointed by such pointers, we need to use this copy\_from\_user() function to copy the data into kernel space. in other words, the user space has a buffer, now in your kernel module, which runs in the kernel space, you need to create another buffer, and use this copy\_from\_user() to copy the user space buffer into kernel space, and then your kernel level code can access your kernel space buffer. In the prototype, the pointer "to" points to your kernel buffer, the pointer "from" points to a user space buffer.

how do you find out the user buffer? look at the prototype of sys\_getdents() system call (in include/linux/syscalls.h), see the 2nd parameter, is the user space buffer pointer. 

```c
asmlinkage long sys_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);
```

so you need to, use copy\_from\_user(), to pass the data from this "dirent" to your buffer - the buffer you define in your kernel module.

- copy\_to\_user():

prototype:

```c
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
```

whatever described above for copy\_from\_user() is still applicable to this function. It's just the copying direction is the opposite. Sometimes you want to copy some data from user space to kernel space, sometimes you want to copy some data from kernel space to user space. Just make sure you understand that in copy\_from\_user(), the 2nd parameter "from" points to a user space buffer, but in copy\_to\_user(), it is the 1st first parameter "to" which points to user space buffer - see that "\_\_user" keyword, it tells the compiler this is a user space pointer.

- I also used the string operation function *strstr*() to check if a buffer contains a substring. You normally would use *strstr*() in applications, but the Linux kernel provides its own implementation of this same function, which has the same prototype as its use space counterpart. Refer to this file: include/linux/string.h, to find out what string operation functions are available in the kernel space. In theory you should include this string.h header file in your kernel module, but it seems this one is included already by some other header file which is included in the starter code, thus you don't really need to explicitly include this string.h. The starter code actually has an example of how to use *strstr*() - see *tesla_kill*().

- other global variables: **__NR_getdents** is the index in the system call table for the getdents() system call, this index is what the textbook chapter refers to as the "**system-call number**". You can use it the same way as the starter code uses **__NR_kill**. Also, the *sys_call_table* itself is of course a global variable, and is used in the starter code - see *tesla_init*(), which has the following lines:

```c
/* search in kernel symbol table and find the address of sys_call_table */
	sys_call_table = (long **)kallsyms_lookup_name("sys_call_table");
 
	if (sys_call_table == NULL) {
		printk(KERN_ERR "where the heck is the sys_call_table?\n");
		return -1;
	}
```

These above lines retrieve the address of the system call table and after that, the kernel module can use **sys_call_table**, a pointer defined in tesla.h, to access the system call table. In *tesla_init*() - the function that is executed when you install the kernel module, you should modify the system call table so that your wrapper functions will be called when the user calls the corresponding system call functions. In *tesla_exit*() - the function that is executed when you remove the kernel module, you should modify the system call table so as to restore the original system call functions.

These following lines in *tesla_init*(), using the kill() system call as an example,

```c
/* save the original kill system call into orig_kill, and replace the kill system call with tesla_kill */
	orig_kill= (void *)sys_call_table[__NR_kill];
	sys_call_table[__NR_kill] = (long *)tesla_kill;
```

And these following lines in *tesla_exit*(),
```c
  	/* restore the kill system call to its original version */
	sys_call_table[__NR_kill] = (long *)orig_kill;
```

Tells you how you should modify the system call table.

## Functions You Need to Implement

In this assignment, you will slightly change *tesla_init*() and *tesla_exit*(), but you will spend the majority of your time working on intercepting the system call function *sys_getdents*(). This function is declared in include/linux/syscalls.h:

```c
asmlinkage long sys_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);
```

To intercept it, you need to implement a wrapper function.

```c
asmlinkage long tesla_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);
```

And in your wrapper function (i.e., *tesla_getdents*()), you call the original system call function (i.e., *sys_getdents*()).

## What Exactly Does *sys_getdents*() Do?

*sys_getdents*() is the kernel counterpart of the system call function *getdents*(). When users call *getdents*(), eventually in the kernel space, *sys_getdents*() is the function gets called to complete the task on behalf of the user.

The prototype of *getdents*() is:

```c
int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
```

If you have a file, whose name is “abc”, and if its content is “12345”, then on your disk, there will be a data block storing the content of this file.

But the *ls* command is more about directories: *ls* is a tool to list directory contents. If you have a directory called “foo”, which contains 1 subdirectory (“bar”) and 2 files (”tesla”, and “abc”). Then on your disk, there will be a data block storing the names of the subdirectory and files, but not storing the content of the subdirectory or files – the subdirectory and the files have their own data blocks.

The data block owned by this directory "foo" could therefore look like this (this is just an example):

<!--|ino| offset | record len| file name|-->
|   |        |           |          |
|---|--------|-----------|----------|
|5  | d_off0 | d_reclen0 |  ".”     |
|2  | d_off1 | d_reclen1 |  “..”    |
|12 | d_off2 | d_reclen2 |  “bar”   |
|13 | d_off3 | d_reclen3 |  “tesla” |
|24 | d_off4 | d_reclen4 |  “abc”   |

This is like a table, which right now has 5 rows. In above, 5, 2, 12, 13, 24, are called inodes. In Linux systems, we assign one number to each file or directory, this number is called inode – index node - kernel uses this number to differentiate files (in Linux, directories are also considered as files, it’s just a special type of files).

Each row in the above data block is called a directory entry, or dentry. Each row is represented by one *struct linux_dirent*, which is defined as:

```c
struct linux_dirent {
        unsigned long   d_ino;	/* Inode number */
        unsigned long   d_off;	/* Distance from the start of the directory to the start of the next linux_dirent */
        unsigned short  d_reclen;	/* Length of this linux_dirent */
        char            d_name[1];	/* Filename (null-terminated), variable width. */
};
```

To access these entries, users call *getdents*() and pass the file descriptor of "foo" as the first parameter to *getdents*(), which will eventually call *sys_getdents*() in the kernel level. *sys_getdents*() returns the total size of all of these entries, i.e., the current total size of this whole table. For example, if these 5 entries in total occupy 60 bytes, then *sys_getdents*() returns 60 bytes, and upon return, the 2nd parameter of *sys_getdents*() – **dirp**, which is a pointer, points to the first entry of this table. The 3rd parameter of *sys_getdents*(), which is *count*, is the size of a buffer pointed to by **dirp**.

What if you want to access the 2nd entry? 

```c
(struct linux_dirent*)((char *)dirp + d_reclen0);
```

In *struct linux_dirent*, *d_reclen* represents the length of this entry. The lengths of two entries could be different mainly because the length of their file names may be different. *d_off* represents the distance from the start of the directory (the start of the directory is the address of its first entry, i.e., the first *struct linux_dirent*) to the start of the next *struct linux_dirent*. In this assignment, you likely won't even need to access this *d_off* field. Also, you don't really need to access the inode number; but you will access *d_reclen* as well as the file name - i.e., the *d_name* field of the *struct linux_dirent*.

You are recommended in your *tesla_getdents*() function to first call the original *sys_getdents*(), which will setup the **dirp** pointer, which will be pointing to the starting address of a user-space buffer which contains the above 5 entries (note: 5 entries is just an example). After that you can manipulate these entries to achieve your goal, which is hiding files. The idea is, *sys_getdents*() returns this table to *ls* - store the table in the aforementioned user-space buffer, and this table contains 5 entries, but if you do not want the tesla entry to be reported, then you can just remove this tesla entry from the table, so that *ls* will display every other entry but won't display this tesla entry. In summary, what we are going to do is lying to the *ls* command. So how to lie to *ls*? Or in other words, how do we remove that tesla entry? There are two approaches:

Approach 1: move the entries after the tesla entry one step forward, i.e., move the *abc* entry one step forward, so as to overwrite the tesla entry.

Approach 2: change the *d_reclen* field of the *bar* entry, i.e., change *d_reclen2* to (*d_reclen2* + *d_reclen3*).

Either approach should work for us, but if we are not allowed to call *memmove*(), then probably approach 2 is easier. **Credit**: this second approach was originally proposed and implemented by Ross Rippee who took this class from me in spring 2022.

## Debugging

Ideally, you should setup kgdb which allows you to use gdb to debug kernel, but this requires you to do some research online and find out how to setup it for your specific environment (VMware vs VirtualBox, Windows vs Linux vs MacOS). It may take some time, but you will benefit from it given that there are 5 kernel projects in total throughout the semester.

Without setting up kgdb, a simpler but less efficient debugging technique is using *printk*() to print messages. Note that the kernel print messages will not show on the screen. The messages are, however, logged in the file /var/log/messages. You can open another terminal and watch the output to the system messages file with the command:

```console
sudo tail -f /var/log/messages
```
Alternatively,  you can use the command **sudo dmesg --follow** to watch for kernel log messages.

## More Testing

After implementation, you first test *ls*, *ls -l*, *ls -la* in the starter code directory. Once they all work, you can then use the 4 test directories to test your code. This is the expected behavior when the module is not loaded vesus the module is loaded:

```console
[cs452@localhost cs452-system-call-test]$ ls
Makefile  modules.order  Module.symvers  README.md  README.template  tesla.c  tesla.h  tesla.ko  tesla.mod.c  tesla.mod.o  tesla.o  test1  test2  test3  test4
[cs452@localhost cs452-system-call-test]$ ls test1
tesla.1  tesla.2  tesla.3  tesla.4
[cs452@localhost cs452-system-call-test]$ ls test2
tesla.5  xxx  yyy  zzz
[cs452@localhost cs452-system-call-test]$ ls test3
aaa  bbb  ccc  ddd
[cs452@localhost cs452-system-call-test]$ ls test4
abc  tesla.a  tesla.b  tesla.c  xyz
[cs452@localhost cs452-system-call-test]$ sudo insmod tesla.ko
[sudo] password for cs452: 
[cs452@localhost cs452-system-call-test]$ ls test1
[cs452@localhost cs452-system-call-test]$ ls test2
xxx  yyy  zzz
[cs452@localhost cs452-system-call-test]$ ls test3
aaa  bbb  ccc  ddd
[cs452@localhost cs452-system-call-test]$ ls test4
abc  xyz
[cs452@localhost cs452-system-call-test]$ sudo rmmod tesla
```

As you can see, the names of all files in *test1* contains "tesla", thus they should not be displayed at all; in test2, tesla.5 should be hidden; in test3, all files should be displayed, as there is no *tesla" in their names; in test4, the first file "abc* and the last file "xyz" should be displayed.

# Submission

Due: 23:59pm, September 1st, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)

- [80 pts] Functional Requirements:
  - Hides Tesla files properly			/70
    - hides against 'ls' in the starter code directory.			(10 pts)
    - hides against 'ls -l' in the starter code directory.		(10 pts)
    - hides against 'ls -la' in the starter code directory.		(10 pts)
    - when module is loaded, test1 directory is displayed as expected.	(10 pts)
    - when module is loaded, test1 directory is displayed as expected.	(10 pts)
    - when module is loaded, test1 directory is displayed as expected.	(10 pts)
    - when module is loaded, test1 directory is displayed as expected.	(10 pts)
  - Module can be installed and removed without crashing the system: /10 
    - you won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Compiling:
  - Each compiler warning will result in a 3-point deduction.
  - You are not allowed to suppress warnings. (you won't get these points if your module doesn't implement any of the above functional requirements.)

- [10 pts] Documentation:
  - README.md file (rename this current README file to README.orig and rename the README.template to README.md)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
