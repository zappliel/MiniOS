// trap.c
#include "printk.h"
void trap_handler(unsigned long scause, unsigned long sepc) {
    long a1 = 1;
    long a2 = 1;
    long a3 = 1;
    long a4 = 1;
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
    // `clock_set_next_event()` 见 4.3.4 节
    // 其他interrupt / exception 可以直接忽略

    // YOUR CODE HERE
    if (scause & (a1 << 63) == (a2<<63)) {
        if (scause & (a3 << 4) == (a4 << 4)) {
            printk("kernel is running!\n");
            printk("[S] Supervisor Mode Timer Interrupt\n");
            clock_set_next_event();
        }
    }
}