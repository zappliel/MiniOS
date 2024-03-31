   .section .text.entry
    .align 2
    .globl _traps


_traps:
    # YOUR CODE HERE
    # -----------

        # 1. save 32 registers and sepc to stack
        addi sp,sp,-33*8
        sd x0,0(sp)
        sd x1,8(sp)
        sd x2,8*2(sp)
        sd x3,8*3(sp)
        sd x4,8*4(sp)
        sd x5,8*5(sp)
        sd x6,8*6(sp)
        sd x7,8*7(sp)
        sd x8,8*8(sp)
        sd x9,8*9(sp)
        sd x10,8*10(sp)
        sd x11,8*11(sp)
        sd x12,8*12(sp)
        sd x13,8*13(sp)
        sd x14,8*14(sp)
        sd x15,8*15(sp)
        sd x16,8*16(sp)
        sd x17,8*17(sp)
        sd x18,8*18(sp)
        sd x19,8*19(sp)
        sd x20,8*20(sp)
        sd x21,8*21(sp)
        sd x22,8*22(sp)
        sd x23,8*23(sp)
        sd x24,8*24(sp)
        sd x25,8*25(sp)
        sd x26,8*26(sp)
        sd x27,8*27(sp)
        sd x28,8*28(sp)
        sd x29,8*29(sp)
        sd x30,8*30(sp)
        sd x31,8*31(sp)
        csrr t0,sepc
        sd t0,8*32(sp)

    # -----------

        # 2. call trap_handler
        csrr a0, scause
        csrr a1, sepc
        call trap_handler


    # -----------

        # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
        ld t0, 32*8(sp)
        csrw sepc, t0

        ld x0,0*8(sp)
        ld x1,1*8(sp)
        ld x2,2*8(sp)
        ld x3,3*8(sp)
        ld x4,4*8(sp)
        ld x5,5*8(sp)
        ld x6,6*8(sp)
        ld x7,7*8(sp)
        ld x8,8*8(sp)
        ld x9,9*8(sp)
        ld x10,10*8(sp)
        ld x11,11*8(sp)
        ld x12,12*8(sp)
        ld x13,13*8(sp)
        ld x14,14*8(sp)
        ld x15,15*8(sp)
        ld x16,16*8(sp)
        ld x17,17*8(sp)
        ld x18,18*8(sp)
        ld x19,19*8(sp)
        ld x20,20*8(sp)
        ld x21,21*8(sp)
        ld x22,22*8(sp)
        ld x23,23*8(sp)
        ld x24,24*8(sp)
        ld x25,25*8(sp)
        ld x26,26*8(sp)
        ld x27,27*8(sp)
        ld x28,28*8(sp)
        ld x29,29*8(sp)
        ld x30,30*8(sp)
        ld x31,31*8(sp)

        addi sp, sp, 33*8


    # -----------

        # 4. return from trap
        sret

    # -----------

# arch/riscv/kernel/entry.S

    .global __dummy
__dummy:
    la t0, dummy
    csrw sepc, t0
    sret




    .globl __switch_to
__switch_to:
    # save state to prev process
    # including ra, sp, s0-s11
    add t0, a0, 40
    sd ra, 0(t0)
    sd sp, 8(t0)
    sd s0, 16(t0)
    sd s1, 24(t0)
    sd s2, 32(t0)
    sd s3, 40(t0)
    sd s4, 48(t0)
    sd s5, 56(t0)
    sd s6, 64(t0)
    sd s7, 72(t0)
    sd s8, 80(t0)
    sd s9, 88(t0)
    sd s10, 96(t0)
    sd s11, 104(t0)

    # restore state from next process
    add t0, a1, 40
    ld ra, 0(t0)
    ld sp, 8(t0)
    ld s0, 16(t0)
    ld s1, 24(t0)
    ld s2, 32(t0)
    ld s3, 40(t0)
    ld s4, 48(t0)
    ld s5, 56(t0)
    ld s6, 64(t0)
    ld s7, 72(t0)
    ld s8, 80(t0)
    ld s9, 88(t0)
    ld s10, 96(t0)
    ld s11, 104(t0)

    ret
