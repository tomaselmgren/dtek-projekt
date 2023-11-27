  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #


.data
.text


.global delay
delay:
	# Save registers that will be modified
	PUSH $s0
	PUSH $s1
	PUSH $ra

	# Get the parameter value
	move $s0, $a0

	# Loop while the parameter value is greater than 0
	while:
		ble $s0, $zero, endwhile

		# Decrement the parameter value
		addi $s0, $s0, -1

		# Do nothing for 1 milliseconds
		for:
			addi $s1, $zero, 9422
		loop:
			addi $s1, $s1, -1
			bne $s1, $zero, loop

        	j while

endwhile:

	# Restore saved registers and return
	POP $ra
	POP $s1
	POP $s0

	jr $ra

.global time2string
time2string:
	# Save registers that will be modified
	PUSH $s0
	PUSH $s1
	PUSH $ra

	move $s0,$a0	# Moves the value of register $a0 into register $s0.	     
	move $s1,$a1	# Moves the value of register $a1 into register $s1.

	srl $a0,$s1,12	# Shifts the value in register $s1 right by 12 bits.
	jal hexasc
	nop
	sb $v0,0($s0)	# store the first hexadecimal character in the output string

	srl $a0,$s1,8	# Shifts the value in register $s1 right by 8 bits.
	jal hexasc
	nop
	sb $v0,1($s0)	# store the second hexadecimal character in the output string

	li $t0,0x3A     # hex value for :
    	sb $t0,2($s0)	# store the colon in the output string

    	srl $a0,$s1,4	# Shifts the value in register $s1 right by 4 bits.
    	jal hexasc
    	nop
   	sb $v0,3($s0)	# store the third hexadecimal character in the output string
   	
    	srl $a0,$s1,0	# Shifts the value in register $s1 right by 0 bits.
    	jal hexasc
    	nop
    	
	andi $t1, $s1, 0xf
	beq $t1, 9, add_nine
	nop
	
	sb $v0,4($s0)
	sb $0,5($s0)    # append null byte, stores the null byte at address $s0+5. This is necessary to terminate the string.
    	
	# Restore saved registers and return
	POP $ra
	POP $s1
	POP $s0
    
    	jr $ra
    	nop

add_nine: 
	
	li $t2, 0x4e
	sb $t2, 4($s0) 
	li $t3, 0x49
	sb $t3, 5($s0)
	li $t4, 0x4e
	sb $t4, 6($s0)
	li $t5, 0x45
	sb $t5, 7($s0)
	sb $0, 8($s0)
  
	# Restore saved registers and return
	POP $ra
	POP $s1
	POP $s0
    
    	jr $ra
    	nop		
		


  # you can write your code for subroutine "hexasc" below this line
  #

.global hexasc 
hexasc:
	andi	$v0, $a0, 0xf  # isolate the lower 4 bits of the argument
  	addi	$v0, $v0, 48  # convert the number to ASCII ('0')
  	bgt	$v0, 57, upper  # if the number is greater than 9, then it is a letter
  	jr	$ra  # otherwise, return the number as an ASCII character
  	nop

upper:
  	addi 	$v0, $v0, 7  # add 7 to the number to get the ASCII code for the letter
  	jr 	$ra  # return the ASCII code for the letter
	nop