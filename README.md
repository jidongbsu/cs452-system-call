# Overview

In this assignment, we will write a Linux kernel module called tesla. Note that, you will only be able to test this assignment on a Linux machine where you have root privilege. A VMware-based CentOS 7 (64 bit) VM image is provided. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with sudo to gain root privilege). You can also download a [CentOS 7 (64 bit) ISO file](http://bay.uchicago.edu/centos/7.9.2009/isos/x86_64/CentOS-7-x86_64-DVD-2009.iso) and install it by yourself, and you can also use VirtualBox - but students have reported that sometimes the VM shows some strange behaviors in VirtualBox+MacBook environments: kernel level debugging messages, when we do expect them to be printed, are not printed.

## Learning Objectives

- Gain a better understanding of system calls and how they are implemented in a Linux system.
- Get familar with some of the most frequently used system call functions.
- Understand why system calls should be protected and the system call table should be marked as read-only.
- Learn how to use the strace utility to trace system calls.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64) - which is the version of the default kernel installed on the cs452 VM. You will need to use root in this project is to load and unload the kernel module.

Also note that for all kernel level projects, your VM gets frozen is completely expected, just reboot the VM. When you have a bug in your kernel module, it easily gets the VM frozen, you just reboot your VM and debug your code and try to fix your bug. Throughout the entire semester, you likely will reboot your VM more than 100 times...

Four system call functions are mentioned in this assignment description: read(), write(), getdents(), kill(). I use read() and sys\_read() interchangeably, read() is the system call function that applications can call, whereas sys\_read() is the function defined in the kernel which does the work on behalf of read(). So they are literally the same thing. When users call read() in applications, eventually sys\_read() will be called in the kernel. Using the terminology from the textbook chapter, anytime a read() call occurs, a user-mode to kernel-mode transition will happen, and this is what the textbook chapter refers to as a "**trap**". Once the execution is trapped in the kernel mode, the kernel calls sys\_read(). When sys\_read() returns, the execution returns to user mode, and the user-called read() function also returns - this is what the textbook chapter refers to as "**return-from-trap**". Similarly for write(), getdents(), kill(). And I use write()/sys\_write() interchangeably, use getdents()/sys\_getdents() interchangeably, use kill()/sys\_kill() interchangeably. In the textbook terminology, read(), write(), getdents(), kill() are system calls, and sys\_read(), sys\_write(), sys\_getdents(), and sys\_kill() are "**syscall handlers**" or "**trap handlers**". The data structure which stores the addresses of these syscall handlers, is what most people call the system call table, and the book chapter refers to it as the "**trap table**".

## Book References

- Operating Systems: Three Easy Pieces: [Chapter 6: Direct Execution](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf)  

The chapter tells you what system calls are, why system calls are needed, and how system calls in general work; but the chapter does not tell you how to intercept system calls. In other words the chapter talks about theory, in this assignment, we tackle the practical side of system calls.

## What Are Kernel Modules?

Kernel modules are binary code you can install and remove at runtime. "Install" means insert the code into memory. In Linux, we use the command "sudo insmod xxx.ko" to install a kernel module - which usually has "\*.ko" at the end of its file name. "Remove" means remove it from memory. In Linux, we use the command "sudo rmmod xxx.ko" to remove a kernel module. Once the code is inserted into memory, functions defined in the module will be available/accessible to other parts of the kernel, and will be executed if the PC counter is pointing to the address of these functions. Once the module is removed, these functions will disappear and other parts of the kernel will no longer have access to these functions.

The starter code is already a kernel module, which means if you compile - by running "make", you will produce a .ko file, and you should already be able to install it and remove it. However, the functionality of this module is not complete, and that is what you need add to the module.

# Specification

Your module will intercept several systems calls, so as to achieve the goal of:
1. hide files whose name contains the string "tesla".
2. hide proccesses whose name contains the string "ssh".

## The Starter Code

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

As you can see, once the module is installed, the user can not kill the ssh process, whose pid is 4031, and even using the sudo command is still not enough.

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
[cs452@localhost system-call]$ ls
Makefile  tesla.c
[cs452@localhost system-call]$ make clean
/bin/rm --force .tesla* tesla.o tesla.mod.c tesla.mod.o tesla.ko Module.symvers modules.order
/bin/rm -fr .tmp_versions/
[cs452@localhost system-call]$ make
make -C /lib/modules/`uname -r`/build M=`pwd` modules
make[1]: Entering directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
  CC [M]  /home/cs452/system-call/tesla.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/cs452/system-call/tesla.mod.o
  LD [M]  /home/cs452/system-call/tesla.ko
make[1]: Leaving directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
[cs452@localhost system-call]$ ls
Makefile  modules.order  Module.symvers  tesla.c  tesla.ko  tesla.mod.c  tesla.mod.o  tesla.o
[cs452@localhost system-call]$ ls -l
total 608
-rw-r--r--. 1 cs452 cs452    242 Dec 16 16:33 Makefile
-rw-rw-r--. 1 cs452 cs452     40 Dec 16 22:04 modules.order
-rw-rw-r--. 1 cs452 cs452      0 Dec 16 22:04 Module.symvers
-rw-r--r--. 1 cs452 cs452   7769 Dec 16 22:04 tesla.c
-rw-rw-r--. 1 cs452 cs452 296952 Dec 16 22:04 tesla.ko
-rw-rw-r--. 1 cs452 cs452   1860 Dec 16 22:04 tesla.mod.c
-rw-rw-r--. 1 cs452 cs452  59800 Dec 16 22:04 tesla.mod.o
-rw-rw-r--. 1 cs452 cs452 240664 Dec 16 22:04 tesla.o
[cs452@localhost system-call]$ ls -la
total 688
drwxrwxr-x.  3 cs452 cs452    235 Dec 16 22:04 .
drwx------. 17 cs452 cs452   4096 Dec 16 22:04 ..
-rw-r--r--.  1 cs452 cs452    242 Dec 16 16:33 Makefile
-rw-rw-r--.  1 cs452 cs452     40 Dec 16 22:04 modules.order
-rw-rw-r--.  1 cs452 cs452      0 Dec 16 22:04 Module.symvers
-rw-r--r--.  1 cs452 cs452   7769 Dec 16 22:04 tesla.c
-rw-rw-r--.  1 cs452 cs452 296952 Dec 16 22:04 tesla.ko
-rw-rw-r--.  1 cs452 cs452    207 Dec 16 22:04 .tesla.ko.cmd
-rw-rw-r--.  1 cs452 cs452   1860 Dec 16 22:04 tesla.mod.c
-rw-rw-r--.  1 cs452 cs452  59800 Dec 16 22:04 tesla.mod.o
-rw-rw-r--.  1 cs452 cs452  27103 Dec 16 22:04 .tesla.mod.o.cmd
-rw-rw-r--.  1 cs452 cs452 240664 Dec 16 22:04 tesla.o
-rw-rw-r--.  1 cs452 cs452  43022 Dec 16 22:04 .tesla.o.cmd
drwxrwxr-x.  2 cs452 cs452     23 Dec 16 22:04 .tmp\_versions
[cs452@localhost system-call]$ sudo insmod tesla.ko
[cs452@localhost system-call]$ ls
Makefile  modules.order  Module.symvers
[cs452@localhost system-call]$ ls -l
total 8
-rw-r--r--. 1 cs452 cs452 242 Dec 16 16:33 Makefile
-rw-rw-r--. 1 cs452 cs452  40 Dec 16 22:04 modules.order
-rw-rw-r--. 1 cs452 cs452   0 Dec 16 22:04 Module.symvers
[cs452@localhost system-call]$ ls -la
total 12
drwxrwxr-x.  3 cs452 cs452  235 Dec 16 22:04 .
drwx------. 17 cs452 cs452 4096 Dec 16 22:04 ..
-rw-r--r--.  1 cs452 cs452  242 Dec 16 16:33 Makefile
-rw-rw-r--.  1 cs452 cs452   40 Dec 16 22:04 modules.order
-rw-rw-r--.  1 cs452 cs452    0 Dec 16 22:04 Module.symvers
drwxrwxr-x.  2 cs452 cs452   23 Dec 16 22:04 .tmp\_versions
[cs452@localhost system-call]$ sudo rmmod tesla.ko
[cs452@localhost system-call]$
```
As you can see, at first you have two files: tesla.c and Makefile. After running make, you will have 8 files plus some hidden files - files whose name starts with a period, but with a "ls -la" command, you still can see these hidden files.

After installing your kernel module, all files whose name contains the string of "tesla" are now hidden, and you won't see these files no matter you run "ls", "ls -l", or "ls -a".

## Hiding Processes

This is what you should achieve:

```console
[cs452@localhost system-call]$ ps -ef | grep ssh
root      3348     1  0 23:11 ?        00:00:00 /usr/sbin/sshd -D
root      4035  3348  0 23:11 ?        00:00:00 sshd: cs452 [priv]
cs452     4040  4035  0 23:11 ?        00:00:00 sshd: cs452@pts/0
cs452     6836  4042  0 23:39 pts/0    00:00:00 grep --color=auto ssh
[cs452@localhost system-call]$ pstree | grep ssh
        |-sshd---sshd---sshd---bash-+-grep
[cs452@localhost system-call]$ make clean
/bin/rm --force .tesla* tesla.o tesla.mod.c tesla.mod.o tesla.ko Module.symvers modules.order
/bin/rm -fr .tmp_versions/
[cs452@localhost system-call]$ make
make -C /lib/modules/`uname -r`/build M=`pwd` modules
make[1]: Entering directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
  CC [M]  /home/cs452/system-call/tesla.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/cs452/system-call/tesla.mod.o
  LD [M]  /home/cs452/system-call/tesla.ko
make[1]: Leaving directory `/usr/src/kernels/3.10.0-957.el7.x86_64'
[cs452@localhost system-call]$ sudo insmod tesla.ko
[cs452@localhost system-call]$ ps -ef | grep ssh
[cs452@localhost system-call]$ pstree | grep ssh
[cs452@localhost system-call]$ sudo rmmod tesla.ko
[cs452@localhost system-call]$ 
```

As you can see, at first you can see there are ssh processes. After installing your kernel module, all ssh processes disappear.

## What Does Intercept Mean?

When you run ls or ps, these commands will call some system call functions. These system call functions are stored in the kernel memory and there are pointers pointing to these functions. If you can change these pointers, and make them point to your functions, then the default functions will not be called. Instead, your functions will be called. Look at the starter code,

```c
    orig_kill= (void *)sys_call_table[__NR_kill];
    sys_call_table[__NR_kill] = (long *)tesla_kill;
```

sys\_call\_table[] - this array stores the system call table, which stores all the pointers for all the system call functions. sys\_call\_table[\_\_NR\_kill] - this element of the array stores the address of the sys\_kill() system call function. So what the above two lines are doing are: first save the default system call's address into a function pointer called orig\_kill, and then assign a new function pointer, whose name is tesla\_kill, to the array element.

After these two lines, when applications call kill, they won't be calling the default kill, rather, it is the tesla\_kill() function which will be called. And then, if the goal of tesla\_kill() is to prevent ssh from being killed, then we can use tesla\_kill() as a wrapper function of the original sys\_kill() function. Below is the main code of the tesla\_kill() function.

```c
    target = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
    if(target){
        if(strstr(target->comm, "ssh")){
            return -EACCES;
        }
    }

    ret=orig_kill(pid, sig);
```

"pid" is the first parameter they pass to sys\_kill(), which tells us which process the user wants to kill. Now, because tesla\_kill() is the wrapper of sys\_kill(), that means all parameters which are supposed to be passed to sys\_kill() are now passed to tesla\_kill(). So the first line of the above code snippet uses this pid to search in a list and find out the process name, if the process name contains "ssh", then we just return -EACCES, an error code in the kernel level, indicating permission denied. Thus the process will not be killed. But the last line here is also important, for regular processes, we still want them to be killed - otherwise, the kill command is becoming completely useless - if it can't kill anything. That's why here we call orig\_kill(pid, sig), which basically is calling the original sys\_kill() function - remember we just saved its address above, into the function pointer orig\_kill(). We saved it before, and now it's time to use it, and we just pass the same parameters to orig\_kill(), i.e., whatever passed to tesla\_kill() will now be passed to orig\_kill().

This above example, shows how you can intercept a system call. You basically want the kernel to call your wrapper function first, and then in your wrapper function, you call the original system call() only when it's needed.

## Kernel APIs and Global Variables

I used the following APIs. When unsure how an API should be called, you are encouraged to search in the Linux kernel source code: https://elixir.bootlin.com/linux/v3.10/source/. You should be able to find use examples in the kernel source code. Below, when I say in "include/linux/syscalls.h", it means a path within the Linux kernel source code tree, in other words, "include/linux/syscalls.h" means "https://elixir.bootlin.com/linux/v3.10/source/include/linux/syscalls.h". 

- kmalloc():

prototype: void \*kmalloc(size\_t size, gfp\_t flags); in kernel space, when allocating memory, you can't use malloc() anymore. malloc() is not available. Instead, you use kmalloc(). This function takes two parameters. The first parameter specifies the size (i.e., the number of bytes) you want to allocate, the second parameter is called flags, in this course, you always use GFP\_KERNEL - you can search online to find out what other flags are available and what are the meanings, but throughout this semester, in all your kernel project, this flag is sufficient for you. In other words, in all your kernel projects for this course, whenever you call kmalloc(), the 2nd parameter is always GFP\_KERNEL. kmalloc() returns a void\* pointer, you may want to cast this pointer to other types - just like what you do when calling malloc().

- kfree():

prototype: void kfree(const void * objp); this function frees previously allocated memory. whatever pointer returned by kmalloc(), should then be passed to kfree(), so the memory will be released. In this aspect, kmalloc()/kfree() is similar to malloc()/free(). kfree() does not return anything.

- copy\_from\_user():

prototype: unsigned long copy\_from\_user (void * to, const void \_\_user * from, unsigned long n); copy a block of data from user space to kernel space. This function returns number of bytes that could not be copied. On success, this will be zero. You don't really need to use the return value of this function if it's not successful, but you do need to check the return value of this function to make sure it is successful, otherwise you should return -EFAULT, an error code which says "bad address", indicating "an invalid user space address was specified for an argument".

The system call functions you are going to intercept, take parameters from applications, which are programs running in user space; when these parameters are pointers, they point to a user-space address, such addresses are not accessible to kernel code. Or at least it is not safe for kernel to directly access a user-space address. In order to access the data pointed by such pointers, we need to use this copy\_from\_user() function to copy the data into kernel space. in other words, the user space has a buffer, now in your kernel module, which runs in the kernel space, you need to create another buffer, and use this copy\_from\_user() to copy the user space buffer into kernel space, and then your kernel level code can access your kernel space buffer. In the prototype, the pointer "to" points to your kernel buffer, the pointer "from" points to a user space buffer.

how do you find out the user buffer? look at the prototype of sys\_read() system call (in include/linux/syscalls.h), see the 2nd parameter, is the user space buffer pointer. 

asmlinkage long sys\_read(unsigned int fd, char \_\_user \*buf, size\_t count);

This is one of the system calls you are going to intercept, so you need to, use copy\_from\_user(), to pass the data from this "buf" to your buffer - the buffer you define in your kernel module.

- copy\_to\_user():

prototype: unsigned long copy\_to\_user(void \_\_user \*to, const void \*from, unsigned long n); whatever described above for copy\_from\_user() is still applicable to this function. It's just the copying direction is the opposite. Sometimes you want to copy some data from user space to kernel space, sometimes you want to copy some data from kernel space to user space. Just make sure you understand that in copy\_from\_user(), the 2nd parameter "from" points to a user space buffer, but in copy\_to\_user(), it is the 1st first parameter "to" which points to user space buffer - see that "\_\_user" keyword, it tells the compiler this is a user space pointer.

- I also used some string operation functions. These are the functions you normally would use in applications, but the Linux kernel provides its own implementation of these functions, which typically have the same prototype as their use space counterparts. So you can just look at the man page to find out how to use these functions. Refer to this file: include/linux/string.h, to find out what string operation functions are available in the kernel space. In theory you should include this string.h header file in your kernel module, but it seems this one is included already by some other header file which is included in the starter code, thus you don't really need to explicitly include this string.h. Depending on how you want to manipulate your strings, different students may choose to use different string operation functions. Since these are all commonly used functions by average C programmers (regardless of application developers or kernel developers), I do not describe them here. Just make sure the ones you choose to use are indeed declared in include/linux/string.h. Side note: whatever functions defined in a user level library can not be used by the kernel, and if some kernel level code wants to use such functions, they need to be re-defined in the kernel code.

- global variables: Linux kernel defines a global variable called current, which is a struct task\_struct pointer, points to a struct task\_struct which represents the current running process. This global variable current is accessible to any kernel level code and you may want to use it. In addition, the task\_struct has hundreds of fields, one of them is called "comm", which is the command which was used to launch this process. You can see how this field is used in tesla\_kill() and then decide how you want to use it.

- other global variables: \_\_NR_read, \_\_NR_write, \_\_NR_getdents are the indices in the system call table for read(), write(), getdents() system calls, these indices are what the textbook chapter refers to as the "**system-call number**". You can use them the same way as the starter code uses \_\_NR_kill. Also, the sys_call_table itself is of course a global variable, and is used in the starter code - see tesla\_init(), which has the following lines:

```c
/* search in kernel symbol table and find the address of sys_call_table */
	sys_call_table = (long **)kallsyms_lookup_name("sys_call_table");
 
	if (sys_call_table == NULL) {
		printk(KERN_ERR "where the heck is the sys_call_table?\n");
		return -1;
	}
```

These above lines retrieve the address of the system call table and after that, the kernel module can use sys\_call\_table, a pointer defined in tesla.h, to access the system call table. In tesla\_init() - the function that is executed when you install the kernel module, you should modify the system call table so that your wrapper functions will be called when the user calls the corresponding system call functions. In tesla\_cleanup() - the function that is executed when you remove the kernel module, you should modify the system call table so as to restore the original system call functions.

These following lines in tesla\_init(), using the kill() system call as an example,

```c
/* save the original kill system call into orig_kill, and replace the kill system call with tesla_kill */
	orig_kill= (void *)sys_call_table[__NR_kill];
	sys_call_table[__NR_kill] = (long *)tesla_kill;
```

And these following lines in tesla\_exit(),
```c
  	/* restore the kill system call to its original version */
	sys_call_table[__NR_kill] = (long *)orig_kill;
```

Tells you how you should modify the system call table.

## Functions You Need to Implement

You should use the command strace to trace what system calls are used. You are highly recommended to find some online examples showing you how to use strace, and learn what information strace provides; even though in the following I tell you what system calls you should intercept, you still are recommended to use strace to understand why we are intercepting them - if you don't understand this, you likely won't know what exactly you should do inside your wrapper functions.

The 3 system calls functions you need to intercept are:

```c
asmlinkage long sys_read(unsigned int fd, char __user *buf, size_t count);
asmlinkage long sys_write(unsigned int fd, const char __user *buf, size_t count);
asmlinkage long sys_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);
```

All of them are declared in include/linux/syscalls.h.

To intercept them, you need to implement these 3 wrapper functions.

```c
asmlinkage long tesla_read(unsigned int fd, char __user *buf, size_t count);
asmlinkage long tesla_write(unsigned int fd, char __user *buf, size_t count);
asmlinkage long tesla_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);
```

You can use this command to find out why you need to intercept the sys\_getdents() system call function.

strace -ff -o trace sh -c 'ps -ef | grep ssh'

This strace command will trace child processes as well, so when you run this command it will produce more than one trace file, named "trace.XXX", where XXX is the pid of the process being traced.

## What Exactly Does sys_getdents() Do?

You should easily be able to understand what read()/write() system calls do. So the tricky part of this assignment is to understand what getdents() system call does, or what sys_getdents() function does.

sys_getdents() is the kernel counterpart of the system call function getdents(). When users call getdents(), eventually in the kernel space, sys_getdents() is the function gets called to complete the task on behalf of the user.

The prototype of getdents is:

```c
int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
```

If you have a file, whose name is “abc”, and if its content is “12345”, then on your disk, there will be a data block storing the content of this file.

If you have a directory called “foo”, which contains 1 subdirectory (“bar”) and 2 files (”abc”, and “defgh”). Then on your disk, there will be a data block storing the names of the subdirectory and files, but not storing the content of the subdirectory or files – the subdirectory and the files have their own data blocks.

The data block owned by this directory "foo" could therefore look like this (this is just an example):

```c
5, d_off0, d_reclen0, “.”
2, d_off1, d_reclen1, “..”
12, d_off2, d_reclen2, “bar”
13, d_off3, d_reclen3, “abc”
24, d_off4, d_reclen4, “defgh”
```

In above, 5, 2, 12, 13, 24, are called inodes. In Linux systems, we assign one number to each file or directory, this number is called inode – index node - kernel uses this number to differentiate files (in Linux, directories are also considered as files, it’s just a special type of files). Using inode to differentiate files is like using pid to differentiate processes. 

Each row in the above data block is called a directory entry, or dentry. Each row is represented by one struct linux_dirent (see tesla.h for its definition). To access these entries, users call getdents() and pass the file descriptor of "foo" as the first parameter to getdents(), which will eventually call sys_getdents() in the kernel level. sys_getdents() returns the total size of all of these entries. For example, if these 5 entries in total occupy 60 bytes, then sys_getdents() returns 60 bytes, and upon return, the 2nd parameter of sys_getdents() – dirp, which is a pointer, points to the first entry. The 3rd parameter of sys_getdents(), which is count, is the size of a buffer pointed to by dirp.

What if you want to access the 2nd entry? 

```c
(struct linux_dirent*)((char *)dirp + d_reclen0);
```

In struct linux_dirent, d_reclen represents the length of this entry. The lengths of two entries could be different mainly because the length of their file names may be different. d_off represents the distance from the start of the directory (the start of the directory is the address of its first entry, i.e., the first struct linux_dirent) to the start of the next struct linux_dirent. In this assignment, you likely won't even need to access this d_off field. Also, you don't really need to access the inode number; but you will access d_reclen as well as the file name - i.e., the d_name field of the struct linux_dirent.

You are recommended in your tesla_getdents() function to first call the original sys_getdents(), which will setup the dirp pointer, which will be pointing to the starting address of a user-space buffer which contains the above 5 entries (note: 5 entries is just an example). After that you can manipulate these entries to achieve your goal.

## Debugging

Ideally, you should setup kgdb which allows you to use gdb to debug kernel, but this requires you to do some research online and find out how to setup it for your specific environment (VMware vs VirtualBox, Windows vs Linux vs MacOS). It may take some time, but you will benefit from it given that there are 5 kernel projects in total throughout the semester.

Without setting up kgdb, a simpler but less efficient debugging technique is using printk() to print messages. Note that the kernel print messages will not show on the screen. The messages are, however, logged in the file /var/log/messages. You can open another terminal and watch the output to the system messages file with the command:

```console
sudo tail -f /var/log/messages
```
Alternatively,  you can use the command **sudo dmesg --follow** to watch for kernel log messages.

# Submission

Due: 23:59pm, January 25th, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)

- [80 pts] Functional Requirements:
  - Hides Tesla files properly			/35
    - hides against 'ls',			(10 pts)
    - hides against 'ls -l',			(10 pts)
    - hides against 'ls -la'.			(15 pts)
  - Hides ssh processes				/35
    - hides against 'ps -ef',			(10 pts)
    - hides against 'ps -ef | grep ssh', 	(10 pts)
    - hides against 'pstree | grep ssh'.    	(15 pts)
  - Module can be installed and removed without crashing the system: /10 
    - you won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Compiling
  - Each compiler warning will result in a 3-point deduction.
  - You are not allowed to suppress warnings. (you won't get these points if your module doesn't implement any of the above functional requirements.)

- [10 pts] Documentation:
  - README.md file (replace this current README.md with a new one using the README template, and do not check in this current README file.)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
