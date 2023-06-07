#ifndef _IL_COMMON_HEADER_
#define _IL_COMMON_HEADER_

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief common IL instructions
 */
typedef enum{
	nop,

	// inputs
	ld, 
	ldi,
	ldp,	
	ldf,	

	// outputs
	set,
	reset,
	out,

	// logical
	and,
	or,
	neg,
	xor,
	nand,
	nor,
	nxor,

	andn,
	orn,
	negn,
	xorn,
	nandn,
	norn,
	nxorn,

	// register
	mov,
	add,
	sub,
	mul,
	div,
}il_op_t;

#endif