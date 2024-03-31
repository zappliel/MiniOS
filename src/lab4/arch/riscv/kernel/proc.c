//arch/riscv/kernel/proc.c
#include "proc.h"
#include "stringt.h"
#include "stdint.h"
#include "mm.h"
#include "elf.h"
#include "test.h"
#include "vm.h"
#define OFFSET(TYPE , MEMBER) ((unsigned long)(&(((TYPE *)0)->MEMBER)))

const uint64 OffsetOfThreadInTask = (uint64)OFFSET(struct task_struct, thread);
const uint64 OffsetOfRaInTask = OffsetOfThreadInTask + (uint64)OFFSET(struct thread_struct, ra);
const uint64 OffsetOfSpInTask = OffsetOfThreadInTask + (uint64)OFFSET(struct thread_struct, sp);
const uint64 OffsetOfSInTask = OffsetOfThreadInTask + (uint64)OFFSET(struct thread_struct, s);

//arch/riscv/kernel/proc.c

extern void __dummy();

extern void __switch_to(struct task_struct* prev, struct task_struct* next);
extern int printk(const char *, ...);

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组, 所有的线程都保存在此

extern unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern uint64 uapp_start;
extern uint64 uapp_end;

/**
 * new content for unit test of 2023 OS lab2
*/
extern uint64 task_test_priority[]; // test_init 后, 用于初始化 task[i].priority 的数组
extern uint64 task_test_counter[];  // test_init 后, 用于初始化 task[i].counter  的数组

static uint64_t load_program(struct task_struct* task) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&uapp_start;
    uint64_t phdr_start = (uint64_t)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;
    Elf64_Phdr* phdr;
    int load_phdr_cnt = 0;
    for (int i = 0; i < phdr_cnt; i++) {
        phdr = (Elf64_Phdr*)(phdr_start + sizeof(Elf64_Phdr) * i);
        if (phdr->p_type == PT_LOAD) {
        uint64 phdr_num = PGROUNDUP((uint64)phdr->p_memsz + phdr->p_vaddr - PGROUNDDOWN(phdr->p_vaddr)) / PGSIZE;
        uint64 *phdr_temp = (uint64 *)alloc_pages(phdr_num);
        memcpyt((void *)((uint64)phdr_temp + phdr->p_vaddr - PGROUNDDOWN(phdr->p_vaddr)), 
        (void*)((uint64)&uapp_start + (uint64)phdr->p_offset), (uint64)phdr->p_memsz);
        create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), (uint64)PGROUNDDOWN(phdr->p_vaddr), 
        (uint64)phdr_temp-PA2VA_OFFSET, (uint64)phdr_num, (uint64)phdr->p_flags << 1 | 0b10001);
        }
    }

    uint64 user_stack = alloc_page();
    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_END-PGSIZE, user_stack-PA2VA_OFFSET, 1, 0b10111);
    task->thread.sepc = ehdr->e_entry;
    task->thread.sstatus = csr_read(sstatus);
    task->thread.sstatus &= ~(1<<8);
    task->thread.sstatus |= 1<<18 | 1<<5;
    task->thread.sscratch = USER_END;
}

static uint64_t load(struct task_struct* task) {
    //用户栈分配
    uint64 user_stack = alloc_page();
    //映射uapp和u_mode stack
    uint64 user_pnum = PGROUNDUP(((uint64)&uapp_end - (uint64)&uapp_start))/PGSIZE;
    uint64 *user_temp = (uint64 *)alloc_pages(user_pnum);
    memcpyt((void*)user_temp, (void*)&uapp_start, user_pnum*PGSIZE);

    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_START, (uint64)user_temp-PA2VA_OFFSET, user_pnum, 31);        
    
    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_END-PGSIZE, user_stack-PA2VA_OFFSET, 1, 23);
    
    task->thread.sepc = USER_START;
    task->thread.sstatus = csr_read(sstatus);
    task->thread.sstatus &= ~(1<<8);
    task->thread.sstatus |= 1<<18 | 1<<5;
    task->thread.sscratch = USER_END;
    return 0;
}

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle

    /* YOUR CODE HERE */
    idle = (struct task_struct*)kalloc();  //分配物理页
    idle->state = TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;
    idle->pid = 0;
    current = idle;     //当前进程(最开始)为idle
    task[0] = idle;     //task[0]即idle

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址

    /* YOUR CODE HERE */
    for(int i=1; i < NR_TASKS; i++)
    {
        //为进程分配空间（一个物理页）
        task[i] = (struct task_struct*)kalloc();
        task[i]->counter = task_test_counter[i];
        task[i]->priority = task_test_priority[i];
        task[i]->pid = i;
        task[i]->state = TASK_RUNNING;
        task[i]->thread.ra = (uint64)&__dummy;
        task[i]->thread.sp = (uint64)((uint64)task[i]+PGSIZE);
        task[i]->pgd = (pagetable_t)(alloc_page()-PA2VA_OFFSET);       //存的是物理地址
        memcpyt((void*)((uint64)task[i]->pgd+PA2VA_OFFSET), (void*)(swapper_pg_dir), PGSIZE);           //将内核态页表复制到用户进程

        load(task[i]);
        // load_program(task[i]);
        
    }
        // 1. 参考 idle 的设置, 为 task[1] ~task[NR_TASKS - 1] 进行初始化
        // 2. 其中每个线程的 state 为 TASK_RUNNING, 此外，为了单元测试的需要，counter 和 priority 进行如下赋值：
        //      task[i].counter  = task_test_counter[i];
        //      task[i].priority = task_test_priority[i];
        // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
        // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址

        /* YOUR CODE HERE */
    //    printk("%d\n",OffsetOfThreadInTask);
    //    printk("%d\n",OffsetOfRaInTask);
    //    printk("%d\n",OffsetOfSpInTask);
    //    printk("%d\n",OffsetOfSInTask);
    

    printk("...proc_init done!\n");
}

// arch/riscv/kernel/proc.c

void dummy() {
    //    schedule_test();
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while (1) {
        if ((last_counter == -1 || current->counter != last_counter) && current->counter > 0) {
            if (current->counter == 1) {
                --(current->counter);   // forced the counter to be zero if this thread is going to be scheduled
            }                           // in case that the new counter is also 1，leading the information not printed.
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. thread space begin at 0x%016lx\n", current->pid, current);
            //            printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
            //            printk("counter=%d\n",current->counter);
        }
    }
}

extern void __switch_to(struct task_struct* prev, struct task_struct* next);

void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */
//    printk("进行切换\n");
    if (current == next) {
        //        printk("不用切换\n");
        return;
    }
    else {
        //        printk("[PID = %d]\n", next->pid);
        struct task_struct* pre;
        pre = current;
        current = next;
        __switch_to(pre, next);
        //        printk("[PID = %d] is running\n", current->pid);
        //         printk("pid=%d     counter=%d\n",current->pid,current->counter);
    }

}

void do_timer(void) {
    // 1. 如果当前线程是 idle 线程 直接进行调度
    // 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间仍然大于0 则直接返回 否则进行调度
//    switch_to(task[2]);
//    printk("[PID = %d] is running\n", current->pid);
//    printk("123123123\n");
    /* YOUR CODE HERE */
//    for(int i=0;i<NR_TASKS;i++){
//        printk("pid=%d     counter=%d\n",task[i]->pid,task[i]->counter);
//    }
    //printk("(do_timer)pid=%d,counter=%d\n",current->pid,current->counter);
    if (current->pid == 0) {
        schedule();
        return;
    }
    else {
        if (current->counter <= 1) {
            //            printk("counter=%d 进入调度函数\n",current->counter);
            current->counter = 0;
            schedule();
        }
        else {
            current->counter = current->counter - 1;
            //            printk("不进入调度函数\n");
        }
    }
}

void SJF_schedule(void) {
    //    printk("现在在调度\n");
    //    for(int i=0;i<NR_TASKS;i++){
    //        printk("pid=%d,counter=%d\n",task[i]->pid,task[i]->counter);
    //    }
        /* YOUR CODE HERE */
    struct task_struct* next = ((void*)0);
    int min = -1;
    int mini = -1;
    for (int i = 1; i < NR_TASKS; i++) {
        if (min == -1 && task[i]->counter > 0) {
            min = task[i]->counter;
            mini = i;
            next = task[i];
        }

        else if (task[i]->counter < min && task[i]->counter>0) {
            mini = i;
            min = task[i]->counter;
            next = task[i];
        }
    }

    if (min == -1) {
        for (int i = 1; i < NR_TASKS; i++) {
            task[i]->counter = 1;
            //printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
        }
        schedule();
    }
    else {
        if (next) {
            //printk("\nswitch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
            switch_to(next);
        }
    }


}


void Priority_schedule() {
    uint64 i, next, c;
    struct task_struct** p;
    while (1) {
        c = 0;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        while (--i) {
            if (!*--p)
                continue;
            if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
                c = (*p)->counter, next = i;
        }
        if (c) break;
        printk("\n");
        for (p = &task[NR_TASKS - 1]; p > &task[0]; --p)
            if (*p) {
                (*p)->counter = ((*p)->counter >> 1) +
                    (*p)->priority;
                //printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", (*p)->pid, (*p)->priority, (*p)->counter);
            }
    }
    //printk("\nswitch to [PID = %d PRIORITY = %d COUNTER = %d]\n", task[next]->pid, task[next]->priority, task[next]->counter);
    switch_to(task[next]);
}

void schedule() {
#ifdef SJF
    SJF_schedule();
#endif

#ifdef PRIORITY
    Priority_schedule();
#endif
}