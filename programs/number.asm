.size 256
.org 0

CLR A
ADD A, 0x12		;; Number to be displayed

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

hlt:
JMP hlt
