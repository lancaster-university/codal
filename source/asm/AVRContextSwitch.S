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
    ;NEED TO ADD NULL CHECKS
; load our old fibers RAM stack pointer (args r21, r20)
    MOV ZL, r20
    MOV ZH, r21

; pre load our terminating condition into r15, and r16
store_stack_pointer:
    IN r15, SPLO
    IN r16, SPHI

    LD XL, r24
    LD XH, r25

;STORE SPLO AND HI into OUR TCB
    ST X+, r15
    ST X+, r16

    LDI YL, LOW(RAMEND)
    LDI YH, HIGH(RAMEND)

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

restore_stack:
    CLZ
    CPI r18, 0
    BRNE restore_stack_cont
    CLZ
    CPI r19, 0
    BREQ check_lr

restore_stack_pointer:
;NEED TO ADD NULL CHECKS
    LD XL, r22
    LD XH, r23

    LD r16, X+
    LD r17, X+

    OUT SPL, r16
    OUT SPH, r17

restore_stack_cont:
; load our new fibers RAM stack pointer (args r19, r18)
    MOV ZL, r18
    MOV ZH, r19

    LDI YL, LOW(RAMEND)
    LDI YH, HIGH(RAMEND)

    JMP restore_stack_validation

restore_stack_loop:
    LD r17, -Z
    ST -Y, r17

restore_stack_validation:
    CP ZH, r16
    BRNE restore_stack_loop
    CP ZL, r15
    BRNE restore_stack_loop

; check if our faux LR is set, and modify the stack frame return address if required.
check_lr:
; low 8 bits, and reset to 0
    LD r16, X
    ST X+, r1
; upper 8 bits, and reset to 0
    LD r17, X
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
