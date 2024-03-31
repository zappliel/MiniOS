#include "printk.h"
#include "sbi.h"
#include "defs.h"
#include "proc.h"
#include "test_schedule.h"
extern void test();

int start_kernel() {
    printk("2022");
    printk(" Hello RISC-V\n");
    //printk("%llx\n", csr_read(sstatus));
    //printk("%llx\n", csr_read(sie));
    //do_timer();
    test(); // DO NOT DELETE !!!

	return 0;
}
