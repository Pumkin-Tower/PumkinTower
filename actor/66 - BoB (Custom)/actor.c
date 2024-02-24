//BoB- Ball of Bones Custom Miniboss for use in z64rom
//Var 003f - Switch Flag

#include "EnBoB.h"

static Vec3f gZeroVec = { 0, 0, 0 };

// XXX is this right?
#define Actor_ChangeCategory func_800BC154
#define func_8008EEAC func_80123E90
#define Actor_MoveForward Actor_MoveWithGravity//Actor_UpdateVelocityWithGravity
#define Animation_ChangeByInfo Actor_ChangeAnimationByInfo
#define func_8002F6D4 func_800B8D50

asm("memset = __osMemset");

// inspired by OoT, is it equivalent?
#define DMG_DEFAULT ~(DMG_NORMAL_SHIELD | DMG_LIGHT_RAY)

// TODO don't use placeholder
//#define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_2 | ACTOR_FLAG_4 | ACTOR_FLAG_5)
#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_2000000)

void EnBoB_Init(EnBoB* this, PlayState* play);
void EnBoB_Destroy(EnBoB* this, PlayState* play);
void EnBoB_Update(EnBoB* this, PlayState* play);
void EnBoB_Draw(EnBoB* this, PlayState* play);

void EnBoB_Idle(EnBoB* this, PlayState* play);
void EnBoB_Cry(EnBoB* this, PlayState* play);
void EnBoB_Pray(EnBoB* this, PlayState* play);
void EnBoB_Death(EnBoB* this, PlayState* play);
void EnBoB_SetupDeath(EnBoB* this, PlayState* play);
void EnBoB_DislodgeBones(EnBoB* this);
void EnBoB_Shoot(EnBoB* this, PlayState* play);
void EnBoB_SetupWander(EnBoB* this, PlayState* play);
void EnBoB_Wander(EnBoB* this);
void EnBoB_SetupRoll(EnBoB* this, PlayState* play);
void EnBoB_Roll(EnBoB* this, PlayState* play);
void EnBoB_PickupBones(EnBoB* this, PlayState* play);
void EnBoB_KnockOver(EnBoB* this, PlayState* play);
void EnBoB_InitProjectiles(EnBoB* this, PlayState* play);
s32 PingPongS(s32 v, s32 min, s32 max);
f32 PingPongF(f32 v, f32 min, f32 max);

const ActorInit ZzrtlInitVars = {
	66,
	ACTORCAT_ENEMY,
	FLAGS,
	47,
	sizeof(EnBoB),
	(ActorFunc)EnBoB_Init,
	(ActorFunc)EnBoB_Destroy,
	(ActorFunc)EnBoB_Update,
	(ActorFunc)EnBoB_Draw
};


static Gfx setup25[] = {
	/* SETUPDL_25 */
	gsDPPipeSync(),
	gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
	gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_MODULATEIA_PRIM2),
	gsDPSetOtherMode(G_AD_NOTPATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
		G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE,
		G_AC_NONE | G_ZS_PIXEL | G_RM_FOG_SHADE_A | G_RM_AA_ZB_OPA_SURF2),
	gsSPLoadGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
	gsSPEndDisplayList(),
};

/* static void Gfx_SetupDL25_Xlu(GraphicsContext* gfxCtx) {
	OPEN_DISPS(gfxCtx);
	
	gSPDisplayList(POLY_XLU_DISP++, setup25);
	
	CLOSE_DISPS(gfxCtx);
} */

/* static void Gfx_SetupDL25_Opa(GraphicsContext* gfxCtx) {
	OPEN_DISPS(gfxCtx);
	
	gSPDisplayList(POLY_OPA_DISP++, setup25);
	
	CLOSE_DISPS(gfxCtx);
} */

typedef enum {
	/* 0x0 */ Color_None,
	/* 0x1 */ Color_White,
	/* 0x2 */ Color_Red,
	/* 0x3 */ Color_Blue,
} ColorEffect;

static Color_RGB8 sColorTable[] = {
	{255,255,255}, //none
	{255,255,255}, //white
	{255,100,100}, //red
	{200,200,255}, //blue
};

static Color_RGB8 sFogColorTable[] = {
	{0,0,0},       //none
	{255,255,255}, //white
	{180,0,0}, 	   //red
	{0,0,180},     //blue

};

typedef enum {
	/* 0x0 */ BoB_DMGEFF_NONE,
	/* 0x1 */ BoB_DMGEFF_STUN,
	/* 0x2 */ BoB_DMGEFF_ICE,
	/* 0x3 */ BoB_DMGEFF_FIRE,
	/* 0x4 */ BoB_DMGEFF_LIGHT,
	/* 0x5 */ BoB_DMGEFF_BONUS,
} BoBDamageEffect;

static DamageTable sDamageTable = {
	/* Deku nut      */ DMG_ENTRY(0, BoB_DMGEFF_STUN),
	/* Deku stick    */ DMG_ENTRY(1, 0x0),
	/* Slingshot     */ DMG_ENTRY(0, 0x0),
	/* Explosive     */ DMG_ENTRY(1, BoB_DMGEFF_BONUS),
	/* Boomerang     */ DMG_ENTRY(0, BoB_DMGEFF_STUN),
	/* Normal arrow  */ DMG_ENTRY(1, 0x0),
	/* Hammer swing  */ DMG_ENTRY(2, BoB_DMGEFF_BONUS),
	/* Hookshot      */ DMG_ENTRY(0, BoB_DMGEFF_STUN),
	/* Kokiri sword  */ DMG_ENTRY(1, 0x0),
	/* Master sword  */ DMG_ENTRY(2, 0x0),
	/* Giant's Knife */ DMG_ENTRY(2, BoB_DMGEFF_BONUS),
	/* Fire arrow    */ DMG_ENTRY(4, BoB_DMGEFF_FIRE),
	/* Ice arrow     */ DMG_ENTRY(2, BoB_DMGEFF_ICE),
	/* Light arrow   */ DMG_ENTRY(4, BoB_DMGEFF_LIGHT),
	/* Unk arrow 1   */ DMG_ENTRY(1, 0x0),
	/* Unk arrow 2   */ DMG_ENTRY(1, 0x0),
	/* Unk arrow 3   */ DMG_ENTRY(1, 0x0),
	/* Fire magic    */ DMG_ENTRY(4, BoB_DMGEFF_FIRE),
	/* Ice magic     */ DMG_ENTRY(1, BoB_DMGEFF_ICE),
	/* Light magic   */ DMG_ENTRY(1, BoB_DMGEFF_LIGHT),
	/* Shield        */ DMG_ENTRY(0, 0x0),
	/* Mirror Ray    */ DMG_ENTRY(0, BoB_DMGEFF_LIGHT),
	/* Kokiri spin   */ DMG_ENTRY(2, 0x0),
	/* Giant spin    */ DMG_ENTRY(4, 0x0),
	/* Master spin   */ DMG_ENTRY(2, 0x0),
	/* Kokiri jump   */ DMG_ENTRY(2, 0x0),
	/* Giant jump    */ DMG_ENTRY(4, BoB_DMGEFF_BONUS),
	/* Master jump   */ DMG_ENTRY(2, 0x0),
	/* Unknown 1     */ DMG_ENTRY(0, 0x0),
	/* Unblockable   */ DMG_ENTRY(0, 0x0),
	/* Hammer jump   */ DMG_ENTRY(4, BoB_DMGEFF_BONUS),
	/* Unknown 2     */ DMG_ENTRY(0, 0x0),
};

typedef enum {
	/* 0x0 */ AnimCry,
	/* 0x1 */ AnimDeath,
	/* 0x2 */ AnimPray,
	/* 0x3 */ AnimShootAttack,
	/* 0x4 */ AnimCrawl,
	/* 0x5 */ AnimDamage,
	/* 0x6 */ AnimCrawlFast,
} BoBAnims;

//typedef struct {
//    /* 0x00 */ AnimationHeader* animation;
//    /* 0x04 */ f32              playSpeed;
//    /* 0x08 */ f32              startFrame;
//    /* 0x0C */ f32              frameCount;
//    /* 0x10 */ u8               mode;
//    /* 0x14 */ f32              morphFrames;
//} AnimationInfo; // size = 0x18

AnimationInfo sBoBAnims[] = {
	{ gBoB_AnimCry,           1, 0, -1, ANIMMODE_LOOP, 10 },
	{ gBoB_AnimDeath,         1, 0, -1, ANIMMODE_ONCE, 40 },
	{ gBoB_AnimPray,          1, 0, -1, ANIMMODE_LOOP, 5  },
	{ gBoB_AnimShootAttack,   1, 0, -1, ANIMMODE_ONCE, 5  },
	{ gBoB_AnimCrawl, 	      1, 0, -1, ANIMMODE_LOOP, 5  },
	{ gBoB_AnimDamage,        1, 0, -1, ANIMMODE_ONCE, 1  },
	{ gBoB_AnimCrawl, 	      2, 0, -1, ANIMMODE_LOOP, 5  }
};

void EnBoB_SetAction(EnBoB* this, EnBoBFunc func) {
	this->func = func;
}

int GetCurrentBoneMax(EnBoB* this) {
		return (f32)this->boneHP / boneMaxHP * BoBone_MAX;
}

Color_RGB8 GetCurrentColor (EnBoB* this){
	return sColorTable[this->targetColor];
}

Color_RGB8 GetFogCurrentColor (EnBoB* this){
	return sFogColorTable[this->targetColor];
}

f32 GetMassRangeMin(EnBoB* this) {
	f32 target = 25;

	if(PLAYER_FORM_HUMAN)
		target = 20;
	
	if (this->skelAnime.animation == gBoB_AnimShootAttack)
		target += 12;

	if(this->func == EnBoB_Roll || this->func == EnBoB_Pray)
		target += 30;
	
	Math_SmoothStepToF(&this->bone_range_min, target, 0.25f, 0.015f, 0.001f);
	return this->bone_range_min;
}

f32 GetMassRangeMax(EnBoB* this) {
	f32 target = 35;

	if(PLAYER_FORM_HUMAN/* LINK_IS_CHILD */)
		target = 30;	

	if (this->skelAnime.animation == gBoB_AnimShootAttack)
		target += 14;

	if(this->func == EnBoB_Roll || this->func == EnBoB_Pray)
		target += 30;
		
	Math_SmoothStepToF(&this->bone_range_max, target, 0.25f, 0.015f, 0.001f);
	return this->bone_range_max;
}

int excluderandom(int lastchosen){
	//Prevent Rerolls to same position
	int excluderandom = Rand_S16Offset(0, 6);
	
	while (excluderandom == lastchosen){
		excluderandom = Rand_S16Offset(0, 6);
	}
	return excluderandom;
}

void EnBoB_UpdateDamage(EnBoB* this, PlayState* play) {

	if (this->collider.base.acFlags & AC_HIT && this->colorTimer == 0 && this->boneHP > 0) {
		this->collider.base.acFlags &= ~AC_HIT;
		//If Bob still has bones around him remove 1 from boneHP when hit and exit function
		if (this->boneHP > 0) {
			this->boneHP = CLAMP_MIN(this->boneHP-1, 0);
			EnBoB_DislodgeBones(this);
			Actor_PlaySfx(&this->actor, NA_SE_EV_HONEYCOMB_BROKEN);

			//bonus damage against bones
			if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_BONUS)
				this->boneHP = CLAMP_MIN(this->boneHP-1, 0);

			this->colorTimer = 8;
			this->colorTimerMax = this->colorTimer;
			return;
		}
	}
	
	//Reset SunsSong active flag if bob still has bones
	if (this->boneHP > 0) {
		gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
		return;
	}

	//Handles Freezing from SunsSong Effect
	if (this->boneHP == 0 && gSaveContext.sunsSongState != SUNSSONG_INACTIVE && this->colorTimer == 0 && AggroRange(150)){
			this->sunsSongStunTimer = sunsSongStunTimeMAX;
			this->targetColor = Color_White;
			this->colorTimer = sunsSongStunTimeMAX;
			this->colorTimerMax = this->colorTimer;

			 Audio_PlaySfx_2(NA_SE_EN_LIGHT_ARROW_HIT);
	}

	//If hit main body check for damage effects
	if (this->bodycollider.base.acFlags & AC_HIT){
		this->bodycollider.base.acFlags &= ~AC_HIT;

		if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_STUN) {
			this->stunTimer = stunTimeMAX;
			this->targetColor = Color_Blue;
			this->colorTimer = stunTimeMAX;
			this->colorTimerMax = this->colorTimer;
			gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
			Audio_PlaySfx_2(NA_SE_EN_COMMON_FREEZE);
			return;
		}
		this->stunTimer = 0;
		EnBoB_SetupDeath(this, play);

		if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_LIGHT) {
			this->sunsSongStunTimer = sunsSongStunTimeMAX;
			this->targetColor = Color_White;
			this->colorTimer = sunsSongStunTimeMAX;
			this->colorTimerMax = this->colorTimer;
			 Audio_PlaySfx_2(NA_SE_EN_LIGHT_ARROW_HIT);
			Actor_ApplyDamage(&this->actor);
			return;
		}
		this->sunsSongStunTimer = 0;
		gSaveContext.sunsSongState = SUNSSONG_INACTIVE;

		if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_ICE) {
			this->iceTimer = frozenTimeMAX;
			this->targetColor = Color_Blue;
			this->colorTimer = frozenTimeMAX;
			this->actor.colorFilterTimer = frozenTimeMAX; //EffectSsEnIce_SpawnFlyingVec3f
			this->colorTimerMax = this->colorTimer;
			Actor_ApplyDamage(&this->actor);
			return;
		}
		this->iceTimer = 0;
		this->timeStop = false;
		
		if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_NONE || this->actor.colChkInfo.damageEffect == BoB_DMGEFF_BONUS){
			this->targetColor = Color_Red;
			this->colorTimer = 8;
			this->colorTimerMax = this->colorTimer;
			Actor_ApplyDamage(&this->actor);

			if (this->EnBoB_Dead != true){
				Audio_PlaySfx_2(NA_SE_EN_REDEAD_DAMAGE);
				this->hitcount++;
				Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimDamage);
			}

			// XXX health
			if ((this->hitcount == 3 || this->hitcount == 8 || this->hitcount == 12) && this->actor.colChkInfo.health != 0 && this->numprojectile > 0){
				EnBoB_SetAction(this, EnBoB_SetupRoll);
				this->lastaction = 4;
			}
			return;
		}
		
		if (this->actor.colChkInfo.damageEffect == BoB_DMGEFF_FIRE) {
			this->targetColor = Color_Red;
			this->fireTimer = fireTimeMAX;
			this->actor.colorFilterTimer = 0x50; // for EffectSsEnFire_SpawnVec3s
			this->colorTimer = fireTimeMAX;
			this->onFire = true;
			this->onFireTimer = 70;
			this->colorTimerMax = this->colorTimer;
			Actor_ApplyDamage(&this->actor);
			Audio_PlaySfx_2(NA_SE_EN_REDEAD_DAMAGE);
			return;
		}
	}
}

void EnBoB_SetupDeath(EnBoB* this, PlayState* play) {
   // XXX health
	if (this->actor.colChkInfo.health == 0 && this->EnBoB_Dead == false){
		this->despawntimer = 120;
		Enemy_StartFinishingBlow(play, &this->actor);
		Audio_PlaySfx_2(NA_SE_EN_REDEAD_DEAD);
		Item_DropCollectibleRandom(play, NULL, &this->actor.world.pos, 0x90);
		Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimDeath);
		this->EnBoB_Dead = true;
		Audio_RestorePrevBgm();
	}
}

void EnBoB_DislodgeBones(EnBoB* this) {
	u8 newnum = this->numparticles;

	this->numparticles += Rand_S16Offset(8, 8); //(Minimum, Rand Addition Max)
	this->numparticles = CLAMP_MAX(this->numparticles, BoBParticleMax);
	newnum = this->numparticles - newnum;
	
	if (newnum == 0)
		return;
	
	for (int i = 0; i < BoBParticleMax && newnum > 0; ++i) {
		BoBParticle* p = &this->particles[i];
		//shorthand for &this->particles.pos[i]
		
		if (p->timer == 0) {
			newnum--;
			p->pos = this->bone_mass_center;
			p->pos.x += Rand_CenteredFloat(5.0f);
			p->pos.y += Rand_CenteredFloat(5.0f);
			p->pos.z += Rand_CenteredFloat(5.0f);
			p->vel.x = Rand_CenteredFloat(12.0f);
			p->vel.y = Rand_CenteredFloat(5.0f) + 8.0f;
			p->vel.z = Rand_CenteredFloat(12.0f);
			p->rot.x += Rand_S16Offset(0, 0x4000);
			p->rot.y += Rand_S16Offset(0, 0x4000);
			p->rot.z += Rand_S16Offset(0, 0x4000);
			p->timer = 40;
		}
	}
}

void EnBoB_DislodgeBonesUpdateDraw(EnBoB* this, PlayState *play)
{
	OPEN_DISPS(play->state.gfxCtx);
	
	if (this->numparticles!= 0)
		gSPDisplayList(POLY_OPA_DISP++, gBoB_MtlBones);
	
	for (int i = 0; i < BoBParticleMax && this->numparticles > 0; ++i) {
		BoBParticle* p = &this->particles[i];
		
		Vec3f prevpos = p->pos;
		
		if (p->timer == 0)
			continue;
		
		p->timer--;
		
		if (p->timer == 0)
			this->numparticles--;
		
		p->pos = Vec3f_Add(p->pos, p->vel);
		p->vel = Vec3f_MulVal(Vec3f_Sub(p->pos, prevpos), 0.95f);
		p->vel.y -= boneGravity;
		p->rot.x += Rand_S16Offset(500, 200);
		p->rot.y += Rand_S16Offset(500, 200);
		p->rot.z += Rand_S16Offset(500, 200);
		
		Matrix_Translate(UnfoldVec3f(p->pos), MTXMODE_NEW);
		Matrix_RotateZYX(UnfoldVec3f(p->rot), MTXMODE_APPLY);
		Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
		gSPMatrix(POLY_OPA_DISP++, NEW_MATRIX(), G_MTX_LOAD);
		
		gSPDisplayList(POLY_OPA_DISP++, gBoB_DlFlatBone + 1);
	}
	
	CLOSE_DISPS();
}

void EnBoB_Shoot(EnBoB* this, PlayState* play) {

	if (Animation_OnFrame(&this->skelAnime, 7.0f))
		Audio_PlaySfx_2(NA_SE_EN_BUBLE_BITE);

	if (Animation_OnFrame(&this->skelAnime, 11.0f)){
		for (int i = 0; i < BoBProjectileMax; ++i) {
			BoBProjectile* p = &this->projectiles[i];

			if (p->isOut == false) {
				p->timer = CLAMP_MIN(this->actor.xzDistToPlayer / 30.0f + 1, 0);
				p->isLanded = false;
				p->isDeflected = false;
				p->vel = Math_Vec3f_YawPitchDist(30.0f, this->actor.yawTowardsPlayer, 0.0);
				p->rot.y = this->actor.yawTowardsPlayer - DEG_TO_BINANG(90);
				p->pos = this->bone_mass_center;
				p->grav = 0;
				p->isOut = true;
				this->numprojectile++;
				break;
			}
		}
	}

	if (Animation_OnFrame(&this->skelAnime, Animation_GetLastFrame(gBoB_AnimShootAttack)) || this->colorTimer == 1){	
		Animation_ChangeByInfo(&this->skelAnime, sBoBAnims, AnimCry);
		EnBoB_SetAction(this, EnBoB_Cry);
	}
}

void EnBoB_ProjectileCollision(EnBoB* this, PlayState* play) {

	Player* player = GET_PLAYER(play);

	for (int i = 0; i < BoBProjectileMax; i++) {
		if (this->projectiles[i].isOut == false)
			continue;
		BoBProjectile* p = &this->projectiles[i];
		ColliderCylinder* atkcol = &this->attackcollider[i];

		if (atkcol->base.at == &player->actor && p->isLanded != true){
			ColliderInfo* info = atkcol->info.atHitInfo;

			if (info != NULL && info->toucher.dmgFlags == DMG_NORMAL_SHIELD && p->isDeflected == false){
				p->isDeflected = true;
				player->invincibilityTimer = 4;
				p->vel.x = -0.9f*p->vel.x;
				p->vel.z = -0.9f*p->vel.z;
			}
		}

		Collider_UpdateCylinder(&this->actor, atkcol);
		CollisionCheck_SetOC(play, &play->colChkCtx, &atkcol->base);
		CollisionCheck_SetAT(play, &play->colChkCtx, &atkcol->base);
		CollisionCheck_SetAC(play, &play->colChkCtx, &atkcol->base);
	}
}

void EnBoB_ShootUpdateDraw(EnBoB* this, PlayState* play)
{
	OPEN_DISPS(play->state.gfxCtx);
	
	for (int i = 0; i < BoBProjectileMax; ++i) {
		BoBProjectile* p = &this->projectiles[i];

		if (p->isOut == false)
			continue;

		Vec3s vec = { UnfoldVec3f(p->pos) };
		this->attackcollider[i].dim.pos = vec;

		if (p->isLanded == false){
			CollisionPoly* poly;
			Vec3f result, next;

			if (this->initTimer != 0){
				p->rot.x += 0x800;
				p->rot.y = 0x0;
				p->rot.z = DEG_TO_BINANG(-90);
			}else{
				p->rot.z = 0x0;
				p->rot.x += 0x800;//Rotate projectile in motion along roll axis
			}

			if(p->isDeflected == true)
				p->rot.y = this->actor.yawTowardsPlayer + DEG_TO_BINANG(90);
			
			this->attackcollider[i].dim.pos.y -= 20;
			next = Vec3f_Add(p->pos, p->vel);
			
			if (!BgCheck_AnyLineTest1(&play->colCtx,&p->pos,&next,&result,&poly,true)){
				p->pos = Vec3f_Add(p->pos, p->vel);
				if (p->timer-- <= 0)
					p->vel.y -= SQ(p->grav);
				if(p->timer == -40){
					p->isOut = false;
					DECR(this->numprojectile);
				}// failsafe if projectile is flying for 40frames without hit

				Math_StepToF(&p->grav,1.0f,0.25f);
			}else{
				p->pos = result;

				if (p->timer <= -4){
					p->rot.x = 0x0; //reset roll rotation
					p->rot.y = 0x0;
					p->rot.z = DEG_TO_BINANG(-90);
				}

				p->isLanded = true;
				AudioSfx_PlaySfx(NA_SE_EN_GOMA_DOWN,&p->pos, 4, &gSfxDefaultFreqAndVolScale,&gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
			}
		}

		Matrix_Translate(UnfoldVec3f(p->pos), MTXMODE_NEW);
		Matrix_RotateZYX(UnfoldVec3f(p->rot), MTXMODE_APPLY);
		Matrix_Scale(this->actor.scale.y*0.67f, 0.01f, 0.01f, MTXMODE_APPLY);
		gSPMatrix(POLY_OPA_DISP++, NEW_MATRIX(), G_MTX_LOAD);
	
		gSPDisplayList(POLY_OPA_DISP++, gBoB_DlProjectile);
	}
	CLOSE_DISPS();
}

void EnBoB_Death(EnBoB* this, PlayState* play) {
	if (this->actor.category != ACTORCAT_PROP)
		Actor_ChangeCategory(play, &play->actorCtx, &this->actor, ACTORCAT_PROP);
	
	if (SkelAnime_Update(&this->skelAnime)) {
		if (this->despawntimer == 0) {
			//Audio_PlayAmbience(u8 ambienceId);// TODO something related to sequences?
			if (this->alpha != 0) {
				this->actor.scale.y -= 0.000075f;
				this->alpha -= 5;
			} else {
				Actor_Kill(&this->actor);
				Flags_SetSwitch(play, this->switchFlag);
			}
		}
	}
}

void EnBoB_HealthManager(EnBoB* this) {
	
	//Bone Health Management
	if (this->boneHP < boneMaxHP && this->boneHP > 0) {
		if ((this->timer == 0 && this->idle == true) || (this->timer == 0 &&this->func == EnBoB_Pray)) {
			this->boneHP++;
			this->timer = Rand_S16Offset(60, 10);
		}
		return;
	}
}

void EnBoB_Init(EnBoB* this, PlayState* play) {

	Actor_SetScale(&this->actor, BoB_SCALE);
	this->actor.shape.yOffset = 40.0f / BoB_SCALE;

	SKELANIME_INIT(gBoB_SkelBody, gBoB_AnimCry, BODY_LIMB_MAX);
	EnBoB_SetAction(this, EnBoB_Idle);
	ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
	EnBoB_InitProjectiles(this,play);
	this->EnBoB_Dead = false;
	this->alpha = 255;
	this->initTimer = 20;
	this->idle = true;
	this->hitcount = 0;
	this->wanderTimerMax = 80;
	this->actor.gravity = -2.0f;
	this->boneColor[0] = 0xFF;
	this->boneColor[1] = 0xFF;
	this->boneColor[2] = 0xFF;
	
	this->switchFlag = this->actor.params & 0xFF;

	//Kill actor if switchflag set
	if ((this->switchFlag != 0xFF) && Flags_GetSwitch(play, this->switchFlag))
    Actor_Kill(&this->actor);

	this->actor.colChkInfo.damageTable = &sDamageTable;
	this->actor.colChkInfo.health = MaxHP;
	
	this->boneHP = boneMaxHP;
	//this->actor.naviEnemyId = 0xB;
	this->actor.targetMode = 2;
	
	for (int i = 0; i < BoBone_MAX; ++i) {
		this->Bone[i].dist = Rand_S16Offset(INTN_MIN(8), 0xFF);
		this->Bone[i].roll = Rand_S16Offset(INTN_MIN(8), 0xFF);
	}
	
	this->collider = (ColliderCylinder) {
		.base.shape            = COLSHAPE_CYLINDER,
		.base.actor            = &this->actor,
		.base.colType          = COLTYPE_HIT3,
		.base.atFlags          = AT_TYPE_ENEMY | AT_ON,
		.base.ocFlags1         = OC1_ON | OC1_TYPE_ALL,
		.base.ocFlags2         = OC2_TYPE_1,
		.base.acFlags          = AC_ON | AC_TYPE_PLAYER,
		.info.ocElemFlags      = OCELEM_ON,
		.info.toucher.dmgFlags = DMG_DEFAULT,
		.info.toucher.damage   = 0x5,
		.info.toucherFlags     = TOUCH_ON,
		.info.bumper.dmgFlags  = DMG_DEFAULT,
		.info.bumperFlags      = BUMP_ON,
		.dim.radius            = 50,
		.dim.height            = 70,
	};
	
	this->bodycollider = (ColliderCylinder) {
		.base.shape            = COLSHAPE_CYLINDER,
		.base.actor            = &this->actor,
		.base.colType          = COLTYPE_HIT7,
		.base.atFlags          = AT_TYPE_ENEMY | AT_ON,
		.base.ocFlags1         = OC1_ON | OC1_TYPE_ALL,
		.base.ocFlags2         = OC2_TYPE_1,
		.base.acFlags          = AC_ON | AC_TYPE_PLAYER,
		.info.ocElemFlags      = OCELEM_ON,
		.info.toucher.dmgFlags = DMG_DEFAULT,
		.info.toucher.damage   = 0x5,
		.info.toucherFlags     = TOUCH_ON,
		.info.bumper.dmgFlags  = DMG_DEFAULT,
		.info.bumperFlags      = BUMP_ON,
		.dim.radius            = 35,
		.dim.height            = 70,
	};

	this->attackcollider[0] = (ColliderCylinder) {
	attackcollidermacro
	};
	this->attackcollider[1] = (ColliderCylinder) {
	attackcollidermacro
	};
	this->attackcollider[2] = (ColliderCylinder) {
	attackcollidermacro
	};
	this->attackcollider[3] = (ColliderCylinder) {
	attackcollidermacro
	};
	this->attackcollider[4] = (ColliderCylinder) {
	attackcollidermacro
	};
	this->attackcollider[5] = (ColliderCylinder) {
	attackcollidermacro
	};

	this->actor.colChkInfo.damageTable = &sDamageTable;
	this->actor.colChkInfo.mass = MASS_IMMOVABLE; // mass -> colChkInfo.mass
	this->timeStop = false;
}

void EnBoB_Idle (EnBoB* this, PlayState* play) {
	
	s16 idletemprand = Rand_S16Offset(0, 3);

	if (idletemprand == 0 && this->lastaction != 0 && this->idleTimer == 0){
		this->idleTimer = 60;
		this->lastaction = 0;

	}else if (idletemprand == 1 && this->lastaction != 2 && this->idleTimer == 0){
		Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimPray);
		this->prayTimer = prayTimeMAX;
		EnBoB_SetAction(this, EnBoB_Pray);
		this->lastaction = 2;
		this->actor.gravity = 0.5f;
		this->actor.velocity.y = 0.0f;

	}else if (idletemprand == 2 && this->lastaction != 3 && this->idleTimer == 0){
		if (this->boneHP == 0)
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCrawl);
		EnBoB_SetAction(this, EnBoB_SetupWander);
		this->lastaction = 3;
	}
}

void EnBoB_Cry(EnBoB* this, PlayState* play) {
	
	s16 temprand = Rand_S16Offset(0, 5);
	play->envCtx.lightSettingOverride = 0;

	//Delay before picking new action
	DECR(this->delayTimer);

	if (this->delayTimer == 0){

		if (temprand < 2 && this->lastaction != 4  && this->numprojectile < BoBProjectileMax){
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimShootAttack);
			EnBoB_SetAction(this, EnBoB_Shoot);
			this->lastaction = 1;
			this->delayTimer = 10;

		}else if (temprand == 1 && this->lastaction != 2 && this->boneHP != 0) {
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimPray);
			this->prayTimer = prayTimeMAX;
			EnBoB_SetAction(this, EnBoB_Pray);
			this->lastaction = 2;
			this->actor.gravity = 0.5f;
			this->actor.velocity.y = 0.0f;
			this->delayTimer = 30;

		}else if (temprand == 2 && this->lastaction != 3){
			if (this->boneHP == 0)
				Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCrawl);
			EnBoB_SetAction(this, EnBoB_SetupWander);
			this->lastaction = 3;
			this->delayTimer = 10;

		}else if(temprand == 3 && this->numprojectile > 0 && this->lastaction != 4){
			if (this->boneHP == 0){
				if (this->hitcount == 3 || this->hitcount == 8 || this->hitcount == 12){
					Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCrawlFast);
				}else{
					Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCrawl);
				}
			}
			EnBoB_SetAction(this, EnBoB_SetupRoll);
			this->lastaction = 4;
			this->delayTimer = 35;

		}else{
			EnBoB_SetAction(this, EnBoB_Cry);
		}
	}
}

void EnBoB_Pray(EnBoB* this, PlayState* play) {

	if (this->prayTimer > prayTimeMAX/2 + 5){
		Math_SmoothStepToF(&this->actor.gravity, 0.0f, 0.1f, 0.5f, 0.0f);
	}else{
		Math_SmoothStepToF(&this->actor.gravity, -0.5f, 0.1f, 0.1f, 0.0f);
	}

	this->actor.speed = 0.0f;
	Actor_MoveForward(&this->actor);

	if (this->idle == true){
		if (this->prayTimer > 0){
			DECR(this->prayTimer);
		}else{
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCry);
			EnBoB_SetAction(this, EnBoB_Idle);
			this->actor.gravity = -2.0f;
			this->actor.velocity.y = -2.0f;
		}

	}else{
		if (this->prayTimer > 0){
			DECR(this->prayTimer);
			if (this->prayTimer < prayTimeMAX - 10)
				play->envCtx.lightSettingOverride = 1; 
		}else{
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCry);
			EnBoB_SetAction(this, EnBoB_Cry);
			this->actor.gravity = -2.0f;
			this->actor.velocity.y = -2.0f;
		}
	}
}

void EnBoB_SetupWander(EnBoB* this,PlayState* play){

	if (this->wanderTimer == 0){
		if (this->idle == true){
			this->wanderAngle = DEG_TO_BINANG(Rand_S16Offset(0, 360));
			this->wanderTimerMax = 40 + Rand_S16Offset(0, 30);

		}else if (this->boneHP == 0){
			if(Rand_ZeroOne() < 0.5f){
	           this->wanderAngle = this->actor.yawTowardsPlayer + Rand_S16Offset(0, DEG_TO_BINANG(25));
	        }else{
	           this->wanderAngle = this->actor.yawTowardsPlayer - Rand_S16Offset(0, DEG_TO_BINANG(25));
	        }
	        this->wanderTimerMax = 20 + Rand_S16Offset(0, 10);

		}else{
			if(Rand_ZeroOne() < 0.5f){
	           this->wanderAngle = this->actor.yawTowardsPlayer + Rand_S16Offset(0, DEG_TO_BINANG(40));
	        }else{
	           this->wanderAngle = this->actor.yawTowardsPlayer - Rand_S16Offset(0, DEG_TO_BINANG(40));
	        }
	        this->wanderTimerMax = 25 + Rand_S16Offset(0, 15);
		}
	}

	//Wander Timer to move actor
	if (this->wanderTimer < this->wanderTimerMax){
		this->wanderTimer++;
		EnBoB_Wander(this);
	}else{
		this->wanderTimer = 0;
		this->wandercount++;
	}
}

void EnBoB_Wander(EnBoB* this){

	f32 targetSpeed = 4.0f;

	if (this->boneHP == 0){
		targetSpeed = 4.0f;

		if(this->timeStop == true || this->skelAnime.animation == gBoB_AnimDamage)
			targetSpeed = 0.0f;
	}

	if (this->idle == true)
		targetSpeed = 1.0f;

	Math_SmoothStepToF(&this->actor.speed, targetSpeed, 0.7f, 0.5f, 0.0f);
	Actor_MoveForward(&this->actor);
	Math_SmoothStepToS(&this->actor.world.rot.y, this->wanderAngle, 5, 0x1000, 0x80);

	//reset wander count and return to Cry function when wanderMax reached
	if (this->wandercount == wanderMAX || this->actor.bgCheckFlags & BGCHECKFLAG_WALL){
		this->wandercount = 0;
		if (this->idle == true){
			EnBoB_SetAction(this, EnBoB_Idle);
		}else{
			EnBoB_SetAction(this, EnBoB_Cry);
		}
	}
}

void EnBoB_SetupRoll(EnBoB* this, PlayState* play){

	this->chosenOne = excluderandom(this->chosenOne);

	//Check to see if chosen projectile out if not loop
	if (this->projectiles[this->chosenOne].isOut == false){
		EnBoB_SetupRoll(this, play);
		return;
	}

	Player* player = GET_PLAYER(play);

	this->rollTarget.x = this->projectiles[this->chosenOne].pos.x;
	this->rollTarget.y = this->projectiles[this->chosenOne].pos.y;
	this->rollTarget.z = this->projectiles[this->chosenOne].pos.z;

	if (this->hitcount == 3 || this->hitcount == 8 || this->hitcount == 12){
		player->actor.freezeTimer = 30;
	}else{
		player->actor.freezeTimer = 20;
	}

	this->actor.speed = 0.0f;
	this->func = EnBoB_Roll;
	func_8008EEAC(play, &this->actor);// point camera to actor
	Audio_PlaySfx_2(NA_SE_EN_REDEAD_AIM);
}

void EnBoB_Roll(EnBoB* this, PlayState* play){
	f32 targetSpeed = 10.0f;

	EnBoB_KnockOver(this,play);

	if(this->boneHP == 0){

		if (this->hitcount == 3 || this->hitcount == 8 || this->hitcount == 12){
			targetSpeed = 8.0f;
		}else{
			targetSpeed = 4.0f;
		}

		if(this->timeStop == true || this->skelAnime.animation == gBoB_AnimDamage)
			targetSpeed = 0.0f;
	}

	Math_SmoothStepToF(&this->actor.speed, targetSpeed, 0.7f, 0.5f, 0.0f);
	Actor_MoveForward(&this->actor);
	// XXX pos -> world.pos
	Math_SmoothStepToS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &this->rollTarget), 5, 0x1000, 0x80);
	EnBoB_PickupBones(this,play);
}

void EnBoB_PickupBones(EnBoB* this, PlayState* play){
   // XXX pos -> world.pos
	f32 distance = Math_Vec3f_DistXZ(&this->actor.world.pos, &this->rollTarget);

	if (distance < 50.0f || (distance < 75.0f && (this->actor.bgCheckFlags & BGCHECKFLAG_WALL))){
		this->func = EnBoB_Cry;
		this->boneHP = CLAMP_MAX(this->boneHP+2, boneMaxHP);
		this->projectiles[this->chosenOne].isOut = false;
		DECR(this->numprojectile);

		//return to Cry function when bones picked up
		Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCry); // Change to Cry Animation
		EnBoB_SetAction(this, EnBoB_Cry);
	}
}

void EnBoB_KnockOver(EnBoB* this, PlayState* play){
	Player* player = GET_PLAYER(play);

	//Check if Link is near EnBoB. If close unfreeze player timer and knock link back.
	if (AggroRange(GetMassRangeMax(this)+5.0f)){
		player->actor.freezeTimer = 0;
		func_8002F6D4(play, &this->actor, 12.0f, this->actor.yawTowardsPlayer, 5.0f, 0x20);
		//play, &this->actor, speed, direction to knockback, gravity, damage)
	}
}

void EnBoB_InitProjectiles(EnBoB* this, PlayState* play){

	this->numprojectile = Rand_S16Offset(2, 3);

	for (int i = 0; i < this->numprojectile; ++i) { //BoBProjectileMax instead of Rand_S16
		BoBProjectile* p = &this->projectiles[i];
		f32 min = 70.0f;
		f32 max = 225.0f;
		f32 tempRand2 = Rand_CenteredFloat(max);
		f32 tempRand3 = Rand_CenteredFloat(max);

		if (tempRand2 > 0){
			tempRand2 += min;
		}else{
			tempRand2 -= min;
		}

		if (tempRand3 > 0){
			tempRand3 += min;
		}else{
			tempRand3 -= min;
		}
	
		p->isOut = true;
		p->pos.x = this->actor.world.pos.x + tempRand2;
		p->pos.z = this->actor.world.pos.z + tempRand3;
		p->pos.y = this->actor.world.pos.y + 150.0f + Rand_CenteredFloat(75.0f);
	}
}

void EnBoB_UpdateCurrentBone(EnBoB* this, PlayState *play)
{
	OPEN_DISPS(play->state.gfxCtx);
	
	int CurrentBoneMax = GetCurrentBoneMax(this);
	
	f32 goldenAngle = M_PI * (3.0f - sqrtf(5.0f));
	f32 offset = 2.0f / CurrentBoneMax;
	
	gSPDisplayList(POLY_OPA_DISP++, gBoB_MtlBones);
	
	for (int i = 0; i < CurrentBoneMax; ++i) {
		BoBone* curbone = &this->Bone[i]; //current bone instance
		
		//Fibonacci Sphere algorithm
		f32 y = 1 - (i * offset);
		f32 radius = sqrtf(1 - y * y);
		f32 theta = goldenAngle * i;
		f32 dist = Remap(curbone->dist, INTN_MIN(8), INTN_MAX(8), 0, 2);
		dist = Lerp(PingPongF(dist, 0, 1), GetMassRangeMin(this), GetMassRangeMax(this));
		
		Vec3f pos = {
			Math_CosF(theta) * radius * dist,
			y * dist,
			Math_SinF(theta) * radius * dist,
		};
		
		Matrix_Translate(UnfoldVec3f(this->bone_mass_center), MTXMODE_NEW);
		Matrix_RotateZYX(UnfoldVec3f(this->bone_mass_rot), MTXMODE_APPLY);
		Matrix_Translate(UnfoldVec3f(pos), MTXMODE_APPLY);
		
		Matrix_RotateYS(Math_Vec3f_Yaw(&gZeroVec, &pos), MTXMODE_APPLY);
		Matrix_RotateXS(Math_Vec3f_Pitch(&gZeroVec, &pos), MTXMODE_APPLY);
		Matrix_RotateZF(Remap(curbone->roll, INTN_MIN(8), INTN_MAX(8), -180, 180), MTXMODE_APPLY);
		Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
		gSPMatrix(POLY_OPA_DISP++, NEW_MATRIX(), G_MTX_LOAD);
		curbone->dist += Rand_S16Offset(1, 4);

		if (this->idle == true  || this->skelAnime.animation == gBoB_AnimShootAttack){
			curbone->roll += Rand_S16Offset(5, 10);
		}else{
			curbone->roll += Rand_S16Offset(10, 18);
		}

		gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->boneColor[0], this->boneColor[1], this->boneColor[2],0xFF);

		gSPDisplayList(POLY_OPA_DISP++, gBoB_DlFlatBone + 1);
	}
	
	CLOSE_DISPS();
}

void EnBoB_Destroy(EnBoB* this, PlayState* play) {
	if (gSaveContext.sunsSongState != SUNSSONG_INACTIVE)
		gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
}

void EnBoB_Update(EnBoB* this, PlayState* play) {

	if (this->skelAnime.animation == gBoB_AnimPray && this->idle == false){
			this->boneColorTarget[0] = 0x30;
			this->boneColorTarget[1] = 0x30;
			this->boneColorTarget[2] = 0x30;
	}else{
			this->boneColorTarget[0] = 0xFF;
			this->boneColorTarget[1] = 0xFF;
			this->boneColorTarget[2] = 0xFF;
	}
	Math_ApproachS(&this->boneColor[0],this->boneColorTarget[0], 1, 4);
	Math_ApproachS(&this->boneColor[1],this->boneColorTarget[1], 1, 4);
	Math_ApproachS(&this->boneColor[2],this->boneColorTarget[2], 1, 4);

	if (this->EnBoB_Dead == false && this->timeStop != true)
		this->func(this, play);

	Player* player = GET_PLAYER(play);

	f32 yOffsetTarget = (GetMassRangeMax(this)+GetMassRangeMin(this))* 0.45f / BoB_SCALE;

	//Exit Idle
	if (AggroRange(600) && this->idle == true && this->actor.gravity < -1.8f && this->initTimer == 0){
		this->idleTimer = 0;
		this->idle = false;
        //Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);

		if (this->skelAnime.animation != gBoB_AnimCry)
			Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCry);

		EnBoB_SetAction(this, EnBoB_Cry);
	}

	//check if time stopped for actor
	if (this->timeStop == false){
		SkelAnime_Update(&this->skelAnime);

		if (this->skelAnime.animation == gBoB_AnimCrawl && (this->skelAnime.curFrame == 8.0f || this->skelAnime.curFrame == 15.0f))
			Audio_PlaySfx_2(NA_SE_EN_RIZA_WALK);
	}

	Actor_UpdateBgCheckInfo(play, &this->actor, 30.0f, this->collider.dim.radius, 50.0f, UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);

	//Update colliders and Bob position when bones removed. Dynamically change radius/ height to fit bone position
	if (this->boneHP == 0){
		yOffsetTarget = 14.0f / BoB_SCALE;
	}

	if (this->boneHP > 0 && (player->actor.freezeTimer > 0 || this->func == EnBoB_Roll)){
		this->collider.dim.radius = (GetMassRangeMax(this)+GetMassRangeMin(this))* 0.4f;
		this->collider.dim.height = (GetMassRangeMax(this)+GetMassRangeMin(this))* 0.8f;
	}else{
		this->collider.dim.radius = (GetMassRangeMax(this)+GetMassRangeMin(this))* 0.55f;
		this->collider.dim.height = (GetMassRangeMax(this)+GetMassRangeMin(this))* 1.1f;
	}

	//Increase or decrease size of bone shield
	Math_SmoothStepToF(&this->actor.shape.yOffset, yOffsetTarget, 0.5f, 10.0f / BoB_SCALE, 1.0f / BoB_SCALE);
	
	//update collision of projectiles that are active
	if(this->numprojectile > 0){
		EnBoB_ProjectileCollision(this, play);
	}

	DECR(this->idleTimer);
	DECR(this->colorTimer);
	DECR(this->onFireTimer);

	if (this->EnBoB_Dead == false) {

		//Intermittent Crying
		if ((play->gameplayFrames & 0x5F) == 0){
        	Audio_PlaySfx_2(NA_SE_EN_REDEAD_CRY);
		}

		//turn actor to face Link
		if (this->func != EnBoB_Roll && this->func != EnBoB_SetupWander && this->timeStop == false){
		   // XXX pos -> world.pos
			Math_ApproachS(&this->actor.shape.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos), 2, 1200);
			Math_ApproachS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos), 2, 1200);
		}


		//turn actor to face roll target
		if (this->func == EnBoB_Roll){
		   // XXX pos -> world.pos
			Math_ApproachS(&this->actor.shape.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &this->rollTarget), 2, 1200);
			Math_ApproachS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &this->rollTarget), 2, 1200);

			if (this->skelAnime.animation == gBoB_AnimDamage && Animation_OnFrame(&this->skelAnime, Animation_GetLastFrame(gBoB_AnimDamage))&& this->boneHP == 0)
				Animation_ChangeByInfo(&this->skelAnime, sBoBAnims, AnimCrawl);
		}

		//turn actor to wander direction
		if (this->func == EnBoB_SetupWander){
			Math_ApproachS(&this->actor.shape.rot.y, this->wanderAngle, 2, 1200);

			if (this->skelAnime.animation == gBoB_AnimDamage && Animation_OnFrame(&this->skelAnime, Animation_GetLastFrame(gBoB_AnimDamage))&& this->boneHP == 0)
				Animation_ChangeByInfo(&this->skelAnime, sBoBAnims, AnimCrawl);
		}

		EnBoB_UpdateDamage(this, play);
		EnBoB_HealthManager(this);

		//Passive Health Manager Timer
		DECR(this->timer);
		DECR(this->initTimer);
		
		if (this->boneHP > 0) {
			Collider_UpdateCylinder(&this->actor, &this->collider);
			CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base); //object collider - pushes other OCs mofo
			CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base); //attack toucher -sword attacks
			CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base); //attack collider -checking for attacks
		} else {
			Collider_UpdateCylinder(&this->actor, &this->bodycollider);
			CollisionCheck_SetOC(play, &play->colChkCtx, &this->bodycollider.base);
			CollisionCheck_SetAT(play, &play->colChkCtx, &this->bodycollider.base);
			// Don't set AC when filter timer is between 1 and 7 unless actor is stunned
			if (this->colorTimer > 8 || this->colorTimer == 0 || this->timeStop == true)
				CollisionCheck_SetAC(play, &play->colChkCtx, &this->bodycollider.base);
		}
		
	} else {
		DECR(this->despawntimer);
		EnBoB_Death(this, play);
	}
}

s32 EnBoB_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dl, Vec3f* pos, Vec3s* rot, Actor* thisx, Gfx** gfx) {
	
	return 0;
}

void EnBoB_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dl, Vec3s* rot, Actor* thisx, Gfx** gfx) {
	EnBoB* this = (void*)thisx;
	Vec3f bodyPartVec = {300.0f, 0.0f, 0.0f};
	Vec3f ModelOffset = {}; //Initializes a vector of value of 0,0,0
	Vec3f BoneMassOffset = {};
	s32 bodyPartIndex = -1;
	
	if (limbIndex == BODY_LIMB_HEAD) {
		Matrix_MultVec3f(&ModelOffset, &this->actor.focus.pos);
		Matrix_MultVec3f(&BoneMassOffset, &this->bone_mass_center);
	}

	//set up of fire positions
	if (this->fireTimer != 0) {
        switch (limbIndex) {
            case BODY_LIMB_LEGS:
                bodyPartIndex = 0;
                break;
            case BODY_LIMB_MAINBODY:
                bodyPartIndex = 1;
                break;
            case BODY_LIMB_HEAD:
                bodyPartIndex = 2;
                break;
            case BODY_LIMB_ARM3_R:
                bodyPartIndex = 3;
                break;
            case BODY_LIMB_ARM3_L:
                bodyPartIndex = 4;
                break;
	    }
		
		if (bodyPartIndex >= 0) {
			Vec3f bodyPartPos;
			
			Matrix_MultVec3f(&bodyPartVec, &bodyPartPos);

			this->bodyPartsPos[bodyPartIndex].x = bodyPartPos.x;
			this->bodyPartsPos[bodyPartIndex].y = bodyPartPos.y - 10.0f;
			this->bodyPartsPos[bodyPartIndex].z = bodyPartPos.z;
		}
	}
}

void EnBoB_Draw(EnBoB* this, PlayState* play) {

	Color_RGB8 color = GetCurrentColor(this);
	Color_RGB8 fogcolor = GetFogCurrentColor(this);
	
	OPEN_DISPS(play->state.gfxCtx);
		
	if (this->colorTimer > 0){
		f32 foglerp = Remap(this->colorTimer, 0, this->colorTimerMax, 16000,2000);

		if (this->onFire == true){
			gDPSetFogColor(POLY_OPA_DISP++,fogcolor.r,fogcolor.g,fogcolor.b,255);
			gSPFogPosition(POLY_OPA_DISP++,0,(s16)foglerp);
		}else{

			gDPSetFogColor(POLY_OPA_DISP++,fogcolor.r,fogcolor.g,fogcolor.b,255);
			gSPFogPosition(POLY_OPA_DISP++,0,(s16)foglerp);

			color.r = Lerp(this->colorTimer/(f32)this->colorTimerMax,255,color.r);
			color.g = Lerp(this->colorTimer/(f32)this->colorTimerMax,255,color.g);
			color.b = Lerp(this->colorTimer/(f32)this->colorTimerMax,255,color.b);
		}	
	}

	SKELANIME_DRAW_ALPHA(EnBoB_OverrideLimbDraw, EnBoB_PostLimbDraw, this->alpha, color);

	if (this->colorTimer > 0)
		Fog_RestoreOpa(play);

	EnBoB_UpdateCurrentBone(this, play);
	EnBoB_DislodgeBonesUpdateDraw(this, play);
	EnBoB_ShootUpdateDraw(this, play);

	//Drawing of billboard effects when Bob has bones
	if (this->boneHP > 0) {
		Gfx_SetupDL25_Xlu(play->state.gfxCtx);
		gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0xD6, 0xB4, 0x95, 0xFF * ((f32)this->boneHP / boneMaxHP));
		Matrix_Translate(UnfoldVec3f(this->bone_mass_center), MTXMODE_NEW);
		Matrix_ReplaceRotation(&play->billboardMtxF);
		
		Matrix_Push();
		{
			if (this->timeStop != true)
				Matrix_RotateZS(play->gameplayFrames * 700, MTXMODE_APPLY);
			Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
			gSPMatrix(POLY_XLU_DISP++, NEW_MATRIX(), G_MTX_LOAD);
			gSPDisplayList(POLY_XLU_DISP++, gBoB_DlEffect);
		} 
		Matrix_Pop();
		
		if (this->timeStop != true)
			Matrix_RotateZS(play->gameplayFrames * -1200, MTXMODE_APPLY);
		Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
		gSPMatrix(POLY_XLU_DISP++, NEW_MATRIX(), G_MTX_LOAD);
		gSPDisplayList(POLY_XLU_DISP++, gBoB_DlEffect2);
		
	}
	
	//Decrease Stun Timers if active and set timeStop
	if (this->stunTimer > 1 || this->sunsSongStunTimer > 1) {
		DECR(this->stunTimer);
		DECR(this->sunsSongStunTimer);
		this->timeStop = true;
	}
	//reset actorColor and return to previous animation
	if (this->colorTimer == 1 && this->onFireTimer == 0){
		this->targetColor = Color_None;
		if (this->actor.colChkInfo.health != 0){ // XXX health
			if (this->boneHP == 0 && (this->func == EnBoB_SetupWander || this->func == EnBoB_Roll)){
				Animation_ChangeByInfo (&this->skelAnime, sBoBAnims, AnimCrawl);
			}
		}
	}

	if (this->onFireTimer == 1)
		this->targetColor = Color_None;

	//release actor from fire
	if (this->onFireTimer == 1 && this->onFire == true){
		this->onFire = false;
		this->onFireTimer = 0;
	}

	//release actor from timeStops
	if (this->stunTimer == 1 || this->sunsSongStunTimer == 1 || this->iceTimer == 1){
		this->stunTimer = 0;
		this->sunsSongStunTimer = 0;
		this->iceTimer = 0;
		this->timeStop = false;

		gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
	}
	
	//Spawning Ice on BoB. Ice is prevented from moving by timeStop.
	if (this->iceTimer > 1) {
		this->iceTimer--;
		this->timeStop = true;
		
		if (this->iceTimer == frozenTimeMAX - 1) {
			Vec3f icePos = this->bone_mass_center;
			icePos.y -= 8.0f;
			icePos.z += 10.0f;
			//EffectSsEnIce_SpawnFlyingVec3f(play, &this->actor, &icePos, 150, 150, 150, 250, 235, 245, 255, 2.6f);
			icePos.x += 10.0f;
			icePos.z -= 20.0f;
			//EffectSsEnIce_SpawnFlyingVec3f(play, &this->actor, &icePos, 150, 150, 150, 250, 235, 245, 255, 2.6f);
			icePos.x -= 20.0f;
			//EffectSsEnIce_SpawnFlyingVec3f(play, &this->actor, &icePos, 150, 150, 150, 250, 235, 245, 255, 2.6f);
		}
	}
	
	//Spawning Fire on BoB
	if (this->fireTimer != 0) {
        if (this->fireTimer >= 24 && this->fireTimer % 4 == 0) {
        	int index = (40 - this->fireTimer) / 4;
            //EffectSsEnFire_SpawnVec3s(play, &this->actor,  &this->bodyPartsPos[index], 100, 0, 0,index);
        }
		this->fireTimer--;
	}
	
	CLOSE_DISPS();
}

//Bounce between two values
s32 PingPongS(s32 v, s32 min, s32 max) {
    min = WrapS(v, min, max * 2);
    if (min < max)
        return min;
    else
        return 2 * max - min;
}

f32 PingPongF(f32 v, f32 min, f32 max) {
    min = WrapF(v, min, max * 2);
    if (min < max)
        return min;
    else
        return 2 * max - min;
}
