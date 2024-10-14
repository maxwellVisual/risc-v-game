.section .init
.global _entry
_entry:
	lui	sp, %hi(_estack)
	addi sp, sp, %lo(_estack)

    jal _start
    j _halt

.global _exit
_exit:
_halt:
    j _halt
