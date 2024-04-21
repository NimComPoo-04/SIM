.size 256
.org 0

CLR A		; Loading Address
ADD A, msg

print:
	CLR T
	ADD T, [A]
	ADD A, 1
	MOV [0xffff], T
	ADD T, 0

	JMP COND print

halt:
JMP halt

msg:
.string "Hello, World!"
