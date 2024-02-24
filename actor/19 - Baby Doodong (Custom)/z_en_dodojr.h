#ifndef Z_EN_DODOJR_H
#define Z_EN_DODOJR_H

#include "ultra64.h"
#include "global.h"

struct EnDodojr;

typedef void (*EnDodojrActionFunc)(struct EnDodojr*, PlayState*);

typedef struct EnDodojr {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ SkelAnime skelAnime;
    /* 0x0190 */ EnDodojrActionFunc actionFunc;
    /* 0x0194 */ ColliderCylinder collider;
    /* 0x01E0 */ Actor* bomb;
    /* 0x01E4 */ Vec3f headPos;
    /* 0x01F0 */ Vec3f dustPos;
    /* 0x01FC */ s16 counter; // Used for bouncing and flashing when dying.
    /* 0x01FE */ s16 stunTimer;
    /* 0x0200 */ s16 freezeFrameTimer;
    /* 0x0202 */ s16 timer; // Used for burrowing/despawning, bomb swallowing, and dying.
    /* 0x0204 */ s16 crawlSfxTimer;
    /* 0x0208 */ f32 rootScale; // scale used with the root limb
    /* 0x020C */ Vec3s jointTable[15];
    /* 0x0266 */ Vec3s morphTable[15];
} EnDodojr; // size = 0x02C0

struct EnBom;

typedef void (*EnBomActionFunc)(struct EnBom*, PlayState*);

#define ENBOM_GET_1(thisx) ((thisx)->shape.rot.x & 1)
#define ENBOM_GET_80(thisx) ((thisx)->shape.rot.z & 0x80)
#define ENBOM_GET_FF00(thisx) (((thisx)->shape.rot.z & 0xFF00) >> 8)

typedef enum BombType {
    /* 0 */ BOMB_TYPE_BODY,
    /* 1 */ BOMB_TYPE_EXPLOSION
} BombType;

// Passed via x rotation
typedef enum BombExplosiveType {
    /* 0 */ BOMB_EXPLOSIVE_TYPE_BOMB,
    /* 1 */ BOMB_EXPLOSIVE_TYPE_POWDER_KEG
} BombExplosiveType;

typedef struct EnBom {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderCylinder collider1;
    /* 0x190 */ ColliderJntSph collider2;
    /* 0x1B0 */ ColliderJntSphElement collider2Elements[1];
    /* 0x1F0 */ s16 timer;
    /* 0x1F2 */ s16 flashSpeedScale;
    /* 0x1F4 */ f32 unk_1F4;
    /* 0x1F8 */ u8 unk_1F8;
    /* 0x1F9 */ u8 isPowderKeg;
    /* 0x1FA */ s16 unk_1FA;
    /* 0x1FC */ u8 unk_1FC;
    /* 0x200 */ EnBomActionFunc actionFunc;
} EnBom; // size = 0x204

#endif
