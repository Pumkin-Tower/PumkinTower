#ifndef __EN_BO_B_H__
#define __EN_BO_B_H__

#include "ultra64/types.h"
#include <uLib.h>
#include <object/0x0004-EnBoB.h>
#include <ulib_math.h>
#include "uLib_vector.h"

asm("Fog_SetOpa = 0x80026400");
asm("Fog_SetXlu = 0x80026690");
asm("Fog_RestoreOpa = 0x80026608");
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
	SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, this);
#define SKELANIME_DRAW_ALPHA(override, post, alpha, color) \
	do { \
		if (alpha >= 255) { \
			Gfx_SetupDL_25Opa(__gfxCtx); \
			gDPSetEnvColor(POLY_OPA_DISP++, color.r, color.g, color.b, alpha);\
			POLY_OPA_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, this, POLY_OPA_DISP); \
		} else { \
			Gfx_SetupDL_25Xlu(__gfxCtx); \
			gDPSetEnvColor(POLY_XLU_DISP++, color.r, color.g, color.b, alpha); \
			POLY_XLU_DISP = SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->jointTbl, this->skelAnime.dListCount, override, post, this, POLY_XLU_DISP); \
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
#define NEW_MATRIX() Matrix_NewMtx(__gfxCtx, __FILE__, __LINE__)

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