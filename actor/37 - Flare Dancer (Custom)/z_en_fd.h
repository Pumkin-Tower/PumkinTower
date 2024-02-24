#ifndef Z_EN_FD_H
#define Z_EN_FD_H

#include "ultra64.h"
#include "global.h"

asm(" \
gFlareDancerCastingFireAnim = 0x060010B4; \
gFlareDancerBackflipAnim = 0x06005C64; \
gFlareDancerGettingUpAnim = 0x06006044; \
gFlareDancerChasingAnim = 0x06006A18; \
gFlareDancerTwirlAnim = 0x06006b64; \
gFlareDancerSquareParticleDL = 0x06007938; \
gFlareDancerTriangleParticleDL = 0x06007A78; \
gFlareDancerSkel = 0x06005810; ");

struct EnFd;

typedef void (*EnFdActionFunc)(struct EnFd* this, PlayState* play);

typedef enum {
    FD_EFFECT_NONE,
    FD_EFFECT_FLAME,
    FD_EFFECT_DOT
} FDEffectType;

#define EN_FD_EFFECT_COUNT 200

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
} EnFdEffect; // size = 0x38

typedef struct EnFd {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ SkelAnime skelAnime;
    /* 0x0190 */ EnFdActionFunc actionFunc;
    /* 0x0194 */ ColliderJntSph collider;
    /* 0x01B4 */ ColliderJntSphElement colSphs[12];
    /* 0x04B4 */ u8 coreActive;
    /* 0x04B6 */ s16 initYawToInitPos;
    /* 0x04B8 */ s16 curYawToInitPos;
    /* 0x04BA */ s16 runDir;
    /* 0x04BC */ s16 firstUpdateFlag;
    /* 0x04BE */ s16 spinTimer;
    /* 0x04C0 */ s16 circlesToComplete;
    /* 0x04C2 */ s16 invincibilityTimer;
    /* 0x04C4 */ s16 attackTimer;
    /* 0x04C8 */ f32 runRadius;
    /* 0x04CC */ f32 fadeAlpha;
    /* 0x04D0 */ Vec3f corePos;
    /* 0x04DC */ Vec3s jointTable[27];
    /* 0x057E */ Vec3s morphTable[27];
    /* 0x0620 */ EnFdEffect effects[EN_FD_EFFECT_COUNT];
} EnFd; // size = 0x31E0

struct ArmsHook;

typedef void (*ArmsHookActionFunc)(struct ArmsHook*, PlayState*);

typedef struct ArmsHook {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderQuad collider;
    /* 0x1C4 */ WeaponInfo weaponInfo;
    /* 0x1E0 */ Vec3f unk1E0;
    /* 0x1EC */ Vec3f unk1EC;
    /* 0x1F8 */ Actor* grabbed;
    /* 0x1FC */ Vec3f unk1FC;
    /* 0x208 */ s8 unk_208;
    /* 0x20A */ s16 timer;
    /* 0x20C */ ArmsHookActionFunc actionFunc;
} ArmsHook; // size = 0x210

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
