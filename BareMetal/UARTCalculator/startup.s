.section .vector_table, "x"
.global _Reset
_Reset:
	b Reset_Handler

.section .text	
Reset_Handler:
	ldr sp, =stack_top
 	bl main
 	b .
