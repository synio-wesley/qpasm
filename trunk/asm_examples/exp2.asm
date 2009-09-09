This program calculates 2^i where i is the inputted value

== Program code ==
input:	INP
	STA [i]		store i in memory
	LDA #1
	STA [result]	store 1 in memory
loop:	LDA [i]
	JIZ [output]	if i = 0 jump to output
	LDB #1
	SUB
	STA [i]		--i  
	LDB [result]
	LDA [result]
	ADD
	STA [result]	result *= 2
	JMP [loop]	loop!
output:	LDA [result]
	OUT		output the result!
	HLT		stop execution here...

== Variable data ==
i:	nop
result:	nop
