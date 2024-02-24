/*
 * File: rat.c
 * Overlay: ovl_Rat
 * Description: Rat enemy from the Hylian Modding actor pack.
 */

#include "rat.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_20)



#define COLORFILTER_COLORFLAG_GRAY 0x8000
#define COLORFILTER_COLORFLAG_RED  0x4000
#define COLORFILTER_COLORFLAG_BLUE 0x0000

#define COLORFILTER_INTENSITY_FLAG 0x8000

#define COLORFILTER_BUFFLAG_XLU    0x2000
#define COLORFILTER_BUFFLAG_OPA    0x0000



void Rat_Init(Actor* thisx, PlayState* play);
void Rat_Destroy(Actor* thisx, PlayState* play);
void Rat_Update(Actor* thisx, PlayState* play);
void Rat_Draw(Actor* thisx, PlayState* play);

void Rat_Idle(Rat* this, PlayState* play);
void Rat_Notice(Rat* this, PlayState* play);
void Rat_StartRunFromNotice(Rat* this, PlayState* play);
void Rat_Run(Rat* this, PlayState* play);
void Rat_EndRun(Rat* this, PlayState* play);
void Rat_Attack(Rat* this, PlayState* play);
void Rat_Damaged(Rat* this, PlayState* play);
void Rat_Stunned(Rat* this, PlayState* play);
void Rat_Die(Rat* this, PlayState* play);

typedef enum {
    RAT_DMGEFF_NONE,
    RAT_DMGEFF_STUN,
    RAT_DMGEFF_DEFAULT,
} RatDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, RAT_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Horse trample  */ DMG_ENTRY(1, RAT_DMGEFF_DEFAULT),
    /* Explosives     */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Zora boomerang */ DMG_ENTRY(1, RAT_DMGEFF_STUN),
    /* Normal arrow   */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* UNK_DMG_0x06   */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Hookshot       */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Goron punch    */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Sword          */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Goron pound    */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Fire arrow     */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Ice arrow      */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Light arrow    */ DMG_ENTRY(16, RAT_DMGEFF_DEFAULT),
    /* Goron spikes   */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Deku spin      */ DMG_ENTRY(0, RAT_DMGEFF_STUN),
    /* Deku bubble    */ DMG_ENTRY(1, RAT_DMGEFF_DEFAULT),
    /* Deku launch    */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* Zora barrier   */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Normal shield  */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, RAT_DMGEFF_DEFAULT),
    /* Zora punch     */ DMG_ENTRY(2, RAT_DMGEFF_DEFAULT),
    /* Spin attack    */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Sword beam     */ DMG_ENTRY(4, RAT_DMGEFF_DEFAULT),
    /* Normal Roll    */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, RAT_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(16, RAT_DMGEFF_DEFAULT),
};

const ActorInit ZzrtlInitVars = {
    48,
    ACTORCAT_ENEMY,
    FLAGS,
    43,
    sizeof(Rat),
    (ActorFunc)Rat_Init,
    (ActorFunc)Rat_Destroy,
    (ActorFunc)Rat_Update,
    (ActorFunc)Rat_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_HIT5,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0xFFCFFFFF, 0x08, 0x08 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_ON,
    },
    { 20, 30, 0, { 0, 0, 0 } },
};

void Rat_Init(Actor* thisx, PlayState* play) {
    Rat* this = (Rat*)thisx;

    Actor_SetScale(&this->actor, 0.01f);

    this->actor.colChkInfo.mass = MASS_HEAVY;
    this->actor.colChkInfo.health = 4;
    this->actor.gravity = -1.0f;
    //this->actor.naviEnemyId = 0xCB;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    SkelAnime_InitFlex(play, &this->skelAnime, (void*)(0x06001928), (void*)(0x06003734), this->jointTable, this->morphTable,
                       GRATSKEL_NUM_LIMBS);

    this->actor.colChkInfo.damageTable = &sDamageTable;

    this->actionFunc = Rat_Idle;
}

void Rat_Destroy(Actor* thisx, PlayState* play) {
    Rat* this = (Rat*)thisx;

    Collider_DestroyCylinder(play, &this->collider);
}

// HELPERS

void Rat_RotateTowardPoint(Rat* this, Vec3f* point, s16 step) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, Actor_WorldYawTowardPoint(&this->actor, point), 3, step, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;
}

// SETUP FUNCTIONS

void Rat_SetupIdle(Rat* this, PlayState* play) {
    this->actor.speed = 0.0f;
    this->actor.velocity.y = 0.0f;
    Animation_MorphToLoop(&this->skelAnime, (void*)(0x06003734), -3.0f);
    this->actionFunc = Rat_Idle;
}

#define RAT_FLEE_RADIUS 200.0f

void Rat_SetupNotice(Rat* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06003898));
    this->actionFunc = Rat_Notice;
}

void Rat_SetupStartRunFromNotice(Rat* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06003cac));
    this->actionFunc = Rat_StartRunFromNotice;
}

void Rat_SetupRun(Rat* this, PlayState* play) {
    s16 angle;

    if (this->fleeTimer != 0) {
        angle = this->actor.yawTowardsPlayer + DEG_TO_BINANG(180.0f) +
                Rand_S16Offset(DEG_TO_BINANG(-45.0f), DEG_TO_BINANG(90.0f));
        this->fleePos.x = (Math_CosS(angle) * RAT_FLEE_RADIUS) + this->actor.home.pos.x;
        this->fleePos.z = (Math_SinS(angle) * RAT_FLEE_RADIUS) + this->actor.home.pos.z;
    }

    Animation_MorphToLoop(&this->skelAnime, (void*)(0x06003a2c), -4.0f);
    this->actionFunc = Rat_Run;
}

void Rat_SetupEndRun(Rat* this, PlayState* play) {
    Animation_MorphToPlayOnce(&this->skelAnime, (void*)(0x06002cdc), -4.0f);
    this->actor.speed = 0.0f;
    this->actionFunc = Rat_EndRun;
}

void Rat_SetupAttack(Rat* this, PlayState* play) {
    this->actor.speed = 8.0f;
    this->actor.velocity.y = 2.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    Animation_MorphToPlayOnce(&this->skelAnime, (void*)(0x06001c6c), -3.0f);
    //Audio_PlayActorSfx2(&this->actor, NA_SE_EN_EIER_CRY);
    this->actionFunc = Rat_Attack;
}

void Rat_SetupDamaged(Rat* this, PlayState* play) {
    this->actor.speed = -4.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    Animation_MorphToPlayOnce(&this->skelAnime, (void*)(0x06001fac), -3.0f);
    this->actionFunc = Rat_Damaged;
}

void Rat_SetupStunned(Rat* this, PlayState* play) {
    this->actor.speed = 0.0f;
    Animation_Change(&this->skelAnime, (void*)(0x06001fac), 0.0f, 3.0f, 0.0f, ANIMMODE_ONCE, 0.0f);
    this->actionFunc = Rat_Stunned;
}

void Rat_SetupDie(Rat* this, PlayState* play) {
    this->actor.speed = 0.0f;
    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    this->actor.world.rot.y = this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    Animation_MorphToPlayOnce(&this->skelAnime, (void*)(0x06002348), -3.0f);
    this->actionFunc = Rat_Die;
}

// MAIN

#define RAT_NOTICE_RADIUS 300.0f

void Rat_Idle(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.xzDistToPlayer < RAT_NOTICE_RADIUS) {
        Rat_SetupNotice(this, play);
    }
}

#define RAT_ATTACK_RADIUS 100.0f

void Rat_Notice(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    u8 animDone = SkelAnime_Update(&this->skelAnime);

    Rat_RotateTowardPoint(this, &player->actor.world.pos, DEG_TO_BINANG(15.0f));

    if (Actor_IsFacingPlayer(&this->actor, DEG_TO_BINANG(20.0f))) {
        if (animDone) {
            if (this->actor.xzDistToPlayer < RAT_ATTACK_RADIUS) {
                Rat_SetupAttack(this, play);
            } else {
                Rat_SetupStartRunFromNotice(this, play);
            }
        }
    }
}

void Rat_StartRunFromNotice(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.speed, 4.0f, 0.1f, 1.0f, 0.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        Rat_SetupRun(this, play);
    }
}

#define RAT_HOME_RADIUS 450.0f

void Rat_Run(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    u8 playerOutsideHomeRadius = Actor_WorldDistXYZToPoint(&player->actor, &this->actor.home.pos) > RAT_HOME_RADIUS;
    u8 ratOutsideHomeRadius = Actor_WorldDistXYZToPoint(&this->actor, &this->actor.home.pos) > RAT_HOME_RADIUS;
    u8 ratAtHome = Actor_WorldDistXYZToPoint(&this->actor, &this->actor.home.pos) <= 100.0f;
    u8 playerOutsideNoticeRadius = this->actor.xzDistToPlayer > RAT_NOTICE_RADIUS;
    u8 readyToAttack = Actor_IsFacingAndNearPlayer(&this->actor, RAT_ATTACK_RADIUS, DEG_TO_BINANG(35.0f)) ||
                       Actor_IsFacingAndNearPlayer(&this->actor, RAT_ATTACK_RADIUS * 0.5f, DEG_TO_BINANG(75.0f));

    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToF(&this->actor.speed, 4.0f, 0.1f, 1.0f, 0.0f);

    Rat_RotateTowardPoint(this, &this->targetPos, DEG_TO_BINANG(10.0f));

    if (ratAtHome && playerOutsideNoticeRadius) {
        Rat_SetupEndRun(this, play);
        return;
    }

    if (ratOutsideHomeRadius || playerOutsideHomeRadius) {
        this->targetPos = this->actor.home.pos;
        return;
    }

    if (DECR(this->fleeTimer) == 0 || Actor_WorldDistXZToPoint(&this->actor, &this->fleePos) < 100.0f) {
        this->fleeTimer = 0;
        this->targetPos = player->actor.world.pos;
    } else {
        this->targetPos = this->fleePos;
    }

    if (readyToAttack && this->fleeTimer == 0) {
        Rat_SetupAttack(this, play);
    }
}

void Rat_EndRun(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (SkelAnime_Update(&this->skelAnime)) {
        Rat_SetupIdle(this, play);
    }
}

void Rat_Attack(Rat* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 0.1f, 1.0f, 0.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        this->fleeTimer = 2 * 20;
        Rat_SetupRun(this, play);
    }
}

void Rat_Damaged(Rat* this, PlayState* play) {
    Math_SmoothStepToF(&this->actor.speed, 0.0f, 3.0f, 0.5f, 0.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        this->fleeTimer = 6 * 20;
        Rat_SetupRun(this, play);
    }
}

void Rat_Stunned(Rat* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (this->actor.colorFilterTimer == 0) {
        if (this->actor.colChkInfo.health == 0) {
            Rat_SetupDie(this, play);
        } else {
            Rat_SetupRun(this, play);
        }
    }
}

void Rat_Die(Rat* this, PlayState* play) {
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    Vec3f effectVel = { 0.0f, 4.0f, 0.0f };
    Vec3f effectPos = this->actor.world.pos;

    if (SkelAnime_Update(&this->skelAnime) || this->drowned) {
        Actor_SetScale(&this->actor, this->actor.scale.x * 0.8f);

        if (this->actor.scale.x <= 0.001f) {
            effectPos.y += 10.0f;
/*             EffectSsDeadDb_Spawn(play, &effectPos, &effectVel, &zeroVec, 90, 0, 255, 255, 255, 255, 0, 0, 255, 1, 9,
                                 true); */
/*             EffectSsDeadDb_Spawn(play, &effectPos, &gZeroVec3f, &gZeroVec3f, &sLightswitchEffectPrimColor,
                    &sLightswitchEffectEnvColor, 100, 0, 9); */
            Item_DropCollectible(play, &this->actor.world.pos, ITEM00_RECOVERY_HEART);
            Actor_Kill(&this->actor);
        }
    }
}

void Rat_CheckDrowned(Rat* this, PlayState* play) {
    if (!this->drowned && (this->actor.bgCheckFlags & BGCHECKFLAG_WATER) && (this->actor.depthInWater > 5.0f)) {
        Actor_SetDropFlag(&this->actor, &this->collider.info);
        //Audio_PlayActorSfx2(&this->actor, NA_SE_EN_EIER_ATTACK);
        Enemy_StartFinishingBlow(play, &this->actor);
        this->drowned = true;
        this->actor.gravity = -0.1f;
        this->actionFunc = Rat_SetupDie;
        return;
    }
}

void Rat_CheckDamage(Rat* this, PlayState* play) {
    Rat_CheckDrowned(this, play);

    if (!this->drowned && this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_SetDropFlag(&this->actor, &this->collider.info);

        if ((this->actionFunc != Rat_Die) && (this->actionFunc != Rat_Damaged)) {
            switch (this->actor.colChkInfo.damageEffect) {
                case RAT_DMGEFF_STUN:
                    if (this->actor.colChkInfo.health > 1) {
                        //Audio_PlayActorSfx2(&this->actor, NA_SE_EN_GOMA_JR_FREEZE);
                        Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_BLUE, 255, COLORFILTER_BUFFLAG_OPA,
                                             80);
                        this->actionFunc = Rat_SetupStunned;
                        break;
                    }
                    break;
                case RAT_DMGEFF_DEFAULT:
                    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 200, COLORFILTER_BUFFLAG_OPA, 20);
                    Actor_ApplyDamage(&this->actor);
                    if (this->actor.colChkInfo.health == 0) {
                        //Audio_PlayActorSfx2(&this->actor, NA_SE_EN_EIER_ATTACK);
                        Enemy_StartFinishingBlow(play, &this->actor);
                        this->actionFunc = Rat_SetupDie;
                    } else {
                        //Audio_PlayActorSfx2(&this->actor, NA_SE_EN_DEKU_DAMAGE);
                        this->actionFunc = Rat_SetupDamaged;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

void Rat_Update(Actor* thisx, PlayState* play) {
    Rat* this = (Rat*)thisx;

    Rat_CheckDamage(this, play);
    this->actionFunc(this, play);

    Collider_UpdateCylinder(&this->actor, &this->collider);

    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 35.0f, 60.0f, 60.0f,
                            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                                UPDBGCHECKINFO_FLAG_10);

    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    if (this->actionFunc != Rat_Damaged && this->actionFunc != Rat_Die) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
    if (this->actionFunc == Rat_Attack) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    }
}

void Rat_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    Rat* this = (Rat*)thisx;

    if (limbIndex == GRATSKEL_HEAD_LIMB) {
        Vec3f src = { 0.0f, 10.0f, 0.0f };
        Vec3f dest;

        Matrix_MultVec3f(&src, &dest);
        this->actor.focus.pos.x = dest.x;
        this->actor.focus.pos.y = dest.y;
        this->actor.focus.pos.z = dest.z;
    }
}

void Rat_Draw(Actor* thisx, PlayState* play) {
    Rat* this = (Rat*)thisx;

    SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount, NULL,
                          Rat_PostLimbDraw, &this->actor);
}
