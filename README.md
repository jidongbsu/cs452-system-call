# Overview

In this assignment, we will write a Linux kernel module called tesla. Note that, you will only be able to test this assignment on a Linux machine where you have root privilege. A VMware-based CentOS 7 (64 bit) VM image is provided. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with sudo to gain root privilege). You can also download a [CentOS 7 (64 bit) ISO file](http://bay.uchicago.edu/centos/7.9.2009/isos/x86_64/CentOS-7-x86_64-DVD-2009.iso) and install it by yourself, and you can also use VirtualBox - but students have reported that sometimes the VM shows some strange behaviors in VirtualBox+MacBook environments: kernel level debugging messages, when we do expect them to be printed, are not printed.

## Important notes

You MUST build against the kernel version (3.10.0-1160.el7.x86_64) installed on the cs452 VM. You will need to use root in this project is to load and unload the kernel module.

# Specification

Your module will intercept several systems calls, so as to achieve the goal of:
1. hide files whose name contains the string "tesla".
2. hide proccesses whose name contains the string "ssh".

## The Starter Code

The starter code shows an example of how you can intercept a system call. It intercepts sys_kill, and it prevents users to kill a process whose name contains the string "ssh".

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
drwxrwxr-x.  2 cs452 cs452     23 Dec 16 22:04 .tmp_versions
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
drwxrwxr-x.  2 cs452 cs452   23 Dec 16 22:04 .tmp_versions
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

You can use this command to find out what system calls you need to hijack:

strace -ff -o trace sh -c 'ps -ef | grep ssh'

# References

Operating systems: three easy pieces: chapter 6: Direct Execution. The chapter tells you what system calls are and why system calls are needed, but the chapter does not tell you how to intercept system calls. In other words the chapter talks about theory, in this assignment, we tackle the practical side of system calls.

# Submission

Due: 23:59pm, January 25th, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)

- [80 pts] Functional Requirements:
  - Hides Tesla files properly (ls, ls -l, ls -la):    35/35
  - Hides ssh processes (ps -ef, ps -ef | grep ssh, pstree | grep ssh):    35/35
  - Module can be installed and removed without crashing the system: 10/10 (you won't get these points if your module doesn't implement any of the above functional requirements.)

- [10 pts] Compiling
  - Each compiler warning will result in a 3 point deduction.
  - You are not allowed to suppress warnings. (you won't get these points if your module doesn't implement any of the above functional requirements.)

- [10 pts] Documentation:
  - README.md file (replace this current README.md with a new one using the README template, and do not check in this current README file.)
