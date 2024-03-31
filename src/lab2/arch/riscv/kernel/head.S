.extern start_kernel
.extern _traps

    .section .text.init
    .globl _start
_start:

    # YOUR CODE HERE

    la sp,boot_stack_top
    jal ra, mm_init # initialize physical memory

    jal ra, task_init # initialize task threads

    # ------------------

        # set stvec = _traps
        la t0,_traps
        csrw stvec,t0


    # ------------------

        # set sie[STIE] = 1
        csrr t0,sie
        add t1,zero,1<<5
        or t0,t0,t1
        csrw sie,t0

    # ------------------
        # set first time interrupt
        rdtime a0
        li a1,10000000
        add a0,a0,a1
        call sbi_init



    # ------------------

        # set sstatus[SIE] = 1
        csrr t0,sstatus
        add t1,zero,1<<1
        or t0,t0,t1
        csrw sstatus,t0

    # ------------------


    # ------------------
    # - your code here -
    # ------------------

    call start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: