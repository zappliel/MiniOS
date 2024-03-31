#include "syscall.h"
#include "printk.h"
#include "mm.h"
#include "defs.h"
#include "vm.h"

extern uint64 task_test_priority[];

//64号系统调用--打印
void sys_write(unsigned int fd, const char* buf, uint64 count) {
    if(fd == 1)    {
        //输出字符
        for(uint64 i=0; i<count; i++){
            printk("%c",buf[i]);
        }
    }
}

//172号系统调用--获取进程id
extern struct task_struct* current;


uint64 sys_getpid() {
    return current->pid;
}

uint64_t sys_clone(struct pt_regs* regs) {
    /*
     1. 参考 task_init 创建一个新的 task, 将的 parent task 的整个页复制到新创建的
        task_struct 页上(这一步复制了哪些东西?）。将 thread.ra 设置为
        __ret_from_fork, 并正确设置 thread.sp
        (仔细想想，这个应该设置成什么值?可以根据 child task 的返回路径来倒推)

     2. 利用参数 regs 来计算出 child task 的对应的 pt_regs 的地址，
        并将其中的 a0, sp, sepc 设置成正确的值(为什么还要设置 sp?)

     3. 为 child task 申请 user stack, 并将 parent task 的 user stack
        数据复制到其中。

     4. 为 child task 分配一个根页表，并仿照 setup_vm_final 来创建内核空间的映射

     5. 根据 parent task 的页表和 vma 来分配并拷贝 child task 在用户态会用到的内存

     6. 返回子 task 的 pid
    */
    int i;
    for (i = 1; i < NR_TASKS; i++) {
        if (task(i) == NULL) { 
            break; 
        }
    }
    if (i == NR_TASKS) {
        printk("No available pid");
        return 0;
    }

    struct task_struct* task_child = (struct task_struct*)kalloc();
    memcpy(task_child, get_current_task(), PGSIZE);
    task_child->pid = i;
    task_child->state = TASK_RUNNING;
    task_child->counter = 0;
    task_child->priority = task_test_priority[i];
    task_child->pid = i;
    task_child->thread.ra = (uint64)(&__ret_from_fork);
    task_child->thread.sp = (uint64)task[i] + (uint64)regs - (uint64)current;
    task_child->thread.ra = (uint64)__ret_from_fork;


    struct pt_regs* pt_regs_child = (struct pt_regs*)(task_child->thread.sp);
    memcpy(pt_regs_child, regs, sizeof(struct pt_regs));
    pt_regs_child->x[10] = 0;                      // a0
    pt_regs_child->x[2] = (uint64)pt_regs_child;  // sp
    pt_regs_child->sepc = regs->sepc + 4;         // sepc

    task_child->thread.sp = (uint64)pt_regs_child; 

    
    task_child->pgd = (pagetable_t)kalloc();
    memcpy(task_child->pgd, swapper_pg_dir, PGSIZE);

    
    uint64 task_child_user_stack = kalloc();
    memcpy((char*)task_child_user_stack, (char*)(USER_END - PGSIZE), PGSIZE);
    create_mapping(task_child->pgd, USER_END - PGSIZE, task_child_user_stack - PA2VA_OFFSET, PGSIZE,
        PTE_USER | PTE_WRITE | PTE_READ | PTE_VALID);

    
    for (i = 0; i < get_current_task()->vma_cnt; i++) {
        struct vm_area_struct vma = get_current_task()->vmas[i];
        uint64 vm_addr_curr = vma.vm_start;
        while (vm_addr_curr < vma.vm_end) {
            uint64 vm_addr_pg = PGROUNDDOWN(vm_addr_curr);
            if (check_mapping(get_current_task()->pgd, vm_addr_curr)) {
                uint64 pg_copy = kalloc();

                memcpy((char*)pg_copy, (char*)vm_addr_pg, PGSIZE);
                create_mapping(task_child->pgd, vm_addr_pg, pg_copy - PA2VA_OFFSET,
                    PGSIZE, vma.vm_flags | PTE_USER | PTE_VALID);
            }
            vm_addr_curr = vm_addr_pg + PGSIZE;
        }
    }

    /* 6. 返回子 task 的 pid */
    return task_child->pid;

}