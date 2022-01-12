# Overview

In this assignment, we will write a Linux kernel module called tesla. Note that, you will only be able to test this assignment on a Linux machine where you have root privilege. A VMware-based CentOS 7 (64 bit) VM image is provided. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with sudo to gain root privilege). You can also download a [CentOS 7 (64 bit) ISO file](http://bay.uchicago.edu/centos/7.9.2009/isos/x86_64/CentOS-7-x86_64-DVD-2009.iso) and install it by yourself, and you can also use VirtualBox - but students have reported that sometimes the VM shows some strange behaviors in VirtualBox+MacBook environments: kernel level debugging messages, when we do expect them to be printed, are not printed.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64) - which is the version of the default kernel installed on the cs452 VM. You will need to use root in this project is to load and unload the kernel module.

# Specification

Your module will intercept several systems calls, so as to achieve the goal of:
1. hide files whose name contains the string "tesla".
2. hide proccesses whose name contains the string "ssh".

## The Starter Code

The starter code shows an example of how you can intercept a system call. It intercepts sys\_kill, and it prevents users from killing a process whose name contains the string "ssh".

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

## APIs

I used the following APIs. When unsure how an API should be called, you are encouraged to search in the Linux kernel source code: https://elixir.bootlin.com/linux/v3.10/source/. You should be able to find use examples in the kernel source code. Below, when I say in "include/linux/syscalls.h", it means a path within the Linux kernel source code tree, in other words, "include/linux/syscalls.h" means "https://elixir.bootlin.com/linux/v3.10/source/include/linux/syscalls.h". 

- kmalloc():

prototype: void \*kmalloc(size\_t size, gfp\_t flags); in kernel space, when allocating memory, you can't use malloc() anymore. malloc() is not available. instead, you use kmalloc(). This function takes two parameters. The first parameter specifies the size (i.e., the number of bytes) you want to allocate, the second parameter is called flags, in this course, you always use GFP\_KERNEL - you can search online to find out what other flags are available and what are the meanings, but throughout this semester, in all your kernel project, this flag is sufficient for you. In other words, in all your kernel projects for this course, whenever you call kmalloc(), the 2nd parameter is always GFP\_KERNEL. kmalloc() returns a void\* pointer, you may want to cast this pointer to other types - just like what you do when calling malloc().

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

- I also used some string operation functions. These are the functions you normally would use in applications, but the Linux kernel provides its own implementation of these functions, which typically have the same prototype as their use space counterparts. So you can just look at the man page to find out how to use these functions. Refer to this file: include/linux/string.h, to find out what string operation functions are available in the kernel space. In theory you should include this string.h header file in your kernel module, but it seems this one is included already by some other header file which is included in the starter code, thus you don't really need to explicitly include this string.h. Depending on how you want to manipulate your strings, different students may choose to use different string operation functions. Since these are all commonly used functions by average C programmers (regardless of application developers or kernel developers), I do not describe them here. Just make sure the ones you choose to use are indeed declared in include/linux/string.h. Side note: whatever functions defined in a user level library can not be used by the kernel, and if some kernel level code wants to use such functions, they need to be refined in the kernel code.

## Functions You Need to Implement

You should use the command strace to trace what system calls are used. You are highly recommended to find some online examples showing you how to use strace, and learn what information strace provides; even though in the following I tell you what system calls you should intercept, you still are recommended to use strace to understand why we are intercepting them - if you don't understand this, you likely won't know what exactly you should do inside your wrapper functions.

The 3 system calls functions you need to intercept are:

asmlinkage long sys\_read(unsigned int fd, char \_\_user \*buf, size\_t count);
asmlinkage long sys\_write(unsigned int fd, const char \_\_user \*buf, size\_t count);
asmlinkage long sys\_getdents(unsigned int fd, struct linux\_dirent \_\_user \*dirent, unsigned int count);

All of them are declared in include/linux/syscalls.h.

To intercept them, you need to implement these 3 wrapper functions.

asmlinkage long tesla\_read(unsigned int fd, char \_\_user \*buf, size\_t count);
asmlinkage long tesla\_write(unsigned int fd, char \_\_user \*buf, size\_t count);
asmlinkage long tesla\_getdents(unsigned int fd, struct linux\_dirent \_\_user \*dirp, unsigned int count);

You can use this command to find out why you need to intercept the sys\_getdents() system call function.

strace -ff -o trace sh -c 'ps -ef | grep ssh'

This strace command will trace child processes as well, so when you run this command it will produce more than one trace file, named "trace.XXX", where XXX is the pid of the process being traced.

# Book References

[Direct Execution](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf). 

The chapter tells you what system calls are, why system calls are needed, and how system calls in general works; but the chapter does not tell you how to intercept system calls. In other words the chapter talks about theory, in this assignment, we tackle the practical side of system calls.

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
