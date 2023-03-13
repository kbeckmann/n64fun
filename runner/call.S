
.set noreorder
.set noat

.section .text
.global call_and_save

call_and_save:
    # $a0 uint32_t address
    # $ra return address
    # $sp stack pointer

    # Store all registers that should be saved
    addiu   $sp,$sp,-80
    sd      $ra,72($sp)
    sd      $s8,64($sp)
    sd      $s7,56($sp)
    sd      $s6,48($sp)
    sd      $s5,40($sp)
    sd      $s4,32($sp)
    sd      $s3,24($sp)
    sd      $s2,16($sp)
    sd      $s1,8($sp)
    sd      $s0,0($sp)

    # Copy a0 to at, so we can clear a0 later
    addiu   $at,$a0,0


    # Clear all cpu registers except at/1, gp/28, sp/29 and ra/31
    # addiu $1, $0, 0
    addiu $2, $0, 0
    addiu $3, $0, 0
    addiu $4, $0, 0
    addiu $5, $0, 0
    addiu $6, $0, 0
    addiu $7, $0, 0
    addiu $8, $0, 0
    addiu $9, $0, 0
    addiu $10, $0, 0
    addiu $11, $0, 0
    addiu $12, $0, 0
    addiu $13, $0, 0
    addiu $14, $0, 0
    addiu $15, $0, 0
    addiu $16, $0, 0
    addiu $17, $0, 0
    addiu $18, $0, 0
    addiu $19, $0, 0
    addiu $20, $0, 0
    addiu $21, $0, 0
    addiu $22, $0, 0
    addiu $23, $0, 0
    addiu $24, $0, 0
    addiu $25, $0, 0
    addiu $26, $0, 0
    addiu $27, $0, 0
    # addiu $28, $0, 0
    # addiu $29, $0, 0
    addiu $30, $0, 0
    # addiu $31, $0, 0

    # Clear all FPU registers
    dmtc1 $0, $f0
    dmtc1 $0, $f1
    dmtc1 $0, $f2
    dmtc1 $0, $f3
    dmtc1 $0, $f4
    dmtc1 $0, $f5
    dmtc1 $0, $f6
    dmtc1 $0, $f7
    dmtc1 $0, $f8
    dmtc1 $0, $f9
    dmtc1 $0, $f10
    dmtc1 $0, $f11
    dmtc1 $0, $f12
    dmtc1 $0, $f13
    dmtc1 $0, $f14
    dmtc1 $0, $f15
    dmtc1 $0, $f16
    dmtc1 $0, $f17
    dmtc1 $0, $f18
    dmtc1 $0, $f19
    dmtc1 $0, $f20
    dmtc1 $0, $f21
    dmtc1 $0, $f22
    dmtc1 $0, $f23
    dmtc1 $0, $f24
    dmtc1 $0, $f25
    dmtc1 $0, $f26
    dmtc1 $0, $f27
    dmtc1 $0, $f28
    dmtc1 $0, $f29
    dmtc1 $0, $f30
    dmtc1 $0, $f31




    # Call address
    jal $at
    nop


    # Store all registers to memory
    #la $a2, regs_address
    #ld $a3, 0($a2)
    # For some reason the above doesn't work?? hardcode address for now.

    la $at, 0x80102000

    # Store CPU registers
    sd $0, 0($at)
    sd $1, 8($at)
    sd $2, 16($at)
    sd $3, 24($at)
    sd $4, 32($at)
    sd $5, 40($at)
    sd $6, 48($at)
    sd $7, 56($at)
    sd $8, 64($at)
    sd $9, 72($at)
    sd $10, 80($at)
    sd $11, 88($at)
    sd $12, 96($at)
    sd $13, 104($at)
    sd $14, 112($at)
    sd $15, 120($at)
    sd $16, 128($at)
    sd $17, 136($at)
    sd $18, 144($at)
    sd $19, 152($at)
    sd $20, 160($at)
    sd $21, 168($at)
    sd $22, 176($at)
    sd $23, 184($at)
    sd $24, 192($at)
    sd $25, 200($at)
    sd $26, 208($at)
    sd $27, 216($at)
    sd $28, 224($at)
    sd $29, 232($at)
    sd $30, 240($at)
    sd $31, 248($at)


    # Store all FP registers
    sdc1 $f0, 256($at)
    sdc1 $f1, 264($at)
    sdc1 $f2, 272($at)
    sdc1 $f3, 280($at)
    sdc1 $f4, 288($at)
    sdc1 $f5, 296($at)
    sdc1 $f6, 304($at)
    sdc1 $f7, 312($at)
    sdc1 $f8, 320($at)
    sdc1 $f9, 328($at)
    sdc1 $f10, 336($at)
    sdc1 $f11, 344($at)
    sdc1 $f12, 352($at)
    sdc1 $f13, 360($at)
    sdc1 $f14, 368($at)
    sdc1 $f15, 376($at)
    sdc1 $f16, 384($at)
    sdc1 $f17, 392($at)
    sdc1 $f18, 400($at)
    sdc1 $f19, 408($at)
    sdc1 $f20, 416($at)
    sdc1 $f21, 424($at)
    sdc1 $f22, 432($at)
    sdc1 $f23, 440($at)
    sdc1 $f24, 448($at)
    sdc1 $f25, 456($at)
    sdc1 $f26, 464($at)
    sdc1 $f27, 472($at)
    sdc1 $f28, 480($at)
    sdc1 $f29, 488($at)
    sdc1 $f30, 496($at)
    sdc1 $f31, 504($at)

    # lo
    mflo $30
    sd $30,512($at)

    # hi
    mfhi $30
    sd $30,520($at)

    # FCR31
    cfc1 $30,$f31
    sw $30,528($at)



    # Load stored registers
    ld      $ra,72($sp)
    ld      $s8,64($sp)
    ld      $s7,56($sp)
    ld      $s6,48($sp)
    ld      $s5,40($sp)
    ld      $s4,32($sp)
    ld      $s3,24($sp)
    ld      $s2,16($sp)
    ld      $s1,8($sp)
    ld      $s0,0($sp)
    jr      $ra
    addiu   $sp,$sp,80


temp:

