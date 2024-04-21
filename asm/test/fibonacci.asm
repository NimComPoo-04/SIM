.size 256
.org 0

fibo:

CLR A		; Load value of A
ADD A, Ac

CLR T		; Load value of B
ADD T, Bc

ADD A, T	; Acc = Ac + Bc

MOV [Ac], T	; Ac = Bc
MOV [Bc], A	; Bc = Acc

; Check if 
CLR T
ADD T, count
CLR A
ADD A, [T]

COND JMP fibo

halt:
JMP halt


;; We will absolutely not test my assembly skills ffs

Ac: .word 0
Bc: .word 1

count: .word 10
