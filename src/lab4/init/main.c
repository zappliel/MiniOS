#include "printk.h"
#include "sbi.h"
#include "proc.h"

extern void test();

int start_kernel() {
    printk("[S-Mode] Hello RISC-V\n");
    schedule();
    test();

	return 0;
}
