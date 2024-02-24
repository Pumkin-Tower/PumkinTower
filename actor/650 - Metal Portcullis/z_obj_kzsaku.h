#ifndef Z_OBJ_KZSAKU_H
#define Z_OBJ_KZSAKU_H

#include "global.h"

#define KZSAKU_GET_UP_DOWN(thisx) ((thisx)->params >> 15)
#define KZSAKU_GET_SWITCH_FLAG(thisx) (((thisx)->params & 0x7F00) >> 8)
#define KZSAKU_GET_SCALE(thisx) ((thisx)->params & 0xff)

struct ObjKzsaku;

typedef void (*ObjKzsakuActionFunc)(struct ObjKzsaku*, PlayState*);

typedef struct ObjKzsaku {
    /* 0x000 */ DynaPolyActor dyna;
    /* 0x15C */ ObjKzsakuActionFunc actionFunc;
    /* 0x160 */ f32 raisedAmount;
    /* 0x164 */ s32 switchFlag;
    /* 0x168 */ s16 timer;
    float scale;
    float upDown;
} ObjKzsaku; // size = 0x16C

#endif // Z_OBJ_KZSAKU_H
