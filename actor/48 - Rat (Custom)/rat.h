#ifndef Z_RAT_H
#define Z_RAT_H

#include "ultra64.h"
#include "global.h"

#define GRATSKEL_ROOT_POS_LIMB 0
#define GRATSKEL_ROOT_ROT_LIMB 1
#define GRATSKEL_BODY_LIMB 2
#define GRATSKEL_ARM_L_LIMB 3
#define GRATSKEL_HAND_L_LIMB 4
#define GRATSKEL_ARM_R_LIMB 5
#define GRATSKEL_HAND_R_LIMB 6
#define GRATSKEL_HEAD_LIMB 7
#define GRATSKEL_LEG_L_LIMB 8
#define GRATSKEL_LEG_R_LIMB 9
#define GRATSKEL_TAIL_LIMB 10
#define GRATSKEL_NUM_LIMBS 11

struct Rat;

typedef void (*RatActionFunc)(struct Rat*, PlayState*);

typedef struct Rat {
    Actor actor;
    SkelAnime skelAnime;
    Vec3s jointTable[GRATSKEL_NUM_LIMBS];
    Vec3s morphTable[GRATSKEL_NUM_LIMBS];
    ColliderCylinder collider;
    Vec3f targetPos;
    s16 targetYaw;
    u16 fleeTimer;
    Vec3f fleePos;
    u8 drowned;
    RatActionFunc actionFunc;
} Rat;

#endif
