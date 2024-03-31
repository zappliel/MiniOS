#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "../include/types.h"
#include "../arch/riscv/include/proc.h"
#include "../include/printk.h"

struct pt_regs
{
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
    uint64 stval;
    uint64 sscratch;

    uint64 scause;
};

void sys_write(unsigned int fd, const char* buf, uint64 count);
unsigned long sys_getpid();
uint64_t sys_clone(struct pt_regs* regs);

#define SYS_WRITE   64
#define SYS_GETPID  172

#define SYS_MUNMAP   215
#define SYS_CLONE    220 // fork
#define SYS_MMAP     222
#define SYS_MPROTECT 226

extern uint64 uapp_start;
extern uint64 uapp_end;
extern void __ret_from_fork();
extern unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern struct task_struct* current;
extern struct task_struct* task[NR_TASKS];

#endif