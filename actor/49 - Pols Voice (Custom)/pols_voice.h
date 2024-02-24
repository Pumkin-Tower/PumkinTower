#ifndef Z_POLSVOICE_H
#define Z_POLSVOICE_H

#include "ultra64.h"
#include "global.h"


#define GPOLSVOICESKEL_ROOT_POS_LIMB 0
#define GPOLSVOICESKEL_ROOT_ROT_LIMB 1
#define GPOLSVOICESKEL_LOWERBODY_LIMB 2
#define GPOLSVOICESKEL_UPPERBODY_LIMB 3
#define GPOLSVOICESKEL_HEAD_LIMB 4
#define GPOLSVOICESKEL_EAR_L_LIMB 5
#define GPOLSVOICESKEL_EAR_R_LIMB 6
#define GPOLSVOICESKEL_NUM_LIMBS 7


struct PolsVoice;

typedef void (*PolsVoiceActionFunc)(struct PolsVoice*, PlayState*);

typedef struct PolsVoice {
    Actor actor;
    SkelAnime skelAnime;
    Vec3s jointTable[GPOLSVOICESKEL_NUM_LIMBS];
    Vec3s morphTable[GPOLSVOICESKEL_NUM_LIMBS];
    s32 invincibilityTimer;
    ColliderCylinder collider;
    s16 targetYaw;
    u8 drowned;
    u8 gnawTimer;
    u8 isGnawing;
    u8 aggroTimer;
    PolsVoiceActionFunc actionFunc;
} PolsVoice;

#endif
