#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "../include/types.h"
#include "../arch/riscv/include/proc.h"
#include "../include/printk.h"

void sys_write(unsigned int fd, const char* buf, uint64 count);
unsigned long sys_getpid();

#define SYS_WRITE   64
#define SYS_GETPID  172

#define SYS_MUNMAP   215
#define SYS_CLONE    220 // fork
#define SYS_MMAP     222
#define SYS_MPROTECT 226

#endif