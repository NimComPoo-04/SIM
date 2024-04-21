.size 256
.org 0


fibo:
;; Loading A and T
CLR T
ADD T, Ac

CLR A
ADD A, [T]

;; Printing Digits in hex
CLR T
ADD T, A		;; Copy value of A to T

RTR T, 4		;; Get the last digit
ADD T, 48		;; add zero to it

MOV [0xFFFF], T

CLR TL
RTL T, 4
ADD T, 48

MOV [0xFFFF], T

MOV [0xFFFF], 32

;; Calculating fibo

CLR T
ADD T, Bc

ADD A, [T]

MOV [Cc], A

CLR T
ADD T, Bc
CLR A
ADD A, [T]

MOV [Ac], A

ADD T, 1

CLR A
ADD A, [T]
MOV [Bc], A

;; calculating next step
CLR T
ADD T, count

CLR A
ADD A, [T]

SUB A, 1
MOV [count], A

JMP COND  fibo

hlt:
JMP hlt

Ac: .word 0
Bc: .word 1
Cc: .word 1

count: .word 7
