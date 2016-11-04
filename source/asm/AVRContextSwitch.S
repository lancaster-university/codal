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

;--------------------------
; save_context
;
; Saves the entire stack of the old fiber to RAM, and pages in another fibers
; stack.
;
; ARGS:
;       * Pointer to the old Fibers' TCB (r24, r25)
;       * Pointer to the new Fibers' TCB (r22, r23)
;       * Pointer to the where the old Fibers' stack should be saved (r20, r21)
;       * Pointer to the where the new Fibers' stack is saved (r18, r19)
;--------------------------
swap_context:
; we can't really factor this out without pushing registers of another stack frame...
; this repetitive code is a lot simpler
    PUSH r2
    PUSH r3
    PUSH r4
    PUSH r5
    PUSH r6
    PUSH r7
    PUSH r8
    PUSH r9
    PUSH r10
    PUSH r11
    PUSH r12
    PUSH r13
    PUSH r14
    PUSH r15
    PUSH r16
    PUSH r17
    PUSH r28
    PUSH r29

; validation check
    CPI r20, 0
    BRNE swap_context_cont
    CPI r21, 0
    BREQ restore_stack

swap_context_cont:
; load our old fibers RAM stack pointer (args r21, r20)
    MOV ZL, r20
    MOV ZH, r21

    LDI YL, lo8(RAMEND)
    LDI YH, hi8(RAMEND)

; pre load our terminating condition into r15, and r16
store_stack_pointer:
    IN r15, SPLO
    IN r16, SPHI

    MOV XL, r24
    MOV XH, r25

;STORE SPLO AND HI into OUR TCB
    ST X+, r15
    ST X+, r16

    JMP store_stack_validation

; iterate until Y == SP
store_stack:
    LD r17, -Y
    ST -Z, r17

store_stack_validation:
    CP YL, r15
    BRNE store_stack
    CP YH, r16
    BRNE store_stack

restore_stack:
; validation check
    CPI r18, 0
    BRNE restore_stack_pointer
    CPI r19, 0
    BREQ check_lr

restore_stack_pointer:
;NEED TO ADD NULL CHECKS
    MOV XL, r22
    MOV XH, r23

    LD r16, X+
    LD r17, X+

    OUT SPLO, r16
    OUT SPHI, r17

    MOV r15, r16
    MOV r16, r17

restore_stack_cont:
; load our new fibers RAM stack pointer (args r19, r18)
    MOV ZL, r18
    MOV ZH, r19

    LDI YL, lo8(RAMEND)
    LDI YH, hi8(RAMEND)

    JMP restore_stack_validation

restore_stack_loop:
    LD r17, -Z
    ST -Y, r17

restore_stack_validation:
    CP YL, r15
    BRNE restore_stack_loop
    CP YH, r16
    BRNE restore_stack_loop

; check if our faux LR is set, and modify the stack frame return address if required.
check_lr:
    MOV XL, r22
    MOV XH, r23
    ADIW XL, 2

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
    IN ZL, SPLO
    IN ZH, SPHI

    ADIW ZL, 2

    ST Z, r16
    ST -Z, r17

; set our args from the TCB.
set_args:
    LD r20, X+
    LD r21, X+
    LD r22, X+
    LD r23, X+
    LD r24, X+
    LD r25, X+

    RET

restore_stack_complete:
; finished, pop our clogs
    JMP pop_regs


;--------------------------
; save_context
;
; Saves the entire stack, and copies it to RAM.
;
; ARGS:
;       * Pointer to the Fibers' TCB (r24, r25)
;       * Pointer to where the stack should be saved (r22, r23)
;--------------------------
save_context:
; we can't really factor this out without pushing registers of another stack frame...
; this repetitive code is a lot simpler
    PUSH r2
    PUSH r3
    PUSH r4
    PUSH r5
    PUSH r6
    PUSH r7
    PUSH r8
    PUSH r9
    PUSH r10
    PUSH r11
    PUSH r12
    PUSH r13
    PUSH r14
    PUSH r15
    PUSH r16
    PUSH r17
    PUSH r28
    PUSH r29

; load our fibers RAM stack pointer (args r22, r23)
    MOV ZL, r22
    MOV ZH, r23

    LDI YL, lo8(RAMEND)
    LDI YH, hi8(RAMEND)

; pre load our terminating condition into r15, and r16
save_stack_pointer:
    IN r15, SPLO
    IN r16, SPHI

    MOV XL, r24
    MOV XH, r25

;STORE SPLO AND HI into OUR TCB
    ST X+, r15
    ST X+, r16

    JMP save_stack_validation

; iterate until Y == SP
save_stack:
    LD r17, -Y
    ST -Z, r17

save_stack_validation:
    CP YL, r15
    BRNE save_stack
    CP YH, r16
    BRNE save_stack

    RET

;--------------------------
; save_register_context
;
; pushes our call saved regs and simply saves the stack pointer
;
; ARGS:
;       * Pointer to the Fibers' TCB (r24, r25)
;--------------------------
save_register_context:
; we can't really factor this out without pushing registers of another stack frame...
; this repetitive code is a lot simpler
    PUSH r2
    PUSH r3
    PUSH r4
    PUSH r5
    PUSH r6
    PUSH r7
    PUSH r8
    PUSH r9
    PUSH r10
    PUSH r11
    PUSH r12
    PUSH r13
    PUSH r14
    PUSH r15
    PUSH r16
    PUSH r17
    PUSH r28
    PUSH r29

    IN r18, SPLO
    IN r19, SPHI

    MOV XL, r24
    MOV XH, r25

; STORE SPLO AND HI into OUR TCB
    ST X+, r18
    ST X+, r19

; finished, pop our clogs
    JMP pop_regs

;--------------------------
; restore_register_context
;
; simply restores the stack pointer in the case of the AVR
;
; ARGS:
;       * Pointer to the Fibers' TCB (r24, r25)
;--------------------------
restore_register_context:

;NEED TO ADD NULL CHECKS
    IN r18, SPLO
    IN r19, SPHI

    MOV XL, r24
    MOV XH, r25

    LD r18, X+
    LD r19, X+

    OUT SPLO, r18
    OUT SPHI, r19

; finished, pop our clogs
    JMP pop_regs

;--------------------------
; pop_regs:
;--------------------------
pop_regs:
    POP r2
    POP r3
    POP r4
    POP r5
    POP r6
    POP r7
    POP r8
    POP r9
    POP r10
    POP r11
    POP r12
    POP r13
    POP r14
    POP r15
    POP r16
    POP r17
    POP r28
    POP r29
    RET
