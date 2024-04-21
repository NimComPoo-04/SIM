;; Program prints english letters

.size 256
.org 0

CLR A
ADD A, 26

CLR T
ADD T, 65

prnt:

MOV [0xFFFF], T
ADD T, 1

SUB A, 1

JMP COND prnt

halt:
JMP halt
