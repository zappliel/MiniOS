#define PA2VA_OFFSET 0xffffffdf80000000
.extern start_kernel
.extern _traps

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
    

    call setup_vm
    call relocate
    

    call mm_init
    call setup_vm_final
    call task_init


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
        #csrr t0,sstatus
        #add t1,zero,1<<1
        #or t0,t0,t1
        #csrw sstatus,t0

    # ------------------


    # ------------------
    # - your code here -
    # ------------------
    
    jal ra, start_kernel #跳转到start_kernel

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    # li t0, PA2VA_OFFSET

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
##    addi t0, zero, 8
#    slli t0, t0, 60
#    add t1, t1, t0

    la t1, early_pgtbl
    sub t1,t1,t0
    li t2,8
    slli t2, t2, 60
    srli t1,t1,12
    or t1,t1,t2
    auipc t2,0
    addi t2,t2,20
    add t2,t2,t0
    csrw stvec, t2

    csrw satp, t1


    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack



boot_stack:
    .space 4096 # 4K size <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: