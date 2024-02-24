/*
 * File: z_en_test.c
 * Overlay: ovl_En_Test
 * Description: Stalfos
 */

#include "z_en_skelliboi.h"
#include "z_en_part.h"
//#include "assets/objects/object_sk2/object_sk2.h"

#undef ACTOR_EN_TEST
#define ACTOR_EN_TEST 64 // self

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY | ACTOR_FLAG_10)

#define SECOND * 20
#define SECONDS * 20
#define REVIVE_TIMER_START (int)(15 SECONDS) // how long a fallen stalfos waits before getting back up
#define GETUP_TIME 1 SECOND // how long it takes a fallen stalfos to reassemble itself after waiting

// keeps track of how many linked stalfos are currently loaded
static u8 sFriends;
static u8 sFriendsDown;
#define PARAM_IS_FRIEND 0x8000

void EnTest_Init(Actor* thisx, PlayState* play);
void EnTest_Destroy(Actor* thisx, PlayState* play);
void EnTest_Update(Actor* thisx, PlayState* play);
void EnTest_Draw(Actor* thisx, PlayState* play);

void EnTest_SetupIdle(EnTest* this);
void EnTest_SetupWaitGround(EnTest* this);
void EnTest_SetupWaitAbove(EnTest* this);
void EnTest_SetupJumpBack(EnTest* this);
void EnTest_SetupSlashDownEnd(EnTest* this);
void EnTest_SetupSlashUp(EnTest* this);
void EnTest_SetupJumpslash(EnTest* this);
void EnTest_SetupWalkAndBlock(EnTest* this);
void func_80860EC0(EnTest* this);
void EnTest_SetupSlashDown(EnTest* this);
void func_80860BDC(EnTest* this);
void EnTest_SetupIdleFromBlock(EnTest* this);
void EnTest_SetupRecoil(EnTest* this);
void func_80862398(EnTest* this);
void func_80862154(EnTest* this);
void EnTest_SetupStopAndBlock(EnTest* this);
void func_808627C4(EnTest* this, PlayState* play);

void EnTest_WaitGround(EnTest* this, PlayState* play);
void EnTest_WaitAbove(EnTest* this, PlayState* play);
void EnTest_Fall(EnTest* this, PlayState* play);
void EnTest_Land(EnTest* this, PlayState* play);
void EnTest_Rise(EnTest* this, PlayState* play);
void EnTest_Idle(EnTest* this, PlayState* play);
void EnTest_WalkAndBlock(EnTest* this, PlayState* play);
void func_80860C24(EnTest* this, PlayState* play);
void func_80860F84(EnTest* this, PlayState* play);
void EnTest_SlashDown(EnTest* this, PlayState* play);
void EnTest_SlashDownEnd(EnTest* this, PlayState* play);
void EnTest_SlashUp(EnTest* this, PlayState* play);
void EnTest_JumpBack(EnTest* this, PlayState* play);
void EnTest_Jumpslash(EnTest* this, PlayState* play);
void EnTest_JumpUp(EnTest* this, PlayState* play);
void EnTest_StopAndBlock(EnTest* this, PlayState* play);
void EnTest_IdleFromBlock(EnTest* this, PlayState* play);
void func_808621D4(EnTest* this, PlayState* play);
void func_80862418(EnTest* this, PlayState* play);
void EnTest_Stunned(EnTest* this, PlayState* play);
void func_808628C8(EnTest* this, PlayState* play);
void func_80862E6C(EnTest* this, PlayState* play);
void FallOverBackwardsAndDie(EnTest* this, PlayState* play);
void FallOverForwardsAndDie(EnTest* this, PlayState* play);
void EnTest_Recoil(EnTest* this, PlayState* play);
void BreakApartAndDie(EnTest* this, PlayState* play);
void SetupFallOverBackwardsAndDie(EnTest* this, PlayState* play);
void BodyBreak_Alloc(BodyBreak* bodyBreak, s32 count, PlayState* play);
void BodyBreak_SetInfo(BodyBreak* bodyBreak, s32 limbIndex, s32 minLimbIndex, s32 maxLimbIndex, u32 count, Gfx** dList,
                       s16 objectId);
s32 BodyBreak_SpawnParts(Actor* actor, BodyBreak* bodyBreak, PlayState* play, s16 type);

s32 EnTest_ReactToProjectile(PlayState* play, EnTest* this);

void BodyBreak_Alloc(BodyBreak* bodyBreak, s32 count, PlayState* play) {
    u32 matricesSize;
    u32 dListsSize;
    u32 objectIdsSize;

    matricesSize = (count + 1) * sizeof(*bodyBreak->matrices);
    bodyBreak->matrices = ZeldaArena_Malloc(matricesSize);

    if (bodyBreak->matrices != NULL) {
        dListsSize = (count + 1) * sizeof(*bodyBreak->dLists);
        bodyBreak->dLists = ZeldaArena_Malloc(dListsSize);

        if (bodyBreak->dLists != NULL) {
            objectIdsSize = (count + 1) * sizeof(*bodyBreak->objectIds);
            bodyBreak->objectIds = ZeldaArena_Malloc(objectIdsSize);

            if (bodyBreak->objectIds != NULL) {
                Lib_MemSet((u8*)bodyBreak->matrices, matricesSize, 0);
                Lib_MemSet((u8*)bodyBreak->dLists, dListsSize, 0);
                Lib_MemSet((u8*)bodyBreak->objectIds, objectIdsSize, 0);
                bodyBreak->val = 1;
                return;
            }
        }
    }

    if (bodyBreak->matrices != NULL) {
        ZeldaArena_Free(bodyBreak->matrices);
    }

    if (bodyBreak->dLists != NULL) {
        ZeldaArena_Free(bodyBreak->dLists);
    }

    if (bodyBreak->objectIds != NULL) {
        ZeldaArena_Free(bodyBreak->objectIds);
    }
}

void BodyBreak_SetInfo(BodyBreak* bodyBreak, s32 limbIndex, s32 minLimbIndex, s32 maxLimbIndex, u32 count, Gfx** dList,
                       s16 objectId) {
    PlayState* play = Effect_GetPlayState();

    if ((play->actorCtx.freezeFlashTimer == 0) && (bodyBreak->val > 0)) {
        if ((limbIndex >= minLimbIndex) && (limbIndex <= maxLimbIndex) && (*dList != NULL)) {
            bodyBreak->dLists[bodyBreak->val] = *dList;
            Matrix_Get(&bodyBreak->matrices[bodyBreak->val]);
            bodyBreak->objectIds[bodyBreak->val] = objectId;
            bodyBreak->val++;
        }

        if (limbIndex != bodyBreak->prevLimbIndex) {
            bodyBreak->count++;
        }

        if ((u32)bodyBreak->count >= count) {
            bodyBreak->count = bodyBreak->val - 1;
            bodyBreak->val = BODYBREAK_STATUS_READY;
        }
    }

    bodyBreak->prevLimbIndex = limbIndex;
}

s32 BodyBreak_SpawnParts(Actor* actor, BodyBreak* bodyBreak, PlayState* play, s16 type) {
    EnPart* spawnedEnPart;
    MtxF* mtx;
    s16 objectSlot;

    if (bodyBreak->val != BODYBREAK_STATUS_READY) {
        return false;
    }

    while (bodyBreak->count > 0) {
        Matrix_Put(&bodyBreak->matrices[bodyBreak->count]);
        Matrix_Scale(1.0f / actor->scale.x, 1.0f / actor->scale.y, 1.0f / actor->scale.z, MTXMODE_APPLY);
        Matrix_Get(&bodyBreak->matrices[bodyBreak->count]);

        if (1) {
            if (bodyBreak->objectIds[bodyBreak->count] >= 0) {
                objectSlot = bodyBreak->objectIds[bodyBreak->count];
            } else {
                objectSlot = actor->objectSlot;
            }
        }

        mtx = &bodyBreak->matrices[bodyBreak->count];

        spawnedEnPart = (EnPart*)Actor_SpawnAsChild(&play->actorCtx, actor, play, ACTOR_EN_PART, mtx->xw, mtx->yw,
                                                    mtx->zw, 0, 0, objectSlot, type);

        if (spawnedEnPart != NULL) {
            Matrix_MtxFToYXZRot(&bodyBreak->matrices[bodyBreak->count], &spawnedEnPart->actor.shape.rot, 0);
            spawnedEnPart->dList = bodyBreak->dLists[bodyBreak->count];
            spawnedEnPart->actor.scale = actor->scale;
        }

        bodyBreak->count--;
    }

    bodyBreak->val = BODYBREAK_STATUS_FINISHED;

    ZeldaArena_Free(bodyBreak->matrices);
    ZeldaArena_Free(bodyBreak->dLists);
    ZeldaArena_Free(bodyBreak->objectIds);

    return true;
}

static u8 sJointCopyFlags[] = {
    false, // STALFOS_LIMB_NONE
    false, // STALFOS_LIMB_ROOT
    false, // STALFOS_LIMB_UPPERBODY_ROOT
    false, // STALFOS_LIMB_CORE_LOWER_ROOT
    true,  // STALFOS_LIMB_CORE_UPPER_ROOT
    true,  // STALFOS_LIMB_NECK_ROOT
    true,  // STALFOS_LIMB_HEAD_ROOT
    true,  // STALFOS_LIMB_7
    true,  // STALFOS_LIMB_8
    true,  // STALFOS_LIMB_JAW_ROOT
    true,  // STALFOS_LIMB_JAW
    true,  // STALFOS_LIMB_HEAD
    true,  // STALFOS_LIMB_NECK_UPPER
    true,  // STALFOS_LIMB_NECK_LOWER
    true,  // STALFOS_LIMB_CORE_UPPER
    true,  // STALFOS_LIMB_CHEST
    true,  // STALFOS_LIMB_SHOULDER_R_ROOT
    true,  // STALFOS_LIMB_SHOULDER_ARMOR_R_ROOT
    true,  // STALFOS_LIMB_SHOULDER_ARMOR_R
    true,  // STALFOS_LIMB_SHOULDER_L_ROOT
    true,  // STALFOS_LIMB_SHOULDER_ARMOR_L_ROOT
    true,  // STALFOS_LIMB_SHOULDER_ARMOR_L
    true,  // STALFOS_LIMB_ARM_L_ROOT
    true,  // STALFOS_LIMB_UPPERARM_L_ROOT
    true,  // STALFOS_LIMB_FOREARM_L_ROOT
    true,  // STALFOS_LIMB_HAND_L_ROOT
    true,  // STALFOS_LIMB_HAND_L
    true,  // STALFOS_LIMB_SHIELD
    true,  // STALFOS_LIMB_FOREARM_L
    true,  // STALFOS_LIMB_UPPERARM_L
    true,  // STALFOS_LIMB_ARM_R_ROOT
    true,  // STALFOS_LIMB_UPPERARM_R_ROOT
    true,  // STALFOS_LIMB_FOREARM_R_ROOT
    true,  // STALFOS_LIMB_HAND_R_ROOT
    true,  // STALFOS_LIMB_SWORD
    true,  // STALFOS_LIMB_HAND_R
    true,  // STALFOS_LIMB_FOREARM_R
    true,  // STALFOS_LIMB_UPPERARM_R
    true,  // STALFOS_LIMB_CORE_LOWER
    false, // STALFOS_LIMB_LOWERBODY_ROOT
    false, // STALFOS_LIMB_WAIST_ROOT
    false, // STALFOS_LIMB_LEGS_ROOT
    false, // STALFOS_LIMB_LEG_L_ROOT
    false, // STALFOS_LIMB_THIGH_L_ROOT
    false, // STALFOS_LIMB_LOWERLEG_L_ROOT
    false, // STALFOS_LIMB_ANKLE_L_ROOT
    false, // STALFOS_LIMB_ANKLE_L
    false, // STALFOS_LIMB_FOOT_L_ROOT
    false, // STALFOS_LIMB_FOOT_L
    false, // STALFOS_LIMB_LOWERLEG_L
    false, // STALFOS_LIMB_THIGH_L
    false, // STALFOS_LIMB_LEG_R_ROOT
    false, // STALFOS_LIMB_THIGH_R_ROOT
    false, // STALFOS_LIMB_LOWERLEG_R_ROOT
    false, // STALFOS_LIMB_ANKLE_R_ROOT
    false, // STALFOS_LIMB_ANKLE_R
    false, // STALFOS_LIMB_FOOT_R_ROOT
    false, // STALFOS_LIMB_FOOT_R
    false, // STALFOS_LIMB_LOWERLEG_R
    false, // STALFOS_LIMB_THIGH_R
    false, // STALFOS_LIMB_WAIST
};

const ActorInit ZzrtlInitVars = {
    ACTOR_EN_TEST,
    ACTORCAT_ENEMY,
    FLAGS,
    46,
    sizeof(EnTest),
    (ActorFunc)EnTest_Init,
    (ActorFunc)EnTest_Destroy,
    (ActorFunc)EnTest_Update,
    (ActorFunc)EnTest_Draw,
};

static ColliderCylinderInit sBodyColliderInit = {
    {
        COLTYPE_HIT5,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_ON,
        OCELEM_ON,
    },
    { 25, 65, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sShieldColliderInit = {
    {
        COLTYPE_METAL,
        AT_NONE,
        AC_ON | AC_HARD | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0xFFC1FFFF, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_ON,
        OCELEM_NONE,
    },
    { 20, 70, -50, { 0, 0, 0 } },
};

static ColliderQuadInit sSwordColliderInit = {
    {
        COLTYPE_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_QUAD,
    },
    {
        ELEMTYPE_UNK0,
        { 0xFFCFFFFF, 0x00, 0x10 },
        { 0x00000000, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL | TOUCH_UNK7,
        BUMP_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

typedef enum {
    /* 0x0 */ STALFOS_DMGEFF_NORMAL,
    /* 0x1 */ STALFOS_DMGEFF_STUN,
    /* 0x6 */ STALFOS_DMGEFF_FIREMAGIC = 6,
    /* 0xD */ STALFOS_DMGEFF_SLING = 0xD,
    /* 0xE */ STALFOS_DMGEFF_LIGHT,
    /* 0xF */ STALFOS_DMGEFF_FREEZE
} StalfosDamageEffect;

static DamageTable sDamageTable = {
    /* Deku nut        */ DMG_ENTRY(0, STALFOS_DMGEFF_STUN),
    /* Deku stick      */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Horse trample   */ DMG_ENTRY(2, STALFOS_DMGEFF_SLING),
    /* Explosives      */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Zora boomerang  */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Normal arrow    */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* UNK_DMG_0x06    */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Hookshot        */ DMG_ENTRY(0, STALFOS_DMGEFF_STUN),
    /* Goron Punch     */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Sword           */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Goron Pound     */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Fire arrow      */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Ice arrow       */ DMG_ENTRY(4, STALFOS_DMGEFF_FREEZE),
    /* Light arrow     */ DMG_ENTRY(2, STALFOS_DMGEFF_LIGHT),
    /* Goron spikes    */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Deku bubble     */ DMG_ENTRY(1, STALFOS_DMGEFF_NORMAL),
    /* Deku bubble     */ DMG_ENTRY(1, STALFOS_DMGEFF_NORMAL),
    /* Deku launch     */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* UNK_DMG_0x12    */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Zora barrier    */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Shield          */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Mirror Ray      */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Thrown object   */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Zora punch      */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Spin attack     */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Sword beam      */ DMG_ENTRY(2, STALFOS_DMGEFF_NORMAL),
    /* Normal roll     */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* UNK_DMG_0x1B    */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* UNK_DMG_0x1C    */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Unblockable     */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* UNK_DMG_0x1E    */ DMG_ENTRY(0, STALFOS_DMGEFF_NORMAL),
    /* Powder Keg      */ DMG_ENTRY(4, STALFOS_DMGEFF_NORMAL),
};

static InitChainEntry sInitChain[] = {
    //ICHAIN_S8(naviEnemyId, NAVI_ENEMY_STALFOS, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 500, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 15, ICHAIN_CONTINUE),
    ICHAIN_F32(scale.y, 0, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, -1500, ICHAIN_STOP),
};

void EnTest_SetupAction(EnTest* this, EnTestActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

void EnTest_Init(Actor* thisx, PlayState* play) {
    EffectBlureInit1 slashBlure;
    EnTest* this = (EnTest*)thisx;
    
    // types work a little differently now
    if (this->actor.params == 0xffff)
        this->actor.params = STALFOS_TYPE_1;
    
    // uppermost bit is used for pairing
    if (this->actor.params & PARAM_IS_FRIEND)
    {
        this->reviveTimer = REVIVE_TIMER_START;
        this->isFriend = true;
        sFriends += 1;
        this->actor.params &= ~PARAM_IS_FRIEND;
    }
    
    // filter types
    switch (this->actor.params)
    {
        case STALFOS_TYPE_INVISIBLE: case STALFOS_TYPE_CEILING: case STALFOS_TYPE_1: break;
        default: this->actor.params = STALFOS_TYPE_1; break;
    }

    Actor_ProcessInitChain(&this->actor, sInitChain);

    SkelAnime_Init(play, &this->skelAnime, (void*)(0x06007C28), (void*)(0x0600316C), this->jointTable, this->morphTable,
                   STALFOS_LIMB_MAX);
    SkelAnime_Init(play, &this->upperSkelanime, (void*)(0x06007C28), (void*)(0x0600316C), this->upperJointTable,
                   this->upperMorphTable, STALFOS_LIMB_MAX);

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawFeet, 90.0f);

    this->actor.colChkInfo.cylRadius = 40;
    this->actor.colChkInfo.cylHeight = 100;
    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += 45.0f;
    this->actor.colChkInfo.damageTable = &sDamageTable;

    Collider_InitCylinder(play, &this->bodyCollider);
    Collider_SetCylinder(play, &this->bodyCollider, &this->actor, &sBodyColliderInit);

    Collider_InitCylinder(play, &this->shieldCollider);
    Collider_SetCylinder(play, &this->shieldCollider, &this->actor, &sShieldColliderInit);

    Collider_InitQuad(play, &this->swordCollider);
    Collider_SetQuad(play, &this->swordCollider, &this->actor, &sSwordColliderInit);

    this->actor.colChkInfo.mass = MASS_HEAVY;
    this->actor.colChkInfo.health = 10;

    slashBlure.p1StartColor[0] = slashBlure.p1StartColor[1] = slashBlure.p1StartColor[2] = slashBlure.p1StartColor[3] =
        slashBlure.p2StartColor[0] = slashBlure.p2StartColor[1] = slashBlure.p2StartColor[2] =
            slashBlure.p1EndColor[0] = slashBlure.p1EndColor[1] = slashBlure.p1EndColor[2] = slashBlure.p2EndColor[0] =
                slashBlure.p2EndColor[1] = slashBlure.p2EndColor[2] = 255;

    slashBlure.p1EndColor[3] = 0;
    slashBlure.p2EndColor[3] = 0;
    slashBlure.p2StartColor[3] = 64;

    slashBlure.elemDuration = 4;
    slashBlure.unkFlag = 0;
    slashBlure.calcMode = 2;

    Effect_Add(play, &this->effectIndex, EFFECT_BLURE1, 0, 0, &slashBlure);

    if (this->actor.params != STALFOS_TYPE_CEILING) {
        EnTest_SetupWaitGround(this);
    } else {
        EnTest_SetupWaitAbove(this);
    }

    if (this->actor.params == STALFOS_TYPE_INVISIBLE) {
        this->actor.flags |= ACTOR_FLAG_REACT_TO_LENS;
    }
}

void EnTest_Destroy(Actor* thisx, PlayState* play) {
    EnTest* this = (EnTest*)thisx;
    
    if (this->isFriend)
    {
        sFriends = 0;
        sFriendsDown = 0;
        //Audio_RestorePrevBgm(); // turn off miniboss music for demo
    }

    // it originally worked like this:
    /*
    if ((this->actor.params != STALFOS_TYPE_2) &&
        !Actor_FindNearby(play, &this->actor, ACTOR_EN_TEST, ACTORCAT_ENEMY, 8000.0f)) {
        Audio_RestorePrevBgm();
    }
    */

    Effect_Destroy(play, this->effectIndex);
    Collider_DestroyCylinder(play, &this->shieldCollider);
    Collider_DestroyCylinder(play, &this->bodyCollider);
    Collider_DestroyQuad(play, &this->swordCollider);
}

/**
 * If EnTest_ChooseAction failed to pick a new action, this function will unconditionally pick
 * a new action as a last resort
 */
void EnTest_ChooseRandomAction(EnTest* this, PlayState* play) {
    switch ((u32)(Rand_ZeroOne() * 10.0f)) {
        case 0:
        case 1:
        case 5:
        case 6:
            if ((this->actor.xzDistToPlayer < 220.0f) && (this->actor.xzDistToPlayer > 170.0f) &&
                Actor_IsFacingPlayer(&this->actor, 0x71C) && Actor_IsTargeted(play, &this->actor)) {
                EnTest_SetupJumpslash(this);
                break;
            }
            FALLTHROUGH;
        case 8:
            EnTest_SetupWalkAndBlock(this);
            break;

        case 3:
        case 4:
        case 7:
            func_808627C4(this, play);
            break;

        case 2:
        case 9:
        case 10:
            EnTest_SetupStopAndBlock(this);
            break;
    }
}

void EnTest_ChooseAction(EnTest* this, PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);
    s16 yawDiff = player->actor.shape.rot.y - this->actor.shape.rot.y;

    yawDiff = ABS(yawDiff);

    if (yawDiff >= 0x61A8) {
        switch ((u32)(Rand_ZeroOne() * 10.0f)) {
            case 0:
            case 3:
            case 7:
                EnTest_SetupStopAndBlock(this);
                break;

            case 1:
            case 5:
            case 6:
            case 8:
                func_808627C4(this, play);
                break;

            case 2:
            case 4:
            case 9:
                if (this->actor.params != STALFOS_TYPE_CEILING) {
                    this->actor.world.rot.y = this->actor.yawTowardsPlayer;
                    EnTest_SetupJumpBack(this);
                }
                break;
        }
    } else if (yawDiff <= 0x3E80) {
        if (ABS((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y)) > 0x3E80) {
            if (((play->gameplayFrames % 2) != 0) && (this->actor.params != STALFOS_TYPE_CEILING)) {
                this->actor.world.rot.y = this->actor.yawTowardsPlayer;
                EnTest_SetupJumpBack(this);
            } else if ((this->actor.xzDistToPlayer < 220.0f) && (this->actor.xzDistToPlayer > 170.0f)) {
                if (Actor_IsFacingPlayer(&this->actor, 0x71C) && !Actor_IsTargeted(play, &this->actor)) {
                    EnTest_SetupJumpslash(this);
                }
            } else {
                EnTest_SetupWalkAndBlock(this);
            }
        } else {
            if (this->actor.xzDistToPlayer < 110.0f) {
                if (Rand_ZeroOne() > 0.2f) {
                    if (player->stateFlags1 & PLAYER_STATE1_4) {
                        if (this->actor.isLockedOn) {
                            EnTest_SetupSlashDown(this);
                        } else {
                            func_808627C4(this, play);
                        }
                    } else {
                        EnTest_SetupSlashDown(this);
                    }
                }
            } else {
                EnTest_ChooseRandomAction(this, play);
            }
        }
    } else {
        EnTest_ChooseRandomAction(this, play);
    }
}

void EnTest_SetupWaitGround(EnTest* this) {
    Animation_PlayLoop(&this->skelAnime, (void*)(0x0600316C));
    this->unk_7C8 = 0;
    this->timer = 15;
    this->actor.scale.y = 0.0f;
    this->actor.world.pos.y = this->actor.home.pos.y - 3.5f;
    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    EnTest_SetupAction(this, EnTest_WaitGround);
}

void EnTest_WaitGround(EnTest* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if ((this->timer == 0) && (ABS(this->actor.playerHeightRel) < 150.0f)) {
        this->unk_7C8 = 3;
        EnTest_SetupAction(this, EnTest_Rise);
        this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;

        // turn off miniboss music on stalfos
        //Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        // old code
        /*
        if (this->actor.params != STALFOS_TYPE_2) {
            Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        }
        */
    } else {
        if (this->timer != 0) {
            this->timer--;
        }

        this->actor.world.pos.y = this->actor.home.pos.y - 3.5f;
    }
}

static void Revive(EnTest *this, PlayState *play)
{
    if (SkelAnime_Update(&this->skelAnime))
    {
        EnTest_SetupIdle(this);
        this->timer = 0;
        this->actor.colChkInfo.health = 10;
        this->actor.flags |= ACTOR_FLAG_TARGETABLE;
    }
}

static void SetupRevive(EnTest *this, PlayState *play)
{
    sFriendsDown -= 1;
    EnTest_SetupAction(this, Revive);
    Animation_MorphToPlayOnce(&this->skelAnime, (void*)(0x0600316C), GETUP_TIME);
    func_800BC154(play, &play->actorCtx, &this->actor, ACTORCAT_ENEMY);
    this->reviveTimer = REVIVE_TIMER_START;
}

void EnTest_SetupWaitAbove(EnTest* this) {
    Animation_PlayLoop(&this->skelAnime, (void*)(0x0600316C));
    this->unk_7C8 = 0;
    this->actor.world.pos.y = this->actor.home.pos.y + 150.0f;
    Actor_SetScale(&this->actor, 0.0f);
    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    EnTest_SetupAction(this, EnTest_WaitAbove);
}

void EnTest_WaitAbove(EnTest* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    this->actor.world.pos.y = this->actor.home.pos.y + 150.0f;

    if ((this->actor.xzDistToPlayer < 200.0f) && (ABS(this->actor.playerHeightRel) < 450.0f)) {
        EnTest_SetupAction(this, EnTest_Fall);
        this->actor.flags |= ACTOR_FLAG_TARGETABLE;
        this->actor.shape.rot.y = this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        Actor_SetScale(&this->actor, 0.015f);
    }
}

void EnTest_SetupIdle(EnTest* this) {
    Animation_PlayLoop(&this->skelAnime, (void*)(0x0600316C));
    this->unk_7C8 = 0xA;
    this->timer = (Rand_ZeroOne() * 10.0f) + 5.0f;
    this->actor.speed = 0.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnTest_SetupAction(this, EnTest_Idle);
}

void EnTest_Idle(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yawDiff;

    SkelAnime_Update(&this->skelAnime);

    if (!EnTest_ReactToProjectile(play, this)) {
        yawDiff = player->actor.shape.rot.y - this->actor.shape.rot.y;

        if (this->actor.xzDistToPlayer < 100.0f) {
            if ((player->meleeWeaponState != 0) && (ABS(yawDiff) >= 0x1F40)) {
                this->actor.shape.rot.y = this->actor.world.rot.y = this->actor.yawTowardsPlayer;

                if (Rand_ZeroOne() > 0.7f && player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START) {
                    EnTest_SetupJumpBack(this);
                } else {
                    func_808627C4(this, play);
                }
                return;
            }
        }

        if (this->timer != 0) {
            this->timer--;
        } else {
            if (Actor_IsFacingPlayer(&this->actor, 0x1555)) {
                if ((this->actor.xzDistToPlayer < 220.0f) && (this->actor.xzDistToPlayer > 160.0f) &&
                    (Rand_ZeroOne() < 0.3f)) {
                    if (Actor_IsTargeted(play, &this->actor)) {
                        EnTest_SetupJumpslash(this);
                    } else {
                        func_808627C4(this, play);
                    }
                } else {
                    if (Rand_ZeroOne() > 0.3f) {
                        EnTest_SetupWalkAndBlock(this);
                    } else {
                        func_808627C4(this, play);
                    }
                }
            } else {
                if (Rand_ZeroOne() > 0.7f) {
                    func_80860BDC(this);
                } else {
                    EnTest_ChooseAction(this, play);
                }
            }
        }
    }
}

void EnTest_Fall(EnTest* this, PlayState* play) {
    Animation_PlayOnceSetSpeed(&this->skelAnime, (void*)(0x0600C438), 0.0f);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.world.pos.y <= this->actor.floorHeight) {
        this->skelAnime.playSpeed = 1.0f;
        this->unk_7C8 = 0xC;
        this->timer = this->unk_7E4 * 0.15f;
        Actor_PlaySfx(&this->actor, NA_SE_EN_GERUDOFT_DOWN);
        EnTest_SetupAction(this, EnTest_Land);
    }
}

void EnTest_Land(EnTest* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        EnTest_SetupIdle(this);
        this->timer = (Rand_ZeroOne() * 10.0f) + 5.0f;
    }
}

void EnTest_SetupWalkAndBlock(EnTest* this) {
    Animation_Change(&this->upperSkelanime, (void*)(0x06001C20), 2.0f, 0.0f,
                     Animation_GetLastFrame((void*)(0x06001C20)), 2, 2.0f);
    Animation_PlayLoop(&this->skelAnime, (void*)(0x060081B4));
    this->timer = (s16)(Rand_ZeroOne() * 5.0f);
    this->unk_7C8 = 0xD;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnTest_SetupAction(this, EnTest_WalkAndBlock);
}

void EnTest_WalkAndBlock(EnTest* this, PlayState* play) {
    s32 pad;
    f32 checkDist = 0.0f;
    s32 pad1;
    s32 prevFrame;
    s32 beforeCurFrame;
    f32 playSpeed;
    Player* player = GET_PLAYER(play);
    s32 absPlaySpeed;
    s16 yawDiff;

    if (!EnTest_ReactToProjectile(play, this)) {
        this->timer++;

        if (Actor_OtherIsTargeted(play, &this->actor)) {
            checkDist = 150.0f;
        }

        if (this->actor.xzDistToPlayer <= (80.0f + checkDist)) {
            Math_SmoothStepToF(&this->actor.speed, -5.0f, 1.0f, 0.8f, 0.0f);
        } else if (this->actor.xzDistToPlayer > (110.0f + checkDist)) {
            Math_SmoothStepToF(&this->actor.speed, 5.0f, 1.0f, 0.8f, 0.0f);
        }

        if (this->actor.speed >= 5.0f) {
            this->actor.speed = 5.0f;
        } else if (this->actor.speed < -5.0f) {
            this->actor.speed = -5.0f;
        }

        if ((this->actor.params == STALFOS_TYPE_CEILING) &&
            !Actor_TestFloorInDirection(&this->actor, play, this->actor.speed, this->actor.world.rot.y)) {
            this->actor.speed *= -1.0f;
        }

        if (ABS(this->actor.speed) < 3.0f) {
            Animation_Change(&this->skelAnime, (void*)(0x060081B4), 0.0f, this->skelAnime.curFrame,
                             Animation_GetLastFrame((void*)(0x060081B4)), 0, -6.0f);
            playSpeed = this->actor.speed * 10.0f;
        } else {
            Animation_Change(&this->skelAnime, (void*)(0x060026D4), 0.0f, this->skelAnime.curFrame,
                             Animation_GetLastFrame((void*)(0x060026D4)), 0, -4.0f);
            playSpeed = this->actor.speed * 10.0f * 0.02f;
        }

        if (this->actor.speed >= 0.0f) {
            if (this->unk_7DE == 0) {
                this->unk_7DE++;
            }

            playSpeed = CLAMP_MAX(playSpeed, 2.5f);
            this->skelAnime.playSpeed = playSpeed;
        } else {
            playSpeed = CLAMP_MIN(playSpeed, -2.5f);
            this->skelAnime.playSpeed = playSpeed;
        }

        yawDiff = player->actor.shape.rot.y - this->actor.shape.rot.y;

        if ((this->actor.xzDistToPlayer < 100.0f) && (player->meleeWeaponState != 0)) {
            if (ABS(yawDiff) >= 0x1F40) {
                this->actor.shape.rot.y = this->actor.world.rot.y = this->actor.yawTowardsPlayer;

                if ((Rand_ZeroOne() > 0.7f) && (player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START)) {
                    EnTest_SetupJumpBack(this);
                } else {
                    EnTest_SetupStopAndBlock(this);
                }

                return;
            }
        }

        prevFrame = (s32)this->skelAnime.curFrame;
        SkelAnime_Update(&this->skelAnime);
        beforeCurFrame = (s32)(this->skelAnime.curFrame - ABS(this->skelAnime.playSpeed));
        absPlaySpeed = (s32)(f32)ABS(this->skelAnime.playSpeed);

        if ((s32)this->skelAnime.curFrame != prevFrame) {
            s32 afterPrevFrame = absPlaySpeed + prevFrame;

            if (((afterPrevFrame > 1) && (beforeCurFrame < 1)) || ((beforeCurFrame < 7) && (afterPrevFrame > 7))) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WALK);
            }
        }

        if ((this->timer % 32) == 0) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WARAU);
            this->timer += (s16)(Rand_ZeroOne() * 5.0f);
        }

        if ((this->actor.xzDistToPlayer < 220.0f) && (this->actor.xzDistToPlayer > 160.0f) &&
            (Actor_IsFacingPlayer(&this->actor, 0x71C))) {
            if (Actor_IsTargeted(play, &this->actor)) {
                if (Rand_ZeroOne() < 0.1f) {
                    EnTest_SetupJumpslash(this);
                    return;
                }
            } else if (player->heldItemAction != PLAYER_IA_NONE) {
                if (this->actor.isLockedOn) {
                    if ((play->gameplayFrames % 2) != 0) {
                        func_808627C4(this, play);
                        return;
                    }

                    EnTest_ChooseAction(this, play);
                } else {
                    func_80860EC0(this);
                }
            }
        }

        if (Rand_ZeroOne() < 0.4f) {
            this->actor.shape.rot.y = this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        }

        if (!Actor_IsFacingPlayer(&this->actor, 0x11C7)) {
            EnTest_SetupIdle(this);
            this->timer = (Rand_ZeroOne() * 10.0f) + 10.0f;
            return;
        }

        if (this->actor.xzDistToPlayer < 110.0f) {
            if (Rand_ZeroOne() > 0.2f) {
                if (player->stateFlags1 & PLAYER_STATE1_4) {
                    if (this->actor.isLockedOn) {
                        EnTest_SetupSlashDown(this);
                    } else {
                        func_808627C4(this, play);
                    }
                } else {
                    EnTest_SetupSlashDown(this);
                }
            } else {
                EnTest_SetupStopAndBlock(this);
            }
        } else if (Rand_ZeroOne() < 0.1f) {
            this->actor.speed = 5.0f;
        }
    }
}

// a variation of sidestep
void func_80860BDC(EnTest* this) {
    Animation_PlayLoop(&this->skelAnime, (void*)(0x0600E2B0));
    this->unk_7C8 = 0xE;
    EnTest_SetupAction(this, func_80860C24);
}

// a variation of sidestep
void func_80860C24(EnTest* this, PlayState* play) {
    s16 yawDiff;
    s16 yawChange;
    f32 playSpeed;
    s32 prevFrame;
    s32 beforeCurFrame;
    s32 afterPrevFrame;
    s32 absPlaySpeed;

    if (!EnTest_ReactToProjectile(play, this)) {
        yawDiff = this->actor.yawTowardsPlayer;
        yawDiff -= this->actor.shape.rot.y;

        if (yawDiff > 0) {
            yawChange = (yawDiff / 42.0f) + 300.0f;
            this->actor.shape.rot.y += yawChange * 2;
        } else {
            yawChange = (yawDiff / 42.0f) - 300.0f;
            this->actor.shape.rot.y += yawChange * 2;
        }

        this->actor.world.rot.y = this->actor.shape.rot.y;

        if (yawDiff > 0) {
            playSpeed = yawChange * 0.02f;
            playSpeed = CLAMP_MAX(playSpeed, 1.0f);
            this->skelAnime.playSpeed = playSpeed;
        } else {
            playSpeed = yawChange * 0.02f;
            playSpeed = CLAMP_MIN(playSpeed, -1.0f);
            this->skelAnime.playSpeed = playSpeed;
        }

        prevFrame = (s32)this->skelAnime.curFrame;
        SkelAnime_Update(&this->skelAnime);
        beforeCurFrame = (s32)(this->skelAnime.curFrame - ABS(this->skelAnime.playSpeed));
        absPlaySpeed = (s32)(f32)ABS(this->skelAnime.playSpeed);

        if ((s32)this->skelAnime.curFrame != prevFrame) {
            afterPrevFrame = absPlaySpeed + prevFrame;

            if (((afterPrevFrame > 2) && (beforeCurFrame <= 0)) || ((beforeCurFrame < 7) && (afterPrevFrame >= 9))) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WALK);
            }
        }

        if (Actor_IsFacingPlayer(&this->actor, 0x71C)) {
            if (Rand_ZeroOne() > 0.8f) {
                if ((Rand_ZeroOne() > 0.7f)) {
                    func_80860EC0(this);
                } else {
                    EnTest_ChooseAction(this, play);
                }
            } else {
                EnTest_SetupWalkAndBlock(this);
            }
        }
    }
}

// a variation of sidestep
void func_80860EC0(EnTest* this) {
    Animation_PlayLoop(&this->skelAnime, (void*)(0x0600E2B0));
    this->unk_7C8 = 0xF;
    this->actor.speed = (Rand_ZeroOne() > 0.5f) ? -0.5f : 0.5f;
    this->timer = (s16)((Rand_ZeroOne() * 15.0f) + 25.0f);
    this->unk_7EC = 0.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnTest_SetupAction(this, func_80860F84);
}

// a variation of sidestep
void func_80860F84(EnTest* this, PlayState* play) {
    s16 playerYaw180;
    s32 pad;
    s32 prevFrame;
    s32 beforeCurFrame;
    s16 yawDiff;
    Player* player = GET_PLAYER(play);
    f32 checkDist = 0.0f;
    s16 newYaw;
    s32 absPlaySpeed;

    if (!EnTest_ReactToProjectile(play, this)) {
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xFA0, 1);
        this->actor.world.rot.y = this->actor.shape.rot.y + 0x3E80;
        playerYaw180 = player->actor.shape.rot.y + 0x8000;

        if (this->actor.speed >= 0.0f) {
            if (this->actor.speed < 6.0f) {
                this->actor.speed += 0.5f;
            } else {
                this->actor.speed = 6.0f;
            }
        } else {
            if (this->actor.speed > -6.0f) {
                this->actor.speed -= 0.5f;
            } else {
                this->actor.speed = -6.0f;
            }
        }

        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) ||
            ((this->actor.params == STALFOS_TYPE_CEILING) &&
             !Actor_TestFloorInDirection(&this->actor, play, this->actor.speed, this->actor.world.rot.y))) {
            if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
                if (this->actor.speed >= 0.0f) {
                    newYaw = this->actor.shape.rot.y + 0x3FFF;
                } else {
                    newYaw = this->actor.shape.rot.y - 0x3FFF;
                }

                newYaw = this->actor.wallYaw - newYaw;
            } else {
                this->actor.speed *= -0.8f;
                newYaw = 0;
            }

            if (ABS(newYaw) > 0x4000) {
                this->actor.speed *= -0.8f;

                if (this->actor.speed < 0.0f) {
                    this->actor.speed -= 0.5f;
                } else {
                    this->actor.speed += 0.5f;
                }
            }
        }

        if (Actor_OtherIsTargeted(play, &this->actor)) {
            checkDist = 200.0f;
        }

        if (this->actor.xzDistToPlayer <= (80.0f + checkDist)) {
            Math_SmoothStepToF(&this->unk_7EC, -2.5f, 1.0f, 0.8f, 0.0f);
        } else if (this->actor.xzDistToPlayer > (110.0f + checkDist)) {
            Math_SmoothStepToF(&this->unk_7EC, 2.5f, 1.0f, 0.8f, 0.0f);
        } else {
            Math_SmoothStepToF(&this->unk_7EC, 0.0f, 1.0f, 6.65f, 0.0f);
        }

        if (this->unk_7EC != 0.0f) {
            this->actor.world.pos.x += Math_SinS(this->actor.shape.rot.y) * this->unk_7EC;
            this->actor.world.pos.z += Math_CosS(this->actor.shape.rot.y) * this->unk_7EC;
        }

        this->skelAnime.playSpeed = this->actor.speed * 0.5f;

        prevFrame = (s32)this->skelAnime.curFrame;
        SkelAnime_Update(&this->skelAnime);
        beforeCurFrame = (s32)(this->skelAnime.curFrame - ABS(this->skelAnime.playSpeed));
        absPlaySpeed = (s32)(f32)ABS(this->skelAnime.playSpeed);

        if ((s32)this->skelAnime.curFrame != prevFrame) {
            s32 afterPrevFrame = absPlaySpeed + prevFrame;

            if (((afterPrevFrame > 1) && (beforeCurFrame < 1)) || ((beforeCurFrame < 7) && (afterPrevFrame > 7))) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WALK);
            }
        }

        if ((play->gameplayFrames & 95) == 0) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WARAU);
        }

        yawDiff = playerYaw180 - this->actor.shape.rot.y;
        yawDiff = ABS(yawDiff);

        if ((yawDiff > 0x6800) || (this->timer == 0)) {
            EnTest_ChooseAction(this, play);
        } else if (this->timer != 0) {
            this->timer--;
        }
    }
}

void EnTest_SetupSlashDown(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x0600B00C));
    //Audio_StopSfxByPosAndId(&this->actor.projectedPos, NA_SE_EN_STAL_WARAU);
    this->swordCollider.base.atFlags &= ~AT_BOUNCED;
    this->unk_7C8 = 0x10;
    this->actor.speed = 0.0f;
    EnTest_SetupAction(this, EnTest_SlashDown);
    this->swordCollider.info.toucher.damage = 16;

    if (this->unk_7DE != 0) {
        this->unk_7DE = 3;
    }
}

void EnTest_SlashDown(EnTest* this, PlayState* play) {
    this->actor.speed = 0.0f;

    if ((s32)this->skelAnime.curFrame < 4) {
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xBB8, 0);
    }

    if ((s32)this->skelAnime.curFrame == 7) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_SAKEBI);
    }

    if ((this->skelAnime.curFrame > 7.0f) && (this->skelAnime.curFrame < 11.0f)) {
        this->swordState = 1;
    } else {
        this->swordState = 0;
    }

    if (SkelAnime_Update(&this->skelAnime)) {
        if ((play->gameplayFrames % 2) != 0) {
            EnTest_SetupSlashDownEnd(this);
        } else {
            EnTest_SetupSlashUp(this);
        }
    }
}

void EnTest_SetupSlashDownEnd(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x0600B4E4));
    this->unk_7C8 = 0x12;
    this->actor.speed = 0.0f;
    EnTest_SetupAction(this, EnTest_SlashDownEnd);
}

void EnTest_SlashDownEnd(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yawDiff;

    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->swordCollider.base.atFlags & AT_HIT) {
            this->swordCollider.base.atFlags &= ~AT_HIT;
            if (this->actor.params != STALFOS_TYPE_CEILING) {
                EnTest_SetupJumpBack(this);
                return;
            }
        }

        if (Rand_ZeroOne() > 0.7f) {
            EnTest_SetupIdle(this);
            this->timer = (Rand_ZeroOne() * 5.0f) + 5.0f;
            return;
        }

        this->actor.world.rot.y = Actor_WorldYawTowardActor(&this->actor, &player->actor);

        if (Rand_ZeroOne() > 0.7f) {
            if (this->actor.params != STALFOS_TYPE_CEILING) {
                EnTest_SetupJumpBack(this);
                return;
            }
        }

        yawDiff = player->actor.shape.rot.y - this->actor.shape.rot.y;

        if (ABS(yawDiff) <= 0x2710) {
            yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;

            if ((ABS(yawDiff) > 0x3E80) && (this->actor.params != STALFOS_TYPE_CEILING)) {
                this->actor.world.rot.y = this->actor.yawTowardsPlayer;
                EnTest_SetupJumpBack(this);
            } else if (player->stateFlags1 & PLAYER_STATE1_4) {
                if (this->actor.isLockedOn) {
                    EnTest_SetupSlashDown(this);
                } else if ((play->gameplayFrames % 2) != 0) {
                    func_808627C4(this, play);
                } else {
                    EnTest_SetupJumpBack(this);
                }
            } else {
                EnTest_SetupSlashDown(this);
            }
        } else {
            func_808627C4(this, play);
        }
    }
}

void EnTest_SetupSlashUp(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x0600BE4C));
    this->swordCollider.base.atFlags &= ~AT_BOUNCED;
    this->unk_7C8 = 0x11;
    this->swordCollider.info.toucher.damage = 16;
    this->actor.speed = 0.0f;
    EnTest_SetupAction(this, EnTest_SlashUp);

    if (this->unk_7DE != 0) {
        this->unk_7DE = 3;
    }
}

void EnTest_SlashUp(EnTest* this, PlayState* play) {
    this->actor.speed = 0.0f;

    if ((s32)this->skelAnime.curFrame == 2) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_SAKEBI);
    }

    if ((this->skelAnime.curFrame > 1.0f) && (this->skelAnime.curFrame < 8.0f)) {
        this->swordState = 1;
    } else {
        this->swordState = 0;
    }

    if (SkelAnime_Update(&this->skelAnime)) {
        EnTest_SetupSlashDown(this);
    }
}

void EnTest_SetupJumpBack(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06001978));
    Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP);
    this->unk_7C8 = 0x14;
    this->timer = 5;
    EnTest_SetupAction(this, EnTest_JumpBack);

    if (this->unk_7DE != 0) {
        this->unk_7DE = 3;
    }

    if (this->actor.params != STALFOS_TYPE_CEILING) {
        this->actor.speed = -11.0f;
    } else {
        this->actor.speed = -7.0f;
    }
}

void EnTest_JumpBack(EnTest* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xBB8, 1);

    if (this->timer == 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WARAU);
    } else {
        this->timer--;
    }

    if (SkelAnime_Update(&this->skelAnime)) {
        if (!EnTest_ReactToProjectile(play, this)) {
            if (this->actor.xzDistToPlayer <= 100.0f) {
                if (Actor_IsFacingPlayer(&this->actor, 0x1555)) {
                    EnTest_SetupSlashDown(this);
                } else {
                    EnTest_SetupIdle(this);
                    this->timer = (Rand_ZeroOne() * 5.0f) + 5.0f;
                }
            } else {
                if ((this->actor.xzDistToPlayer <= 220.0f) && Actor_IsFacingPlayer(&this->actor, 0xE38)) {
                    EnTest_SetupJumpslash(this);
                } else {
                    EnTest_SetupIdle(this);
                    this->timer = (Rand_ZeroOne() * 5.0f) + 5.0f;
                }
            }
            this->actor.flags |= ACTOR_FLAG_TARGETABLE;
        }
    } else if (this->skelAnime.curFrame == (this->skelAnime.endFrame - 4.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
    }
}

void EnTest_SetupJumpslash(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x0600A324));
    //Audio_StopSfxByPosAndId(&this->actor.projectedPos, NA_SE_EN_STAL_WARAU);
    this->timer = 0;
    this->unk_7C8 = 0x17;
    this->actor.velocity.y = 10.0f;
    this->actor.speed = 8.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->swordCollider.base.atFlags &= ~AT_BOUNCED;
    EnTest_SetupAction(this, EnTest_Jumpslash);
    this->swordCollider.info.toucher.damage = 32;

    if (this->unk_7DE != 0) {
        this->unk_7DE = 3;
    }
}

void EnTest_Jumpslash(EnTest* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->timer == 0) {
            Animation_PlayOnce(&this->skelAnime, (void*)(0x0600A99C));
            this->timer = 1;
            this->swordState = 1;
            Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_SAKEBI);
            Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP);
        } else {
            this->actor.speed = 0.0f;
            EnTest_SetupIdle(this);
        }
    }

    if ((this->timer != 0) && (this->skelAnime.curFrame >= 5.0f)) {
        this->swordState = 0;
    }

    if (this->actor.world.pos.y <= this->actor.floorHeight) {
        if (this->actor.speed != 0.0f) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
        }

        this->actor.world.pos.y = this->actor.floorHeight;
        this->actor.velocity.y = 0.0f;
        this->actor.speed = 0.0f;
    }
}

void EnTest_SetupJumpUp(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x0600A324));
    this->timer = 0;
    this->unk_7C8 = 4;
    this->actor.velocity.y = 14.0f;
    this->actor.speed = 6.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnTest_SetupAction(this, EnTest_JumpUp);
}

void EnTest_JumpUp(EnTest* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xFA0, 1);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.world.pos.y <= this->actor.floorHeight) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
        this->actor.world.pos.y = this->actor.floorHeight;
        this->unk_7E4 = -(s32)this->actor.velocity.y;

        if (this->unk_7E4 == 0) {
            this->unk_7E4 = 1;
        }

        this->actor.velocity.y = 0.0f;
        this->actor.speed = 0.0f;
        this->unk_7C8 = 0xC;
        this->timer = 4;
        Animation_Change(&this->skelAnime, (void*)(0x0600C438), 0.0f, 0.0f, 0.0f, 2, 0.0f);
        EnTest_SetupAction(this, EnTest_Land);
    }
}

void EnTest_SetupStopAndBlock(EnTest* this) {
    Animation_Change(&this->skelAnime, (void*)(0x06001C20), 2.0f, 0.0f,
                     Animation_GetLastFrame((void*)(0x06001C20)), 2, 2.0f);
    this->unk_7C8 = 0x15;
    this->actor.speed = 0.0f;
    this->timer = (Rand_ZeroOne() * 10.0f) + 11.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->unk_7DE = 5;
    EnTest_SetupAction(this, EnTest_StopAndBlock);
}

void EnTest_StopAndBlock(EnTest* this, PlayState* play) {
    Math_SmoothStepToF(&this->actor.speed, 0.0f, 1.0f, 0.5f, 0.0f);
    SkelAnime_Update(&this->skelAnime);

    if ((ABS((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y)) > 0x3E80) &&
        (this->actor.params != STALFOS_TYPE_CEILING) && ((play->gameplayFrames % 2) != 0)) {
        this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        EnTest_SetupJumpBack(this);
    }

    if (this->timer == 0) {
        EnTest_SetupIdleFromBlock(this);
    } else {
        this->timer--;
    }
}

void EnTest_SetupIdleFromBlock(EnTest* this) {
    Animation_MorphToLoop(&this->skelAnime, (void*)(0x0600316C), -4.0f);
    this->unk_7C8 = 0x16;
    EnTest_SetupAction(this, EnTest_IdleFromBlock);
}

void EnTest_IdleFromBlock(EnTest* this, PlayState* play) {
    Math_SmoothStepToF(&this->actor.speed, 0.0f, 1.0f, 1.5f, 0.0f);
    SkelAnime_Update(&this->skelAnime);

    if (this->skelAnime.morphWeight == 0.0f) {
        this->actor.speed = 0.0f;
        this->unk_7DE = 0;

        if (!EnTest_ReactToProjectile(play, this)) {
            if (this->actor.xzDistToPlayer < 500.0f) {
                EnTest_ChooseAction(this, play);
            } else {
                func_808627C4(this, play);
            }
        }
    }
}

void func_80862154(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06008604));
    Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_DAMAGE);
    this->unk_7C8 = 8;
    this->actor.speed = -2.0f;
    Actor_SetColorFilter(&this->actor, 0x4000, 0xFF, 0, 8);
    EnTest_SetupAction(this, func_808621D4);
}

void func_808621D4(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 1.0f, 0.1f, 0.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        this->actor.speed = 0.0f;

        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
            ((ABS((s16)(this->actor.wallYaw - this->actor.shape.rot.y)) < 0x38A4) &&
             (this->actor.xzDistToPlayer < 80.0f))) {
            EnTest_SetupJumpUp(this);
        } else if (!EnTest_ReactToProjectile(play, this)) {
            EnTest_ChooseAction(this, play);
        } else {
            return;
        }
    }

    if (player->meleeWeaponState != 0) {
        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
            ((ABS((s16)(this->actor.wallYaw - this->actor.shape.rot.y)) < 0x38A4) &&
             (this->actor.xzDistToPlayer < 80.0f))) {
            EnTest_SetupJumpUp(this);
        } else if ((Rand_ZeroOne() > 0.7f) && (this->actor.params != STALFOS_TYPE_CEILING) &&
                   (player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START)) {
            EnTest_SetupJumpBack(this);
        } else {
            EnTest_SetupStopAndBlock(this);
        }

        this->unk_7C8 = 8;
    }
}

void func_80862398(EnTest* this) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06000444));
    Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_DAMAGE);
    this->unk_7C8 = 9;
    this->actor.speed = -2.0f;
    Actor_SetColorFilter(&this->actor, 0x4000, 0xFF, 0, 8);
    EnTest_SetupAction(this, func_80862418);
}

void func_80862418(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 1.0f, 0.1f, 0.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        this->actor.speed = 0.0f;

        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
            ((ABS((s16)(this->actor.wallYaw - this->actor.shape.rot.y)) < 0x38A4) &&
             (this->actor.xzDistToPlayer < 80.0f))) {
            EnTest_SetupJumpUp(this);
        } else if (!EnTest_ReactToProjectile(play, this)) {
            EnTest_ChooseAction(this, play);
        } else {
            return;
        }
    }

    if (player->meleeWeaponState != 0) {
        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
            ((ABS((s16)(this->actor.wallYaw - this->actor.shape.rot.y)) < 0x38A4) &&
             (this->actor.xzDistToPlayer < 80.0f))) {
            EnTest_SetupJumpUp(this);
        } else if ((Rand_ZeroOne() > 0.7f) && (this->actor.params != STALFOS_TYPE_CEILING) &&
                   (player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START)) {
            EnTest_SetupJumpBack(this);
        } else {
            EnTest_SetupStopAndBlock(this);
        }

        this->unk_7C8 = 8;
    }
}

void EnTest_SetupStunned(EnTest* this) {
    this->unk_7C8 = 0xB;
    this->unk_7DE = 0;
    this->swordState = 0;
    this->skelAnime.playSpeed = 0.0f;
    this->actor.speed = -4.0f;

    if (this->lastDamageEffect == STALFOS_DMGEFF_LIGHT) {
        Actor_SetColorFilter(&this->actor, -0x8000, 0x78, 0, 0x50);
    } else {
        Actor_SetColorFilter(&this->actor, 0, 0x78, 0, 0x50);

        if (this->lastDamageEffect == STALFOS_DMGEFF_FREEZE) {
            this->iceTimer = 36;
        } else {
            Animation_PlayOnceSetSpeed(&this->skelAnime, (void*)(0x06008604), 0.0f);
        }
    }

    Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
    EnTest_SetupAction(this, EnTest_Stunned);
}

void EnTest_Stunned(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Math_SmoothStepToF(&this->actor.speed, 0.0f, 1.0f, 1.0f, 0.0f);

    if (this->actor.colorFilterTimer == 0) {
        if (this->actor.colChkInfo.health == 0) {
            SetupFallOverBackwardsAndDie(this, play);
        } else if (player->meleeWeaponState != 0) {
            if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) &&
                ((ABS((s16)(this->actor.wallYaw - this->actor.shape.rot.y)) < 0x38A4) &&
                 (this->actor.xzDistToPlayer < 80.0f))) {
                EnTest_SetupJumpUp(this);
            } else if ((Rand_ZeroOne() > 0.7f) && (player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START)) {
                EnTest_SetupJumpBack(this);
            } else {
                EnTest_SetupStopAndBlock(this);
            }

            this->unk_7C8 = 8;
        } else {
            this->actor.speed = 0.0f;
            if (!EnTest_ReactToProjectile(play, this)) {
                EnTest_ChooseAction(this, play);
            }
        }
    }
}

// a variation of sidestep
void func_808627C4(EnTest* this, PlayState* play) {
    if (Actor_OtherIsTargeted(play, &this->actor)) {
        func_80860EC0(this);
        return;
    }

    Animation_MorphToLoop(&this->skelAnime, (void*)(0x0600E2B0), -2.0f);
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xFA0, 1);
    this->actor.speed = ((play->gameplayFrames % 2) != 0) ? -4.0f : 4.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y + 0x3FFF;
    this->timer = (Rand_ZeroOne() * 20.0f) + 20.0f;
    this->unk_7C8 = 0x18;
    EnTest_SetupAction(this, func_808628C8);
    this->unk_7EC = 0.0f;
}

// a variation of sidestep
void func_808628C8(EnTest* this, PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);
    s32 pad1;
    s32 prevFrame;
    s32 beforeCurFrame;
    s32 pad2;
    f32 checkDist = 0.0f;
    s16 newYaw;
    f32 absPlaySpeed;

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0xFA0, 1);

    if (this->unk_7DE == 0) {
        this->unk_7DE++;
    }

    if (this->actor.speed >= 0.0f) {
        if (this->actor.speed < 6.0f) {
            this->actor.speed += 0.125f;
        } else {
            this->actor.speed = 6.0f;
        }
    } else {
        if (this->actor.speed > -6.0f) {
            this->actor.speed -= 0.125f;
        } else {
            this->actor.speed = -6.0f;
        }
    }

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) ||
        ((this->actor.params == STALFOS_TYPE_CEILING) &&
         !Actor_TestFloorInDirection(&this->actor, play, this->actor.speed, this->actor.shape.rot.y + 0x3FFF))) {
        if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
            if (this->actor.speed >= 0.0f) {
                newYaw = (this->actor.shape.rot.y + 0x3FFF);
            } else {
                newYaw = (this->actor.shape.rot.y - 0x3FFF);
            }

            newYaw = this->actor.wallYaw - newYaw;
        } else {
            this->actor.speed *= -0.8f;
            newYaw = 0;
        }

        if (ABS(newYaw) > 0x4000) {
            this->actor.speed *= -0.8f;

            if (this->actor.speed < 0.0f) {
                this->actor.speed -= 0.5f;
            } else {
                this->actor.speed += 0.5f;
            }
        }
    }

    this->actor.world.rot.y = this->actor.shape.rot.y + 0x3FFF;

    if (Actor_OtherIsTargeted(play, &this->actor)) {
        checkDist = 200.0f;
    }

    if (this->actor.xzDistToPlayer <= (80.0f + checkDist)) {
        Math_SmoothStepToF(&this->unk_7EC, -2.5f, 1.0f, 0.8f, 0.0f);
    } else if (this->actor.xzDistToPlayer > (110.0f + checkDist)) {
        Math_SmoothStepToF(&this->unk_7EC, 2.5f, 1.0f, 0.8f, 0.0f);
    } else {
        Math_SmoothStepToF(&this->unk_7EC, 0.0f, 1.0f, 6.65f, 0.0f);
    }

    if (this->unk_7EC != 0.0f) {
        this->actor.world.pos.x += (Math_SinS(this->actor.shape.rot.y) * this->unk_7EC);
        this->actor.world.pos.z += (Math_CosS(this->actor.shape.rot.y) * this->unk_7EC);
    }

    this->skelAnime.playSpeed = this->actor.speed * 0.5f;

    prevFrame = (s32)this->skelAnime.curFrame;
    SkelAnime_Update(&this->skelAnime);
    beforeCurFrame = (s32)(this->skelAnime.curFrame - ABS(this->skelAnime.playSpeed));
    absPlaySpeed = ABS(this->skelAnime.playSpeed);

    if ((this->timer % 32) == 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WARAU);
    }
    if ((s32)this->skelAnime.curFrame != prevFrame) {
        s32 afterPrevFrame = (s32)absPlaySpeed + prevFrame;

        if (((afterPrevFrame > 1) && (beforeCurFrame < 1)) || ((beforeCurFrame < 7) && (afterPrevFrame > 7))) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_WALK);
        }
    }

    if (this->timer == 0) {
        if (Actor_OtherIsTargeted(play, &this->actor)) {
            EnTest_SetupIdle(this);
        } else if (Actor_IsTargeted(play, &this->actor)) {
            if (!EnTest_ReactToProjectile(play, this)) {
                EnTest_ChooseAction(this, play);
            }
        } else if (player->heldItemAction != PLAYER_IA_NONE) {
            if ((play->gameplayFrames % 2) != 0) {
                EnTest_SetupIdle(this);
            } else {
                EnTest_SetupWalkAndBlock(this);
            }
        } else {
            EnTest_SetupWalkAndBlock(this);
        }

    } else {
        this->timer--;
    }
}

void CheckReviveStuff(EnTest *this, PlayState *play)
{
    if (SkelAnime_Update(&this->skelAnime))
    {
        if (this->reviveTimer > 0)
        {
            // knocked down just now
            if (this->reviveTimer == REVIVE_TIMER_START)
            {
                sFriendsDown += 1;
                func_800BC154(play, &play->actorCtx, &this->actor, ACTORCAT_PROP);
            }
            
            // timed out
            this->reviveTimer -= 1;
            if (this->reviveTimer == 0)
                SetupRevive(this, play);
            
            // sFriendsDown could change at any time, so do check once per frame
            if (sFriendsDown == sFriends)
                this->reviveTimer = 0;
        }
        // no revive, so die
        if (this->reviveTimer == 0)
        {
            this->timer = (Rand_ZeroOne() * 10.0f) + 10.0f;
            this->unk_7C8 = 7;
            EnTest_SetupAction(this, BreakApartAndDie);
            BodyBreak_Alloc(&this->bodyBreak, 60, play);
        }
    }
}

void func_80862DBC(EnTest* this, PlayState* play) {
    Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_DAMAGE);
    this->unk_7C8 = 2;
    BodyBreak_Alloc(&this->bodyBreak, 60, play);
    this->actor.home.rot.x = 0;

    if (this->swordState >= 0) {
        EffectBlure_AddSpace(Effect_GetByIndex(this->effectIndex));
        this->swordState = -1;
    }

    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;

    if (this->actor.params == STALFOS_TYPE_5) {
        func_800BC154(play, &play->actorCtx, &this->actor, ACTORCAT_PROP);
    }

    EnTest_SetupAction(this, func_80862E6C);
}

void func_80862E6C(EnTest* this, PlayState* play) {
    if (this->actor.child == NULL) {
        if (this->actor.home.rot.x == 0) {
            this->actor.home.rot.x = this->bodyBreak.count;
        }

        if (BodyBreak_SpawnParts(&this->actor, &this->bodyBreak, play, this->actor.params + 8)) {
            this->actor.child = &this->actor;
        }
    } else {
        if (this->actor.home.rot.x == 0) {
            this->actor.colChkInfo.health = 10;

            if (this->actor.params == STALFOS_TYPE_4) {
                this->actor.params = -1;
            } else {
                func_800BC154(play, &play->actorCtx, &this->actor, ACTORCAT_ENEMY);
            }

            this->actor.child = NULL;
            this->actor.flags |= ACTOR_FLAG_TARGETABLE;
            EnTest_SetupJumpBack(this);
        } else if ((this->actor.params == STALFOS_TYPE_5) &&
                   !Actor_FindNearby(play, &this->actor, ACTOR_EN_TEST, ACTORCAT_ENEMY, 8000.0f)) {
            Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0xD0);

            if (this->actor.parent != NULL) {
                this->actor.parent->home.rot.z--;
            }

            Actor_Kill(&this->actor);
        }
    }
}

void SetupFallOverBackwardsAndDie(EnTest* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06001420));
    Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_DEAD);
    this->unk_7DE = 0;
    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    this->actor.colorFilterTimer = 0;
    this->actor.speed = 0.0f;

    if (this->actor.params <= STALFOS_TYPE_CEILING) {
        this->unk_7C8 = 5;
        EnTest_SetupAction(this, FallOverBackwardsAndDie);
    } else {
        func_80862DBC(this, play);
    }
}

void FallOverBackwardsAndDie(EnTest* this, PlayState* play) {
    CheckReviveStuff(this, play);

    if ((s32)this->skelAnime.curFrame == 15) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_GERUDOFT_DOWN);
    }
}

void SetupFallOverForwardsAndDie(EnTest* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, (void*)(0x06009A90));
    Actor_PlaySfx(&this->actor, NA_SE_EN_STAL_DEAD);
    this->unk_7C8 = 6;
    this->actor.colorFilterTimer = 0;
    this->unk_7DE = 0;
    this->actor.speed = 0.0f;

    if (this->actor.params <= STALFOS_TYPE_CEILING) {
        this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
        EnTest_SetupAction(this, FallOverForwardsAndDie);
    } else {
        func_80862DBC(this, play);
    }
}

void FallOverForwardsAndDie(EnTest* this, PlayState* play) {
    CheckReviveStuff(this, play);

    if (((s32)this->skelAnime.curFrame == 10) || ((s32)this->skelAnime.curFrame == 25)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_GERUDOFT_DOWN);
    }
}

void EnTest_SetupRecoil(EnTest* this) {
    this->swordState = 0;
    this->skelAnime.moveFlags = 2;
    this->unk_7C8 = 0x13;
    this->skelAnime.playSpeed = -1.0f;
    this->skelAnime.startFrame = this->skelAnime.curFrame;
    this->skelAnime.endFrame = 0.0f;
    EnTest_SetupAction(this, EnTest_Recoil);
}

void EnTest_Recoil(EnTest* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        if (Rand_ZeroOne() > 0.7f) {
            EnTest_SetupIdle(this);
            this->timer = (Rand_ZeroOne() * 5.0f) + 5.0f;
        } else if (((play->gameplayFrames % 2) != 0) && (this->actor.params != STALFOS_TYPE_CEILING)) {
            EnTest_SetupJumpBack(this);
        } else {
            func_808627C4(this, play);
        }
    }
}

void EnTest_Rise(EnTest* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.scale.y < 0.015f) {
        this->actor.scale.y += 0.002f;
        this->actor.world.pos.y = this->actor.home.pos.y - 3.5f;
    } else {
        this->actor.world.pos.y = this->actor.home.pos.y;
        EnTest_SetupJumpBack(this);
    }
}

void BreakApartAndDie(EnTest* this, PlayState* play) {
    this->actor.params = STALFOS_TYPE_1;
    
    if (BodyBreak_SpawnParts(&this->actor, &this->bodyBreak, play, this->actor.params)) {
        Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0xD0);
        
        if (this->actor.parent != NULL) {
            this->actor.parent->home.rot.z--;
        }
        
        Actor_Kill(&this->actor);
    }
}

void EnTest_UpdateHeadRot(EnTest* this, PlayState* play) {
    s16 lookAngle = this->actor.yawTowardsPlayer;

    lookAngle -= (s16)(this->headRot.y + this->actor.shape.rot.y);

    this->headRotOffset.y = CLAMP(lookAngle, -0x7D0, 0x7D0);
    this->headRot.y += this->headRotOffset.y;
    this->headRot.y = CLAMP(this->headRot.y, -0x382F, 0x382F);
}

void EnTest_UpdateDamage(EnTest* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->shieldCollider.base.acFlags & AC_BOUNCED) {
        this->shieldCollider.base.acFlags &= ~AC_BOUNCED;
        this->bodyCollider.base.acFlags &= ~AC_HIT;

        if (this->unk_7C8 >= 0xA) {
            this->actor.speed = -4.0f;
        }
    } else if (this->bodyCollider.base.acFlags & AC_HIT) {
        this->bodyCollider.base.acFlags &= ~AC_HIT;

        if ((this->actor.colChkInfo.damageEffect != STALFOS_DMGEFF_SLING) &&
            (this->actor.colChkInfo.damageEffect != STALFOS_DMGEFF_FIREMAGIC)) {
            this->lastDamageEffect = this->actor.colChkInfo.damageEffect;
            if (this->swordState >= 1) {
                this->swordState = 0;
            }
            this->unk_7DC = player->unk_ADD;
            this->actor.world.rot.y = this->actor.yawTowardsPlayer;
            Actor_SetDropFlag(&this->actor, &this->bodyCollider.info);
            //Audio_StopSfxByPosAndId(&this->actor.projectedPos, NA_SE_EN_STAL_WARAU);

            if ((this->actor.colChkInfo.damageEffect == STALFOS_DMGEFF_STUN) ||
                (this->actor.colChkInfo.damageEffect == STALFOS_DMGEFF_FREEZE) ||
                (this->actor.colChkInfo.damageEffect == STALFOS_DMGEFF_LIGHT)) {
                if (this->unk_7C8 != 0xB) {
                    Actor_ApplyDamage(&this->actor);
                    EnTest_SetupStunned(this);
                }
            } else {
                if (Actor_IsFacingPlayer(&this->actor, 0x4000)) {
                    if (Actor_ApplyDamage(&this->actor) == 0) {
                        Enemy_StartFinishingBlow(play, &this->actor);
                        SetupFallOverBackwardsAndDie(this, play);
                    } else {
                        func_80862154(this);
                    }
                } else if (Actor_ApplyDamage(&this->actor) == 0) {
                    SetupFallOverForwardsAndDie(this, play);
                    Enemy_StartFinishingBlow(play, &this->actor);
                } else {
                    func_80862398(this);
                }
            }
        }
    }
}

void EnTest_Update(Actor* thisx, PlayState* play) {
    EnTest* this = (EnTest*)thisx;
    f32 oldWeight;
    u32 floorProperty;
    s32 pad;

    EnTest_UpdateDamage(this, play);

    if (this->actor.colChkInfo.damageEffect != STALFOS_DMGEFF_FIREMAGIC) {
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f,
                                UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                                    UPDBGCHECKINFO_FLAG_10);

        if (this->actor.params == STALFOS_TYPE_1) {
            if (this->actor.world.pos.y <= this->actor.home.pos.y) {
                this->actor.world.pos.y = this->actor.home.pos.y;
                this->actor.velocity.y = 0.0f;
            }

            if (this->actor.floorHeight <= this->actor.home.pos.y) {
                this->actor.floorHeight = this->actor.home.pos.y;
            }
        } else if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            floorProperty = SurfaceType_GetFloorProperty(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId);

            if ((floorProperty == FLOOR_PROPERTY_5) || (floorProperty == FLOOR_PROPERTY_12) ||
                SurfaceType_GetFloorType(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId) == FLOOR_TYPE_9) {
                Actor_Kill(&this->actor);
                return;
            }
        }

        this->actionFunc(this, play);

        switch (this->unk_7DE) {
            case 0:
                break;

            case 1:
                Animation_Change(&this->upperSkelanime, (void*)(0x06001C20), 2.0f, 0.0f,
                                 Animation_GetLastFrame((void*)(0x06001C20)), 2, 2.0f);
                AnimationContext_SetCopyTrue(play, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                             this->upperSkelanime.jointTable, sJointCopyFlags);
                this->unk_7DE++;
                break;

            case 2:
                SkelAnime_Update(&this->upperSkelanime);
                SkelAnime_CopyFrameTableTrue(&this->skelAnime, this->skelAnime.jointTable,
                                             this->upperSkelanime.jointTable, sJointCopyFlags);
                break;

            case 3:
                this->unk_7DE++;
                this->upperSkelanime.morphWeight = 4.0f;
                FALLTHROUGH;
            case 4:
                oldWeight = this->upperSkelanime.morphWeight;
                this->upperSkelanime.morphWeight -= 1.0f;

                if (this->upperSkelanime.morphWeight <= 0.0f) {
                    this->unk_7DE = 0;
                }

                SkelAnime_CopyFrameTableTrue(&this->skelAnime, this->skelAnime.jointTable,
                                             this->upperSkelanime.jointTable, sJointCopyFlags);
                break;
        }

        if ((this->actor.colorFilterTimer == 0) && (this->actor.colChkInfo.health != 0)) {
            if ((this->unk_7C8 != 0x10) && (this->unk_7C8 != 0x17)) {
                EnTest_UpdateHeadRot(this, play);
            } else {
                Math_SmoothStepToS(&this->headRot.y, 0, 1, 0x3E8, 0);
            }
        }
    }

    Collider_UpdateCylinder(&this->actor, &this->bodyCollider);

    this->actor.focus.pos = this->actor.world.pos;
    this->actor.focus.pos.y += 45.0f;

    if ((this->actor.colChkInfo.health > 0) || (this->actor.colorFilterTimer != 0)) {
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->bodyCollider.base);

        if ((this->unk_7C8 >= 0xA) &&
            ((this->actor.colorFilterTimer == 0) || !(this->actor.colorFilterParams & 0x4000))) {
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->bodyCollider.base);
        }

        if (this->unk_7DE != 0) {
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->shieldCollider.base);
        }
    }

    if (this->swordState >= 1) {
        if (!(this->swordCollider.base.atFlags & AT_BOUNCED)) {
            CollisionCheck_SetAT(play, &play->colChkCtx, &this->swordCollider.base);
        } else {
            this->swordCollider.base.atFlags &= ~AT_BOUNCED;
            EnTest_SetupRecoil(this);
        }
    }

    if (this->actor.params == STALFOS_TYPE_INVISIBLE) {
        if (play->actorCtx.lensActive) {
            this->actor.flags |= ACTOR_FLAG_REACT_TO_LENS;
            if (this->actionFunc != Revive && this->actor.colChkInfo.health > 0)
                this->actor.flags |= ACTOR_FLAG_TARGETABLE;
            this->actor.shape.shadowDraw = ActorShadow_DrawFeet;
        } else {
            this->actor.flags &= ~(ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_REACT_TO_LENS);
            this->actor.shape.shadowDraw = NULL;
        }
    }
}

s32 EnTest_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnTest* this = (EnTest*)thisx;
    s32 pad;

    if (limbIndex == STALFOS_LIMB_HEAD_ROOT) {
        rot->x += this->headRot.y;
        rot->y -= this->headRot.x;
        rot->z += this->headRot.z;
    } else if (limbIndex == STALFOS_LIMB_HEAD) {
        OPEN_DISPS(play->state.gfxCtx);

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetEnvColor(POLY_OPA_DISP++, 80 + ABS((s16)(Math_SinS(play->gameplayFrames * 2000) * 175.0f)), 0, 0, 255);

        CLOSE_DISPS(play->state.gfxCtx);
    }

    if ((this->actor.params == STALFOS_TYPE_INVISIBLE) && !CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_REACT_TO_LENS)) {
        *dList = NULL;
    }

    return false;
}

void EnTest_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    static Vec3f unused1 = { 1100.0f, -700.0f, 0.0f };
    static Vec3f D_80864658 = { 300.0f, 0.0f, 0.0f };
    static Vec3f D_80864664 = { 3400.0f, 0.0f, 0.0f };
    static Vec3f D_80864670 = { 0.0f, 0.0f, 0.0f };
    static Vec3f D_8086467C = { 7000.0f, 1000.0f, 0.0f };
    static Vec3f D_80864688 = { 3000.0f, -2000.0f, -1000.0f };
    static Vec3f D_80864694 = { 3000.0f, -2000.0f, 1000.0f };
    static Vec3f D_808646A0 = { -1300.0f, 1100.0f, 0.0f };
    static Vec3f unused2 = { -3000.0f, 1900.0f, 800.0f };
    static Vec3f unused3 = { -3000.0f, -1100.0f, 800.0f };
    static Vec3f unused4 = { 1900.0f, 1900.0f, 800.0f };
    static Vec3f unused5 = { -3000.0f, -1100.0f, 800.0f };
    static Vec3f unused6 = { 1900.0f, -1100.0f, 800.0f };
    static Vec3f unused7 = { 1900.0f, 1900.0f, 800.0f };
    s32 bodyPart = -1;
    Vec3f sp70;
    Vec3f sp64;
    EnTest* this = (EnTest*)thisx;
    s32 pad;
    Vec3f sp50;

    BodyBreak_SetInfo(&this->bodyBreak, limbIndex, 0, 60, 60, dList, BODYBREAK_OBJECT_DEFAULT);

    if (limbIndex == STALFOS_LIMB_SWORD) {
        Matrix_MultVec3f(&D_8086467C, &this->swordCollider.dim.quad[1]);
        Matrix_MultVec3f(&D_80864688, &this->swordCollider.dim.quad[0]);
        Matrix_MultVec3f(&D_80864694, &this->swordCollider.dim.quad[3]);
        Matrix_MultVec3f(&D_808646A0, &this->swordCollider.dim.quad[2]);

        Collider_SetQuadVertices(&this->swordCollider, &this->swordCollider.dim.quad[0],
                                 &this->swordCollider.dim.quad[1], &this->swordCollider.dim.quad[2],
                                 &this->swordCollider.dim.quad[3]);

        Matrix_MultVec3f(&D_80864664, &sp70);
        Matrix_MultVec3f(&D_80864670, &sp64);

        if ((this->swordState >= 1) && ((this->actor.params != STALFOS_TYPE_INVISIBLE) || play->actorCtx.lensActive)) {
            EffectBlure_AddVertex(Effect_GetByIndex(this->effectIndex), &sp70, &sp64);
        } else if (this->swordState >= 0) {
            EffectBlure_AddSpace(Effect_GetByIndex(this->effectIndex));
            this->swordState = -1;
        }

    } else if ((limbIndex == STALFOS_LIMB_SHIELD) && (this->unk_7DE != 0)) {
        Matrix_MultVec3f(&D_80864670, &sp64);

        this->shieldCollider.dim.pos.x = sp64.x;
        this->shieldCollider.dim.pos.y = sp64.y;
        this->shieldCollider.dim.pos.z = sp64.z;
    } else {
        Actor_SetFeetPos(&this->actor, limbIndex, STALFOS_LIMB_FOOT_L, &D_80864658, STALFOS_LIMB_ANKLE_R, &D_80864658);

        if ((limbIndex == STALFOS_LIMB_FOOT_L) || (limbIndex == STALFOS_LIMB_ANKLE_R)) {
            if ((this->unk_7C8 == 0x15) || (this->unk_7C8 == 0x16)) {
                if (this->actor.speed != 0.0f) {
                    Matrix_MultVec3f(&D_80864658, &sp64);
                    Actor_SpawnFloorDustRing(play, &this->actor, &sp64, 10.0f, 1, 8.0f, 100, 15, false);
                }
            }
        }
    }

    if (this->iceTimer != 0) {
        switch (limbIndex) {
            case STALFOS_LIMB_HEAD:
                bodyPart = 0;
                break;
            case STALFOS_LIMB_CHEST:
                bodyPart = 1;
                break;
            case STALFOS_LIMB_SWORD:
                bodyPart = 2;
                break;
            case STALFOS_LIMB_SHIELD:
                bodyPart = 3;
                break;
            case STALFOS_LIMB_UPPERARM_R:
                bodyPart = 4;
                break;
            case STALFOS_LIMB_UPPERARM_L:
                bodyPart = 5;
                break;
            case STALFOS_LIMB_WAIST:
                bodyPart = 6;
                break;
            case STALFOS_LIMB_FOOT_L:
                bodyPart = 7;
                break;
            case STALFOS_LIMB_FOOT_R:
                bodyPart = 8;
                break;
        }

        if (bodyPart >= 0) {
            Matrix_MultVec3f(&D_80864670, &sp50);

            this->bodyPartsPos[bodyPart].x = sp50.x;
            this->bodyPartsPos[bodyPart].y = sp50.y;
            this->bodyPartsPos[bodyPart].z = sp50.z;
        }
    }
}

void EnTest_Draw(Actor* thisx, PlayState* play) {
    EnTest* this = (EnTest*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    func_800B8050(&this->actor, play, 1);

    if ((thisx->params <= STALFOS_TYPE_CEILING) || (thisx->child == NULL)) {
        SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, EnTest_OverrideLimbDraw,
                          EnTest_PostLimbDraw, (void*)this);
    }

/*     if (this->iceTimer != 0) {
        thisx->colorFilterTimer++;
        this->iceTimer--;

        if ((this->iceTimer % 4) == 0) {
            s32 iceIndex = this->iceTimer >> 2;

            EffectSsEnIce_SpawnFlyingVec3s(play, thisx, &this->bodyPartsPos[iceIndex], 150, 150, 150, 250, 235, 245,
                                           255, 1.5f);
        }
    } */
}

// a variation of sidestep
void func_80864158(EnTest* this, f32 xzSpeed) {
    Animation_MorphToLoop(&this->skelAnime, (void*)(0x0600E2B0), -2.0f);
    this->actor.speed = xzSpeed;
    this->actor.world.rot.y = this->actor.shape.rot.y + 0x3FFF;
    this->timer = (Rand_ZeroOne() * 20.0f) + 15.0f;
    this->unk_7C8 = 0x18;
    EnTest_SetupAction(this, func_808628C8);
}

/**
 * Check if a projectile actor is within 300 units and react accordingly.
 * Returns true if the projectile test passes and a new action is performed.
 */
s32 EnTest_ReactToProjectile(PlayState* play, EnTest* this) {
    Actor* projectileActor = func_800BC270(play, &this->actor, 80.0f, 0x138B0);
    s16 yawToProjectile;
    s16 wallYawDiff;
    s16 touchingWall;
    s16 directionFlag;

    if (projectileActor != NULL) {
        yawToProjectile = Actor_WorldYawTowardActor(&this->actor, projectileActor) - (u16)this->actor.shape.rot.y;

        if ((u8)(this->actor.bgCheckFlags & BGCHECKFLAG_WALL)) {
            wallYawDiff = ((u16)this->actor.wallYaw - (u16)this->actor.shape.rot.y);
            touchingWall = true;
        } else {
            touchingWall = false;
        }

        if (Math_Vec3f_DistXYZ(&this->actor.world.pos, &projectileActor->world.pos) < 200.0f) {
            if (Actor_IsTargeted(play, &this->actor) && (projectileActor->id == ACTOR_ARMS_HOOK)) {
                EnTest_SetupJumpUp(this);
            } else if (ABS(yawToProjectile) < 0x2000) {
                EnTest_SetupStopAndBlock(this);
            } else if (ABS(yawToProjectile) < 0x6000) {
                EnTest_SetupJumpBack(this);
            } else {
                EnTest_SetupJumpUp(this);
            }

            return true;
        }

        if (Actor_IsTargeted(play, &this->actor) && (projectileActor->id == ACTOR_ARMS_HOOK)) {
            EnTest_SetupJumpUp(this);
            return true;
        }

        if ((ABS(yawToProjectile) < 0x2000) || (ABS(yawToProjectile) > 0x6000)) {
            directionFlag = play->gameplayFrames % 2;

            if (touchingWall && (wallYawDiff > 0x2000) && (wallYawDiff < 0x6000)) {
                directionFlag = false;
            } else if (touchingWall && (wallYawDiff < -0x2000) && (wallYawDiff > -0x6000)) {
                directionFlag = true;
            }

            if (directionFlag) {
                func_80864158(this, 4.0f);
            } else {
                func_80864158(this, -4.0f);
            }
        } else if (ABS(yawToProjectile) < 0x6000) {
            directionFlag = play->gameplayFrames % 2;

            if (touchingWall && (ABS(wallYawDiff) > 0x6000)) {
                directionFlag = false;
            } else if (touchingWall && (ABS(wallYawDiff) < 0x2000)) {
                directionFlag = true;
            }

            if (directionFlag) {
                EnTest_SetupJumpBack(this);
            } else {
                EnTest_SetupJumpUp(this);
            }
        }

        return true;
    }

    return false;
}
