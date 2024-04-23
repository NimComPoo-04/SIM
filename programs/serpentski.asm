.size 256
.org 0

;; CODE

init:

MOV [300], 1		;; placing a 1 at a lonesomeplace

MOV [i], 40		;; Iterations
loop:

MOV [j], 80		;; Row size
MOV [index], 0		;; start of the buffer
iloop:
	;; CHECKING STATE

	CLR A
	CLR T
	ADD A, index
	ADD T, [A]

	ADD T, start

	CLR A
	ADD A, [T]

	;; PRITING # if T == 1 otherwise 0
	
	MOV COND [0xFFFF], 35		;; #
	JMP COND noPrint
	MOV [0xFFFF], 32		;; space
noPrint:

	;; COMPARISN TO CHANGE
	
	CLR A
	ADD A, [T]
	RTL A, 1
	ADD T, 1
	ADD A, [T]
	RTL A, 1
	ADD T, 1
	ADD A, [T]

	SUB A, 1
	JMP COND not1
	MOV [T], 1
	JMP quitif
	
not1:
	ADD A, 1
	SUB A, 4

	MOV COND [T], 0
	JMP COND quitif
	MOV [T], 1

quitif:
	CLR CR

	;; SAVE VALUE OF INDEX

	CLR A
	CLR T
	ADD A, index

	ADD T, [A]
	ADD T, 1

	MOV [index], T	

	;; DECREMENT COUNTER J
	CLR A
	CLR T

	ADD A, j			;; loading memory address of variable j
	ADD T, [A]			;; load the value at j to T
	SUB T, 1
	MOV [A], T

	JMP COND iloop

;; PRITING $

MOV [0xFFFF], 0xa

;; DECREMENT COUNTER I
CLR A
CLR T

ADD A, i			;; loading memory address of variable j
ADD T, [A]			;; load the value at j to T
SUB T, 1
MOV [A], T

JMP COND loop


hlt:
JMP hlt

;; VARIABLES

.org 258 i:
.org 259 j:
.org 260 index:

.org 300 start:

