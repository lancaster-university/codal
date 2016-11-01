.global swap_context
.global save_context
.global save_register_context
.global restore_register_context
.global modify_ret

#include <avr/io.h>

#define XL r26
#define XH r27
#define YL r28
#define YH r29
#define ZL r30
#define ZH r31

#define SPLO    61
#define SPHI    62
#define SR      63

; r25 - 24 TCB paged out
; r23 - r22 TCB paged in
; r21 - r20 base stack pointer for out
; r19 - r18 base stack pointer for in
swap_context:
    CLZ
    CPI r24, 0 ; HELLO
    BRNE swap_context_cont
    CLZ
    CPI r25, 0
    BREQ store_context_complete

; store general purpose registers
swap_context_cont:
; store arg 1, into X
    MOV XL, r24
    MOV XH, r25

; 0 is the address of r0
    LDI ZL, 0
    LDI ZH, 0

store_loop:
; move GPR to RAM...
    LD YL, Z+
    ST X+, YL

; test if we have reached the top of GPR's
    CPI ZL, 26
    BRNE store_loop

    ADIW XL, 6

; preserve status reg and stack pointer
    IN r17, SR
    ST X+, r17
    IN r17, SPLO
    ST X+, r17
    IN r17, SPHI
    ST X+, r17

; we have now preserved all GP regs, move onto stack allocated variables.
; For this we should look at the stack frame, stored indirectly in Y
; Indirect X is now at stack_base in our C struct
; load stack base into indirect Z

store_context_complete:
; check if null parameter
    CLZ
    CPI r20, 0
    BRNE store_context_cont
    CLZ
    CPI r21, 0
    BREQ store_stack_complete

store_context_cont:
; load our new fibers stack pointer (args r21, r20)
    MOV ZL, r20
    MOV ZH, r21

; pre load our terminating condition into r15, and r16
    IN r15, SPLO
    IN r16, SPHI

    LDI YL, lo8(RAMEND)
    LDI YH, hi8(RAMEND)

    JMP store_stack_validation

; iterate until Y == SP
store_stack:
    LD r17, -Y
    ST -Z, r17

store_stack_validation:

    CP YH, r16
    BRNE store_stack
    CP YL, r15
    BRNE store_stack

store_stack_complete:
; check for a null TCB pointer
    CLZ
    CPI R22, 0
    BRNE restore_context
    CLZ
    CPI r23, 0
    BREQ check_lr

restore_context:
    MOV XL, r22
    MOV XH, r23
    LDI ZL, 0
    LDI ZH, 0

restore_context_loop:
; move from RAM to GPR...
    LD YL, X+
    ST Z+, YL

; test if we have reached the top of GPR's
    CPI ZL, 26
    BRNE restore_context_loop

    ADIW XL, 6

; restore status reg and stack pointer
    LD r17, X+
    OUT SR, r17
    LD r17, X+
    MOV r15, r17
    OUT SPLO, r17
    LD r17, X+
    MOV r16, r17
    OUT SPHI, r17

restore_stack_frame:
; restore our stack frame pointer for local vars
    ;LD r17, X+
    ;LDS r15, SPL
    ;LDS r16, SPH
    ;MOV YL, r17
    ;LD r17, X+
    ;MOV YH, r17

restore_stack_chk:
; check for null parameter
    CLZ
    CPI r19, 0
    BRNE restore_stack
    CLZ
    CPI r18, 0
    BREQ check_lr

restore_stack:

; load our new fibers stack pointer (args r19, r18)
    MOV ZL, r18
    MOV ZH, r19

    LDI YL, lo8(RAMEND)
    LDI YH, hi8(RAMEND)

    JMP restore_stack_validation

restore_stack_loop:
    LD r17, -Z
    ST -Y, r17

restore_stack_validation:
    CP ZH, r16
    BRNE restore_stack_loop
    CP ZL, r15
    BRNE restore_stack_loop

    ;MOV YL, r15
    ;MOV YH, r16

; check if our faux LR is set, and modify the stack frame return address if required.
check_lr:
    LD r17, X+
    LD r17, X+
; low 8 bits, and reset to 0
    LD r17, X
    ST X+, r1
; upper 8 bits, and reset to 0
    LD r16, X
    ST X+, r1

; check if we have some semblence of an address in LR...
    CLZ
    CPI r16, 0
    BRNE restore_lr
    CLZ
    CPI r17, 0
    BRNE restore_lr

; we have no address, skip...
    JMP restore_stack_complete

; modify the return point of our stack frame...
restore_lr:
    CLZ
; first we unset
    IN ZL, SPLO
    IN ZH, SPHI

    ADIW ZL, 2
    ST Z, r16
    ST -Z, r17

restore_stack_complete:
    RET

;--------------------------

modify_ret:
    LDS ZL, SPL
    LDS ZH, SPH

    ADIW ZL, 2
    ST Z, r24
    ST -Z, r25
    RET

;--------------------------


save_context:
    MOV XL, r24
    MOV XH, r25

    ; 0 is the address of r0
    LDI ZL, 0
    LDI ZH, 0

save_context_loop:
; move GPR to RAM...
    LD r17, Z+
    ST X+, r17

; test if we have reached the top of GPR's
    CPI ZL, 32
    BRNE save_context_loop

; preserve status reg and stack pointer
    LDS r17, SREG
    ST X+, r17
    LDS r17, SPL
    ST X+, r17
    LDS r17, SPH
    ST X+, r17

; preserve our stack frame pointer, local vars etc.
    MOV r17, YL
    ST X+, r17
    MOV r17, YH
    ST X+, r17

; SAVE THE STACK FRAME
; load our old fibers stack pointer (args r23, r22)
    MOV ZL, r22
    MOV ZH, r23

; pre load our terminating condition into r15, and r16
    LDS r15, SPL
    LDS r16, SPH

    LD r17, Y
    ST Z, r17
    JMP preserve_stack_validation

; iterate until the top of the stack == the bottom of the stack.
preserve_stack:
    LD r17, -Y
    ST -Z, r17

preserve_stack_validation:
    CP YH, r16
    BRNE preserve_stack
    CP YL, r15
    BRNE preserve_stack

; finished
    RET


;--------------------------

save_register_context:
    MOV XL, r24
    MOV XH, r25

    ; 0 is the address of r0
    LDI ZL, 0
    LDI ZH, 0

save_r_context_loop:
; move GPR to RAM...
    LD r17, Z+
    ST X+, r17

; test if we have reached the top of GPR's
    CPI ZL, 31
    BRNE save_r_context_loop

; preserve status reg and stack pointer
    LDS r17, SREG
    ST X+, r17
    LDS r17, SPL
    ST X+, r17
    LDS r17, SPH
    ST X+, r17

; preserve our stack frame pointer, local vars etc.
    MOV r17, YL
    ST X+, r17
    MOV r17, YH
    ST X+, r17

; finished
    RET

;--------------------------

restore_register_context:
    MOV XL, r24
    MOV XH, r25
    LDI ZL, 0
    LDI ZH, 0

restore_r_context_loop:
; move RAM to GPR...
    LD r17, X+
    ST Z+, r17

; test if we have reached the top of GPR's
    CPI ZL, 31
    BRNE restore_r_context_loop

; restore status reg and stack pointer
    LD r17, X+
    STS SREG, r17
    LD r17, X+
    STS SPL, r17
    LD r17, X+
    STS SPH, r17

; restore our stack frame pointer for local vars
    LD r17, X+
    MOV YL, r17
    LD r17, X+
    MOV YH, r17

; finished
    RET
