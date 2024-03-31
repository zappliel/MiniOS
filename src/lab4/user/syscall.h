#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "../include/types.h"
#include "../arch/riscv/include/proc.h"
#include "../include/printk.h"

void sys_write(unsigned int fd, const char* buf, uint64 count);
unsigned long sys_getpid();

#define SYS_WRITE   64
#define SYS_GETPID  172

#endif