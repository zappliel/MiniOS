// arch/riscv/kernel/vm.c
#include "defs.h"
#include "types.h"
#include "mm.h"
#include "string.h"
#include "printk.h"

/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void) {
    /*
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    uint64 pa =0x80000000, va = 0x80000000;
    int index = (va>>30)&0x1ff;

    early_pgtbl[index]=(pa>>30)<<28;

    printk("%x\n",early_pgtbl[index]);
    early_pgtbl[index]=(uint64)(15+early_pgtbl[index]);
    va=VM_START;
    index = (va>>30)&0x1ff;
    early_pgtbl[index]=(pa>>30)<<28;

    printk("%x\n",early_pgtbl[index]);
    early_pgtbl[index]=(uint64)(15+early_pgtbl[index]);

    printk("...setup_vm done!\n");
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

extern uint64* _stext;
extern uint64* _etext;
extern uint64* _srodata;
extern uint64* _erodata;
extern uint64* _sdata;

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    printk("进入setup_vm_final\n");
//    printk("%d\n",_stext);
//    printk("%d\n",&_stext);
//    printk("%d\n",_srodata);
//    printk("%d\n",_sdata);
    // No OpenSBI mapping required
    // mapping kernel text X|-|R|V
//    printk("%d\n",(_srodata-_stext)/PGSIZE);
    create_mapping(swapper_pg_dir, (uint64)&_stext, (uint64)(&_stext) - PA2VA_OFFSET, (uint64)(&_etext)-(uint64)(&_stext), 11);
//    printk("%d\n",(_srodata-_stext)/PGSIZE);
    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, (uint64)&_srodata, (uint64)(&_srodata) - PA2VA_OFFSET, (uint64)(&_erodata)-(uint64)(&_srodata), 3);
//    printk("%d\n",0xffffffe000000000-(uint64)(_srodata)+(uint64)(_sdata));
    // mapping other memory -|W|R|V
    uint64 vm_end=(uint64)PHY_END+PA2VA_OFFSET;
    create_mapping(swapper_pg_dir, (uint64)&_sdata, (uint64)(&_sdata) - PA2VA_OFFSET, vm_end-(uint64)(&_sdata), 7);

    // set satp with swapper_pg_dir
    asm volatile (

        "addi t0,zero,8\n"
        "slli t0,t0,28\n"
        "lui t1,0xfffff\n"
        "addi t1,t1,0x700\n"
        "addi t1,t1,0x700\n"
        "addi t1,t1,0x1df\n"
        "slli t1, t1, 32\n"
        "add t0, t0, t1\n"

        "mv t2,%[swapper_pg_dir]\n"
        "sub t2,t2,t0\n"
        "srli t2,t2,12\n"
        "addi t3,zero,1\n"
        "slli t3,t3,63\n"
        "add t2,t2,t3\n"
        "csrw satp,t2"
        :
        :[swapper_pg_dir]"r"(swapper_pg_dir)
        :"memory"
    );

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    printk("...setup_vm_final done!\n");
    return;
}


/* 创建多级页表映射关系 */
/* 不要修改该接口的参数和返回值 */
create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */


    for (uint64 va_start=va;va < va_start+sz;va += PGSIZE,pa += PGSIZE){
        uint64 vpn2 = (va >> 30)&0x1ff;
        uint64 vpn1 = (va >> 21)&0x1ff;
        uint64 vpn0 = (va >> 12)&0x1ff;
        uint64* newpage2=pgtbl;
        uint64* newpage1;
        uint64* newpage0;
        uint64* newpage20;
        uint64 newpage22,newpage21;
        if (newpage2[vpn2]&1) {

        }
        else{
            newpage1 = (uint64*)(kalloc()-PA2VA_OFFSET);

            newpage21 = (uint64)newpage1 >> 12;
            newpage22 = newpage21<<10;
            newpage21 = newpage22+1;
            newpage2[vpn2]=newpage21;
        }

        //newpage1 = (uint64*)((newpage2[vpn2]>>10)<<12 + PA2VA_OFFSET);

        if (newpage1[vpn1]&1) {

        }
        else{
            newpage0 = (uint64*)(kalloc()-PA2VA_OFFSET);

            uint64 newpage11 = (uint64)newpage0 >> 12;
            uint64 newpage12 = newpage11<<10;
            newpage11 = newpage12+1;
            newpage1[vpn1]=newpage11;
        }

        //newpage0 = (uint64*)((newpage1[vpn1]>>10)<<12 + PA2VA_OFFSET);

        uint64 pa1=pa>>12;
        uint64 pa2=pa1<<10;
        pa2=pa2+perm;
        newpage0[vpn0] =pa2;
        //printk("Address of newpage0: %d\n", (void*)&newpage0);

    }
}
