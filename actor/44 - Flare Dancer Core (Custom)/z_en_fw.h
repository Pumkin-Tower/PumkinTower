#ifndef Z_EN_FW_H
#define Z_EN_FW_H

#include "ultra64.h"
#include "global.h"

struct EnFw;

typedef void (*EnFwActionFunc)(struct EnFw* this, PlayState* play);

#define EN_FW_EFFECT_COUNT 20

typedef struct {
    /* 0x0000 */ u8 type;
    /* 0x0001 */ u8 timer;
    /* 0x0002 */ u8 initialTimer;
    /* 0x0004 */ f32 scale;
    /* 0x0008 */ f32 scaleStep;
    /* 0x000C */ Color_RGBA8 color;
    /* 0x0010 */ char unk_10[4];
    /* 0x0014 */ Vec3f pos;
    /* 0x0020 */ Vec3f velocity;
    /* 0x002C */ Vec3f accel;
} EnFwEffect;

typedef struct EnFw {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ SkelAnime skelAnime;
    /* 0x0190 */ EnFwActionFunc actionFunc;
    /* 0x0194 */ ColliderJntSph collider;
    /* 0x01B4 */ ColliderJntSphElement sphs[1];
    /* 0x01F4 */ Vec3f bompPos;
    /* 0x0200 */ u8 lastDmgHook;
    /* 0x0202 */ s16 runDirection;
    /* 0x0204 */ s16 bounceCnt;
    /* 0x0206 */ char unk_206[0x2];
    /* 0x0208 */ s16 damageTimer;
    /* 0x020A */ s16 explosionTimer;
    /* 0x020C */ char unk_20C[0x2];
    /* 0x020E */ s16 slideTimer;
    /* 0x0210 */ s16 slideSfxTimer;
    /* 0x0212 */ s16 returnToParentTimer;
    /* 0x0214 */ s16 turnAround;
    /* 0x0218 */ f32 runRadius;
    /* 0x021C */ Vec3s jointTable[11];
    /* 0x025E */ Vec3s morphTable[11];
    /* 0x02A0 */ EnFwEffect effects[EN_FW_EFFECT_COUNT];
} EnFw; // size = 0x0700

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
