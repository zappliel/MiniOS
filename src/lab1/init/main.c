#include "printk.h"
#include "sbi.h"
#include "defs.h"
extern void test();

int start_kernel() {
    printk("2022");
    printk(" Hello RISC-V\n");
    printk("%llx\n", csr_read(sstatus));
    printk("%llx\n", csr_read(sie));
    uint64 t = csr_read(sstatus);
    // puti(t);

    csr_write(sscratch, 0x233333);
    printk("%llx\n", csr_read(sscratch));
    test(); // DO NOT DELETE !!!

	return 0;
}
