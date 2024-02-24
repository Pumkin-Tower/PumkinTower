/* this updates code in Interface_UpdateButtonsPart2 that only allows Fierce Deity Mask in specific rooms */
/*00BA76D8*/	beq         v0, at, 0x000d
/*00BA76DC*/	addiu       at, r0, 0xffff
/*00BA76E0*/	bne         v0, at, 0x000b
/*00BA76E4*/	addiu       at, r0, 0xffff
/*00BA76E8*/	bne         v0, at, 0x0009
/*00BA76EC*/	addiu       at, r0, 0xffff
/*00BA76F0*/	beql        v0, at, 0x0008

