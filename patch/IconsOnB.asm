/* this updates part of Interface_DrawBButtonIcons() so that any icon can be drawn on the B button for any Link form */
/*00BAE7C0*/	lui         t0, 0x801f
/*00BAE7C4*/	addiu       t0, t0, 0xf670
/*00BAE7C8*/	beq         v0, r0, 0x0003
/*00BAE7CC*/	addiu       a1, r0, 0x0005 /* use 0x0005 instead of PLAYER_FORM_HUMAN (0x0004) to allow all cases */
/*00BAE7D0*/	beql        a1, v0, 0x017c /* bnel -> beql so we always branch */
/*00BAE7D4*/	lw          ra, 0x0014(sp)
/*00BAE7D8*/	lbu         v0, 0x0020(t0)

