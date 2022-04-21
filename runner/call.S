
.set noreorder

.section .text
.global call_and_save

call_and_save:
    # $a0 uint32_t address
    # $ra return address
    # $sp stack pointer

    # Push all registers
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


    # Call address
    jal $a0
    nop


    # Store all registers to memory
    #la $a2, regs_address
    #ld $a3, 0($a2)
    # For some reason the above doesn't work?? hardcode address for now.

    la $a1, 0x80102000

    # Store CPU registers
    sd $0, 0($a1)
    sd $1, 8($a1)
    sd $2, 16($a1)
    sd $3, 24($a1)
    sd $4, 32($a1)
    sd $5, 40($a1)
    sd $6, 48($a1)
    sd $7, 56($a1)
    sd $8, 64($a1)
    sd $9, 72($a1)
    sd $10, 80($a1)
    sd $11, 88($a1)
    sd $12, 96($a1)
    sd $13, 104($a1)
    sd $14, 112($a1)
    sd $15, 120($a1)
    sd $16, 128($a1)
    sd $17, 136($a1)
    sd $18, 144($a1)
    sd $19, 152($a1)
    sd $20, 160($a1)
    sd $21, 168($a1)
    sd $22, 176($a1)
    sd $23, 184($a1)
    sd $24, 192($a1)
    sd $25, 200($a1)
    sd $26, 208($a1)
    sd $27, 216($a1)
    sd $28, 224($a1)
    sd $29, 232($a1)
    sd $30, 240($a1)
    sd $31, 248($a1)


    # Store all FP registers
    sdc1 $f0, 256($a1)
    sdc1 $f1, 264($a1)
    sdc1 $f2, 272($a1)
    sdc1 $f3, 280($a1)
    sdc1 $f4, 288($a1)
    sdc1 $f5, 296($a1)
    sdc1 $f6, 304($a1)
    sdc1 $f7, 312($a1)
    sdc1 $f8, 320($a1)
    sdc1 $f9, 328($a1)
    sdc1 $f10, 336($a1)
    sdc1 $f11, 344($a1)
    sdc1 $f12, 352($a1)
    sdc1 $f13, 360($a1)
    sdc1 $f14, 368($a1)
    sdc1 $f15, 376($a1)
    sdc1 $f16, 384($a1)
    sdc1 $f17, 392($a1)
    sdc1 $f18, 400($a1)
    sdc1 $f19, 408($a1)
    sdc1 $f20, 416($a1)
    sdc1 $f21, 424($a1)
    sdc1 $f22, 432($a1)
    sdc1 $f23, 440($a1)
    sdc1 $f24, 448($a1)
    sdc1 $f25, 456($a1)
    sdc1 $f26, 464($a1)
    sdc1 $f27, 472($a1)
    sdc1 $f28, 480($a1)
    sdc1 $f29, 488($a1)
    sdc1 $f30, 496($a1)
    sdc1 $f31, 504($a1)

    # lo
    mflo $30
    sd $30,512($a1)

    # hi
    mfhi $30
    sd $30,520($a1)

    # FC31
    cfc1 $30,$f31
    sw $30,528($a1)



    # Pop all registers
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

