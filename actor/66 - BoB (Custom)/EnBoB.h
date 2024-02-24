#ifndef __EN_BO_B_H__
#define __EN_BO_B_H__

#include "global.h"

extern f32 roundf(f32 x);

/*
#include "ultra64/types.h"
#include <uLib.h>
#include <object/0x0004-EnBoB.h>
#include <ulib_math.h>
#include "uLib_vector.h"
*/

typedef enum {
    BODY_LIMB_ROOT_TRANSLATE,
    BODY_LIMB_MAINBODY,
    BODY_LIMB_LEGS,
    BODY_LIMB_ARM1_L,
    BODY_LIMB_ARM2_L,
    BODY_LIMB_ARM3_L,
    BODY_LIMB_HEAD,
    BODY_LIMB_ARM1_R,
    BODY_LIMB_ARM2_R,
    BODY_LIMB_ARM3_R,
    BODY_LIMB_MAX,
} SkelBodyLimbs;

#if 1 /* region uLib */


static inline Vec3f Vec3f_Add(Vec3f a, Vec3f b) {
    return (Vec3f) { a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline Vec3f Vec3f_MulVal(Vec3f a, f32 val) {
    return (Vec3f) { a.x* val, a.y* val, a.z* val };
}

static inline Vec3f Vec3f_Sub(Vec3f a, Vec3f b) {
    return (Vec3f) { a.x - b.x, a.y - b.y, a.z - b.z };
}

f32 MaxF(f32 a, f32 b) {
    return a >= b ? a : b;
}

f32 MinF(f32 a, f32 b) {
    return a < b ? a : b;
}

s32 MaxS(s32 a, s32 b) {
    return a >= b ? a : b;
}

s32 MinS(s32 a, s32 b) {
    return a < b ? a : b;
}

s32 WrapS(s32 x, s32 min, s32 max) {
    s32 range = max - min;
    
    if (x < min)
        x += range * ((min - x) / range + 1);
    
    return min + (x - min) % range;
}

f32 WrapF(f32 x, f32 min, f32 max) {
    f64 range = max - min;
    
    if (x < min)
        x += range * roundf((min - x) / range + 1);
    
    return min + fmodf((x - min), range);
}

#define UnfoldVec3f(vec) (vec).x, (vec).y, (vec).z

f32 fminf(f32, f32);
f32 fmaxf(f32, f32);
f32 rintf(f32);

static f32 Lerp(f32 x, f32 min, f32 max) {
    return min + (max - min) * x;
}

static f32 Normalize(f32 v, f32 min, f32 max) {
    return (v - min) / (max - min);
}

static f32 Remap(f32 v, f32 iMin, f32 iMax, f32 oMin, f32 oMax) {
    return Lerp(Normalize(v, iMin, iMax), oMin, oMax);
}

Vec3f Math_Vec3f_YawPitchDist(f32 dist, s16 yaw, s16 pitch) {
    Vec3f ret = {
        .x = Math_SinS(yaw) * dist,
        .y = Math_SinS(-pitch) * dist,
        .z = Math_CosS(yaw) * dist,
    };
    
    return ret;
}

#endif

asm("Fog_SetOpa = 0x80026400"); // XXX these others are oot, seem to be unused?
asm("Fog_SetXlu = 0x80026690");
asm("Fog_RestoreOpa = 0x800AE8EC"); // mm
asm("Fog_RestoreXlu = 0x80026A6C");
extern void Fog_SetOpa(PlayState* play, Color_RGBA8* color, s16 mix, s16 magic);
extern void Fog_SetXlu (PlayState* play, Color_RGBA8* color, s16 mix, s16 magic);
extern void Fog_RestoreOpa (PlayState* play);
extern void Fog_RestoreXlu (PlayState* play);

#define SKELANIME_MEMBER(limb_max) \
	SkelAnime skelAnime; \
	Vec3s jointTbl[limb_max]; \
	Vec3s morphTbl[limb_max]
#define SKELANIME_INIT(skeleton, animation, maxLimb) \
	SkelAnime_InitFlex(play, &this->skelAnime, skeleton, animation, this->jointTbl, this->morphTbl, maxLimb);
#define SKELANIME_DRAW_OPA(override, post) \
	SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, (void*)this);
#define SKELANIME_DRAW_ALPHA(override, post, alpha, color) \
	do { \
		if (alpha >= 255) { \
			Gfx_SetupDL25_Opa(play->state.gfxCtx); \
			gDPSetEnvColor(POLY_OPA_DISP++, color.r, color.g, color.b, alpha);\
			POLY_OPA_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, (void*)this, POLY_OPA_DISP); \
		} else { \
			Gfx_SetupDL25_Xlu(play->state.gfxCtx); \
			gDPSetEnvColor(POLY_XLU_DISP++, color.r, color.g, color.b, alpha); \
			POLY_XLU_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, (void*)this, POLY_XLU_DISP); \
		} \
	} while (0)

#define BoB_SCALE             0.015f
#define BoBone_MAX            120
#define MaxHP                 16
#define boneMaxHP             14
#define BoBParticleMax             32
#define BoBProjectileMax      6
#define boneGravity           1.2f
#define stunTimeMAX           30
#define sunsSongStunTimeMAX   100
#define frozenTimeMAX         42
#define fireTimeMAX           40   //Specific for lighting fires on BoB in sequence.
#define rollTimeMAX           100
#define wanderMAX             4
#define prayTimeMAX           70

//object collider -pushes other OCs
//attack toucher -sword attacks
//attack collider -checking for attacks against

#define attackcollidermacro\
  	.base.shape            = COLSHAPE_CYLINDER,\
		.base.actor            = &this->actor,\
		.base.colType          = COLTYPE_HIT3,\
		.base.atFlags          = AT_TYPE_ENEMY | AT_ON,\
		.base.ocFlags1         = OC1_ON | OC1_TYPE_PLAYER,\
		.base.ocFlags2         = OC2_TYPE_1,\
		.base.acFlags          = AC_ON | AC_TYPE_PLAYER,\
		.info.ocElemFlags      = OCELEM_ON,\
		.info.elemType         = ELEMTYPE_UNK0,\
		.info.toucher.dmgFlags = DMG_DEFAULT,\
		.info.toucher.damage   = 0x20,\
		.info.toucherFlags     = TOUCH_ON | TOUCH_SFX_HARD,\
		.info.bumper.dmgFlags  = DMG_DEFAULT,\
		.info.bumperFlags      = BUMP_ON,\
		.dim.radius            = 15,\
		.dim.height            = 30,\

#define AggroRange(radius) this->actor.xyzDistToPlayerSq < SQ(radius)
#define IdleRange(radius)  this->actor.xyzDistToPlayerSq > SQ(radius)

#define INTN_MAX(x)  (((int)1 << ((x) - 1)) - 1)
#define INTN_MIN(x)  (-((int)1 << ((x) - 1)))
#define NEW_MATRIX() Matrix_NewMtx(play->state.gfxCtx)

typedef struct BoBone {
	s8 dist;
	s8 roll;
} BoBone;

typedef struct {
	Vec3f pos;
	Vec3f vel;
	Vec3s rot;
	u8    timer;
} BoBParticle;

typedef struct {
	Vec3f pos;
	Vec3f vel;
	Vec3s rot;
	f32 grav;
	s16 timer;
	_Bool isOut:1;
	_Bool isLanded:1;
	_Bool isDeflected:1;
} BoBProjectile;

struct EnBoB;

typedef void (* EnBoBFunc)(struct EnBoB*, PlayState*);

typedef struct EnBoB {
	Actor actor;
	Vec3s bodyPartsPos[5]; // DO NOT MOVE THIS NEEDED FOR FIRE AT THIS LOCATION
	SKELANIME_MEMBER(BODY_LIMB_MAX);
	
	ColliderCylinder collider;
	ColliderCylinder bodycollider;
	ColliderCylinder attackcollider[BoBProjectileMax];
	
	EnBoBFunc func; // Type   Name
	s16       	alpha;
	s16       	timer;
	s16 	  	initTimer;
	s16       	despawntimer;
	s16       	fireTimer;
	s16       	iceTimer;
	s16       	stunTimer;
	u8        	targetColor;
	u8        	hitcount;
	s16      	colorTimer;
	s16      	colorTimerMax;
	s16      	onFireTimer;
	u8        	prayTimer;
	u8       	lastaction;
	s16       	idleTimer;
	s16       	delayTimer;
	s16       	wanderTimer;
	s16       	wanderTimerMax;
	s16       	wandercount;
	s16       	wanderAngle;
	s16 	    boneColor[3];
	s16 	    boneColorTarget[3];
	
	Vec3f bone_mass_center;
	Vec3s bone_mass_rot;
	f32   bone_range_min;
	f32   bone_range_max;
	
	BoBProjectile projectiles[BoBProjectileMax];
	u8 numprojectile;
	
	BoBParticle particles[BoBParticleMax];
	u8 numparticles;
	
	BoBone Bone[BoBone_MAX];
	int    numbones;
	s16    boneHP;
	
	s16 sunsSongStunTimer;
	s16 switchFlag;
	Vec3f rollTarget;
	Vec3f wanderTarget;
	s16 chosenOne;
	
	_Bool timeStop:1;
	_Bool EnBoB_Dead:1;
	_Bool onFire:1;
	_Bool isMoving:1;
	_Bool idle:1;
} EnBoB;

#endif // __EN_BO_B_H__S



#ifndef __BOB_H__
#define __BOB_H__

extern u16 gBoB_TexBoBody1[];
extern u16 gBoB_TexBoBFace1[];
extern u16 gBoB_TexBone2[];
extern u16 gBoB_TexEffectBall[];
extern u16 gBoB_TexBoneSpear[];
extern Gfx gBoB_MtlBody[];
extern Gfx gBoB_MtlHead[];
extern Gfx gBoB_MtlBones[];
extern Gfx gBoB_MtlBallEffect[];
extern Gfx gBoB_MtlProjectile[];
extern Gfx gBoB_DlBodyMainBody[];
extern Gfx gBoB_DlBodyLegs[];
extern Gfx gBoB_DlBodyArm1L[];
extern Gfx gBoB_DlBodyArm2L[];
extern Gfx gBoB_DlBodyArm3L[];
extern Gfx gBoB_DlBodyHead[];
extern Gfx gBoB_DlBodyArm1R[];
extern Gfx gBoB_DlBodyArm2R[];
extern Gfx gBoB_DlBodyArm3R[];
extern FlexSkeletonHeader gBoB_SkelBody[];

extern Gfx gBoB_DlFlatBone[];
extern Gfx gBoB_DlEffect[];
extern Gfx gBoB_DlEffect2[];
extern Gfx gBoB_DlProjectile[];
extern AnimationHeader gBoB_AnimCrawl[];
extern AnimationHeader gBoB_AnimCry[];
extern AnimationHeader gBoB_AnimDamage[];
extern AnimationHeader gBoB_AnimDeath[];
extern AnimationHeader gBoB_AnimPray[];
extern AnimationHeader gBoB_AnimShootAttack[];

#endif /* __BOB_H__ */



asm("gBoB_TexBoBody1 = 0x06000000");
asm("gBoB_TexBoBFace1 = 0x06000800");
asm("gBoB_TexBone2 = 0x06001000");
asm("gBoB_TexEffectBall = 0x06001200");
asm("gBoB_TexBoneSpear = 0x06002200");
asm("gBoB_MtlBody = 0x06002870");
asm("gBoB_MtlHead = 0x06002920");
asm("gBoB_MtlBones = 0x060029D0");
asm("gBoB_MtlBallEffect = 0x06002A88");
asm("gBoB_MtlProjectile = 0x06002B08");
asm("gBoB_DlBodyMainBody = 0x06002F10");
asm("gBoB_DlBodyLegs = 0x06003550");
asm("gBoB_DlBodyArm1L = 0x060037E8");
asm("gBoB_DlBodyArm2L = 0x06003908");
asm("gBoB_DlBodyArm3L = 0x06003A18");
asm("gBoB_DlBodyHead = 0x06003B78");
asm("gBoB_DlBodyArm1R = 0x06003CD8");
asm("gBoB_DlBodyArm2R = 0x06003DF8");
asm("gBoB_DlBodyArm3R = 0x06003F08");
asm("gBoB_SkelBody = 0x06003FE8");
asm("gBoB_DlFlatBone = 0x06004028");
asm("gBoB_DlEffect = 0x06004088");
asm("gBoB_DlEffect2 = 0x060040E8");
asm("gBoB_DlProjectile = 0x06004238");
asm("gBoB_AnimCrawl = 0x06004610");
asm("gBoB_AnimCry = 0x06004ED0");
asm("gBoB_AnimDamage = 0x06005198");
asm("gBoB_AnimDeath = 0x06005FA0");
asm("gBoB_AnimPray = 0x06006BC0");
asm("gBoB_AnimShootAttack = 0x060075C4");



