#include "printk.h"
#include "sbi.h"
#include "defs.h"
#include "proc.h"
#include "test_schedule.h"
extern void test();
extern char _stext[];
extern char _srodata[];
extern char _sdata[];
int start_kernel() {
    printk("2022");
    printk(" Hello RISC-V\n");
//    printk("_stext = %ld\n", (unsigned long long)*_stext);
//    printk("_srodata = %ld\n", (unsigned long long)*_srodata);
//    printk("_sdata = %ld\n", (unsigned long long)*_sdata);
//    *_stext=0;
//    *_srodata=0;
//    printk("_stext = %ld\n", (unsigned long long)*_stext);
//    printk("_srodata = %ld\n", (unsigned long long)*_srodata);
//    printk("_sdata = %ld\n", (unsigned long long)*_sdata);
    //printk("%llx\n", csr_read(sstatus));
    //printk("%llx\n", csr_read(sie));
    //do_timer();
    test(); // DO NOT DELETE !!!

	return 0;
}
