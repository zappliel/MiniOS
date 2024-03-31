// trap.c 
#include "printk.h"
#include "syscall.h"
#include "stringt.h"

extern uint64 uapp_start;
extern uint64 uapp_end;
struct pt_regs
{
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
    uint64 stval;
    uint64 sscratch;

    uint64 scause;
};
extern struct task_struct* current;

extern void clock_set_next_event();
void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs) {
    long a1 = 1;
    long a2 = 1;
    long a3 = 1;
    long a4 = 1;
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
    // `clock_set_next_event()` 见 4.3.4 节
    // 其他interrupt / exception 可以直接忽略


//    printk("scause: %lx, ", scause);
//    printk("stval: %lx, ", regs->stval);
//    printk("sepc: %lx\n", regs->sepc);

    //printk("scause=%d\n",scause);
    //while(1);
    // YOUR CODE HERE
    if (scause == 0x8000000000000005) {

        //printk("kernel is running!\n");
        printk("[S] Supervisor Mode Timer Interrupt\n");
        clock_set_next_event();
        do_timer();
        //printk("1\n");

    }
    else{
        //printk("2\n");
        //printk("scause=%d\n",scause);
        if(scause == 0x0000000000000008){
            if(regs->x[17] == 64){
                sys_write(regs->x[10],(char*)regs->x[11], regs->x[12]);
            }
            else if(regs->x[17] == 172){
                regs->x[10] = sys_getpid();
            }
            regs->sepc = regs->sepc + (uint64)4;
        }else if(scause == 0x000000000000000C || scause == 0x000000000000000D || scause == 0x000000000000000F){ //Instruction Page Fault
            printk("[S] Supervisor Page Fault, scause: %lx, stval: %lx, sepc: %lx ,sstatus: %lx, sscratch: %lx\n",scause,regs->stval,regs->sepc,regs->sstatus,regs->sscratch);
            //printk("stval=%lx\n",regs->stval-PA2VA_OFFSET);
            do_page_fault(regs);
        }else{
            printk("[S] Unhandled trap, scause: %lx, stval: %lx, sepc: %lx ,sstatus: %lx, sscratch: %lx\n",scause,regs->stval,regs->sepc,regs->sstatus,regs->sscratch);
            while (1);
        }
    }
}

void do_page_fault(struct pt_regs *regs) {
    /*
     1. 通过 stval 获得访问出错的虚拟内存地址（Bad Address）
     2. 通过 find_vma() 查找 Bad Address 是否在某个 vma 中
     3. 分配一个页，将这个页映射到对应的用户地址空间
     4. 通过 (vma->vm_flags | VM_ANONYM) 获得当前的 VMA 是否是匿名空间
     5. 根据 VMA 匿名与否决定将新的页清零或是拷贝 uapp 中的内容
    */
    uint64 bad_address=regs->stval;
    struct vm_area_struct* bad_address_vma=find_vma(current,bad_address);

    if(bad_address_vma!=NULL){

        uint64 new_page = alloc_page();
        uint64 *pgtbl=(uint64)current->pgd+PA2VA_OFFSET;
        uint64 va=regs->stval;
        uint64 pa=new_page-PA2VA_OFFSET;
        uint64 flag=bad_address_vma->vm_flags;
        //printk("权限为：%lx\n",flag);
        create_mapping(pgtbl,va,pa,1,0x1F);

        if((bad_address_vma->vm_flags&VM_ANONYM)==0){


            uint64 dst = new_page;
            uint64 src = (uint64)&uapp_start;
            uint64 sz = PGSIZE;
            //printk("bad_address_vma_size=%lx\n",bad_address_vma->vm_end-bad_address_vma->vm_start);
            int n=(bad_address_vma->vm_end-bad_address_vma->vm_start)/PGSIZE;
            //printk("有%d页\n",n);
            for(int i=0;i<n;i++){
                memcpyt((void*)dst, (void*)src, sz);
                new_page = alloc_page();
                va+=PGSIZE;
                pa=new_page-PA2VA_OFFSET;
                create_mapping(pgtbl,va,pa,1,0x1F);
                dst=new_page;
                src+=sz;
            }
        }
        else{
        }
    }
}