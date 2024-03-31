.extern start_kernel
.extern _traps
.extern PA2VA_OFFSET

    .section .text.init
    .globl _start
_start:

    # YOUR CODE HERE
    #la sp,boot_stack_top



    addi t0,zero,8
    slli t0,t0,28
    lui t1,0xfffff
    addi t1,t1,0x700
    addi t1,t1,0x700
    addi t1,t1,0x1df
    slli t1, t1, 32
    add t0, t0, t1

    la t1,boot_stack_top
    sub t1,t1,t0
    mv sp,t1



    jal ra, setup_vm
    jal ra, relocate

    jal ra, mm_init
    jal ra, setup_vm_final
    jal ra, task_init

    # ------------------

        # set stvec = _traps
        la t0,_traps
        csrw stvec,t0


    # ------------------

        # set sie[STIE] = 1
        csrr t0,sie
        addi t1,zero,1
        slli t1,t1,5
        add t0,t0,t1
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
        add t1,zero,1
        slli t1,t1,1
        add t0,t0,t1
        csrw sstatus,t0

    # ------------------


    # ------------------
    # - your code here -
    # ------------------

    call start_kernel


relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)

    ######################
    #   YOUR CODE HERE   #
    ######################

    addi t0,zero,8
    slli t0,t0,28
    lui t1,0xfffff
    addi t1,t1,0x700
    addi t1,t1,0x700
    addi t1,t1,0x1df
    slli t1, t1, 32
    add t0, t0, t1

    add ra, ra, t0
    add sp, sp, t0




    # set satp with early_pgtbl

    ######################
    #   YOUR CODE HERE   #
    ######################


#    la t1, early_pgtbl
#    sub t1, t1, t0
#    srli t1, t1, 12
#    addi t0, zero, 8
#    slli t0, t0, 60
#    add t1, t1, t0

    la t1, early_pgtbl
    sub t1,t1,t0

    addi t2,zero,1
    slli t2, t2, 63
    srli t1,t1,12
    add t1,t1,t2

    addi t2,zero,20
    add t2,t2,t0
    csrw stvec, t2

    csrw satp, t1


    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack



boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: